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

#ifndef TONY_BACKEND_SETTINGS_PERSISTENCE_SERVICE_H
#define TONY_BACKEND_SETTINGS_PERSISTENCE_SERVICE_H

#include "BackendSettingsFileStore.h"
#include "BackendSettingsPersistenceConfig.h"

namespace Tony {
namespace Backend {

struct BackendSettingsPersistenceLoadResult
{
    BackendSettingsStore store;
    ValidationReport report;
    QString path;
    BackendSettingsPersistencePathSource source =
        BackendSettingsPersistencePathSource::DefaultLocal;
    int loadedCount = 0;
    int rejectedCount = 0;

    bool isValid() const;
};

struct BackendSettingsPersistenceSaveResult
{
    ValidationReport report;
    QString path;
    BackendSettingsPersistencePathSource source =
        BackendSettingsPersistencePathSource::DefaultLocal;

    bool isValid() const;
};

class BackendSettingsPersistenceService
{
public:
    BackendSettingsPersistenceLoadResult
    load(const BackendSettingsPersistenceConfig &config) const;

    BackendSettingsPersistenceSaveResult
    save(const BackendSettingsPersistenceConfig &config,
         const BackendSettingsStore &store) const;

private:
    static bool resolvePreferredPath(
        const BackendSettingsPersistenceConfig &config,
        BackendSettingsPersistencePath &path,
        ValidationReport &report);
};

}
}

#endif
