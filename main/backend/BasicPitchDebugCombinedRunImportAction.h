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

#ifndef TONY_BASIC_PITCH_DEBUG_COMBINED_RUN_IMPORT_ACTION_H
#define TONY_BASIC_PITCH_DEBUG_COMBINED_RUN_IMPORT_ACTION_H

#include "BasicPitchDebugManualRunAction.h"
#include "BasicPitchDebugPostImportProofAction.h"
#include "BasicPitchDebugPostRunImportAction.h"

namespace sv {
class Document;
class View;
}

namespace Tony {
namespace Backend {

struct BasicPitchDebugCombinedRunImportActionOptions
{
    BasicPitchRealRunHandoffProofConfig manualRunConfig;
    double sampleRate = 44100.0;
    int resolution = 1;
    sv::Document *document = nullptr;
    sv::View *view = nullptr;
    bool insertLayerIntoView = true;
    bool requireViewForImport = false;
    bool runPostImportProof = true;
    QString postImportProofExportCsvPath;
};

struct BasicPitchDebugCombinedRunImportActionReport
{
    ValidationReport report;
    BasicPitchDebugManualRunActionReport manualRunReport;
    BasicPitchDebugPostRunImportActionReport importReport;
    BasicPitchDebugPostImportProofActionReport postImportProofReport;
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
    bool manualRunAllowed = false;
    bool manualRunAttempted = false;
    bool ranBasicPitch = false;
    bool resultJsonWritten = false;
    bool loadedResult = false;
    bool importAttempted = false;
    bool possiblePolyphony = false;
    bool pitchBendMappingDeferred = false;
    bool importedIntoTonyLayers = false;
    bool insertedIntoView = false;
    bool documentProvided = false;
    bool viewProvided = false;
    bool viewInsertionRequested = false;
    bool editProofTested = false;
    bool editProofPassed = false;
    bool undoRedoProofTested = false;
    bool undoRedoProofPassed = false;
    bool saveLoadProofTested = false;
    bool saveLoadProofPassed = false;
    bool exportProofTested = false;
    bool exportProofPassed = false;
    int exportedNoteCount = 0;
    bool productionTranscription = false;
    bool testOnlyDebugOnly = true;
    bool readyInstalledCompletedMutation = false;

    bool isValid() const;
    bool wasSkipped() const;
    QString debugSummaryString() const;
};

class BasicPitchDebugCombinedRunImportAction
{
public:
    BasicPitchDebugCombinedRunImportActionReport runAndImport(
        const BasicPitchDebugCombinedRunImportActionOptions &options) const;

private:
    static void appendIssues(ValidationReport &target,
                             const ValidationReport &source);
    static void collectIssueCodes(const ValidationReport &source,
                                  QStringList &warnings,
                                  QStringList &errors);
    static void appendUnique(QStringList &target,
                             const QStringList &values);
    static void addStage(BasicPitchDebugCombinedRunImportActionReport &report,
                         const QString &stage);
};

}
}

#endif
