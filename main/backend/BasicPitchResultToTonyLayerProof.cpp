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

#include "BasicPitchResultToTonyLayerProof.h"

#include "BasicPitchAdapterContract.h"

namespace Tony {
namespace Backend {

namespace {

bool
resultHasWarning(const UnifiedResult &result, const QString &code)
{
    for (const ResultWarning &warning: result.warnings) {
        if (warning.code == code) {
            return true;
        }
    }
    return false;
}

QStringList
warningCodes(const UnifiedResult &result)
{
    QStringList codes;
    for (const ResultWarning &warning: result.warnings) {
        if (!warning.code.trimmed().isEmpty()) {
            codes << warning.code.trimmed();
        }
    }
    return codes;
}

QString
backendVersionString(const UnifiedResult &result)
{
    if (result.engine.engineVersion.has_value() &&
        !result.engine.engineVersion->trimmed().isEmpty()) {
        return result.engine.engineVersion->trimmed();
    }
    return result.engine.adapterVersion.trimmed();
}

bool
provenanceBool(const UnifiedResult &result,
               const QString &key,
               bool defaultValue = false)
{
    return result.provenance.value(key, defaultValue).toBool();
}

}

bool
BasicPitchResultToTonyLayerProofResult::isValid() const
{
    return report.isValid() &&
        loadedResult &&
        basicPitchShaped &&
        importResult.isValid() &&
        !productionTranscription &&
        !readyInstalledCompletedMutation;
}

QString
BasicPitchResultToTonyLayerProofResult::debugSummaryString() const
{
    return QString("basic_pitch_result_to_tony_layer path=%1 loaded=%2 "
                   "basic_pitch=%3 notes=%4 polyphony=%5 bends_deferred=%6 "
                   "imported=%7 inserted=%8 production=%9 ready_mutation=%10 "
                   "valid=%11")
        .arg(resultJsonPath)
        .arg(loadedResult ? QString("true") : QString("false"))
        .arg(basicPitchShaped ? QString("true") : QString("false"))
        .arg(noteCount)
        .arg(possiblePolyphony ? QString("true") : QString("false"))
        .arg(pitchBendMappingDeferred ? QString("true") : QString("false"))
        .arg(importedIntoTonyLayers ? QString("true") : QString("false"))
        .arg(insertedIntoView ? QString("true") : QString("false"))
        .arg(productionTranscription ? QString("true") : QString("false"))
        .arg(readyInstalledCompletedMutation ? QString("true") :
                                               QString("false"))
        .arg(isValid() ? QString("true") : QString("false"));
}

BasicPitchResultToTonyLayerProofResult
BasicPitchResultToTonyLayerProof::importResultJson(
    const QString &resultJsonPath,
    const TonyLayerImportOptions &importOptions) const
{
    BackendRunResultLoader loader;
    const BackendRunResultLoadResult loaded = loader.load(resultJsonPath);
    return importLoadedResult(loaded, resultJsonPath, importOptions);
}

BasicPitchResultToTonyLayerProofResult
BasicPitchResultToTonyLayerProof::importLoadedResult(
    const BackendRunResultLoadResult &loadResult,
    const QString &resultJsonPath,
    const TonyLayerImportOptions &importOptions) const
{
    BasicPitchResultToTonyLayerProofResult result;
    result.resultJsonPath = resultJsonPath.trimmed();
    result.loadResult = loadResult;
    result.loadedResult = loadResult.isValid();
    result.readyInstalledCompletedMutation = false;

    appendIssues(result.report, loadResult.report);
    if (!result.loadedResult || !loadResult.loadedResult.has_value()) {
        return result;
    }

    const UnifiedResult &loaded = *loadResult.loadedResult;
    result.basicPitchShaped =
        loaded.engine.engineId == BasicPitchAdapterContract::backendId();
    result.warningCodes = warningCodes(loaded);
    result.noteCount = loaded.notes.size();
    result.possiblePolyphony =
        resultHasWarning(loaded, "possible_polyphony");
    result.pitchBendMappingDeferred =
        resultHasWarning(loaded, "pitch_bend_mapping_deferred");
    result.productionTranscription =
        provenanceBool(loaded, "production_transcription", false);

    if (!result.basicPitchShaped) {
        result.report.addError(
            "loaded_result_is_not_basic_pitch",
            "Loaded UnifiedResult is not identified as a Basic Pitch result.");
        return result;
    }

    if (result.productionTranscription) {
        result.report.addError(
            "basic_pitch_result_claims_production_transcription",
            "CODEX-101 Basic Pitch result-to-layer proof only accepts "
            "manual/test-only results.");
        return result;
    }

    if (result.possiblePolyphony) {
        result.report.addIssue(
            ValidationSeverity::Warning,
            "basic_pitch_possible_polyphony_not_resolved",
            "Basic Pitch result reports possible polyphony. CODEX-101 imports "
            "the note events into a real Tony note layer for proof only; it "
            "does not claim a production polyphony policy.");
    }

    if (result.pitchBendMappingDeferred) {
        result.report.addIssue(
            ValidationSeverity::Warning,
            "basic_pitch_pitch_bend_tony_mapping_deferred",
            "Basic Pitch pitch-bend data remains in UnifiedResult, but "
            "CODEX-101 does not import pitch bends into a Tony layer.");
    }

    result.report.addIssue(
        ValidationSeverity::Warning,
        "basic_pitch_result_to_tony_layer_test_only",
        "Basic Pitch result-to-Tony-layer proof is manual/test-only and is "
        "not production Basic Pitch UI integration.");

    const TonyLayerImportOptions enrichedOptions =
        importOptionsWithBasicPitchProvenance(loaded,
                                             result.resultJsonPath,
                                             result.warningCodes,
                                             importOptions);

    TonyLayerImporter importer;
    result.importResult = importer.importResult(loaded, enrichedOptions);
    appendIssues(result.report, result.importResult.report);
    result.importedIntoTonyLayers = result.importResult.importedIntoTonyLayers;
    result.insertedIntoView = result.importResult.insertedIntoView;
    result.noteCount = result.importResult.noteCount;

    return result;
}

BasicPitchResultToTonyLayerProofResult
BasicPitchResultToTonyLayerProof::importHandoffResult(
    const BasicPitchUnifiedResultHandoffResult &handoffResult,
    const TonyLayerImportOptions &importOptions) const
{
    return importLoadedResult(handoffResult.loadResult,
                              handoffResult.expectedUnifiedResultJsonPath,
                              importOptions);
}

BasicPitchResultToTonyLayerProofResult
BasicPitchResultToTonyLayerProof::importRealRunResult(
    const BasicPitchRealRunHandoffProofResult &realRunResult,
    const TonyLayerImportOptions &importOptions) const
{
    BasicPitchResultToTonyLayerProofResult result =
        importHandoffResult(realRunResult.handoffResult, importOptions);
    appendIssues(result.report, realRunResult.report);
    return result;
}

TonyLayerImportOptions
BasicPitchResultToTonyLayerProof::importOptionsWithBasicPitchProvenance(
    const UnifiedResult &result,
    const QString &resultJsonPath,
    const QStringList &warningCodes,
    const TonyLayerImportOptions &importOptions)
{
    TonyLayerImportOptions enriched = importOptions;

    if (enriched.provenance.backendId.trimmed().isEmpty()) {
        enriched.provenance.backendId =
            result.engine.engineId.trimmed().isEmpty() ?
                BasicPitchAdapterContract::backendId() :
                result.engine.engineId.trimmed();
    }

    if (enriched.provenance.backendName.trimmed().isEmpty()) {
        enriched.provenance.backendName =
            result.engine.displayName.trimmed().isEmpty() ?
                QString("Basic Pitch") :
                result.engine.displayName.trimmed();
    }

    if (enriched.provenance.backendVersion.trimmed().isEmpty()) {
        enriched.provenance.backendVersion = backendVersionString(result);
    }

    if (enriched.provenance.inputAudioPath.trimmed().isEmpty()) {
        enriched.provenance.inputAudioPath = result.audio.path.trimmed();
    }

    if (enriched.provenance.resultJsonPath.trimmed().isEmpty()) {
        enriched.provenance.resultJsonPath = resultJsonPath.trimmed();
    }

    if (enriched.provenance.runId.trimmed().isEmpty()) {
        enriched.provenance.runId =
            result.requestId.trimmed().isEmpty() ?
                result.resultId.trimmed() :
                result.requestId.trimmed();
    }

    if (enriched.provenance.warningSummary.trimmed().isEmpty() &&
        !warningCodes.isEmpty()) {
        enriched.provenance.warningSummary = warningCodes.join(",");
    }

    enriched.provenance.testOnly = true;
    enriched.provenance.devMock = false;

    return enriched;
}

void
BasicPitchResultToTonyLayerProof::appendIssues(ValidationReport &target,
                                               const ValidationReport &source)
{
    for (const ValidationIssue &issue: source.issues) {
        target.addIssue(issue.severity, issue.code, issue.message);
    }
}

}
}
