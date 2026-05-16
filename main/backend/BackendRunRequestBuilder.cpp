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

#include "BackendRunRequestBuilder.h"

namespace Tony {
namespace Backend {

namespace {

QString
trimmed(const QString &value)
{
    return value.trimmed();
}

QString
formatSeconds(double value)
{
    return QString::number(value, 'f', 6);
}

void
appendIssues(ValidationReport &target, const ValidationReport &source)
{
    for (const auto &issue: source.issues) {
        target.addIssue(issue.severity, issue.code, issue.message);
    }
}

}

bool
BackendRunRequestBuildResult::isValid() const
{
    return report.isValid();
}

QString
BackendRunRequestBuildResult::debugSummaryString() const
{
    return QString("backend=%1 run_id=%2 executable=%3 working_directory=%4 "
                   "input=%5 result=%6 valid=%7")
        .arg(backendId.isEmpty() ? QString("unknown") : backendId)
        .arg(request.runId.isEmpty() ? QString("unknown") : request.runId)
        .arg(request.executablePath)
        .arg(request.workingDirectory)
        .arg(inputAudioFilePath)
        .arg(expectedUnifiedResultJsonPath)
        .arg(isValid() ? QString("true") : QString("false"));
}

BackendRunRequestBuildResult
BackendRunRequestBuilder::build(
    const BackendManifest &manifest,
    const BackendRunWorkspace &workspace,
    const BackendRunRequestParameters &parameters) const
{
    return build(manifest, std::nullopt, workspace, parameters);
}

BackendRunRequestBuildResult
BackendRunRequestBuilder::build(
    const BackendManifest &manifest,
    const std::optional<BackendSettings> &settings,
    const BackendRunWorkspace &workspace,
    const BackendRunRequestParameters &parameters) const
{
    BackendRunRequestBuildResult result;
    result.backendId = manifest.id();
    result.inputAudioFilePath = trimmed(parameters.inputAudioFilePath);
    result.expectedUnifiedResultJsonPath =
        trimmed(parameters.expectedUnifiedResultJsonPath);
    result.selectedRegion = parameters.selectedRegion;
    result.hasSelectedRegion = parameters.selectedRegion.has_value();

    if (!manifest.isValidBackendId()) {
        result.report.addError("invalid_backend_id",
                               "Backend run requests require a valid backend ID.");
    }

    if (settings.has_value()) {
        appendIssues(result.report, settings->validate());
        if (settings->isValidBackendId() &&
            manifest.isValidBackendId() &&
            settings->backendId != manifest.id()) {
            result.report.addError(
                "settings_backend_id_mismatch",
                "Backend settings backend ID does not match the manifest backend ID.");
        }
    }

    appendIssues(result.report, workspace.validate());

    if (result.inputAudioFilePath.isEmpty()) {
        result.report.addError("empty_input_audio_path",
                               "Backend run request input audio path is empty.");
    }
    if (result.expectedUnifiedResultJsonPath.isEmpty()) {
        result.report.addError("empty_output_result_path",
                               "Backend run request result JSON path is empty.");
    }
    if (parameters.selectedRegion.has_value() &&
        !parameters.selectedRegion->isValid()) {
        result.report.addError("invalid_selected_region",
                               "Backend run request selected region is invalid.");
    }
    if (parameters.timeoutMsec < 0) {
        result.report.addError("invalid_timeout",
                               "Backend run request timeout must not be negative.");
    }

    result.request.runId = workspace.runId;
    result.request.executablePath =
        effectiveExecutablePath(manifest,
                                settings,
                                result.usedSettingsExecutableOverride);
    result.request.workingDirectory =
        effectiveWorkingDirectory(manifest,
                                  settings,
                                  workspace,
                                  result.usedSettingsWorkingDirectoryOverride);
    result.request.environmentOverrides =
        settings.has_value() ? settings->environmentVariables :
                               QMap<QString, QString>();
    result.request.timeoutMsec = parameters.timeoutMsec;

    if (result.request.executablePath.isEmpty()) {
        result.report.addError(
            "empty_executable_path",
            "Backend run request executable path is empty.");
    }

    result.request.arguments
        << "--backend-id" << manifest.id()
        << "--run-id" << workspace.runId
        << "--input" << result.inputAudioFilePath
        << "--result" << result.expectedUnifiedResultJsonPath
        << "--workspace" << workspace.runDirectoryPath;

    if (parameters.selectedRegion.has_value()) {
        result.request.arguments
            << "--region-start" << formatSeconds(parameters.selectedRegion->startSec)
            << "--region-end" << formatSeconds(parameters.selectedRegion->endSec);
    }


    result.request.arguments << parameters.additionalArguments;

    return result;
}

QString
BackendRunRequestBuilder::effectiveExecutablePath(
    const BackendManifest &manifest,
    const std::optional<BackendSettings> &settings,
    bool &usedSettingsOverride)
{
    usedSettingsOverride = false;

    if (settings.has_value() &&
        settings->hasExecutablePathOverride()) {
        usedSettingsOverride = true;
        return trimmed(settings->executablePathOverride);
    }

    return trimmed(manifest.executablePath);
}

QString
BackendRunRequestBuilder::effectiveWorkingDirectory(
    const BackendManifest &manifest,
    const std::optional<BackendSettings> &settings,
    const BackendRunWorkspace &workspace,
    bool &usedSettingsOverride)
{
    usedSettingsOverride = false;

    if (settings.has_value() &&
        settings->hasWorkingDirectoryOverride()) {
        usedSettingsOverride = true;
        return trimmed(settings->workingDirectoryOverride);
    }
    if (!trimmed(manifest.workingDirectory).isEmpty()) {
        return trimmed(manifest.workingDirectory);
    }

    return trimmed(workspace.runDirectoryPath);
}

}
}
