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

#include "BasicPitchDebugCombinedRunImportAction.h"

namespace Tony {
namespace Backend {

namespace {

QString
boolString(bool value)
{
    return value ? QString("true") : QString("false");
}

QString
trimmed(const QString &value)
{
    return value.trimmed();
}

void
addWarning(ValidationReport &report,
           const QString &code,
           const QString &message)
{
    report.addIssue(ValidationSeverity::Warning, code, message);
}

}

bool
BasicPitchDebugCombinedRunImportActionReport::isValid() const
{
    if (wasSkipped()) {
        return report.isValid() &&
            manualRunReport.isValid() &&
            !manualRunAttempted &&
            !ranBasicPitch &&
            !resultJsonWritten &&
            !loadedResult &&
            !importAttempted &&
            !importedIntoTonyLayers &&
            !insertedIntoView &&
            !productionTranscription &&
            testOnlyDebugOnly &&
            !readyInstalledCompletedMutation;
    }

    return report.isValid() &&
        manualRunReport.isValid() &&
        importReport.isValid() &&
        manualRunAllowed &&
        manualRunAttempted &&
        ranBasicPitch &&
        resultJsonWritten &&
        loadedResult &&
        importAttempted &&
        importedIntoTonyLayers &&
        (!viewInsertionRequested || insertedIntoView) &&
        (!postImportProofReport.resultJsonPath.trimmed().isEmpty() ?
             postImportProofReport.isValid() : true) &&
        !productionTranscription &&
        testOnlyDebugOnly &&
        !readyInstalledCompletedMutation;
}

bool
BasicPitchDebugCombinedRunImportActionReport::wasSkipped() const
{
    return manualRunReport.wasSkipped() &&
        !manualRunAttempted &&
        !importAttempted;
}

QString
BasicPitchDebugCombinedRunImportActionReport::debugSummaryString() const
{
    return QString("basic_pitch_debug_combined_run_import stages=%1 "
                   "manual_allowed=%2 attempted=%3 ran=%4 result_json=%5 "
                   "loaded=%6 import_attempted=%7 notes=%8 polyphony=%9 "
                   "bends_deferred=%10 imported=%11 inserted=%12 "
                   "edit=%13 undo_redo=%14 save_load=%15 export=%16 "
                   "exported=%17 production=%18 debug=%19 "
                   "ready_mutation=%20 valid=%21")
        .arg(stageStates.join(","))
        .arg(boolString(manualRunAllowed))
        .arg(boolString(manualRunAttempted))
        .arg(boolString(ranBasicPitch))
        .arg(boolString(resultJsonWritten))
        .arg(boolString(loadedResult))
        .arg(boolString(importAttempted))
        .arg(noteCount)
        .arg(boolString(possiblePolyphony))
        .arg(boolString(pitchBendMappingDeferred))
        .arg(boolString(importedIntoTonyLayers))
        .arg(boolString(insertedIntoView))
        .arg(boolString(editProofPassed))
        .arg(boolString(undoRedoProofPassed))
        .arg(boolString(saveLoadProofPassed))
        .arg(boolString(exportProofPassed))
        .arg(exportedNoteCount)
        .arg(boolString(productionTranscription))
        .arg(boolString(testOnlyDebugOnly))
        .arg(boolString(readyInstalledCompletedMutation))
        .arg(boolString(isValid()));
}

BasicPitchDebugCombinedRunImportActionReport
BasicPitchDebugCombinedRunImportAction::runAndImport(
    const BasicPitchDebugCombinedRunImportActionOptions &options) const
{
    BasicPitchDebugCombinedRunImportActionReport result;
    result.productionTranscription = false;
    result.testOnlyDebugOnly = true;
    result.readyInstalledCompletedMutation = false;
    result.documentProvided = options.document != nullptr;
    result.viewProvided = options.view != nullptr;
    result.viewInsertionRequested =
        options.insertLayerIntoView && options.view != nullptr;

    BasicPitchDebugManualRunAction manualRunAction;
    result.manualRunReport = manualRunAction.run(options.manualRunConfig);
    appendIssues(result.report, result.manualRunReport.report);

    result.manualRunAllowed =
        result.manualRunReport.preflightStatus.manualRunAllowed;
    result.manualRunAttempted = result.manualRunReport.manualRunAttempted;
    result.ranBasicPitch = result.manualRunReport.ranBasicPitch;
    result.resultJsonWritten = result.manualRunReport.resultJsonWritten;
    result.loadedResult = result.manualRunReport.loadedResult;
    result.commandUsed = result.manualRunReport.commandUsed;
    result.inputAudioPath = result.manualRunReport.inputAudioPath;
    result.outputDirectoryPath = result.manualRunReport.outputDirectoryPath;
    result.resultJsonPath = result.manualRunReport.resultJsonPath;
    result.selectedNoteEventsArtifactPath =
        result.manualRunReport.selectedNoteEventsArtifactPath;
    result.discoveredArtifacts = result.manualRunReport.discoveredArtifacts;
    result.noteCount = result.manualRunReport.noteCount;
    result.possiblePolyphony = result.manualRunReport.possiblePolyphony;
    result.pitchBendMappingDeferred =
        result.manualRunReport.pitchBendMappingDeferred;
    result.productionTranscription =
        result.manualRunReport.productionTranscription;
    result.importedIntoTonyLayers =
        result.manualRunReport.importedIntoTonyLayers;
    result.readyInstalledCompletedMutation =
        result.manualRunReport.readyInstalledCompletedMutation;

    appendUnique(result.stageStates, result.manualRunReport.stageStates);
    appendUnique(result.warningCodes, result.manualRunReport.warningCodes);
    appendUnique(result.errorCodes, result.manualRunReport.errorCodes);

    if (result.manualRunReport.wasSkipped()) {
        addStage(result, "combined_manual_run_skipped");
        addStage(result, "skipped");
        addWarning(
            result.report,
            "basic_pitch_debug_combined_run_import_skipped",
            "Basic Pitch combined debug run/import was skipped before "
            "execution because required manual-run configuration was missing.");
        collectIssueCodes(result.report,
                          result.warningCodes,
                          result.errorCodes);
        return result;
    }

    if (!result.manualRunReport.isValid()) {
        addStage(result, "combined_manual_run_failed");
        collectIssueCodes(result.report,
                          result.warningCodes,
                          result.errorCodes);
        return result;
    }

    if (!result.resultJsonWritten ||
        !result.loadedResult ||
        trimmed(result.resultJsonPath).isEmpty()) {
        addStage(result, "combined_result_json_unavailable");
        result.report.addError(
            "basic_pitch_debug_combined_missing_loaded_result_json",
            "Basic Pitch combined debug run/import cannot import because "
            "the manual handoff did not produce a loaded result.json.");
        collectIssueCodes(result.report,
                          result.warningCodes,
                          result.errorCodes);
        return result;
    }

    addStage(result, "combined_manual_handoff_loaded");

    BasicPitchDebugPostRunImportActionOptions importOptions;
    importOptions.resultJsonPath = result.resultJsonPath;
    importOptions.manualRunConfig = options.manualRunConfig;
    importOptions.deriveResultJsonPathFromManualConfig = false;
    importOptions.sampleRate = options.sampleRate;
    importOptions.resolution = options.resolution;
    importOptions.document = options.document;
    importOptions.view = options.view;
    importOptions.insertLayerIntoView = options.insertLayerIntoView;
    importOptions.requireViewForImport = options.requireViewForImport;

    BasicPitchDebugPostRunImportAction importAction;
    result.importAttempted = true;
    addStage(result, "combined_import_attempted");
    result.importReport = importAction.importResult(importOptions);
    appendIssues(result.report, result.importReport.report);

    result.loadedResult = result.importReport.loadedResult;
    result.noteCount = result.importReport.noteCount;
    result.possiblePolyphony = result.importReport.possiblePolyphony;
    result.pitchBendMappingDeferred =
        result.importReport.pitchBendMappingDeferred;
    result.importedIntoTonyLayers =
        result.importReport.importedIntoTonyLayers;
    result.insertedIntoView = result.importReport.insertedIntoView;
    result.productionTranscription =
        result.importReport.productionTranscription;
    result.readyInstalledCompletedMutation =
        result.importReport.readyInstalledCompletedMutation;
    result.editProofTested = result.importReport.editProofTested;
    result.editProofPassed = result.importReport.editProofPassed;
    result.saveLoadProofTested = result.importReport.saveLoadProofTested;
    result.saveLoadProofPassed = result.importReport.saveLoadProofPassed;
    result.exportProofTested = result.importReport.exportProofTested;
    result.exportProofPassed = result.importReport.exportProofPassed;

    appendUnique(result.stageStates, result.importReport.stageStates);
    appendUnique(result.warningCodes, result.importReport.warningCodes);
    appendUnique(result.errorCodes, result.importReport.errorCodes);

    if (result.importedIntoTonyLayers) {
        addStage(result, "combined_imported_into_real_layer");
    }
    if (result.insertedIntoView) {
        addStage(result, "combined_inserted_into_view");
    }
    if (!result.importReport.isValid()) {
        addStage(result, "combined_import_failed");
    }

    if (result.importReport.isValid() && options.runPostImportProof) {
        BasicPitchDebugPostImportProofActionOptions proofOptions;
        proofOptions.resultJsonPath = result.resultJsonPath;
        proofOptions.exportCsvPath = options.postImportProofExportCsvPath;
        proofOptions.sampleRate = options.sampleRate;
        proofOptions.resolution = options.resolution;

        BasicPitchDebugPostImportProofAction proofAction;
        result.postImportProofReport = proofAction.prove(proofOptions);
        appendIssues(result.report, result.postImportProofReport.report);
        appendUnique(result.warningCodes,
                     result.postImportProofReport.warningCodes);
        appendUnique(result.errorCodes,
                     result.postImportProofReport.errorCodes);

        result.editProofTested =
            result.postImportProofReport.editProofTested;
        result.editProofPassed =
            result.postImportProofReport.editProofPassed;
        result.undoRedoProofTested =
            result.postImportProofReport.undoRedoProofTested;
        result.undoRedoProofPassed =
            result.postImportProofReport.undoRedoProofPassed;
        result.saveLoadProofTested =
            result.postImportProofReport.saveLoadProofTested;
        result.saveLoadProofPassed =
            result.postImportProofReport.saveLoadProofPassed;
        result.exportProofTested =
            result.postImportProofReport.exportProofTested;
        result.exportProofPassed =
            result.postImportProofReport.exportProofPassed;
        result.exportedNoteCount =
            result.postImportProofReport.exportedNoteCount;
        result.possiblePolyphony =
            result.possiblePolyphony ||
            result.postImportProofReport.possiblePolyphony;
        result.pitchBendMappingDeferred =
            result.pitchBendMappingDeferred ||
            result.postImportProofReport.pitchBendMappingDeferred;
        result.productionTranscription =
            result.productionTranscription ||
            result.postImportProofReport.productionTranscription;
        result.readyInstalledCompletedMutation =
            result.readyInstalledCompletedMutation ||
            result.postImportProofReport.readyInstalledCompletedMutation;

        if (result.postImportProofReport.isValid()) {
            addStage(result, "combined_post_import_proof_passed");
        } else {
            addStage(result, "combined_post_import_proof_failed");
        }
    }

    result.report.addIssue(
        ValidationSeverity::Warning,
        "basic_pitch_debug_combined_run_import_test_only",
        "Basic Pitch combined run/import is debug/test-only and is not "
        "production Basic Pitch UI integration.");

    collectIssueCodes(result.report,
                      result.warningCodes,
                      result.errorCodes);
    result.warningCodes.removeDuplicates();
    result.errorCodes.removeDuplicates();
    return result;
}

void
BasicPitchDebugCombinedRunImportAction::appendIssues(
    ValidationReport &target,
    const ValidationReport &source)
{
    for (const ValidationIssue &issue: source.issues) {
        target.addIssue(issue.severity, issue.code, issue.message);
    }
}

void
BasicPitchDebugCombinedRunImportAction::collectIssueCodes(
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
BasicPitchDebugCombinedRunImportAction::appendUnique(
    QStringList &target,
    const QStringList &values)
{
    for (const QString &value: values) {
        const QString trimmedValue = value.trimmed();
        if (!trimmedValue.isEmpty() &&
            !target.contains(trimmedValue)) {
            target << trimmedValue;
        }
    }
}

void
BasicPitchDebugCombinedRunImportAction::addStage(
    BasicPitchDebugCombinedRunImportActionReport &report,
    const QString &stage)
{
    const QString trimmedStage = stage.trimmed();
    if (!trimmedStage.isEmpty() &&
        !report.stageStates.contains(trimmedStage)) {
        report.stageStates << trimmedStage;
    }
}

}
}
