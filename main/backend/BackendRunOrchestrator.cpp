/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Tony
    An intonation analysis and annotation tool
    Centre for Digital Music, Queen Mary, University of London.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "BackendRunOrchestrator.h"

#include <QFileInfo>
#include <QSharedPointer>

namespace Tony {
namespace Backend {

namespace {

void
appendIssues(ValidationReport &target, const ValidationReport &source)
{
    for (const auto &issue: source.issues) {
        target.addIssue(issue.severity, issue.code, issue.message);
    }
}

QString
effectiveLogPath(const BackendRunWorkspace &workspace,
                 const BackendRunOrchestrationParameters &parameters)
{
    if (!parameters.processLogFilePath.trimmed().isEmpty()) {
        return parameters.processLogFilePath.trimmed();
    }
    return workspace.logFilePath;
}

void
refreshResultJsonPresence(BackendRunOrchestrationResult &result)
{
    result.resultJsonExists =
        !result.expectedUnifiedResultJsonPath.trimmed().isEmpty() &&
        QFileInfo::exists(result.expectedUnifiedResultJsonPath);
}

}

bool
BackendRunOrchestrationResult::isValid() const
{
    return report.isValid();
}

QString
BackendRunOrchestrationResult::debugSummaryString() const
{
    return QString("run_id=%1 request_file=%2 prepared=%3 ran=%4 "
                   "result_json_exists=%5 imported=%6 valid=%7")
        .arg(processRequest.runId.isEmpty() ? QString("unknown") :
                                             processRequest.runId)
        .arg(requestJsonFilePath)
        .arg(requestPrepared ? QString("true") : QString("false"))
        .arg(processRunAttempted ? QString("true") : QString("false"))
        .arg(resultJsonExists ? QString("true") : QString("false"))
        .arg(importedIntoTonyLayers ? QString("true") : QString("false"))
        .arg(isValid() ? QString("true") : QString("false"));
}

BackendRunOrchestrationResult
BackendRunOrchestrator::prepareOnly(
    const BackendManifest &manifest,
    const BackendRunWorkspace &workspace,
    const BackendRunOrchestrationParameters &parameters) const
{
    return prepareOnly(manifest, std::nullopt, workspace, parameters);
}

BackendRunOrchestrationResult
BackendRunOrchestrator::prepareOnly(
    const BackendManifest &manifest,
    const std::optional<BackendSettings> &settings,
    const BackendRunWorkspace &workspace,
    const BackendRunOrchestrationParameters &parameters) const
{
    BackendRunOrchestrationResult result;
    result.expectedUnifiedResultJsonPath =
        parameters.expectedUnifiedResultJsonPath.trimmed();
    result.requestJsonFilePath = parameters.requestJsonFilePath.trimmed();
    result.processLogFilePath = effectiveLogPath(workspace, parameters);
    result.importedIntoTonyLayers = false;

    if (parameters.prepareWorkspace) {
        result.workspacePreparation = workspace.prepareWorkspace();
        result.workspacePrepared = result.workspacePreparation.isValid();
        appendIssues(result.report, result.workspacePreparation.report);

        if (!result.workspacePreparation.isValid()) {
            refreshResultJsonPresence(result);
            return result;
        }
    }

    BackendRunRequestPreparer preparer;
    result.requestPreparation =
        preparer.prepare(manifest,
                         settings,
                         workspace,
                         toPreparationParameters(parameters));
    appendIssues(result.report, result.requestPreparation.report);

    if (!result.requestPreparation.isValid()) {
        refreshResultJsonPresence(result);
        return result;
    }

    result.processRequest = result.requestPreparation.processRequest;
    result.requestJsonFilePath = result.requestPreparation.requestJsonFilePath;
    result.requestPrepared = true;
    refreshResultJsonPresence(result);

    return result;
}

BackendRunOrchestrationResult
BackendRunOrchestrator::run(
    const BackendManifest &manifest,
    const BackendRunWorkspace &workspace,
    const BackendRunOrchestrationParameters &parameters) const
{
    return run(manifest, std::nullopt, workspace, parameters);
}

BackendRunOrchestrationResult
BackendRunOrchestrator::run(
    const BackendManifest &manifest,
    const std::optional<BackendSettings> &settings,
    const BackendRunWorkspace &workspace,
    const BackendRunOrchestrationParameters &parameters) const
{
    BackendRunOrchestrationResult result =
        prepareOnly(manifest, settings, workspace, parameters);

    if (!result.requestPrepared || !result.isValid()) {
        return result;
    }

    result.processRequest.eventCollector =
        QSharedPointer<ExternalProcessEventCollector>::create();

    ExternalProcessRunner runner;
    result.processRunAttempted = true;
    result.processResult = runner.run(result.processRequest);
    result.processEvents = result.processRequest.eventCollector->events();

    if (!result.processResult->succeeded()) {
        result.report.addError(
            "external_process_failed",
            result.processResult->error.message.isEmpty() ?
                QString("External backend process failed.") :
                result.processResult->error.message);
    }

    if (parameters.writeProcessLog) {
        ExternalProcessLogFileSink logSink;
        result.logWriteResult =
            logSink.write(result.processLogFilePath,
                          result.processEvents,
                          result.processResult);
        result.processLogWritten = result.logWriteResult.isValid();
        appendIssues(result.report, result.logWriteResult.report);
    }

    refreshResultJsonPresence(result);
    return result;
}

BackendRunRequestPreparationParameters
BackendRunOrchestrator::toPreparationParameters(
    const BackendRunOrchestrationParameters &parameters)
{
    BackendRunRequestPreparationParameters preparationParameters;
    preparationParameters.inputAudioFilePath = parameters.inputAudioFilePath;
    preparationParameters.selectedRegion = parameters.selectedRegion;
    preparationParameters.expectedUnifiedResultJsonPath =
        parameters.expectedUnifiedResultJsonPath;
    preparationParameters.requestJsonFilePath = parameters.requestJsonFilePath;
    preparationParameters.requestJsonArgumentFlag =
        parameters.requestJsonArgumentFlag;
    preparationParameters.timeoutMsec = parameters.timeoutMsec;
    preparationParameters.additionalArguments = parameters.additionalArguments;
    return preparationParameters;
}

}
}
