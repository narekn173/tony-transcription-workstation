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

#include "BackendSettingsPersistenceService.h"

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
BackendSettingsPersistenceLoadResult::isValid() const
{
    return report.isValid();
}

bool
BackendSettingsPersistenceSaveResult::isValid() const
{
    return report.isValid();
}

BackendSettingsPersistenceLoadResult
BackendSettingsPersistenceService::load(
    const BackendSettingsPersistenceConfig &config) const
{
    BackendSettingsPersistenceLoadResult loaded;

    BackendSettingsPersistencePath path;
    if (!resolvePreferredPath(config, path, loaded.report)) {
        return loaded;
    }

    loaded.path = path.path;
    loaded.source = path.source;

    BackendSettingsFileStore fileStore;
    const BackendSettingsFileLoadResult fileLoaded = fileStore.load(path.path);

    loaded.store = fileLoaded.store;
    loaded.loadedCount = fileLoaded.loadedCount;
    loaded.rejectedCount = fileLoaded.rejectedCount;
    appendIssues(loaded.report, fileLoaded.report);

    return loaded;
}

BackendSettingsPersistenceSaveResult
BackendSettingsPersistenceService::save(
    const BackendSettingsPersistenceConfig &config,
    const BackendSettingsStore &store) const
{
    BackendSettingsPersistenceSaveResult saved;

    BackendSettingsPersistencePath path;
    if (!resolvePreferredPath(config, path, saved.report)) {
        return saved;
    }

    saved.path = path.path;
    saved.source = path.source;

    BackendSettingsFileStore fileStore;
    const BackendSettingsFileSaveResult fileSaved =
        fileStore.save(path.path, store);

    appendIssues(saved.report, fileSaved.report);

    return saved;
}

bool
BackendSettingsPersistenceService::resolvePreferredPath(
    const BackendSettingsPersistenceConfig &config,
    BackendSettingsPersistencePath &path,
    ValidationReport &report)
{
    appendIssues(report, config.validate());

    path = config.preferredSettingsFilePath();
    if (!path.isUsable()) {
        report.addError(
            "no_settings_file_path",
            "No usable BackendSettings persistence path is configured.");
        return false;
    }

    return true;
}

}
}
