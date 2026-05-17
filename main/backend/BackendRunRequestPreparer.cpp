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

#include "BackendRunRequestPreparer.h"

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

BackendRunRequestParameters
toBuildParameters(const BackendRunRequestPreparationParameters &parameters)
{
    BackendRunRequestParameters buildParameters;
    buildParameters.inputAudioFilePath = parameters.inputAudioFilePath;
    buildParameters.selectedRegion = parameters.selectedRegion;
    buildParameters.expectedUnifiedResultJsonPath =
        parameters.expectedUnifiedResultJsonPath;
    buildParameters.includeRequestJsonFileArgument = true;
    buildParameters.requestJsonFilePath = parameters.requestJsonFilePath;
    buildParameters.requestJsonArgumentFlag = parameters.requestJsonArgumentFlag;
    buildParameters.timeoutMsec = parameters.timeoutMsec;
    buildParameters.additionalArguments = parameters.additionalArguments;
    return buildParameters;
}

}

bool
BackendRunRequestPreparationResult::isValid() const
{
    return report.isValid();
}

QString
BackendRunRequestPreparationResult::debugSummaryString() const
{
    return QString("backend=%1 run_id=%2 request_file=%3 written=%4 valid=%5")
        .arg(builtRequest.backendId.isEmpty() ? QString("unknown") :
                                                builtRequest.backendId)
        .arg(processRequest.runId.isEmpty() ? QString("unknown") :
                                             processRequest.runId)
        .arg(requestJsonFilePath)
        .arg(requestFileWritten ? QString("true") : QString("false"))
        .arg(isValid() ? QString("true") : QString("false"));
}

BackendRunRequestPreparationResult
BackendRunRequestPreparer::prepare(
    const BackendManifest &manifest,
    const BackendRunWorkspace &workspace,
    const BackendRunRequestPreparationParameters &parameters) const
{
    return prepare(manifest, std::nullopt, workspace, parameters);
}

BackendRunRequestPreparationResult
BackendRunRequestPreparer::prepare(
    const BackendManifest &manifest,
    const std::optional<BackendSettings> &settings,
    const BackendRunWorkspace &workspace,
    const BackendRunRequestPreparationParameters &parameters) const
{
    BackendRunRequestPreparationResult prepared;
    prepared.requestJsonFilePath = parameters.requestJsonFilePath.trimmed();

    BackendRunRequestBuilder builder;
    prepared.builtRequest =
        builder.build(manifest,
                      settings,
                      workspace,
                      toBuildParameters(parameters));
    appendIssues(prepared.report, prepared.builtRequest.report);

    if (!prepared.builtRequest.isValid()) {
        return prepared;
    }

    BackendRunRequestFileWriter writer;
    prepared.fileWriteResult =
        writer.write(parameters.requestJsonFilePath, prepared.builtRequest);
    appendIssues(prepared.report, prepared.fileWriteResult.report);

    if (!prepared.fileWriteResult.isValid()) {
        return prepared;
    }

    prepared.processRequest = prepared.builtRequest.request;
    prepared.requestJsonFilePath = prepared.fileWriteResult.path;
    prepared.bytesWritten = prepared.fileWriteResult.bytesWritten;
    prepared.requestFileWritten = true;

    return prepared;
}

}
}
