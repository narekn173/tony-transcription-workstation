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

#include "BasicPitchProductionReadinessPreflight.h"

namespace Tony {
namespace Backend {

namespace {

QString
boolString(bool value)
{
    return value ? QString("true") : QString("false");
}

BasicPitchProductionReadinessGate
makeGate(const QString &id,
         const QString &displayName,
         BasicPitchProductionReadinessGateStatus status,
         const QString &userMessage,
         const QString &technicalMessage,
         const QString &evidenceReference,
         bool blocksProductionReadiness = true,
         bool debugOnly = false)
{
    BasicPitchProductionReadinessGate gate;
    gate.id = id;
    gate.displayName = displayName;
    gate.status = status;
    gate.userMessage = userMessage;
    gate.technicalMessage = technicalMessage;
    gate.evidenceReference = evidenceReference;
    gate.blocksProductionReadiness = blocksProductionReadiness;
    gate.debugOnly = debugOnly;
    return gate;
}

BasicPitchProductionReadinessGateStatus
productionOrDebugStatus(bool productionPassed, bool debugEvidence)
{
    if (productionPassed) {
        return BasicPitchProductionReadinessGateStatus::Passed;
    }
    if (debugEvidence) {
        return BasicPitchProductionReadinessGateStatus::DebugOnly;
    }
    return BasicPitchProductionReadinessGateStatus::Blocked;
}

bool
debugStatus(BasicPitchProductionReadinessGateStatus status)
{
    return status == BasicPitchProductionReadinessGateStatus::DebugOnly;
}

QString
debugEvidenceMessage(bool debugEvidence, const QString &evidence)
{
    if (debugEvidence) {
        return evidence;
    }
    return QString("No matching debug proof evidence was supplied.");
}

}

QString
basicPitchProductionReadinessGateStatusToString(
    BasicPitchProductionReadinessGateStatus status)
{
    switch (status) {
    case BasicPitchProductionReadinessGateStatus::Passed:
        return "passed";
    case BasicPitchProductionReadinessGateStatus::Failed:
        return "failed";
    case BasicPitchProductionReadinessGateStatus::Warning:
        return "warning";
    case BasicPitchProductionReadinessGateStatus::NotTested:
        return "not_tested";
    case BasicPitchProductionReadinessGateStatus::Blocked:
        return "blocked";
    case BasicPitchProductionReadinessGateStatus::DebugOnly:
        return "debug_only";
    }

    return "unknown";
}

bool
BasicPitchProductionReadinessGate::blocksProduction() const
{
    return blocksProductionReadiness &&
        status != BasicPitchProductionReadinessGateStatus::Passed;
}

QString
BasicPitchProductionReadinessGate::statusString() const
{
    return basicPitchProductionReadinessGateStatusToString(status);
}

bool
BasicPitchProductionReadinessReport::isValid() const
{
    return report.isValid() &&
        !productionTranscription &&
        !readyInstalledCompletedMutation &&
        (!productionReady || missingProductionBlockers.isEmpty()) &&
        (!productionReady || !testOnlyDebugOnly);
}

const BasicPitchProductionReadinessGate *
BasicPitchProductionReadinessReport::gateById(const QString &id) const
{
    const QString trimmedId = id.trimmed();
    for (const BasicPitchProductionReadinessGate &gate: gates) {
        if (gate.id == trimmedId) {
            return &gate;
        }
    }

    return nullptr;
}

bool
BasicPitchProductionReadinessReport::hasGateStatus(
    const QString &id,
    BasicPitchProductionReadinessGateStatus status) const
{
    const BasicPitchProductionReadinessGate *gate = gateById(id);
    return gate != nullptr && gate->status == status;
}

QString
BasicPitchProductionReadinessReport::debugSummaryString() const
{
    QStringList gateSummaries;
    for (const BasicPitchProductionReadinessGate &gate: gates) {
        gateSummaries << QString("%1:%2").arg(gate.id, gate.statusString());
    }

    return QString("basic_pitch_production_readiness ready=%1 "
                   "production=%2 debug=%3 workflow=%4 manual=%5 "
                   "import_proof=%6 post_import_proof=%7 "
                   "ready_mutation=%8 blockers=%9 gates=%10 valid=%11")
        .arg(boolString(productionReady))
        .arg(boolString(productionTranscription))
        .arg(boolString(testOnlyDebugOnly))
        .arg(boolString(debugWorkflowAvailable))
        .arg(boolString(manualRunAvailable))
        .arg(boolString(importProofAvailable))
        .arg(boolString(postImportProofAvailable))
        .arg(boolString(readyInstalledCompletedMutation))
        .arg(missingProductionBlockers.join(","))
        .arg(gateSummaries.join(","))
        .arg(boolString(isValid()));
}

BasicPitchProductionReadinessReport
BasicPitchProductionReadinessPreflight::evaluate(
    const BasicPitchProductionReadinessPreflightInput &input) const
{
    BasicPitchProductionReadinessReport result;
    result.productionTranscription = false;
    result.testOnlyDebugOnly = input.preflightBoundaryDebugOnly;
    result.debugWorkflowAvailable = input.debugWorkflowAvailable;
    result.readyInstalledCompletedMutation = false;

    const BasicPitchDebugManualRunStatusResult manualStatus =
        BasicPitchDebugManualRunStatus().fromConfig(input.manualRunConfig);
    result.manualRunAvailable = manualStatus.manualRunAllowed;
    addIssueCodes(manualStatus.report, result.warnings, result.errors);

    const bool combinedImportProof =
        input.hasCombinedRunImportReport &&
        input.combinedRunImportReport.importedIntoTonyLayers;
    const bool combinedVisibilityProof =
        input.hasCombinedRunImportReport &&
        input.combinedRunImportReport.insertedIntoView;
    const bool combinedLoadedProof =
        input.hasCombinedRunImportReport &&
        input.combinedRunImportReport.loadedResult;
    const bool combinedEditProof =
        input.hasCombinedRunImportReport &&
        input.combinedRunImportReport.editProofPassed;
    const bool combinedUndoRedoProof =
        input.hasCombinedRunImportReport &&
        input.combinedRunImportReport.undoRedoProofPassed;
    const bool combinedSaveLoadProof =
        input.hasCombinedRunImportReport &&
        input.combinedRunImportReport.saveLoadProofPassed;
    const bool combinedExportProof =
        input.hasCombinedRunImportReport &&
        input.combinedRunImportReport.exportProofPassed;

    const bool postImportProof =
        input.hasPostImportProofReport &&
        input.postImportProofReport.realNoteLayerExists &&
        input.postImportProofReport.documentOwnedLayer;
    const bool postImportVisibilityProof =
        input.hasPostImportProofReport &&
        input.postImportProofReport.insertedIntoView;
    const bool postImportLoadedProof =
        input.hasPostImportProofReport &&
        input.postImportProofReport.loadedResult;
    const bool postImportEditProof =
        input.hasPostImportProofReport &&
        input.postImportProofReport.editProofPassed;
    const bool postImportUndoRedoProof =
        input.hasPostImportProofReport &&
        input.postImportProofReport.undoRedoProofPassed;
    const bool postImportSaveLoadProof =
        input.hasPostImportProofReport &&
        input.postImportProofReport.saveLoadProofPassed;
    const bool postImportExportProof =
        input.hasPostImportProofReport &&
        input.postImportProofReport.exportProofPassed &&
        input.postImportProofReport.exportedNoteCount > 0;

    result.importProofAvailable = combinedImportProof || postImportProof;
    result.postImportProofAvailable =
        postImportEditProof ||
        postImportUndoRedoProof ||
        postImportSaveLoadProof ||
        postImportExportProof;

    if (input.hasCombinedRunImportReport) {
        addIssueCodes(input.combinedRunImportReport.report,
                      result.warnings,
                      result.errors);
    }
    if (input.hasPostImportProofReport) {
        addIssueCodes(input.postImportProofReport.report,
                      result.warnings,
                      result.errors);
    }

    addGate(result, makeGate(
        "configuration",
        "Configuration",
        input.productionConfigurationAvailable ?
            BasicPitchProductionReadinessGateStatus::Passed :
            BasicPitchProductionReadinessGateStatus::Blocked,
        input.productionConfigurationAvailable ?
            "Basic Pitch production configuration is available." :
            "Basic Pitch still has no production configuration UI or "
            "persistent configuration boundary.",
        "The current env/manual status can describe debug configuration, "
        "but production setup is not implemented.",
        "CODEX-114 configuration gate"));

    const bool debugCommand = manualStatus.commandConfigured;
    BasicPitchProductionReadinessGateStatus commandStatus =
        productionOrDebugStatus(input.productionCommandRuntimeValidated,
                                debugCommand);
    addGate(result, makeGate(
        "command_runtime",
        "Command/runtime",
        commandStatus,
        input.productionCommandRuntimeValidated ?
            "Basic Pitch command/runtime has production validation." :
            "Basic Pitch command/runtime is not production-validated.",
        debugEvidenceMessage(
            debugCommand,
            "Debug manual-run status has a configured command path, "
            "but that remains debug-only evidence."),
        "BasicPitchDebugManualRunStatus",
        true,
        debugStatus(commandStatus)));

    const bool debugAudio = manualStatus.inputAudioConfigured;
    BasicPitchProductionReadinessGateStatus audioStatus =
        productionOrDebugStatus(input.productionAudioInputValidated,
                                debugAudio);
    addGate(result, makeGate(
        "audio_input",
        "Audio input",
        audioStatus,
        input.productionAudioInputValidated ?
            "Production audio input selection is validated." :
            "No production user-facing audio input selection workflow "
            "is proven.",
        debugEvidenceMessage(
            debugAudio,
            "Debug/manual env status has an audio path, but no "
            "production audio chooser or selected-region workflow exists."),
        "CODEX-114 audio input gate",
        true,
        debugStatus(audioStatus)));

    const bool debugOutput = manualStatus.outputDirectoryConfigured;
    BasicPitchProductionReadinessGateStatus outputStatus =
        productionOrDebugStatus(input.productionOutputDirectoryValidated,
                                debugOutput);
    addGate(result, makeGate(
        "output_directory",
        "Output directory",
        outputStatus,
        input.productionOutputDirectoryValidated ?
            "Production output workspace selection is validated." :
            "No production output workspace selection workflow is proven.",
        debugEvidenceMessage(
            debugOutput,
            "Debug/manual env status has an output directory, but "
            "production workspace handling is not implemented."),
        "CODEX-114 output workspace gate",
        true,
        debugStatus(outputStatus)));

    BasicPitchProductionReadinessGateStatus runStatus =
        productionOrDebugStatus(input.productionRunPermissionValidated,
                                manualStatus.manualRunAllowed);
    addGate(result, makeGate(
        "manual_run_permission",
        "Run permission",
        runStatus,
        input.productionRunPermissionValidated ?
            "Production run permission/preflight is validated." :
            "Only debug/manual opt-in can allow a Basic Pitch run today.",
        debugEvidenceMessage(
            manualStatus.manualRunAllowed,
            "Manual run is allowed only by explicit debug env/config; "
            "this is not production run permission."),
        "BasicPitchDebugManualRunStatus",
        true,
        debugStatus(runStatus)));

    const bool debugArtifacts =
        input.hasCombinedRunImportReport &&
        !input.combinedRunImportReport.discoveredArtifacts.isEmpty();
    BasicPitchProductionReadinessGateStatus artifactStatus =
        productionOrDebugStatus(input.productionArtifactDiscoveryValidated,
                                debugArtifacts);
    addGate(result, makeGate(
        "artifact_discovery",
        "Artifact discovery",
        artifactStatus,
        input.productionArtifactDiscoveryValidated ?
            "Production artifact discovery is validated." :
            "Artifact discovery is currently proven only in debug/manual "
            "boundaries.",
        debugEvidenceMessage(
            debugArtifacts,
            "Debug combined run/import reported discovered artifacts."),
        "BasicPitchArtifactDiscovery",
        true,
        debugStatus(artifactStatus)));

    const bool debugLoaded = combinedLoadedProof || postImportLoadedProof;
    BasicPitchProductionReadinessGateStatus resultStatus =
        productionOrDebugStatus(input.productionResultValidationValidated,
                                debugLoaded);
    addGate(result, makeGate(
        "result_validation",
        "Result validation",
        resultStatus,
        input.productionResultValidationValidated ?
            "Production result validation is proven." :
            "Loaded debug results do not by themselves approve production "
            "result validation.",
        debugEvidenceMessage(
            debugLoaded,
            "Debug proof loaded a result.json through the existing loader."),
        "BackendRunResultLoader / UnifiedResultFileLoader",
        true,
        debugStatus(resultStatus)));

    const bool debugLayerMapping = result.importProofAvailable;
    BasicPitchProductionReadinessGateStatus mappingStatus =
        productionOrDebugStatus(input.productionLayerMappingValidated,
                                debugLayerMapping);
    addGate(result, makeGate(
        "layer_mapping",
        "Layer mapping",
        mappingStatus,
        input.productionLayerMappingValidated ?
            "Production layer mapping policy is validated." :
            "Basic Pitch notes can map to a debug-proven NoteLayer, "
            "but production pitch-bend/polyphony mapping remains unresolved.",
        debugEvidenceMessage(
            debugLayerMapping,
            "Debug proof imported Basic Pitch-shaped notes into a real "
            "Tony/SV NoteLayer."),
        "TonyLayerImporter / BasicPitchResultToTonyLayerProof",
        true,
        debugStatus(mappingStatus)));

    BasicPitchProductionReadinessGateStatus importStatus =
        productionOrDebugStatus(input.productionLayerImportValidated,
                                result.importProofAvailable);
    addGate(result, makeGate(
        "layer_import",
        "Layer import",
        importStatus,
        input.productionLayerImportValidated ?
            "Production layer import is validated." :
            "Layer import evidence is still debug/test-only.",
        debugEvidenceMessage(
            result.importProofAvailable,
            "Debug proof created a real Document-owned NoteLayer."),
        "BasicPitchDebugPostRunImportAction",
        true,
        debugStatus(importStatus)));

    const bool debugVisible =
        combinedVisibilityProof || postImportVisibilityProof;
    BasicPitchProductionReadinessGateStatus visibilityStatus =
        productionOrDebugStatus(input.productionVisibilityValidated,
                                debugVisible);
    addGate(result, makeGate(
        "visibility",
        "View visibility",
        visibilityStatus,
        input.productionVisibilityValidated ?
            "Production Pane/View insertion is validated." :
            "Visible Basic Pitch state is still debug/test-only evidence.",
        debugEvidenceMessage(
            debugVisible,
            "Debug proof inserted a real layer into a real Pane/View."),
        "Document::addLayerToView",
        true,
        debugStatus(visibilityStatus)));

    const bool debugEdit = combinedEditProof || postImportEditProof;
    BasicPitchProductionReadinessGateStatus editStatus =
        productionOrDebugStatus(input.productionEditValidated, debugEdit);
    addGate(result, makeGate(
        "edit",
        "Edit",
        editStatus,
        input.productionEditValidated ?
            "Production edit behavior is validated." :
            "Edit proof is available only through debug/test proof paths.",
        debugEvidenceMessage(
            debugEdit,
            "Debug proof exercised the real editable NoteLayer/NoteModel "
            "path."),
        "TonyLayerImporter CommandHistory edit proof",
        true,
        debugStatus(editStatus)));

    const bool debugUndoRedo =
        combinedUndoRedoProof || postImportUndoRedoProof;
    BasicPitchProductionReadinessGateStatus undoStatus =
        productionOrDebugStatus(input.productionUndoRedoValidated,
                                debugUndoRedo);
    addGate(result, makeGate(
        "undo_redo",
        "Undo/redo",
        undoStatus,
        input.productionUndoRedoValidated ?
            "Production undo/redo behavior is validated." :
            "Undo/redo proof is available only through debug/test proof "
            "paths.",
        debugEvidenceMessage(
            debugUndoRedo,
            "Debug proof exercised CommandHistory-safe undo/redo."),
        "CommandHistory / ChangeEventsCommand",
        true,
        debugStatus(undoStatus)));

    const bool debugSaveLoad =
        combinedSaveLoadProof || postImportSaveLoadProof;
    BasicPitchProductionReadinessGateStatus saveLoadStatus =
        productionOrDebugStatus(input.productionSaveLoadValidated,
                                debugSaveLoad);
    addGate(result, makeGate(
        "save_load",
        "Save/load",
        saveLoadStatus,
        input.productionSaveLoadValidated ?
            "Production save/load behavior is validated." :
            "Save/load proof is currently debug/test-only.",
        debugEvidenceMessage(
            debugSaveLoad,
            "Debug proof survived Document::toXml, Pane::toXml, and "
            "SVFileReader::parseXml."),
        "BasicPitchLayerPersistenceExportProof",
        true,
        debugStatus(saveLoadStatus)));

    const bool debugExport = combinedExportProof || postImportExportProof;
    BasicPitchProductionReadinessGateStatus exportStatus =
        productionOrDebugStatus(input.productionExportValidated,
                                debugExport);
    addGate(result, makeGate(
        "export",
        "Export",
        exportStatus,
        input.productionExportValidated ?
            "Production export behavior is validated for claimed formats." :
            "CSV export proof is debug/test-only and does not prove "
            "other export formats.",
        debugEvidenceMessage(
            debugExport,
            "Debug proof produced and counted real CSV rows."),
        "getExportModel(...) / CSVFileWriter",
        true,
        debugStatus(exportStatus)));

    BasicPitchProductionReadinessGateStatus warningStatus =
        productionOrDebugStatus(input.productionWarningVisibilityValidated,
                                input.debugWarningVisibilityAvailable);
    addGate(result, makeGate(
        "warning_visibility",
        "Warning visibility",
        warningStatus,
        input.productionWarningVisibilityValidated ?
            "Production warning visibility is validated." :
            "Warning visibility is currently proven only in debug reports.",
        debugEvidenceMessage(
            input.debugWarningVisibilityAvailable,
            "Debug reports preserve possible_polyphony and "
            "pitch_bend_mapping_deferred warnings."),
        "Basic Pitch debug report formatters",
        true,
        debugStatus(warningStatus)));

    const bool progressAndCancel =
        input.productionProgressReportingValidated &&
        input.productionCancellationValidated;
    addGate(result, makeGate(
        "progress_cancel",
        "Progress/cancel",
        progressAndCancel ?
            BasicPitchProductionReadinessGateStatus::Passed :
            BasicPitchProductionReadinessGateStatus::Blocked,
        progressAndCancel ?
            "Production progress and cancellation are validated." :
            "Production progress/cancel is not validated; fake percent "
            "progress and fake cancellation remain forbidden.",
        "CODEX-114 progress/cancel gate requires stage-based progress "
        "until real backend progress exists.",
        "UI_VISUAL_TRUTH_STATES / PROOF_BUNDLE_POLICY"));

    addGate(result, makeGate(
        "error_recovery",
        "Error recovery",
        input.productionErrorRecoveryValidated ?
            BasicPitchProductionReadinessGateStatus::Passed :
            BasicPitchProductionReadinessGateStatus::Blocked,
        input.productionErrorRecoveryValidated ?
            "Production error recovery is validated." :
            "Production error recovery for missing config, backend "
            "failure, invalid output, no active pane, import failure, "
            "and export failure is not complete.",
        "Debug actions handle several failure states, but production "
        "recovery is not yet implemented.",
        "CODEX-114 error recovery gate"));

    addGate(result, makeGate(
        "documentation",
        "User documentation",
        input.productionUserDocumentationAvailable ?
            BasicPitchProductionReadinessGateStatus::Passed :
            BasicPitchProductionReadinessGateStatus::Blocked,
        input.productionUserDocumentationAvailable ?
            "Production user documentation is available." :
            "Engineering transition docs exist, but production user "
            "documentation is not available.",
        "CODEX-114 added engineering gates, not end-user setup/run docs.",
        "BASIC_PITCH_PRODUCTION_READINESS_CHECKLIST"));

    BasicPitchProductionReadinessGateStatus regressionStatus =
        productionOrDebugStatus(input.productionRegressionProtectionValidated,
                                input.debugRegressionProtectionAvailable);
    addGate(result, makeGate(
        "regression_protection",
        "Regression protection",
        regressionStatus,
        input.productionRegressionProtectionValidated ?
            "Production regression protection is validated." :
            "Current tests protect debug paths, but production workflow "
            "regression protection is not complete.",
        debugEvidenceMessage(
            input.debugRegressionProtectionAvailable,
            "Existing backend-types and suite checks protect debug "
            "boundaries and pYIN/Analyser separation."),
        "CODEX-114 regression protection gate",
        true,
        debugStatus(regressionStatus)));

    addGate(result, makeGate(
        "pitch_bend_policy",
        "Pitch-bend policy",
        input.pitchBendProductionPolicyValidated ?
            BasicPitchProductionReadinessGateStatus::Passed :
            BasicPitchProductionReadinessGateStatus::Blocked,
        input.pitchBendProductionPolicyValidated ?
            "Production pitch-bend mapping policy is validated." :
            "Pitch-bend mapping remains deferred and must stay visible.",
        "Do not silently discard Basic Pitch pitch bends; "
        "pitch_bend_mapping_deferred remains a production blocker.",
        "BASIC_PITCH_PRODUCTION_READINESS_CHECKLIST"));

    addGate(result, makeGate(
        "polyphony_policy",
        "Polyphony policy",
        input.polyphonyProductionPolicyValidated ?
            BasicPitchProductionReadinessGateStatus::Passed :
            BasicPitchProductionReadinessGateStatus::Blocked,
        input.polyphonyProductionPolicyValidated ?
            "Production polyphony policy is validated." :
            "Possible polyphony remains unresolved and must stay visible.",
        "Do not silently collapse Basic Pitch polyphonic output; "
        "possible_polyphony remains a production blocker.",
        "BASIC_PITCH_PRODUCTION_READINESS_CHECKLIST"));

    result.warnings << "basic_pitch_production_readiness_debug_only";
    result.warnings.removeDuplicates();
    result.errors.removeDuplicates();

    result.report.addIssue(
        ValidationSeverity::Warning,
        "basic_pitch_production_readiness_debug_only",
        "Basic Pitch production-readiness preflight is a compile/test "
        "model and does not enable production transcription.");

    result.productionReady =
        !result.testOnlyDebugOnly &&
        !result.productionTranscription &&
        !result.readyInstalledCompletedMutation &&
        result.report.isValid() &&
        result.missingProductionBlockers.isEmpty();

    return result;
}

void
BasicPitchProductionReadinessPreflight::addGate(
    BasicPitchProductionReadinessReport &report,
    const BasicPitchProductionReadinessGate &gate)
{
    report.gates.push_back(gate);
    if (gate.blocksProduction()) {
        report.missingProductionBlockers << gate.id;
    }
}

void
BasicPitchProductionReadinessPreflight::addIssueCodes(
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
}

}
}
