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

#include "BackendDiscoveryService.h"

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

int
issueCount(const ValidationReport &report, const QString &code)
{
    int count = 0;
    for (const auto &issue: report.issues) {
        if (issue.code == code) {
            ++count;
        }
    }
    return count;
}

}

bool
BackendDiscoveryDirectoryResult::isValid() const
{
    return loadResult.isValid();
}

int
BackendDiscoveryDirectoryResult::manifestCount() const
{
    return loadResult.entries.size();
}

int
BackendDiscoveryDirectoryResult::loadedCount() const
{
    return loadResult.loadedCount();
}

int
BackendDiscoveryDirectoryResult::rejectedCount() const
{
    return manifestCount() - loadedCount();
}

bool
BackendDiscoveryResult::isValid() const
{
    return report.isValid();
}

int
BackendDiscoveryResult::directoriesScanned() const
{
    return directories.size();
}

int
BackendDiscoveryResult::manifestsLoaded() const
{
    int count = 0;
    for (const auto &directory: directories) {
        count += directory.loadedCount();
    }
    return count;
}

int
BackendDiscoveryResult::manifestsRejected() const
{
    int count = 0;
    for (const auto &directory: directories) {
        count += directory.rejectedCount();
    }
    return count;
}

int
BackendDiscoveryResult::duplicateIds() const
{
    return issueCount(report, "registry_add_failed");
}

BackendDiscoveryResult
BackendDiscoveryService::discover(const BackendDiscoveryConfig &config,
                                  BackendRegistry &registry) const
{
    BackendDiscoveryResult result;
    result.config = config;

    appendIssues(result.report, config.validate());

    BackendManifestDirectoryLoader loader;
    const QVector<BackendDiscoveryPath> candidates =
        config.manifestDirectoryCandidates();

    for (const auto &candidate: candidates) {
        BackendDiscoveryDirectoryResult directoryResult;
        directoryResult.directory = candidate;
        directoryResult.loadResult = loader.load(candidate.path, registry);

        appendIssues(result.report, directoryResult.loadResult.report);
        result.directories.push_back(directoryResult);
    }

    return result;
}

}
}
