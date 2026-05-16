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

#ifndef TONY_BACKEND_REQUIRED_FILE_PROBE_H
#define TONY_BACKEND_REQUIRED_FILE_PROBE_H

#include "BackendSettingsStore.h"
#include "BackendTypes.h"
#include "ResultValidator.h"

namespace Tony {
namespace Backend {

enum class BackendRequiredFileProbeRole {
    RequiredFile,
    OptionalFile,
    ModelCheckpoint
};

enum class BackendRequiredFileProbeStatus {
    EmptyPath,
    Missing,
    Directory,
    Present
};

struct BackendRequiredFileProbeEntry
{
    BackendRequiredFileProbeRole role =
        BackendRequiredFileProbeRole::RequiredFile;
    BackendRequiredFileProbeStatus status =
        BackendRequiredFileProbeStatus::EmptyPath;
    QString path;
    bool exists = false;
    bool isFile = false;
    bool isDirectory = false;

    bool isPresent() const;
    bool isRequired() const;
    QString roleName() const;
    QString statusName() const;
};

struct BackendRequiredFileProbeResult
{
    ValidationReport report;
    BackendId backendId;
    QVector<BackendRequiredFileProbeEntry> entries;

    bool isValid() const;
    bool hasRequiredFiles() const;
    bool hasModelCheckpointProbe() const;
    bool hasMissingRequiredFile() const;
    bool hasMissingModelCheckpoint() const;
};

class BackendRequiredFileProbe
{
public:
    BackendRequiredFileProbeResult probe(const BackendManifest &manifest) const;
    BackendRequiredFileProbeResult probe(const BackendManifest &manifest,
                                         const BackendSettings &settings) const;
};

QString toString(BackendRequiredFileProbeRole role);
QString toString(BackendRequiredFileProbeStatus status);

}
}

#endif
