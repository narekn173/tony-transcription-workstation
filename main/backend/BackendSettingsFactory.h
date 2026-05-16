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

#ifndef TONY_BACKEND_SETTINGS_FACTORY_H
#define TONY_BACKEND_SETTINGS_FACTORY_H

#include "BackendSettingsPersistenceService.h"

namespace Tony {
namespace Backend {

struct BackendSettingsPersistenceComponents
{
    BackendSettingsPersistenceConfig config;
    BackendSettingsPersistenceService service;
    ValidationReport report;

    bool isValid() const;
};

class BackendSettingsFactory
{
public:
    BackendSettingsPersistenceComponents
    createDefaultPersistenceComponents() const;

    BackendSettingsPersistenceComponents createPersistenceComponents(
        const BackendSettingsPersistenceConfig &config) const;

    BackendSettingsPersistenceComponents createPersistenceComponentsFromResolvedPath(
        const BackendSettingsPathResolutionResult &resolved) const;

    BackendSettingsPersistenceComponents createTestPersistenceComponents(
        const QString &testOverridePath) const;
};

}
}

#endif
