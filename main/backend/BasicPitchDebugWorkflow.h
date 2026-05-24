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

#ifndef TONY_BASIC_PITCH_DEBUG_WORKFLOW_H
#define TONY_BASIC_PITCH_DEBUG_WORKFLOW_H

#include "BasicPitchLayerPersistenceExportProof.h"
#include "BasicPitchRealRunHandoffProof.h"

#include <QStringList>
#include <QVector>

namespace Tony {
namespace Backend {

enum class BasicPitchDebugWorkflowMode {
    SyntheticArtifactOnly,
    DiscoveredArtifactManualOnly,
    RealBasicPitchManualOptIn
};

enum class BasicPitchDebugTruthState {
    NoAudio,
    BackendNotConfigured,
    MissingExecutable,
    MissingModelOrRuntime,
    PathChecksPassed,
    RunningBackend,
    BackendFailed,
    ArtifactsDiscovered,
    ResultJsonWritten,
    UnifiedResultLoaded,
    ImportedIntoRealLayer,
    InsertedIntoView,
    EditProofPassed,
    SaveLoadProofPassed,
    ExportProofPassed,
    CompletedWithWarnings,
    Failed,
    Skipped
};

QString basicPitchDebugWorkflowModeToString(
    BasicPitchDebugWorkflowMode mode);

QString basicPitchDebugTruthStateToString(
    BasicPitchDebugTruthState state);

struct BasicPitchDebugWorkflowRequest
{
    BasicPitchDebugWorkflowMode mode =
        BasicPitchDebugWorkflowMode::SyntheticArtifactOnly;
    BasicPitchArtifactDiscoveryConfig discoveryConfig;
    BasicPitchRealRunHandoffProofConfig realRunConfig;
    QString outputDirectoryPath;
    QString inputAudioPath;
    QString resultJsonPath;
    QString exportCsvPath;
    QString requestId = "basic_pitch_debug_workflow_request";
    QString resultId = "basic_pitch_debug_workflow_result";
    double sampleRate = 44100.0;
    int resolution = 1;
};

struct BasicPitchDebugWorkflowEvent
{
    BasicPitchDebugTruthState state = BasicPitchDebugTruthState::Skipped;
    QString message;
};

struct BasicPitchDebugWorkflowProofBundle
{
    QVector<BasicPitchDebugWorkflowEvent> events;
    QString commandUsed;
    QString inputAudioPath;
    QString outputDirectoryPath;
    QVector<BasicPitchDiscoveredArtifact> discoveredArtifacts;
    QString resultJsonPath;
    QString exportCsvPath;
    bool loadedResult = false;
    int noteCount = 0;
    QString documentPaneLayerModelSnapshotSummary;
    bool importedIntoTonyLayers = false;
    bool insertedIntoView = false;
    bool editProof = false;
    bool saveLoadProof = false;
    bool exportProof = false;
    QStringList warningCodes;
    QStringList errorCodes;
    bool productionTranscription = false;
    bool testOnlyDebugOnly = true;
};

struct BasicPitchDebugWorkflowReport
{
    ValidationReport report;
    BasicPitchDebugWorkflowMode mode =
        BasicPitchDebugWorkflowMode::SyntheticArtifactOnly;
    BasicPitchDebugWorkflowProofBundle proofBundle;
    BasicPitchArtifactDiscoveryResult discoveryResult;
    BasicPitchUnifiedResultHandoffResult handoffResult;
    BasicPitchRealRunHandoffProofResult realRunResult;
    BasicPitchLayerPersistenceExportProofResult persistenceExportProofResult;
    TonyLayerCommandHistoryEditProofResult editProofResult;
    QVector<BasicPitchDebugTruthState> truthStates;
    QString skippedReason;
    bool ranBasicPitch = false;
    bool resultJsonWritten = false;
    bool unifiedResultLoaded = false;
    bool importedIntoTonyLayers = false;
    bool insertedIntoView = false;
    bool editProofPassed = false;
    bool saveLoadProofPassed = false;
    bool exportProofPassed = false;
    bool possiblePolyphony = false;
    bool pitchBendMappingDeferred = false;
    bool productionTranscription = false;
    bool testOnlyDebugOnly = true;
    bool readyInstalledCompletedMutation = false;

    bool isValid() const;
    bool wasSkipped() const;
    bool hasTruthState(BasicPitchDebugTruthState state) const;
    QString debugSummaryString() const;
};

class BasicPitchDebugWorkflow
{
public:
    BasicPitchDebugWorkflowReport run(
        const BasicPitchDebugWorkflowRequest &request) const;

private:
    static void appendIssues(ValidationReport &target,
                             const ValidationReport &source);
    static void addState(BasicPitchDebugWorkflowReport &report,
                         BasicPitchDebugTruthState state,
                         const QString &message);
};

}
}

#endif
