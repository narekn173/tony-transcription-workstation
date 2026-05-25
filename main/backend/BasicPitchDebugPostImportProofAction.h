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

#ifndef TONY_BASIC_PITCH_DEBUG_POST_IMPORT_PROOF_ACTION_H
#define TONY_BASIC_PITCH_DEBUG_POST_IMPORT_PROOF_ACTION_H

#include "BasicPitchLayerPersistenceExportProof.h"

namespace Tony {
namespace Backend {

struct BasicPitchDebugPostImportProofActionOptions
{
    QString resultJsonPath;
    QString exportCsvPath;
    double sampleRate = 44100.0;
    int resolution = 1;
};

struct BasicPitchDebugPostImportProofActionReport
{
    ValidationReport report;
    BasicPitchLayerPersistenceExportProofResult proofResult;
    QStringList warningCodes;
    QStringList errorCodes;
    QString resultJsonPath;
    QString exportCsvPath;
    int noteCount = 0;
    int exportedNoteCount = 0;
    bool loadedResult = false;
    bool realNoteModelExists = false;
    bool realNoteLayerExists = false;
    bool documentOwnedLayer = false;
    bool insertedIntoView = false;
    bool layerEditable = false;
    bool editProofTested = false;
    bool editProofPassed = false;
    bool undoRedoProofTested = false;
    bool undoRedoProofPassed = false;
    bool saveLoadProofTested = false;
    bool saveLoadProofPassed = false;
    bool exportProofTested = false;
    bool exportProofPassed = false;
    bool exportedCsvNonEmpty = false;
    bool exportedTimingDurationPitchVelocity = false;
    bool possiblePolyphony = false;
    bool pitchBendMappingDeferred = false;
    bool productionTranscription = false;
    bool testOnlyDebugOnly = true;
    bool readyInstalledCompletedMutation = false;

    bool isValid() const;
    QString debugSummaryString() const;
};

class BasicPitchDebugPostImportProofAction
{
public:
    BasicPitchDebugPostImportProofActionReport prove(
        const BasicPitchDebugPostImportProofActionOptions &options) const;

private:
    static void appendIssues(ValidationReport &target,
                             const ValidationReport &source);
    static void collectIssueCodes(const ValidationReport &source,
                                  QStringList &warnings,
                                  QStringList &errors);
    static QString resolveExportCsvPath(const QString &resultJsonPath,
                                        const QString &exportCsvPath);
};

}
}

#endif
