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

#ifndef TONY_BACKEND_RUN_OUTPUT_HANDOFF_H
#define TONY_BACKEND_RUN_OUTPUT_HANDOFF_H

#include "BackendRunOrchestrator.h"

#include <optional>

namespace Tony {
namespace Backend {

struct BackendRunOutputHandoffResult
{
    ValidationReport report;
    QString outputPath;
    qint64 fileSizeBytes = 0;
    bool exists = false;
    bool isFile = false;
    bool readable = false;
    bool nonEmpty = false;
    bool importedIntoTonyLayers = false;

    bool isValid() const;
    QString debugSummaryString() const;
};

class BackendRunOutputHandoff
{
public:
    BackendRunOutputHandoffResult inspect(
        const QString &expectedUnifiedResultJsonPath) const;

    BackendRunOutputHandoffResult inspect(
        const QString &expectedUnifiedResultJsonPath,
        const std::optional<ExternalProcessResult> &processResult) const;

    BackendRunOutputHandoffResult inspect(
        const BackendRunOrchestrationResult &runResult) const;
};

}
}

#endif
