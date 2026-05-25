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

#include "BasicPitchDebugWorkflowUiModel.h"

namespace Tony {
namespace Backend {

namespace {

bool
hasState(const BasicPitchDebugWorkflowReport &report,
         BasicPitchDebugTruthState state)
{
    return report.hasTruthState(state);
}

void
addUnique(QStringList &values, const QString &value)
{
    const QString trimmed = value.trimmed();
    if (!trimmed.isEmpty() && !values.contains(trimmed)) {
        values << trimmed;
    }
}

void
collectIssues(const ValidationReport &report,
              QStringList &warnings,
              QStringList &errors)
{
    for (const ValidationIssue &issue: report.issues) {
        if (issue.severity == ValidationSeverity::Error) {
            addUnique(errors, issue.code);
        } else {
            addUnique(warnings, issue.code);
        }
    }
}

void
collectWorkflowWarnings(const BasicPitchDebugWorkflowReport &workflowReport,
                        QStringList &warnings,
                        QStringList &errors)
{
    collectIssues(workflowReport.report, warnings, errors);
    for (const QString &warning: workflowReport.proofBundle.warningCodes) {
        addUnique(warnings, warning);
    }
    for (const QString &error: workflowReport.proofBundle.errorCodes) {
        addUnique(errors, error);
    }

    if (workflowReport.possiblePolyphony) {
        addUnique(warnings, "possible_polyphony");
    }
    if (workflowReport.pitchBendMappingDeferred) {
        addUnique(warnings, "pitch_bend_mapping_deferred");
    }
    if (!workflowReport.productionTranscription) {
        addUnique(warnings, "production_transcription_false");
    }
}

QString
highestProofState(const BasicPitchDebugWorkflowReport &report)
{
    if (report.exportProofPassed &&
        hasState(report, BasicPitchDebugTruthState::ExportProofPassed)) {
        return "export_proof_passed";
    }
    if (report.saveLoadProofPassed &&
        hasState(report, BasicPitchDebugTruthState::SaveLoadProofPassed)) {
        return "save_load_proof_passed";
    }
    if (report.editProofPassed &&
        hasState(report, BasicPitchDebugTruthState::EditProofPassed)) {
        return "edit_proof_passed";
    }
    if (report.insertedIntoView &&
        hasState(report, BasicPitchDebugTruthState::InsertedIntoView)) {
        return "inserted_into_view";
    }
    if (report.importedIntoTonyLayers &&
        hasState(report, BasicPitchDebugTruthState::ImportedIntoRealLayer)) {
        return "imported_into_real_layer";
    }
    if (report.unifiedResultLoaded &&
        hasState(report, BasicPitchDebugTruthState::UnifiedResultLoaded)) {
        return "unified_result_loaded";
    }
    if (report.resultJsonWritten &&
        hasState(report, BasicPitchDebugTruthState::ResultJsonWritten)) {
        return "result_json_written";
    }
    if (hasState(report, BasicPitchDebugTruthState::ArtifactsDiscovered)) {
        return "artifacts_discovered";
    }
    if (hasState(report, BasicPitchDebugTruthState::PathChecksPassed)) {
        return "path_checks_passed";
    }
    return QString();
}

QString
primaryStateForReport(const BasicPitchDebugWorkflowReport &report)
{
    if (hasState(report, BasicPitchDebugTruthState::Failed)) {
        return "failed";
    }
    if (hasState(report, BasicPitchDebugTruthState::BackendFailed)) {
        return "backend_failed";
    }
    if (hasState(report, BasicPitchDebugTruthState::RunningBackend)) {
        return "running_backend";
    }
    if (hasState(report, BasicPitchDebugTruthState::BackendNotConfigured)) {
        return "backend_not_configured";
    }
    if (hasState(report, BasicPitchDebugTruthState::NoAudio)) {
        return "no_audio";
    }
    if (hasState(report, BasicPitchDebugTruthState::MissingExecutable)) {
        return "missing_executable";
    }
    if (hasState(report, BasicPitchDebugTruthState::MissingModelOrRuntime)) {
        return "missing_model_or_runtime";
    }
    if (hasState(report, BasicPitchDebugTruthState::Skipped)) {
        return "skipped";
    }
    if (hasState(report, BasicPitchDebugTruthState::CompletedWithWarnings)) {
        return "completed_with_warnings";
    }

    const QString highest = highestProofState(report);
    return highest.isEmpty() ? QString("skipped") : highest;
}

QString
secondaryStateForReport(const BasicPitchDebugWorkflowReport &report,
                        const QString &primaryState)
{
    const QString highest = highestProofState(report);
    if (primaryState == "backend_not_configured" &&
        hasState(report, BasicPitchDebugTruthState::Skipped)) {
        return "skipped";
    }
    if (primaryState == "completed_with_warnings" && !highest.isEmpty()) {
        return highest;
    }
    if (primaryState == "unified_result_loaded" &&
        !report.importedIntoTonyLayers) {
        return "loaded_not_visible";
    }
    if (primaryState == "imported_into_real_layer" &&
        !report.insertedIntoView) {
        return "imported_not_visible";
    }
    if (primaryState == "result_json_written" &&
        !report.unifiedResultLoaded) {
        return "result_not_loaded";
    }
    return highest == primaryState ? QString() : highest;
}

QString
userMessageForState(const QString &primaryState,
                    const QString &secondaryState)
{
    if (primaryState == "backend_not_configured") {
        return "Configure Basic Pitch before running this workflow.";
    }
    if (primaryState == "no_audio") {
        return "Choose an audio file before running Basic Pitch.";
    }
    if (primaryState == "missing_executable") {
        return "Basic Pitch executable is missing.";
    }
    if (primaryState == "missing_model_or_runtime") {
        return "Basic Pitch model or runtime dependency is missing.";
    }
    if (primaryState == "skipped") {
        return "Nothing ran; the workflow was skipped.";
    }
    if (primaryState == "running_backend") {
        return "Basic Pitch is running. No progress percentage is available.";
    }
    if (primaryState == "backend_failed" || primaryState == "failed") {
        return "Basic Pitch workflow failed. Review the technical details.";
    }
    if (primaryState == "artifacts_discovered") {
        return "Basic Pitch artifacts were found; result loading is not proven yet.";
    }
    if (primaryState == "result_json_written") {
        return "A result file exists; it is not visible until loaded and imported.";
    }
    if (primaryState == "unified_result_loaded") {
        return "UnifiedResult loaded; it is not visible until a real Tony layer is imported.";
    }
    if (primaryState == "imported_into_real_layer") {
        if (secondaryState == "imported_not_visible") {
            return "A real Tony layer exists, but it is not visible in a View/Pane.";
        }
        return "A real Tony layer exists.";
    }
    if (primaryState == "inserted_into_view") {
        return "The imported layer is visible in a real Tony View/Pane.";
    }
    if (primaryState == "edit_proof_passed") {
        return "The imported layer has passed editable-layer proof.";
    }
    if (primaryState == "save_load_proof_passed") {
        return "The imported layer has passed save/load proof.";
    }
    if (primaryState == "export_proof_passed") {
        return "The imported layer has passed export proof.";
    }
    if (primaryState == "completed_with_warnings") {
        return "Debug workflow completed with warnings that must be shown.";
    }
    return "Basic Pitch debug workflow status is available.";
}

QString
eventStatesSummary(const BasicPitchDebugWorkflowReport &report)
{
    QStringList states;
    for (BasicPitchDebugTruthState state: report.truthStates) {
        states << basicPitchDebugTruthStateToString(state);
    }
    return states.join(",");
}

QString
proofBundleSummary(const BasicPitchDebugWorkflowReport &report)
{
    return QString("mode=%1 skipped=%2 ran=%3 opt_in=%4 command=%5 "
                   "input=%6 output_dir=%7 events=%8 artifacts=%9 "
                   "selected_csv=%10 result_json=%11 notes=%12 "
                   "loaded=%13 imported=%14 visible=%15 edit=%16 "
                   "save_load=%17 export=%18 debug_only=%19")
        .arg(basicPitchDebugWorkflowModeToString(report.mode))
        .arg(report.skippedReason.isEmpty() ? QString("none") :
                                              report.skippedReason)
        .arg(report.ranBasicPitch ? QString("true") : QString("false"))
        .arg(report.realRunResult.config.discoveryConfig.explicitOptIn ?
                 QString("true") : QString("false"))
        .arg(report.proofBundle.commandUsed)
        .arg(report.proofBundle.inputAudioPath)
        .arg(report.proofBundle.outputDirectoryPath)
        .arg(report.proofBundle.events.size())
        .arg(report.proofBundle.discoveredArtifacts.size())
        .arg(report.handoffResult.artifactConversion.sourceArtifactPath)
        .arg(report.proofBundle.resultJsonPath)
        .arg(report.proofBundle.noteCount)
        .arg(report.proofBundle.loadedResult ? QString("true") :
                                               QString("false"))
        .arg(report.proofBundle.importedIntoTonyLayers ? QString("true") :
                                                         QString("false"))
        .arg(report.proofBundle.insertedIntoView ? QString("true") :
                                                   QString("false"))
        .arg(report.proofBundle.editProof ? QString("true") :
                                            QString("false"))
        .arg(report.proofBundle.saveLoadProof ? QString("true") :
                                                QString("false"))
        .arg(report.proofBundle.exportProof ? QString("true") :
                                              QString("false"))
        .arg(report.proofBundle.testOnlyDebugOnly ? QString("true") :
                                                    QString("false"));
}

QString
artifactSummary(const BasicPitchDiscoveredArtifact &artifact)
{
    return QString("%1 type=%2 bytes=%3 path=%4")
        .arg(artifact.fileName)
        .arg(artifact.artifactType)
        .arg(artifact.sizeBytes)
        .arg(artifact.path);
}

QString
firstNoteEventsArtifactPath(const BasicPitchDebugWorkflowReport &report)
{
    if (!report.handoffResult.artifactConversion.sourceArtifactPath
            .trimmed().isEmpty()) {
        return report.handoffResult.artifactConversion.sourceArtifactPath
            .trimmed();
    }
    if (!report.realRunResult.selectedNoteEventsArtifactPath
            .trimmed().isEmpty()) {
        return report.realRunResult.selectedNoteEventsArtifactPath.trimmed();
    }
    for (const BasicPitchDiscoveredArtifact &artifact:
         report.proofBundle.discoveredArtifacts) {
        if (artifact.artifactType == "csv_note_events") {
            return artifact.path.trimmed();
        }
    }
    return QString();
}

void
populateProofBundleSummary(
    BasicPitchDebugWorkflowUiProofBundleSummary &target,
    const BasicPitchDebugWorkflowReport &source)
{
    target.workflowMode = basicPitchDebugWorkflowModeToString(source.mode);
    target.skippedReason = source.skippedReason;
    target.eventStates.clear();
    for (BasicPitchDebugTruthState state: source.truthStates) {
        target.eventStates << basicPitchDebugTruthStateToString(state);
    }
    target.eventCount = source.proofBundle.events.size();
    target.artifactCount = source.proofBundle.discoveredArtifacts.size();
    target.artifactSummaries.clear();
    for (const BasicPitchDiscoveredArtifact &artifact:
         source.proofBundle.discoveredArtifacts) {
        target.artifactSummaries << artifactSummary(artifact);
    }
    target.commandUsed = source.proofBundle.commandUsed;
    target.commandConfigured = !target.commandUsed.trimmed().isEmpty();
    target.inputAudioPath = source.proofBundle.inputAudioPath;
    target.inputAudioConfigured = !target.inputAudioPath.trimmed().isEmpty();
    target.outputDirectoryPath = source.proofBundle.outputDirectoryPath;
    target.outputDirectoryConfigured =
        !target.outputDirectoryPath.trimmed().isEmpty();
    target.resultJsonPath = source.proofBundle.resultJsonPath;
    target.resultJsonConfigured = !target.resultJsonPath.trimmed().isEmpty();
    target.exportCsvPath = source.proofBundle.exportCsvPath;
    target.selectedNoteEventsArtifactPath =
        firstNoteEventsArtifactPath(source);
    target.selectedNoteEventsArtifactFound =
        !target.selectedNoteEventsArtifactPath.trimmed().isEmpty();
    target.noteCount = source.proofBundle.noteCount;
    target.ranBasicPitch = source.ranBasicPitch;
    target.realRunExplicitOptIn =
        source.realRunResult.config.discoveryConfig.explicitOptIn;
    target.loadedResult = source.proofBundle.loadedResult;
    target.importedIntoTonyLayers = source.proofBundle.importedIntoTonyLayers;
    target.insertedIntoView = source.proofBundle.insertedIntoView;
    target.editProof = source.proofBundle.editProof;
    target.saveLoadProof = source.proofBundle.saveLoadProof;
    target.exportProof = source.proofBundle.exportProof;
    target.productionTranscription =
        source.proofBundle.productionTranscription;
    target.testOnlyDebugOnly = source.proofBundle.testOnlyDebugOnly;
}

bool
hasBlockingConfigurationState(const BasicPitchDebugWorkflowReport &report)
{
    return hasState(report, BasicPitchDebugTruthState::BackendNotConfigured) ||
        hasState(report, BasicPitchDebugTruthState::NoAudio) ||
        hasState(report, BasicPitchDebugTruthState::MissingExecutable) ||
        hasState(report, BasicPitchDebugTruthState::MissingModelOrRuntime);
}

}

bool
BasicPitchDebugWorkflowUiModelResult::isValid() const
{
    return report.isValid() &&
        !primaryState.trimmed().isEmpty() &&
        !productionTranscription &&
        testOnlyDebugOnly &&
        !readyInstalledCompletedMutation;
}

QString
BasicPitchDebugWorkflowUiModelResult::debugSummaryString() const
{
    return QString("basic_pitch_debug_ui primary=%1 secondary=%2 "
                   "can_run=%3 can_cancel=%4 can_import=%5 can_edit=%6 "
                   "can_save=%7 can_export=%8 warnings=%9 errors=%10 "
                   "production=%11 debug=%12 ready_mutation=%13 valid=%14")
        .arg(primaryState)
        .arg(secondaryState)
        .arg(canRun ? QString("true") : QString("false"))
        .arg(canCancel ? QString("true") : QString("false"))
        .arg(canImport ? QString("true") : QString("false"))
        .arg(canEdit ? QString("true") : QString("false"))
        .arg(canSave ? QString("true") : QString("false"))
        .arg(canExport ? QString("true") : QString("false"))
        .arg(warnings.size())
        .arg(errors.size())
        .arg(productionTranscription ? QString("true") : QString("false"))
        .arg(testOnlyDebugOnly ? QString("true") : QString("false"))
        .arg(readyInstalledCompletedMutation ? QString("true") :
                                               QString("false"))
        .arg(isValid() ? QString("true") : QString("false"));
}

BasicPitchDebugWorkflowUiModelResult
BasicPitchDebugWorkflowUiModel::fromReport(
    const BasicPitchDebugWorkflowReport &workflowReport) const
{
    BasicPitchDebugWorkflowUiModelResult result;
    appendIssues(result.report, workflowReport.report);

    result.productionTranscription = workflowReport.productionTranscription;
    result.testOnlyDebugOnly = workflowReport.testOnlyDebugOnly;
    result.readyInstalledCompletedMutation =
        workflowReport.readyInstalledCompletedMutation;

    result.primaryState = primaryStateForReport(workflowReport);
    result.secondaryState =
        secondaryStateForReport(workflowReport, result.primaryState);
    result.userMessage =
        userMessageForState(result.primaryState, result.secondaryState);
    result.technicalMessage =
        QString("%1 states=%2")
            .arg(workflowReport.debugSummaryString())
            .arg(eventStatesSummary(workflowReport));

    collectWorkflowWarnings(workflowReport, result.warnings, result.errors);
    result.shouldShowWarnings = !result.warnings.isEmpty();
    result.shouldShowProofBundle =
        !workflowReport.proofBundle.events.isEmpty() ||
        workflowReport.proofBundle.loadedResult ||
        workflowReport.proofBundle.importedIntoTonyLayers ||
        !result.warnings.isEmpty() ||
        !result.errors.isEmpty();

    result.canCancel =
        hasState(workflowReport, BasicPitchDebugTruthState::RunningBackend);
    result.canRun =
        hasState(workflowReport, BasicPitchDebugTruthState::PathChecksPassed) &&
        !result.canCancel &&
        !hasBlockingConfigurationState(workflowReport) &&
        !workflowReport.unifiedResultLoaded;
    result.canImport =
        workflowReport.unifiedResultLoaded &&
        !workflowReport.importedIntoTonyLayers;
    result.canEdit = workflowReport.editProofPassed;
    result.canSave = workflowReport.saveLoadProofPassed;
    result.canExport = workflowReport.exportProofPassed;

    if (workflowReport.productionTranscription) {
        result.report.addError(
            "basic_pitch_ui_model_production_claim_forbidden",
            "Basic Pitch debug UI model must not claim production transcription.");
    }
    if (workflowReport.readyInstalledCompletedMutation) {
        result.report.addError(
            "basic_pitch_ui_model_ready_installed_completed_forbidden",
            "Basic Pitch debug UI model must not expose fake Ready, Installed, "
            "or Completed state.");
    }
    if (result.canEdit && !workflowReport.editProofPassed) {
        result.report.addError(
            "basic_pitch_ui_model_edit_without_proof",
            "Basic Pitch debug UI model cannot expose editable state without "
            "edit proof.");
    }
    if (result.canSave && !workflowReport.saveLoadProofPassed) {
        result.report.addError(
            "basic_pitch_ui_model_save_without_proof",
            "Basic Pitch debug UI model cannot expose save verification without "
            "save/load proof.");
    }
    if (result.canExport && !workflowReport.exportProofPassed) {
        result.report.addError(
            "basic_pitch_ui_model_export_without_proof",
            "Basic Pitch debug UI model cannot expose export verification "
            "without export proof.");
    }

    result.proofBundleSummary = proofBundleSummary(workflowReport);
    populateProofBundleSummary(result.proofBundle, workflowReport);
    return result;
}

void
BasicPitchDebugWorkflowUiModel::appendIssues(
    ValidationReport &target,
    const ValidationReport &source)
{
    for (const ValidationIssue &issue: source.issues) {
        target.addIssue(issue.severity, issue.code, issue.message);
    }
}

}
}
