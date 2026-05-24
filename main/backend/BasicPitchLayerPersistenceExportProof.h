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

#ifndef TONY_BASIC_PITCH_LAYER_PERSISTENCE_EXPORT_PROOF_H
#define TONY_BASIC_PITCH_LAYER_PERSISTENCE_EXPORT_PROOF_H

#include "BasicPitchResultToTonyLayerProof.h"

namespace Tony {
namespace Backend {

struct BasicPitchLayerPersistenceExportProofOptions
{
    QString resultJsonPath;
    QString exportCsvPath;
    double sampleRate = 44100.0;
    int resolution = 1;
};

struct BasicPitchLayerPersistenceExportProofResult
{
    ValidationReport report;
    BasicPitchResultToTonyLayerProofResult layerImportResult;
    QString resultJsonPath;
    QString exportCsvPath;
    QString sessionXml;
    QString exportCsvText;
    QString provenanceIdentity;
    int importedNoteCount = 0;
    int reloadedNoteCount = 0;
    int exportedNoteCount = 0;
    bool loadedResult = false;
    bool importedIntoTonyLayers = false;
    bool insertedIntoView = false;
    bool reloadedLayerIsNoteLayer = false;
    bool reloadedModelIsNoteModel = false;
    bool reloadedLayerEditable = false;
    bool saveLoadProven = false;
    bool exportProven = false;
    bool durableIdentityPersisted = false;
    bool possiblePolyphony = false;
    bool pitchBendMappingDeferred = false;
    bool productionTranscription = false;
    bool readyInstalledCompletedMutation = false;
    bool exportedCsvNonEmpty = false;
    bool exportedTimingDurationPitchVelocity = false;

    bool isValid() const;
    QString debugSummaryString() const;
};

class BasicPitchLayerPersistenceExportProof
{
public:
    BasicPitchLayerPersistenceExportProofResult prove(
        const BasicPitchLayerPersistenceExportProofOptions &options) const;

    BasicPitchLayerPersistenceExportProofResult proveHandoffResult(
        const BasicPitchUnifiedResultHandoffResult &handoffResult,
        const BasicPitchLayerPersistenceExportProofOptions &options) const;

private:
    static void appendIssues(ValidationReport &target,
                             const ValidationReport &source);
};

}
}

#endif
