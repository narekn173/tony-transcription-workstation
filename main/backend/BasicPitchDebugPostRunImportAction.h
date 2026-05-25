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

#ifndef TONY_BASIC_PITCH_DEBUG_POST_RUN_IMPORT_ACTION_H
#define TONY_BASIC_PITCH_DEBUG_POST_RUN_IMPORT_ACTION_H

#include "BasicPitchDebugManualRunStatus.h"
#include "BasicPitchResultToTonyLayerProof.h"

#include <QStringList>

namespace sv {
class Document;
class View;
}

namespace Tony {
namespace Backend {

struct BasicPitchDebugPostRunImportActionOptions
{
    QString resultJsonPath;
    BasicPitchRealRunHandoffProofConfig manualRunConfig;
    bool deriveResultJsonPathFromManualConfig = true;
    double sampleRate = 44100.0;
    int resolution = 1;
    sv::Document *document = nullptr;
    sv::View *view = nullptr;
    bool insertLayerIntoView = true;
    bool requireViewForImport = false;
};

struct BasicPitchDebugPostRunImportActionReport
{
    ValidationReport report;
    BasicPitchDebugManualRunStatusResult manualRunStatus;
    BackendRunResultLoadResult loadResult;
    BasicPitchResultToTonyLayerProofResult layerImportResult;
    QStringList stageStates;
    QStringList warningCodes;
    QStringList errorCodes;
    QString resultJsonPath;
    int noteCount = 0;
    bool resultJsonPathResolved = false;
    bool loadedResult = false;
    bool basicPitchShaped = false;
    bool possiblePolyphony = false;
    bool pitchBendMappingDeferred = false;
    bool importedIntoTonyLayers = false;
    bool insertedIntoView = false;
    bool documentProvided = false;
    bool viewProvided = false;
    bool viewInsertionRequested = false;
    bool editProofTested = false;
    bool editProofPassed = false;
    bool saveLoadProofTested = false;
    bool saveLoadProofPassed = false;
    bool exportProofTested = false;
    bool exportProofPassed = false;
    bool productionTranscription = false;
    bool testOnlyDebugOnly = true;
    bool readyInstalledCompletedMutation = false;

    bool isValid() const;
    QString debugSummaryString() const;
};

class BasicPitchDebugPostRunImportAction
{
public:
    BasicPitchDebugPostRunImportActionReport importResult(
        const BasicPitchDebugPostRunImportActionOptions &options) const;

private:
    static void appendIssues(ValidationReport &target,
                             const ValidationReport &source);
    static void collectIssueCodes(const ValidationReport &source,
                                  QStringList &warnings,
                                  QStringList &errors);
    static void addStage(BasicPitchDebugPostRunImportActionReport &report,
                         const QString &stage);
    static QString resolveResultJsonPath(
        const BasicPitchDebugPostRunImportActionOptions &options,
        const BasicPitchDebugManualRunStatusResult &status);
};

}
}

#endif
