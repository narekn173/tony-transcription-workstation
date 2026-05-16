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

#include "BackendSettingsStore.h"

namespace Tony {
namespace Backend {

namespace {

bool
hasText(const QString &value)
{
    return !value.trimmed().isEmpty();
}

}

bool
BackendSettings::isValidBackendId() const
{
    return Tony::Backend::isValidBackendId(backendId);
}

bool
BackendSettings::hasExecutablePathOverride() const
{
    return hasText(executablePathOverride);
}

bool
BackendSettings::hasWorkingDirectoryOverride() const
{
    return hasText(workingDirectoryOverride);
}

bool
BackendSettings::hasModelCheckpointPathOverride() const
{
    return hasText(modelCheckpointPathOverride);
}

bool
BackendSettings::hasPythonExecutablePathOverride() const
{
    return hasText(pythonExecutablePathOverride);
}

bool
BackendSettings::hasAnyPathOverride() const
{
    return hasExecutablePathOverride() ||
        hasWorkingDirectoryOverride() ||
        hasModelCheckpointPathOverride() ||
        hasPythonExecutablePathOverride();
}

bool
BackendSettings::isConfigured() const
{
    return isValidBackendId() &&
        (enabled ||
         hasAnyPathOverride() ||
         !environmentVariables.isEmpty() ||
         hasText(displayLabelOverride) ||
         hasText(userNotes));
}

BackendStatus
BackendSettings::statusFromSettings() const
{
    return BackendStatus::NotConfigured;
}

ValidationReport
BackendSettings::validate() const
{
    ValidationReport report;

    if (!isValidBackendId()) {
        report.addError("invalid_backend_id",
                        "Backend settings require a valid backend ID.");
    }

    return report;
}

QString
BackendSettings::debugSummaryString() const
{
    return backendSettingsSummaryString(*this);
}

bool
BackendSettingsStore::setSettings(const BackendSettings &settings)
{
    if (!settings.validate().isValid()) {
        return false;
    }

    for (int i = 0; i < m_settings.size(); ++i) {
        if (m_settings[i].backendId == settings.backendId) {
            m_settings[i] = settings;
            return true;
        }
    }

    m_settings.push_back(settings);
    return true;
}

std::optional<BackendSettings>
BackendSettingsStore::settingsForBackend(const BackendId &backendId) const
{
    if (!Tony::Backend::isValidBackendId(backendId)) {
        return std::nullopt;
    }

    for (const auto &settings: m_settings) {
        if (settings.backendId == backendId) {
            return settings;
        }
    }

    return std::nullopt;
}

bool
BackendSettingsStore::removeSettings(const BackendId &backendId)
{
    if (!Tony::Backend::isValidBackendId(backendId)) {
        return false;
    }

    for (int i = 0; i < m_settings.size(); ++i) {
        if (m_settings[i].backendId == backendId) {
            m_settings.removeAt(i);
            return true;
        }
    }

    return false;
}

void
BackendSettingsStore::clear()
{
    m_settings.clear();
}

QVector<BackendId>
BackendSettingsStore::configuredBackendIds() const
{
    QVector<BackendId> ids;
    ids.reserve(m_settings.size());
    for (const auto &settings: m_settings) {
        ids.push_back(settings.backendId);
    }
    return ids;
}

int
BackendSettingsStore::size() const
{
    return m_settings.size();
}

QString
backendSettingsSummaryString(const BackendSettings &settings)
{
    QStringList parts;

    if (settings.enabled) parts << "enabled";
    if (settings.hasExecutablePathOverride()) parts << "executable_path";
    if (settings.hasWorkingDirectoryOverride()) parts << "working_directory";
    if (settings.hasModelCheckpointPathOverride()) parts << "model_checkpoint";
    if (settings.hasPythonExecutablePathOverride()) parts << "python_executable";
    if (!settings.environmentVariables.isEmpty()) parts << "environment";
    if (hasText(settings.displayLabelOverride)) parts << "display_label";
    if (parts.isEmpty()) parts << "unconfigured";

    return QString("backend=%1 status=%2 settings=[%3]")
        .arg(settings.backendId.isEmpty() ? QString("unknown") : settings.backendId)
        .arg(statusToString(settings.statusFromSettings()))
        .arg(parts.join(", "));
}

}
}
