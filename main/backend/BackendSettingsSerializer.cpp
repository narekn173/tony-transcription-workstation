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

#include "BackendSettingsSerializer.h"

#include <QJsonValue>

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
optionalString(const QJsonObject &object, const QString &key)
{
    const QJsonValue value = object.value(key);
    if (value.isUndefined() || value.isNull()) {
        return {};
    }
    if (value.isString()) {
        return value.toString();
    }
    return {};
}

bool
optionalBool(const QJsonObject &object, const QString &key)
{
    const QJsonValue value = object.value(key);
    if (value.isUndefined() || value.isNull()) {
        return false;
    }
    if (value.isBool()) {
        return value.toBool();
    }
    return false;
}

}

bool
BackendSettingsSerializationResult::isValid() const
{
    return report.isValid();
}

bool
BackendSettingsStoreSerializationResult::isValid() const
{
    return report.isValid();
}

QJsonObject
BackendSettingsSerializer::toJson(const BackendSettings &settings) const
{
    QJsonObject object;

    object.insert("backend_id", settings.backendId);
    object.insert("executable_path_override",
                  settings.executablePathOverride);
    object.insert("working_directory_override",
                  settings.workingDirectoryOverride);
    object.insert("model_checkpoint_path_override",
                  settings.modelCheckpointPathOverride);
    object.insert("python_executable_path_override",
                  settings.pythonExecutablePathOverride);
    object.insert("environment_variables",
                  environmentToJson(settings.environmentVariables));
    object.insert("enabled", settings.enabled);
    object.insert("display_label_override",
                  settings.displayLabelOverride);
    object.insert("user_notes", settings.userNotes);

    return object;
}

BackendSettingsSerializationResult
BackendSettingsSerializer::fromJson(const QJsonObject &object) const
{
    BackendSettingsSerializationResult result;
    BackendSettings &settings = result.settings;

    settings.backendId = optionalString(object, "backend_id");
    settings.executablePathOverride =
        optionalString(object, "executable_path_override");
    settings.workingDirectoryOverride =
        optionalString(object, "working_directory_override");
    settings.modelCheckpointPathOverride =
        optionalString(object, "model_checkpoint_path_override");
    settings.pythonExecutablePathOverride =
        optionalString(object, "python_executable_path_override");
    settings.enabled = optionalBool(object, "enabled");
    settings.displayLabelOverride =
        optionalString(object, "display_label_override");
    settings.userNotes = optionalString(object, "user_notes");

    const QJsonValue environment = object.value("environment_variables");
    if (environment.isObject()) {
        settings.environmentVariables =
            environmentFromJson(environment.toObject());
    } else if (!environment.isUndefined() && !environment.isNull()) {
        result.report.addError(
            "invalid_environment_variables",
            "BackendSettings field 'environment_variables' must be an object.");
    }

    appendIssues(result.report, settings.validate());

    return result;
}

QJsonArray
BackendSettingsSerializer::storeToJsonArray(const BackendSettingsStore &store) const
{
    QJsonArray array;

    for (const BackendId &backendId: store.configuredBackendIds()) {
        const std::optional<BackendSettings> settings =
            store.settingsForBackend(backendId);
        if (settings.has_value()) {
            array.push_back(toJson(*settings));
        }
    }

    return array;
}

QJsonObject
BackendSettingsSerializer::storeToJsonObject(const BackendSettingsStore &store) const
{
    QJsonObject object;
    object.insert("backend_settings", storeToJsonArray(store));
    return object;
}

BackendSettingsStoreSerializationResult
BackendSettingsSerializer::storeFromJsonArray(const QJsonArray &array) const
{
    BackendSettingsStoreSerializationResult result;

    for (const QJsonValue &value: array) {
        if (!value.isObject()) {
            result.report.addError(
                "invalid_settings_entry",
                "BackendSettingsStore entries must be objects.");
            ++result.rejectedCount;
            continue;
        }

        const BackendSettingsSerializationResult parsed =
            fromJson(value.toObject());
        if (!parsed.isValid()) {
            appendIssues(result.report, parsed.report);
            ++result.rejectedCount;
            continue;
        }

        if (!result.store.setSettings(parsed.settings)) {
            result.report.addError(
                "settings_store_add_failed",
                QString("BackendSettings could not be added: %1")
                    .arg(parsed.settings.backendId));
            ++result.rejectedCount;
            continue;
        }

        ++result.loadedCount;
    }

    return result;
}

BackendSettingsStoreSerializationResult
BackendSettingsSerializer::storeFromJsonObject(const QJsonObject &object) const
{
    const QJsonValue value = object.value("backend_settings");
    if (value.isUndefined() || value.isNull()) {
        return {};
    }
    if (!value.isArray()) {
        BackendSettingsStoreSerializationResult result;
        result.report.addError(
            "invalid_backend_settings",
            "BackendSettingsStore field 'backend_settings' must be an array.");
        return result;
    }
    return storeFromJsonArray(value.toArray());
}

QJsonObject
BackendSettingsSerializer::environmentToJson(const QMap<QString, QString> &environment)
{
    QJsonObject object;
    for (auto it = environment.cbegin(); it != environment.cend(); ++it) {
        object.insert(it.key(), it.value());
    }
    return object;
}

QMap<QString, QString>
BackendSettingsSerializer::environmentFromJson(const QJsonObject &object)
{
    QMap<QString, QString> environment;
    for (auto it = object.constBegin(); it != object.constEnd(); ++it) {
        if (it.value().isString()) {
            environment.insert(it.key(), it.value().toString());
        }
    }
    return environment;
}

}
}
