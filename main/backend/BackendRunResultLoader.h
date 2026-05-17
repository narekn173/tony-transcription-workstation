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

#ifndef TONY_BACKEND_RUN_RESULT_LOADER_H
#define TONY_BACKEND_RUN_RESULT_LOADER_H

#include "BackendRunOutputHandoff.h"
#include "UnifiedResultFileLoader.h"

#include <optional>

namespace Tony {
namespace Backend {

struct BackendRunResultLoadResult
{
    ValidationReport report;
    BackendRunOutputHandoffResult handoffResult;
    UnifiedResultFileLoadResult fileLoadResult;
    std::optional<UnifiedResult> loadedResult;
    QString outputPath;
    bool handoffAccepted = false;
    bool fileLoaded = false;
    bool importedIntoTonyLayers = false;

    bool isValid() const;
    QString debugSummaryString() const;
};

class BackendRunResultLoader
{
public:
    BackendRunResultLoadResult load(
        const QString &expectedUnifiedResultJsonPath) const;

    BackendRunResultLoadResult load(
        const QString &expectedUnifiedResultJsonPath,
        const std::optional<ExternalProcessResult> &processResult) const;

    BackendRunResultLoadResult load(
        const BackendRunOrchestrationResult &runResult) const;

private:
    static void appendIssues(ValidationReport &target,
                             const ValidationReport &source);
};

}
}

#endif
