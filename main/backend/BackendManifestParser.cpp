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

#include "BackendManifestParser.h"

#include <QJsonArray>
#include <QJsonValue>

#include <optional>

namespace Tony {
namespace Backend {

namespace {

std::optional<QString>
optionalString(const QJsonObject &object, const QString &key)
{
    const QJsonValue value = object.value(key);
    if (value.isUndefined() || value.isNull()) {
        return std::nullopt;
    }
    if (value.isString()) {
        return value.toString();
    }
    return std::nullopt;
}

QStringList
stringListFromArray(const QJsonArray &array)
{
    QStringList values;
    for (const QJsonValue &value: array) {
        if (value.isString()) {
            values << value.toString();
        }
    }
    return values;
}

}

bool
BackendManifestParseResult::isValid() const
{
    return report.isValid();
}

BackendManifestParseResult
BackendManifestParser::parse(const QJsonObject &object) const
{
    BackendManifestParseResult parsed;
    BackendManifest &manifest = parsed.manifest;
    ValidationReport &report = parsed.report;

    manifest.contractVersion = object.value("contract_version").toString("0.1");
    manifest.backendId = object.value("backend_id").toString();
    if (manifest.backendId.isEmpty()) {
        manifest.backendId = object.value("engine_id").toString();
    }
    manifest.engineId = manifest.backendId;
    manifest.displayName = object.value("display_name").toString();
    manifest.description = object.value("description").toString();
    manifest.engineVersion = optionalString(object, "engine_version").value_or(QString());
    manifest.version = optionalString(object, "version").value_or(manifest.engineVersion);
    manifest.adapterVersion = object.value("adapter_version").toString();

    const QJsonObject runtime = object.value("runtime").toObject();
    manifest.backendType =
        parseRuntimeType(runtime.value("type").toString(), report);
    manifest.runtimeType = manifest.backendType;
    manifest.capabilities.requiresPython =
        parseBool(runtime, "requires_python", report);
    manifest.capabilities.requiresModelCheckpoint =
        parseBool(runtime, "requires_model_files", report);
    manifest.capabilities.supportsCpu =
        parseBool(runtime, "supports_cpu", report);
    manifest.capabilities.supportsGpuOptional =
        parseBool(runtime, "supports_cuda", report) ||
        parseBool(runtime, "supports_directml", report) ||
        parseBool(runtime, "supports_rocm", report);

    manifest.executablePath = object.value("executable_path").toString();
    manifest.workingDirectory = object.value("working_directory").toString();
    const QJsonObject statusProbe = object.value("status_probe").toObject();
    if (manifest.executablePath.isEmpty()) {
        manifest.executablePath = statusProbe.value("executable_path").toString();
    }
    if (manifest.workingDirectory.isEmpty()) {
        manifest.workingDirectory = statusProbe.value("working_directory").toString();
    }

    if (!object.value("capabilities").isObject()) {
        report.addError("invalid_capabilities",
                        "BackendManifest field 'capabilities' must be an object.");
    }
    const QJsonObject capabilities = object.value("capabilities").toObject();
    manifest.capabilities.supportsFullFile =
        parseBool(capabilities, "supports_full_file", report);
    manifest.capabilities.supportsSelectedRegion =
        parseBool(capabilities, "supports_region", report);
    manifest.capabilities.outputsNotes =
        parseBool(capabilities, "outputs_notes", report);
    manifest.capabilities.outputsPitchCurve =
        parseBool(capabilities, "outputs_pitch_curve", report);
    manifest.capabilities.outputsPitchBends =
        parseBool(capabilities, "outputs_pitch_bends", report);
    manifest.capabilities.outputsTechniqueLabels =
        parseBool(capabilities, "outputs_technique_labels", report);

    const QJsonObject inputs = object.value("inputs").toObject();
    manifest.supportedInputFormats =
        parseStringArray(inputs, "audio_formats", report);

    const QJsonObject outputs = object.value("outputs").toObject();
    manifest.primaryOutputs = parseStringArray(outputs, "primary", report);
    manifest.optionalOutputs = parseStringArray(outputs, "optional", report);
    manifest.supportedOutputTypes =
        manifest.primaryOutputs + manifest.optionalOutputs;

    manifest.requiredFiles = parseStringArray(object, "required_files", report);
    manifest.optionalFiles = parseStringArray(object, "optional_files", report);
    const QJsonObject settings = object.value("default_settings").toObject();
    manifest.defaultSettings = settings.toVariantMap();

    const QJsonObject license = object.value("license").toObject();
    manifest.licenseName = license.value("name").toString();
    manifest.licenseSourceUrl = license.value("source_url").toString();

    manifest.status = BackendStatus::NotConfigured;

    if (manifest.id().isEmpty()) {
        report.addError("missing_backend_id",
                        "BackendManifest backend ID is required.");
    } else if (!manifest.isValidBackendId()) {
        report.addError("invalid_backend_id",
                        "BackendManifest backend ID must be lowercase snake_case.");
    }

    if (manifest.displayName.isEmpty()) {
        report.addError("missing_display_name",
                        "BackendManifest display name is required.");
    }

    return parsed;
}

BackendRuntimeType
BackendManifestParser::parseRuntimeType(const QString &value,
                                        ValidationReport &report)
{
    if (value == "internal") return BackendRuntimeType::Internal;
    if (value == "vamp_plugin") return BackendRuntimeType::VampPlugin;
    if (value == "python_cli") return BackendRuntimeType::PythonCli;
    if (value == "native_cli") return BackendRuntimeType::NativeCli;
    if (value == "onnx_native") return BackendRuntimeType::OnnxNative;
    if (value == "adapter_cli") return BackendRuntimeType::AdapterCli;
    if (value == "development_test") return BackendRuntimeType::DevelopmentTest;

    report.addIssue(ValidationSeverity::Warning,
                    "unknown_backend_type",
                    "BackendManifest runtime type is unknown.");
    return BackendRuntimeType::Unknown;
}

QStringList
BackendManifestParser::parseStringArray(const QJsonObject &object,
                                        const QString &key,
                                        ValidationReport &report)
{
    const QJsonValue value = object.value(key);
    if (value.isUndefined() || value.isNull()) {
        return {};
    }
    if (!value.isArray()) {
        report.addError(QString("invalid_%1").arg(key),
                        QString("BackendManifest field '%1' must be an array.").arg(key));
        return {};
    }
    return stringListFromArray(value.toArray());
}

bool
BackendManifestParser::parseBool(const QJsonObject &object,
                                 const QString &key,
                                 ValidationReport &report)
{
    const QJsonValue value = object.value(key);
    if (value.isUndefined() || value.isNull()) {
        return false;
    }
    if (!value.isBool()) {
        report.addError(QString("invalid_%1").arg(key),
                        QString("BackendManifest field '%1' must be a boolean.").arg(key));
        return false;
    }
    return value.toBool();
}

}
}
