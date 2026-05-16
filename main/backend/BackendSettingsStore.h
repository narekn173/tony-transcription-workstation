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

#ifndef TONY_BACKEND_SETTINGS_STORE_H
#define TONY_BACKEND_SETTINGS_STORE_H

#include "BackendTypes.h"
#include "ResultValidator.h"

#include <QMap>
#include <QVector>

#include <optional>

namespace Tony {
namespace Backend {

struct BackendSettings
{
    BackendId backendId;
    QString executablePathOverride;
    QString workingDirectoryOverride;
    QString modelCheckpointPathOverride;
    QString pythonExecutablePathOverride;
    QMap<QString, QString> environmentVariables;
    bool enabled = false;
    QString displayLabelOverride;
    QString userNotes;

    bool isValidBackendId() const;
    bool hasExecutablePathOverride() const;
    bool hasWorkingDirectoryOverride() const;
    bool hasModelCheckpointPathOverride() const;
    bool hasPythonExecutablePathOverride() const;
    bool hasAnyPathOverride() const;
    bool isConfigured() const;
    BackendStatus statusFromSettings() const;
    ValidationReport validate() const;
    QString debugSummaryString() const;
};

class BackendSettingsStore
{
public:
    bool setSettings(const BackendSettings &settings);
    std::optional<BackendSettings> settingsForBackend(const BackendId &backendId) const;
    bool removeSettings(const BackendId &backendId);
    void clear();
    QVector<BackendId> configuredBackendIds() const;
    int size() const;

private:
    QVector<BackendSettings> m_settings;
};

QString backendSettingsSummaryString(const BackendSettings &settings);

}
}

#endif
