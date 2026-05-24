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

#ifndef TONY_BASIC_PITCH_RESULT_TO_TONY_LAYER_PROOF_H
#define TONY_BASIC_PITCH_RESULT_TO_TONY_LAYER_PROOF_H

#include "BasicPitchRealRunHandoffProof.h"
#include "TonyLayerImporter.h"

namespace Tony {
namespace Backend {

struct BasicPitchResultToTonyLayerProofResult
{
    ValidationReport report;
    BackendRunResultLoadResult loadResult;
    TonyLayerImportResult importResult;
    QString resultJsonPath;
    QStringList warningCodes;
    int noteCount = 0;
    bool loadedResult = false;
    bool basicPitchShaped = false;
    bool possiblePolyphony = false;
    bool pitchBendMappingDeferred = false;
    bool productionTranscription = false;
    bool importedIntoTonyLayers = false;
    bool insertedIntoView = false;
    bool readyInstalledCompletedMutation = false;

    bool isValid() const;
    QString debugSummaryString() const;
};

class BasicPitchResultToTonyLayerProof
{
public:
    BasicPitchResultToTonyLayerProofResult importResultJson(
        const QString &resultJsonPath,
        const TonyLayerImportOptions &importOptions) const;

    BasicPitchResultToTonyLayerProofResult importLoadedResult(
        const BackendRunResultLoadResult &loadResult,
        const QString &resultJsonPath,
        const TonyLayerImportOptions &importOptions) const;

    BasicPitchResultToTonyLayerProofResult importHandoffResult(
        const BasicPitchUnifiedResultHandoffResult &handoffResult,
        const TonyLayerImportOptions &importOptions) const;

    BasicPitchResultToTonyLayerProofResult importRealRunResult(
        const BasicPitchRealRunHandoffProofResult &realRunResult,
        const TonyLayerImportOptions &importOptions) const;

private:
    static TonyLayerImportOptions importOptionsWithBasicPitchProvenance(
        const UnifiedResult &result,
        const QString &resultJsonPath,
        const QStringList &warningCodes,
        const TonyLayerImportOptions &importOptions);

    static void appendIssues(ValidationReport &target,
                             const ValidationReport &source);
};

}
}

#endif
