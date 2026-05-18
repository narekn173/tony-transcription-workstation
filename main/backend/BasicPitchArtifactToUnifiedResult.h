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

#ifndef TONY_BASIC_PITCH_ARTIFACT_TO_UNIFIED_RESULT_H
#define TONY_BASIC_PITCH_ARTIFACT_TO_UNIFIED_RESULT_H

#include "BasicPitchArtifactDiscovery.h"
#include "BasicPitchOutputConverter.h"
#include "ResultValidator.h"

#include <optional>

namespace Tony {
namespace Backend {

struct BasicPitchArtifactToUnifiedResultParameters
{
    QString requestId = "basic_pitch_artifact_request";
    QString resultId = "basic_pitch_artifact_result";
    QString inputAudioPath;
};

struct BasicPitchArtifactToUnifiedResultResult
{
    ValidationReport report;
    ValidationReport unifiedResultValidationReport;
    BasicPitchOutputConversionResult conversion;
    UnifiedResult result;
    QString sourceArtifactPath;
    QString artifactType;
    QString fixtureOrRealArtifact = "unknown";
    bool foundNoteEventsArtifact = false;
    bool conversionSucceeded = false;
    bool unifiedResultValidationPassed = false;
    int noteCount = 0;
    bool possiblePolyphony = true;
    bool pitchBendMappingDeferred = false;
    bool productionTranscription = false;
    bool importedIntoTonyLayers = false;
    bool createdResultJson = false;
    bool marksBackendReadyInstalledOrCompleted = false;

    bool isValid() const;
    QString debugSummaryString() const;
};

class BasicPitchArtifactToUnifiedResult
{
public:
    BasicPitchArtifactToUnifiedResultResult convert(
        const BasicPitchArtifactDiscoveryResult &discoveryResult,
        const BasicPitchArtifactToUnifiedResultParameters &parameters) const;

private:
    static std::optional<BasicPitchDiscoveredArtifact> firstNoteEventsArtifact(
        const BasicPitchArtifactDiscoveryResult &discoveryResult);
};

}
}

#endif
