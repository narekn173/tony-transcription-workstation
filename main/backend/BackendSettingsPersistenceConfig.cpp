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

#include "BackendSettingsPersistenceConfig.h"

namespace Tony {
namespace Backend {

namespace {

QString
cleanPath(const QString &path)
{
    return path.trimmed();
}

void
appendIfUsable(QVector<BackendSettingsPersistencePath> &paths,
               BackendSettingsPersistencePathSource source,
               const QString &path)
{
    const QString cleaned = cleanPath(path);
    if (!cleaned.isEmpty()) {
        paths.push_back({ source, cleaned });
    }
}

}

bool
BackendSettingsPersistencePath::isUsable() const
{
    return !cleanPath(path).isEmpty();
}

QString
BackendSettingsPersistencePath::sourceName() const
{
    switch (source) {
    case BackendSettingsPersistencePathSource::DefaultLocal:
        return "default_local";
    case BackendSettingsPersistencePathSource::UserConfigured:
        return "user_configured";
    case BackendSettingsPersistencePathSource::TestOnly:
        return "test_only";
    }
    return "default_local";
}

BackendSettingsPersistenceConfig
BackendSettingsPersistenceConfig::safeDefaults()
{
    BackendSettingsPersistenceConfig config;
    config.defaultSettingsFilePath = defaultSettingsFilePathConcept();
    return config;
}

QString
BackendSettingsPersistenceConfig::defaultSettingsFilePathConcept()
{
    return "settings/backend_settings.json";
}

QVector<BackendSettingsPersistencePath>
BackendSettingsPersistenceConfig::settingsFilePathCandidates() const
{
    QVector<BackendSettingsPersistencePath> paths;

    appendIfUsable(paths,
                   BackendSettingsPersistencePathSource::DefaultLocal,
                   defaultSettingsFilePath);
    appendIfUsable(paths,
                   BackendSettingsPersistencePathSource::UserConfigured,
                   userConfiguredSettingsFilePath);
    appendIfUsable(paths,
                   BackendSettingsPersistencePathSource::TestOnly,
                   testOnlySettingsFilePath);

    return paths;
}

BackendSettingsPersistencePath
BackendSettingsPersistenceConfig::preferredSettingsFilePath() const
{
    const QVector<BackendSettingsPersistencePath> candidates =
        settingsFilePathCandidates();

    for (const BackendSettingsPersistencePath &candidate: candidates) {
        if (candidate.source == BackendSettingsPersistencePathSource::TestOnly) {
            return candidate;
        }
    }
    for (const BackendSettingsPersistencePath &candidate: candidates) {
        if (candidate.source ==
            BackendSettingsPersistencePathSource::UserConfigured) {
            return candidate;
        }
    }
    if (!candidates.isEmpty()) {
        return candidates.front();
    }
    return {};
}

ValidationReport
BackendSettingsPersistenceConfig::validate() const
{
    ValidationReport report;

    if (cleanPath(defaultSettingsFilePath).isEmpty()) {
        report.addIssue(ValidationSeverity::Warning,
                        "empty_default_settings_file_path",
                        "Default backend settings file path is empty.");
    }
    if (!userConfiguredSettingsFilePath.isEmpty() &&
        cleanPath(userConfiguredSettingsFilePath).isEmpty()) {
        report.addIssue(ValidationSeverity::Warning,
                        "empty_user_settings_file_path",
                        "User-configured backend settings file path is empty.");
    }
    if (!testOnlySettingsFilePath.isEmpty() &&
        cleanPath(testOnlySettingsFilePath).isEmpty()) {
        report.addIssue(ValidationSeverity::Warning,
                        "empty_test_settings_file_path",
                        "Test-only backend settings file path is empty.");
    }
    if (!hasAnyUsableSettingsFilePath()) {
        report.addIssue(ValidationSeverity::Warning,
                        "no_settings_file_paths",
                        "No backend settings file paths are configured.");
    }

    return report;
}

bool
BackendSettingsPersistenceConfig::hasAnyUsableSettingsFilePath() const
{
    return !settingsFilePathCandidates().isEmpty();
}

}
}
