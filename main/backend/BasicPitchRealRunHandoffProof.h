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

#ifndef TONY_BASIC_PITCH_REAL_RUN_HANDOFF_PROOF_H
#define TONY_BASIC_PITCH_REAL_RUN_HANDOFF_PROOF_H

#include "BasicPitchUnifiedResultHandoff.h"

#include <QProcessEnvironment>

namespace Tony {
namespace Backend {

struct BasicPitchRealRunHandoffProofConfig
{
    BasicPitchArtifactDiscoveryConfig discoveryConfig;
    QString resultJsonPath;
};

struct BasicPitchRealRunHandoffProofResult
{
    ValidationReport report;
    BasicPitchRealRunHandoffProofConfig config;
    BasicPitchArtifactDiscoveryResult discoveryResult;
    BasicPitchUnifiedResultHandoffResult handoffResult;
    QString skippedReason;
    QString commandUsed;
    QString inputAudioPath;
    QString outputDirectoryPath;
    QString selectedNoteEventsArtifactPath;
    QString resultJsonPath;
    int noteCount = 0;
    bool ranBasicPitch = false;
    bool loadedResult = false;
    bool possiblePolyphony = true;
    bool pitchBendMappingDeferred = false;
    bool productionTranscription = false;
    bool importedIntoTonyLayers = false;
    bool readyInstalledCompletedMutation = false;

    bool isValid() const;
    bool wasSkipped() const;
    QString debugSummaryString() const;
};

class BasicPitchRealRunHandoffProof
{
public:
    BasicPitchRealRunHandoffProofResult run(
        const BasicPitchRealRunHandoffProofConfig &config) const;

    static BasicPitchRealRunHandoffProofConfig configFromEnvironment();
    static BasicPitchRealRunHandoffProofConfig configFromEnvironment(
        const QProcessEnvironment &environment);

private:
    static QString defaultResultJsonPath(const QString &outputDirectoryPath);
    static QString firstNoteEventsArtifactPath(
        const BasicPitchArtifactDiscoveryResult &discoveryResult);
};

}
}

#endif
