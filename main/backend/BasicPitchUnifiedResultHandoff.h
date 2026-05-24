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

#ifndef TONY_BASIC_PITCH_UNIFIED_RESULT_HANDOFF_H
#define TONY_BASIC_PITCH_UNIFIED_RESULT_HANDOFF_H

#include "BackendRunResultReporter.h"
#include "BasicPitchArtifactToUnifiedResult.h"
#include "UnifiedResultFileWriter.h"

namespace Tony {
namespace Backend {

struct BasicPitchUnifiedResultHandoffParameters
{
    BasicPitchArtifactToUnifiedResultParameters conversionParameters;
    QString expectedUnifiedResultJsonPath;
};

struct BasicPitchUnifiedResultHandoffResult
{
    ValidationReport report;
    BasicPitchArtifactToUnifiedResultResult artifactConversion;
    UnifiedResultFileWriteResult fileWriteResult;
    BackendRunOutputHandoffResult outputHandoffResult;
    BackendRunResultLoadResult loadResult;
    BackendRunResultReport runResultReport;
    QString expectedUnifiedResultJsonPath;
    int noteCount = 0;
    bool wroteResultJson = false;
    bool outputHandoffAccepted = false;
    bool loadedUnifiedResult = false;
    bool reporterLoadedUnifiedResult = false;
    bool possiblePolyphony = true;
    bool pitchBendMappingDeferred = false;
    bool productionTranscription = false;
    bool importedIntoTonyLayers = false;
    bool marksBackendReadyInstalledOrCompleted = false;

    bool isValid() const;
    QString debugSummaryString() const;
};

class BasicPitchUnifiedResultHandoff
{
public:
    BasicPitchUnifiedResultHandoffResult handoff(
        const BasicPitchArtifactDiscoveryResult &discoveryResult,
        const BasicPitchUnifiedResultHandoffParameters &parameters) const;
};

}
}

#endif
