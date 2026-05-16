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

#include "BackendManifestDirectoryLoader.h"

#include <QDir>
#include <QFileInfo>

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

}

bool
BackendManifestDirectoryEntry::isValid() const
{
    return report.isValid() && addedToRegistry;
}

bool
BackendManifestDirectoryLoadResult::isValid() const
{
    return report.isValid();
}

int
BackendManifestDirectoryLoadResult::loadedCount() const
{
    int count = 0;
    for (const auto &entry: entries) {
        if (entry.addedToRegistry) {
            ++count;
        }
    }
    return count;
}

BackendManifestDirectoryLoadResult
BackendManifestDirectoryLoader::load(const QString &directoryPath,
                                     BackendRegistry &registry) const
{
    BackendManifestDirectoryLoadResult result;
    result.directoryPath = directoryPath;

    const QFileInfo directoryInfo(directoryPath);
    if (!directoryInfo.exists()) {
        result.report.addError("directory_not_found",
                               QString("Backend manifest directory does not exist: %1")
                                   .arg(directoryPath));
        return result;
    }
    if (!directoryInfo.isDir()) {
        result.report.addError("not_a_directory",
                               QString("Backend manifest path is not a directory: %1")
                                   .arg(directoryPath));
        return result;
    }

    const QDir directory(directoryPath);
    const QFileInfoList files =
        directory.entryInfoList({ "*.json" },
                                QDir::Files | QDir::NoSymLinks,
                                QDir::Name | QDir::IgnoreCase);

    BackendManifestFileLoader fileLoader;
    for (const QFileInfo &fileInfo: files) {
        BackendManifestDirectoryEntry entry;
        const BackendManifestFileLoadResult loaded =
            fileLoader.load(fileInfo.absoluteFilePath());

        entry.path = loaded.path;
        entry.manifest = loaded.manifest;
        entry.report = loaded.report;

        if (loaded.isValid()) {
            if (registry.addManifest(loaded.manifest)) {
                entry.addedToRegistry = true;
            } else {
                entry.report.addError(
                    "registry_add_failed",
                    QString("BackendManifest could not be added to the registry: %1")
                        .arg(loaded.manifest.id()));
            }
        }

        appendIssues(result.report, entry.report);
        result.entries.push_back(entry);
    }

    return result;
}

}
}
