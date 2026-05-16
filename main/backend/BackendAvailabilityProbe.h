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

#ifndef TONY_BACKEND_AVAILABILITY_PROBE_H
#define TONY_BACKEND_AVAILABILITY_PROBE_H

#include "BackendExecutableProbe.h"
#include "BackendRequiredFileProbe.h"
#include "BackendSettingsStore.h"
#include "ResultValidator.h"

namespace Tony {
namespace Backend {

enum class BackendAvailabilityProbeStatus {
    NotConfigured,
    MissingExecutable,
    MissingModel,
    InvalidPath,
    PathChecksPassed
};

struct BackendAvailabilityReport
{
    ValidationReport report;
    BackendId backendId;
    BackendExecutableProbeResult executableProbe;
    BackendRequiredFileProbeResult requiredFileProbe;
    BackendAvailabilityProbeStatus status =
        BackendAvailabilityProbeStatus::NotConfigured;

    bool isValid() const;
    bool pathChecksPassed() const;
    QVector<ValidationIssue> warnings() const;
    QVector<ValidationIssue> errors() const;
    int warningCount() const;
    int errorCount() const;
    QString statusName() const;
    QString debugSummaryString() const;
};

class BackendAvailabilityProbe
{
public:
    BackendAvailabilityReport probe(const BackendManifest &manifest) const;
    BackendAvailabilityReport probe(const BackendManifest &manifest,
                                    const BackendSettings &settings) const;
};

QString toString(BackendAvailabilityProbeStatus status);

}
}

#endif
