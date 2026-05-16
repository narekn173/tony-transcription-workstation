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

#include "BackendDiscoveryConfig.h"

namespace Tony {
namespace Backend {

namespace {

QString
cleanPath(const QString &path)
{
    return path.trimmed();
}

void
appendIfUsable(QVector<BackendDiscoveryPath> &paths,
               BackendDiscoveryPathSource source,
               const QString &path)
{
    const QString cleaned = cleanPath(path);
    if (!cleaned.isEmpty()) {
        paths.push_back({ source, cleaned });
    }
}

}

bool
BackendDiscoveryPath::isUsable() const
{
    return !cleanPath(path).isEmpty();
}

QString
BackendDiscoveryPath::sourceName() const
{
    switch (source) {
    case BackendDiscoveryPathSource::DefaultLocal:
        return "default_local";
    case BackendDiscoveryPathSource::UserConfigured:
        return "user_configured";
    case BackendDiscoveryPathSource::TestOnly:
        return "test_only";
    }
    return "default_local";
}

BackendDiscoveryConfig
BackendDiscoveryConfig::safeDefaults()
{
    BackendDiscoveryConfig config;
    config.defaultLocalManifestDirectoryPath =
        defaultLocalManifestDirectoryPathConcept();
    return config;
}

QString
BackendDiscoveryConfig::defaultLocalManifestDirectoryPathConcept()
{
    return "backends/manifests";
}

QVector<BackendDiscoveryPath>
BackendDiscoveryConfig::manifestDirectoryCandidates() const
{
    QVector<BackendDiscoveryPath> paths;

    appendIfUsable(paths,
                   BackendDiscoveryPathSource::DefaultLocal,
                   defaultLocalManifestDirectoryPath);
    appendIfUsable(paths,
                   BackendDiscoveryPathSource::UserConfigured,
                   userConfiguredManifestDirectoryPath);
    appendIfUsable(paths,
                   BackendDiscoveryPathSource::TestOnly,
                   testOnlyManifestDirectoryPath);

    return paths;
}

ValidationReport
BackendDiscoveryConfig::validate() const
{
    ValidationReport report;

    if (cleanPath(defaultLocalManifestDirectoryPath).isEmpty()) {
        report.addIssue(ValidationSeverity::Warning,
                        "empty_default_manifest_directory",
                        "Default backend manifest directory is empty.");
    }
    if (!userConfiguredManifestDirectoryPath.isEmpty() &&
        cleanPath(userConfiguredManifestDirectoryPath).isEmpty()) {
        report.addIssue(ValidationSeverity::Warning,
                        "empty_user_manifest_directory",
                        "User-configured backend manifest directory is empty.");
    }
    if (!testOnlyManifestDirectoryPath.isEmpty() &&
        cleanPath(testOnlyManifestDirectoryPath).isEmpty()) {
        report.addIssue(ValidationSeverity::Warning,
                        "empty_test_manifest_directory",
                        "Test-only backend manifest directory is empty.");
    }
    if (!hasAnyUsableManifestDirectory()) {
        report.addIssue(ValidationSeverity::Warning,
                        "no_manifest_directories",
                        "No backend manifest directories are configured.");
    }

    return report;
}

bool
BackendDiscoveryConfig::hasAnyUsableManifestDirectory() const
{
    return !manifestDirectoryCandidates().isEmpty();
}

}
}
