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

#ifndef TONY_BASIC_PITCH_DEBUG_MANUAL_RUN_ACTION_H
#define TONY_BASIC_PITCH_DEBUG_MANUAL_RUN_ACTION_H

#include "BasicPitchDebugManualRunStatus.h"

#include <QStringList>
#include <QVector>

namespace Tony {
namespace Backend {

struct BasicPitchDebugManualRunActionReport
{
    ValidationReport report;
    BasicPitchDebugManualRunStatusResult preflightStatus;
    BasicPitchRealRunHandoffProofResult realRunResult;
    QStringList stageStates;
    QStringList warningCodes;
    QStringList errorCodes;
    QString commandUsed;
    QString inputAudioPath;
    QString outputDirectoryPath;
    QString resultJsonPath;
    QString selectedNoteEventsArtifactPath;
    QVector<BasicPitchDiscoveredArtifact> discoveredArtifacts;
    int noteCount = 0;
    bool preflightPassed = false;
    bool manualRunAttempted = false;
    bool ranBasicPitch = false;
    bool resultJsonWritten = false;
    bool loadedResult = false;
    bool possiblePolyphony = false;
    bool pitchBendMappingDeferred = false;
    bool productionTranscription = false;
    bool testOnlyDebugOnly = true;
    bool importedIntoTonyLayers = false;
    bool readyInstalledCompletedMutation = false;

    bool isValid() const;
    bool wasSkipped() const;
    QString debugSummaryString() const;
};

class BasicPitchDebugManualRunAction
{
public:
    BasicPitchDebugManualRunActionReport run(
        const BasicPitchRealRunHandoffProofConfig &config) const;

private:
    static void appendIssues(ValidationReport &target,
                             const ValidationReport &source);
    static void collectIssueCodes(const ValidationReport &source,
                                  QStringList &warnings,
                                  QStringList &errors);
    static void addStage(BasicPitchDebugManualRunActionReport &report,
                         const QString &stage);
};

}
}

#endif
