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

#include "BasicPitchDebugPostRunImportAction.h"

#include "BackendRunResultLoader.h"

namespace Tony {
namespace Backend {

namespace {

QString
boolString(bool value)
{
    return value ? QString("true") : QString("false");
}

QStringList
warningCodesFromResult(const UnifiedResult &result)
{
    QStringList codes;
    for (const ResultWarning &warning: result.warnings) {
        const QString code = warning.code.trimmed();
        if (!code.isEmpty()) {
            codes << code;
        }
    }
    codes.removeDuplicates();
    return codes;
}

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

}

bool
BasicPitchDebugPostRunImportActionReport::isValid() const
{
    return report.isValid() &&
        resultJsonPathResolved &&
        loadedResult &&
        basicPitchShaped &&
        layerImportResult.isValid() &&
        importedIntoTonyLayers &&
        (!viewInsertionRequested || insertedIntoView) &&
        !productionTranscription &&
        testOnlyDebugOnly &&
        !readyInstalledCompletedMutation;
}

QString
BasicPitchDebugPostRunImportActionReport::debugSummaryString() const
{
    return QString("basic_pitch_debug_post_run_import path=%1 "
                   "resolved=%2 loaded=%3 basic_pitch=%4 notes=%5 "
                   "polyphony=%6 bends_deferred=%7 document=%8 view=%9 "
                   "imported=%10 inserted=%11 production=%12 debug=%13 "
                   "ready_mutation=%14 valid=%15")
        .arg(resultJsonPath)
        .arg(boolString(resultJsonPathResolved))
        .arg(boolString(loadedResult))
        .arg(boolString(basicPitchShaped))
        .arg(noteCount)
        .arg(boolString(possiblePolyphony))
        .arg(boolString(pitchBendMappingDeferred))
        .arg(boolString(documentProvided))
        .arg(boolString(viewProvided))
        .arg(boolString(importedIntoTonyLayers))
        .arg(boolString(insertedIntoView))
        .arg(boolString(productionTranscription))
        .arg(boolString(testOnlyDebugOnly))
        .arg(boolString(readyInstalledCompletedMutation))
        .arg(boolString(isValid()));
}

BasicPitchDebugPostRunImportActionReport
BasicPitchDebugPostRunImportAction::importResult(
    const BasicPitchDebugPostRunImportActionOptions &options) const
{
    BasicPitchDebugPostRunImportActionReport result;
    result.productionTranscription = false;
    result.testOnlyDebugOnly = true;
    result.readyInstalledCompletedMutation = false;
    result.documentProvided = options.document != nullptr;
    result.viewProvided = options.view != nullptr;
    result.viewInsertionRequested =
        options.insertLayerIntoView && options.view != nullptr;

    const bool explicitResultPathProvided =
        !options.resultJsonPath.trimmed().isEmpty();
    result.manualRunStatus =
        BasicPitchDebugManualRunStatus().fromConfig(options.manualRunConfig);
    if (!explicitResultPathProvided &&
        options.deriveResultJsonPathFromManualConfig) {
        appendIssues(result.report, result.manualRunStatus.report);
    }

    result.resultJsonPath =
        resolveResultJsonPath(options, result.manualRunStatus);
    result.resultJsonPathResolved =
        !result.resultJsonPath.trimmed().isEmpty();

    if (!result.resultJsonPathResolved) {
        addStage(result, "missing_result_json_path");
        result.report.addError(
            "missing_basic_pitch_debug_import_result_json_path",
            "Basic Pitch debug post-run import requires a result.json path "
            "or a manual-run output directory from which one can be derived.");
        collectIssueCodes(result.report,
                          result.warningCodes,
                          result.errorCodes);
        return result;
    }

    addStage(result, "result_json_path_resolved");

    BackendRunResultLoader loader;
    result.loadResult = loader.load(result.resultJsonPath);
    appendIssues(result.report, result.loadResult.report);
    result.loadedResult = result.loadResult.isValid() &&
        result.loadResult.loadedResult.has_value();

    if (!result.loadedResult || !result.loadResult.loadedResult.has_value()) {
        addStage(result, "result_load_failed");
        collectIssueCodes(result.report,
                          result.warningCodes,
                          result.errorCodes);
        return result;
    }

    addStage(result, "unified_result_loaded");

    const UnifiedResult &loaded = *result.loadResult.loadedResult;
    result.noteCount = loaded.notes.size();
    result.warningCodes << warningCodesFromResult(loaded);
    result.warningCodes.removeDuplicates();
    result.possiblePolyphony =
        resultHasWarning(loaded, "possible_polyphony");
    result.pitchBendMappingDeferred =
        resultHasWarning(loaded, "pitch_bend_mapping_deferred");
    result.productionTranscription =
        loaded.provenance.value("production_transcription", false).toBool();

    if (!options.document) {
        addStage(result, "import_unavailable_no_document");
        result.report.addError(
            "missing_document_for_basic_pitch_debug_import",
            "Basic Pitch debug post-run import requires a real current "
            "Tony/Sonic Visualiser Document.");
        collectIssueCodes(result.report,
                          result.warningCodes,
                          result.errorCodes);
        return result;
    }

    if (options.requireViewForImport && !options.view) {
        addStage(result, "import_unavailable_no_view");
        result.report.addError(
            "missing_view_for_basic_pitch_debug_import",
            "Basic Pitch debug post-run import requires a real current "
            "Pane/View before it can claim visible insertion.");
        collectIssueCodes(result.report,
                          result.warningCodes,
                          result.errorCodes);
        return result;
    }

    TonyLayerImportOptions importOptions;
    importOptions.sampleRate = options.sampleRate;
    importOptions.resolution = options.resolution;
    importOptions.document = options.document;
    importOptions.createDocumentLayer = true;
    importOptions.view = options.view;
    importOptions.insertLayerIntoView =
        options.insertLayerIntoView && options.view != nullptr;

    BasicPitchResultToTonyLayerProof proof;
    result.layerImportResult =
        proof.importLoadedResult(result.loadResult,
                                 result.resultJsonPath,
                                 importOptions);
    appendIssues(result.report, result.layerImportResult.report);

    result.basicPitchShaped = result.layerImportResult.basicPitchShaped;
    result.loadedResult = result.layerImportResult.loadedResult;
    result.noteCount = result.layerImportResult.noteCount;
    result.possiblePolyphony =
        result.layerImportResult.possiblePolyphony;
    result.pitchBendMappingDeferred =
        result.layerImportResult.pitchBendMappingDeferred;
    result.importedIntoTonyLayers =
        result.layerImportResult.importedIntoTonyLayers;
    result.insertedIntoView =
        result.layerImportResult.insertedIntoView;
    result.productionTranscription =
        result.layerImportResult.productionTranscription;
    result.readyInstalledCompletedMutation =
        result.layerImportResult.readyInstalledCompletedMutation;
    result.warningCodes << result.layerImportResult.warningCodes;
    result.warningCodes.removeDuplicates();

    if (result.importedIntoTonyLayers) {
        addStage(result, "imported_into_real_layer");
    }
    if (result.insertedIntoView) {
        addStage(result, "inserted_into_view");
    }
    if (!result.layerImportResult.isValid()) {
        addStage(result, "import_failed");
    }

    result.report.addIssue(
        ValidationSeverity::Warning,
        "basic_pitch_debug_post_run_import_test_only",
        "Basic Pitch debug post-run import is debug/test-only and is not "
        "production Basic Pitch UI integration.");

    collectIssueCodes(result.report,
                      result.warningCodes,
                      result.errorCodes);
    result.warningCodes.removeDuplicates();
    result.errorCodes.removeDuplicates();
    return result;
}

void
BasicPitchDebugPostRunImportAction::appendIssues(
    ValidationReport &target,
    const ValidationReport &source)
{
    for (const ValidationIssue &issue: source.issues) {
        target.addIssue(issue.severity, issue.code, issue.message);
    }
}

void
BasicPitchDebugPostRunImportAction::collectIssueCodes(
    const ValidationReport &source,
    QStringList &warnings,
    QStringList &errors)
{
    for (const ValidationIssue &issue: source.issues) {
        const QString code = issue.code.trimmed();
        if (code.isEmpty()) {
            continue;
        }
        if (issue.severity == ValidationSeverity::Error) {
            errors << code;
        } else {
            warnings << code;
        }
    }
    warnings.removeDuplicates();
    errors.removeDuplicates();
}

void
BasicPitchDebugPostRunImportAction::addStage(
    BasicPitchDebugPostRunImportActionReport &report,
    const QString &stage)
{
    const QString trimmedStage = stage.trimmed();
    if (!trimmedStage.isEmpty() &&
        !report.stageStates.contains(trimmedStage)) {
        report.stageStates << trimmedStage;
    }
}

QString
BasicPitchDebugPostRunImportAction::resolveResultJsonPath(
    const BasicPitchDebugPostRunImportActionOptions &options,
    const BasicPitchDebugManualRunStatusResult &status)
{
    if (!options.resultJsonPath.trimmed().isEmpty()) {
        return options.resultJsonPath.trimmed();
    }

    if (!options.deriveResultJsonPathFromManualConfig) {
        return QString();
    }

    if (status.resultJsonConfigured &&
        !status.resultJsonPath.trimmed().isEmpty()) {
        return status.resultJsonPath.trimmed();
    }

    if (status.resultJsonWillBeDerived &&
        !status.derivedResultJsonPath.trimmed().isEmpty()) {
        return status.derivedResultJsonPath.trimmed();
    }

    return QString();
}

}
}
