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

#ifndef TONY_BACKEND_SETTINGS_PERSISTENCE_CONFIG_H
#define TONY_BACKEND_SETTINGS_PERSISTENCE_CONFIG_H

#include "ResultValidator.h"

#include <QString>
#include <QVector>

namespace Tony {
namespace Backend {

enum class BackendSettingsPersistencePathSource {
    DefaultLocal,
    UserConfigured,
    TestOnly
};

struct BackendSettingsPersistencePath
{
    BackendSettingsPersistencePathSource source =
        BackendSettingsPersistencePathSource::DefaultLocal;
    QString path;

    bool isUsable() const;
    QString sourceName() const;
};

struct BackendSettingsPersistenceConfig
{
    QString defaultSettingsFilePath;
    QString userConfiguredSettingsFilePath;
    QString testOnlySettingsFilePath;

    static BackendSettingsPersistenceConfig safeDefaults();
    static QString defaultSettingsFilePathConcept();

    QVector<BackendSettingsPersistencePath> settingsFilePathCandidates() const;
    BackendSettingsPersistencePath preferredSettingsFilePath() const;
    ValidationReport validate() const;
    bool hasAnyUsableSettingsFilePath() const;
};

}
}

#endif
