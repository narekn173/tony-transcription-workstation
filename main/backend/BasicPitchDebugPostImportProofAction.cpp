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

#include "BasicPitchDebugPostImportProofAction.h"

#include <QDir>
#include <QFileInfo>

namespace Tony {
namespace Backend {

namespace {

QString
boolString(bool value)
{
    return value ? QString("true") : QString("false");
}

}

bool
BasicPitchDebugPostImportProofActionReport::isValid() const
{
    return report.isValid() &&
        proofResult.isValid() &&
        loadedResult &&
        realNoteModelExists &&
        realNoteLayerExists &&
        documentOwnedLayer &&
        insertedIntoView &&
        layerEditable &&
        editProofTested &&
        editProofPassed &&
        undoRedoProofTested &&
        undoRedoProofPassed &&
        saveLoadProofTested &&
        saveLoadProofPassed &&
        exportProofTested &&
        exportProofPassed &&
        exportedNoteCount > 0 &&
        !productionTranscription &&
        testOnlyDebugOnly &&
        !readyInstalledCompletedMutation;
}

QString
BasicPitchDebugPostImportProofActionReport::debugSummaryString() const
{
    return QString("basic_pitch_debug_post_import_proof result=%1 "
                   "loaded=%2 notes=%3 model=%4 layer=%5 document=%6 "
                   "inserted=%7 editable=%8 edit=%9 undo_redo=%10 "
                   "save_load=%11 export=%12 exported=%13 production=%14 "
                   "debug=%15 ready_mutation=%16 valid=%17")
        .arg(resultJsonPath)
        .arg(boolString(loadedResult))
        .arg(noteCount)
        .arg(boolString(realNoteModelExists))
        .arg(boolString(realNoteLayerExists))
        .arg(boolString(documentOwnedLayer))
        .arg(boolString(insertedIntoView))
        .arg(boolString(layerEditable))
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

BasicPitchDebugPostImportProofActionReport
BasicPitchDebugPostImportProofAction::prove(
    const BasicPitchDebugPostImportProofActionOptions &options) const
{
    BasicPitchDebugPostImportProofActionReport result;
    result.resultJsonPath = options.resultJsonPath.trimmed();
    result.exportCsvPath =
        resolveExportCsvPath(result.resultJsonPath, options.exportCsvPath);
    result.productionTranscription = false;
    result.testOnlyDebugOnly = true;
    result.readyInstalledCompletedMutation = false;

    if (result.resultJsonPath.isEmpty()) {
        result.report.addError(
            "missing_basic_pitch_debug_post_import_result_json_path",
            "Basic Pitch post-import proof requires a result.json path.");
        collectIssueCodes(result.report,
                          result.warningCodes,
                          result.errorCodes);
        return result;
    }

    if (result.exportCsvPath.isEmpty()) {
        result.report.addError(
            "missing_basic_pitch_debug_post_import_export_csv_path",
            "Basic Pitch post-import proof requires or must derive a CSV "
            "export path.");
        collectIssueCodes(result.report,
                          result.warningCodes,
                          result.errorCodes);
        return result;
    }

    BasicPitchLayerPersistenceExportProofOptions proofOptions;
    proofOptions.resultJsonPath = result.resultJsonPath;
    proofOptions.exportCsvPath = result.exportCsvPath;
    proofOptions.sampleRate = options.sampleRate;
    proofOptions.resolution = options.resolution;

    BasicPitchLayerPersistenceExportProof proof;
    result.proofResult = proof.prove(proofOptions);
    appendIssues(result.report, result.proofResult.report);

    result.loadedResult = result.proofResult.loadedResult;
    result.noteCount = result.proofResult.importedNoteCount;
    result.realNoteModelExists = result.proofResult.realNoteModelExists;
    result.realNoteLayerExists = result.proofResult.realNoteLayerExists;
    result.documentOwnedLayer = result.proofResult.documentOwnedLayer;
    result.insertedIntoView = result.proofResult.insertedIntoView;
    result.layerEditable = result.proofResult.importedLayerEditable &&
        result.proofResult.reloadedLayerEditable;
    result.editProofTested =
        result.proofResult.editProofResult.report.isValid() ||
        result.proofResult.editProofProven;
    result.editProofPassed = result.proofResult.editProofProven;
    result.undoRedoProofTested = result.editProofTested;
    result.undoRedoProofPassed = result.proofResult.undoRedoProofProven;
    result.saveLoadProofTested = true;
    result.saveLoadProofPassed = result.proofResult.saveLoadProven;
    result.exportProofTested = !result.exportCsvPath.trimmed().isEmpty();
    result.exportProofPassed = result.proofResult.exportProven;
    result.exportedNoteCount = result.proofResult.exportedNoteCount;
    result.exportedCsvNonEmpty = result.proofResult.exportedCsvNonEmpty;
    result.exportedTimingDurationPitchVelocity =
        result.proofResult.exportedTimingDurationPitchVelocity;
    result.possiblePolyphony = result.proofResult.possiblePolyphony;
    result.pitchBendMappingDeferred =
        result.proofResult.pitchBendMappingDeferred;
    result.productionTranscription =
        result.proofResult.productionTranscription;
    result.readyInstalledCompletedMutation =
        result.proofResult.readyInstalledCompletedMutation;

    if (result.possiblePolyphony) {
        result.warningCodes << "possible_polyphony";
    }
    if (result.pitchBendMappingDeferred) {
        result.warningCodes << "pitch_bend_mapping_deferred";
    }

    result.report.addIssue(
        ValidationSeverity::Warning,
        "basic_pitch_debug_post_import_proof_test_only",
        "Basic Pitch post-import edit/save/load/export proof is "
        "debug/test-only and is not production Basic Pitch UI integration.");

    collectIssueCodes(result.report,
                      result.warningCodes,
                      result.errorCodes);
    result.warningCodes.removeDuplicates();
    result.errorCodes.removeDuplicates();
    return result;
}

void
BasicPitchDebugPostImportProofAction::appendIssues(
    ValidationReport &target,
    const ValidationReport &source)
{
    for (const ValidationIssue &issue: source.issues) {
        target.addIssue(issue.severity, issue.code, issue.message);
    }
}

void
BasicPitchDebugPostImportProofAction::collectIssueCodes(
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

QString
BasicPitchDebugPostImportProofAction::resolveExportCsvPath(
    const QString &resultJsonPath,
    const QString &exportCsvPath)
{
    if (!exportCsvPath.trimmed().isEmpty()) {
        return exportCsvPath.trimmed();
    }

    const QFileInfo info(resultJsonPath.trimmed());
    if (info.absolutePath().trimmed().isEmpty()) {
        return QString();
    }

    return QDir(info.absolutePath())
        .filePath("basic_pitch_debug_post_import_export.csv");
}

}
}
