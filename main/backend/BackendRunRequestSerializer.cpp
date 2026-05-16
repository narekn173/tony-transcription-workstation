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

#include "BackendRunRequestSerializer.h"

#include <QJsonArray>

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

QJsonArray
stringListToJsonArray(const QStringList &values)
{
    QJsonArray array;
    for (const auto &value: values) {
        array.append(value);
    }
    return array;
}

QJsonObject
environmentToJsonObject(const QMap<QString, QString> &environment)
{
    QJsonObject object;
    for (auto it = environment.cbegin(); it != environment.cend(); ++it) {
        object.insert(it.key(), it.value());
    }
    return object;
}

QJsonObject
regionToJsonObject(const AnalysisRegion &region)
{
    QJsonObject object;
    object.insert("start_sec", region.startSec);
    object.insert("end_sec", region.endSec);
    object.insert("coordinate_system", "original_audio_time");
    object.insert("apply_policy", "preview_only");
    return object;
}

}

bool
BackendRunRequestSerializationResult::isValid() const
{
    return report.isValid();
}

BackendRunRequestSerializationResult
BackendRunRequestSerializer::serialize(
    const BackendRunRequestBuildResult &request) const
{
    BackendRunRequestSerializationResult serialized;
    serialized.object = toJsonObject(request);
    serialized.report = validate(request);
    return serialized;
}

ValidationReport
BackendRunRequestSerializer::validate(
    const BackendRunRequestBuildResult &request) const
{
    ValidationReport report;
    appendIssues(report, request.report);

    if (!Tony::Backend::isValidBackendId(request.backendId)) {
        report.addError("invalid_backend_id",
                        "Backend run request JSON requires a valid backend ID.");
    }
    if (request.request.runId.trimmed().isEmpty()) {
        report.addError("empty_run_id",
                        "Backend run request JSON requires a run ID.");
    }
    if (request.request.executablePath.trimmed().isEmpty()) {
        report.addError("empty_executable_path",
                        "Backend run request JSON requires an executable path.");
    }
    if (request.request.workingDirectory.trimmed().isEmpty()) {
        report.addError("empty_working_directory",
                        "Backend run request JSON requires a working directory.");
    }
    if (request.inputAudioFilePath.trimmed().isEmpty()) {
        report.addError("empty_input_audio_path",
                        "Backend run request JSON requires an input audio path.");
    }
    if (request.expectedUnifiedResultJsonPath.trimmed().isEmpty()) {
        report.addError("empty_output_result_path",
                        "Backend run request JSON requires an output result path.");
    }
    if (request.request.timeoutMsec < 0) {
        report.addError("invalid_timeout",
                        "Backend run request JSON timeout must not be negative.");
    }
    if (request.selectedRegion.has_value() &&
        !request.selectedRegion->isValid()) {
        report.addError("invalid_selected_region",
                        "Backend run request JSON selected region is invalid.");
    }

    return report;
}

QJsonObject
BackendRunRequestSerializer::toJsonObject(
    const BackendRunRequestBuildResult &request)
{
    QJsonObject object;
    object.insert("contract_version", "0.1");
    object.insert("backend_id", request.backendId);
    object.insert("run_id", request.request.runId);
    object.insert("executable_path", request.request.executablePath);
    object.insert("arguments", stringListToJsonArray(request.request.arguments));
    object.insert("working_directory", request.request.workingDirectory);
    object.insert("environment_overrides",
                  environmentToJsonObject(request.request.environmentOverrides));
    object.insert("input_audio_path", request.inputAudioFilePath);
    object.insert("output_result_json_path",
                  request.expectedUnifiedResultJsonPath);
    object.insert("timeout_msec", request.request.timeoutMsec);

    if (request.selectedRegion.has_value()) {
        object.insert("selected_region",
                      regionToJsonObject(*request.selectedRegion));
    } else {
        object.insert("selected_region", QJsonValue::Null);
    }

    return object;
}

}
}
