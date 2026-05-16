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

#ifndef TONY_BACKEND_EXECUTABLE_PROBE_H
#define TONY_BACKEND_EXECUTABLE_PROBE_H

#include "BackendSettingsStore.h"
#include "BackendTypes.h"
#include "ResultValidator.h"

namespace Tony {
namespace Backend {

enum class BackendExecutableProbeStatus {
    EmptyPath,
    Missing,
    Directory,
    NotExecutable,
    Present
};

struct BackendExecutableProbeResult
{
    ValidationReport report;
    BackendId backendId;
    QString manifestExecutablePath;
    QString settingsExecutablePathOverride;
    QString effectiveExecutablePath;
    BackendExecutableProbeStatus status =
        BackendExecutableProbeStatus::EmptyPath;
    bool usedSettingsOverride = false;
    bool exists = false;
    bool isFile = false;
    bool isDirectory = false;
    bool isExecutable = false;

    bool isValid() const;
    bool isPresent() const;
    QString statusName() const;
};

class BackendExecutableProbe
{
public:
    BackendExecutableProbeResult probe(const BackendManifest &manifest) const;
    BackendExecutableProbeResult probe(const BackendManifest &manifest,
                                       const BackendSettings &settings) const;
};

QString toString(BackendExecutableProbeStatus status);

}
}

#endif
