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

#include "BasicPitchDebugWorkflow.h"

#include "BasicPitchResultToTonyLayerProof.h"

#include "framework/Document.h"
#include "view/Pane.h"
#include "widgets/CommandHistory.h"

#include <QDir>
#include <QFileInfo>

namespace Tony {
namespace Backend {

namespace {

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

bool
hasError(const ValidationReport &report)
{
    return !report.isValid();
}

void
collectIssueCodes(const ValidationReport &source,
                  QStringList &warnings,
                  QStringList &errors)
{
    for (const ValidationIssue &issue: source.issues) {
        if (issue.code.trimmed().isEmpty()) {
            continue;
        }
        if (issue.severity == ValidationSeverity::Error) {
            errors << issue.code.trimmed();
        } else if (issue.severity == ValidationSeverity::Warning) {
            warnings << issue.code.trimmed();
        }
    }
}

QString
defaultResultJsonPath(const QString &outputDirectoryPath)
{
    const QString outputDirectory = trimmed(outputDirectoryPath);
    if (outputDirectory.isEmpty()) {
        return QString();
    }
    return QDir(outputDirectory).filePath("basic_pitch_debug_result.json");
}

QString
defaultExportCsvPath(const QString &outputDirectoryPath)
{
    const QString outputDirectory = trimmed(outputDirectoryPath);
    if (outputDirectory.isEmpty()) {
        return QString();
    }
    return QDir(outputDirectory).filePath("basic_pitch_debug_notes.csv");
}

bool
hasNoteEventsArtifact(
    const QVector<BasicPitchDiscoveredArtifact> &artifacts)
{
    for (const BasicPitchDiscoveredArtifact &artifact: artifacts) {
        if (artifact.artifactType == "csv_note_events") {
            return true;
        }
    }
    return false;
}

BasicPitchArtifactDiscoveryResult
discoveredArtifactsFromDirectory(const BasicPitchDebugWorkflowRequest &request,
                                 QString outputDirectoryPath,
                                 const QString &inputAudioPath)
{
    BasicPitchArtifactDiscovery discovery;
    BasicPitchArtifactDiscoveryResult discovered =
        discovery.inspectOutputDirectory(outputDirectoryPath);
    if (!outputDirectoryPath.trimmed().isEmpty() ||
        !inputAudioPath.trimmed().isEmpty()) {
        discovered.request.arguments << outputDirectoryPath.trimmed()
                                     << inputAudioPath.trimmed();
    }
    discovered.commandUsed =
        trimmed(request.discoveryConfig.executablePath);
    return discovered;
}

TonyLayerImportOptions
documentPaneImportOptions(sv::Document &document,
                          sv::Pane &pane,
                          double sampleRate,
                          int resolution)
{
    TonyLayerImportOptions options;
    options.sampleRate = sampleRate;
    options.resolution = resolution;
    options.document = &document;
    options.createDocumentLayer = true;
    options.view = &pane;
    options.insertLayerIntoView = true;
    return options;
}

TonyLayerCommandHistoryEditProofResult
proveCommandHistoryEditForHandoff(
    const BasicPitchUnifiedResultHandoffResult &handoffResult,
    double sampleRate,
    int resolution,
    ValidationReport &report,
    bool &importedIntoTonyLayers,
    bool &insertedIntoView)
{
    sv::CommandHistory::getInstance()->clear();

    sv::Pane pane;
    sv::Document document;
    const TonyLayerImportOptions options =
        documentPaneImportOptions(document, pane, sampleRate, resolution);

    BasicPitchResultToTonyLayerProof layerProof;
    const BasicPitchResultToTonyLayerProofResult imported =
        layerProof.importHandoffResult(handoffResult, options);
    for (const ValidationIssue &issue: imported.report.issues) {
        report.addIssue(issue.severity, issue.code, issue.message);
    }

    importedIntoTonyLayers = imported.importedIntoTonyLayers;
    insertedIntoView = imported.insertedIntoView;

    TonyLayerCommandHistoryEditProofResult editProof;
    if (imported.isValid() && imported.importResult.importedIntoTonyLayers) {
        TonyLayerImporter importer;
        editProof = importer.proveCommandHistoryEdit(imported.importResult);
        for (const ValidationIssue &issue: editProof.report.issues) {
            report.addIssue(issue.severity, issue.code, issue.message);
        }
    } else {
        editProof.report.addError(
            "basic_pitch_debug_edit_import_unavailable",
            "Basic Pitch debug workflow could not prove edit behavior "
            "because a real imported layer was not available.");
        for (const ValidationIssue &issue: editProof.report.issues) {
            report.addIssue(issue.severity, issue.code, issue.message);
        }
    }

    sv::CommandHistory::getInstance()->clear();
    return editProof;
}

void
finishProofBundle(BasicPitchDebugWorkflowReport &result)
{
    result.proofBundle.loadedResult = result.unifiedResultLoaded;
    result.proofBundle.noteCount =
        result.persistenceExportProofResult.importedNoteCount > 0 ?
            result.persistenceExportProofResult.importedNoteCount :
            result.handoffResult.noteCount;
    result.proofBundle.importedIntoTonyLayers =
        result.importedIntoTonyLayers;
    result.proofBundle.insertedIntoView = result.insertedIntoView;
    result.proofBundle.editProof = result.editProofPassed;
    result.proofBundle.saveLoadProof = result.saveLoadProofPassed;
    result.proofBundle.exportProof = result.exportProofPassed;
    result.proofBundle.productionTranscription =
        result.productionTranscription;
    result.proofBundle.testOnlyDebugOnly = result.testOnlyDebugOnly;

    collectIssueCodes(result.report,
                      result.proofBundle.warningCodes,
                      result.proofBundle.errorCodes);
}

}

QString
basicPitchDebugWorkflowModeToString(BasicPitchDebugWorkflowMode mode)
{
    switch (mode) {
    case BasicPitchDebugWorkflowMode::SyntheticArtifactOnly:
        return "synthetic_artifact_only";
    case BasicPitchDebugWorkflowMode::DiscoveredArtifactManualOnly:
        return "discovered_artifact_manual_only";
    case BasicPitchDebugWorkflowMode::RealBasicPitchManualOptIn:
        return "real_basic_pitch_manual_opt_in";
    }
    return "unknown";
}

QString
basicPitchDebugTruthStateToString(BasicPitchDebugTruthState state)
{
    switch (state) {
    case BasicPitchDebugTruthState::NoAudio:
        return "no_audio";
    case BasicPitchDebugTruthState::BackendNotConfigured:
        return "backend_not_configured";
    case BasicPitchDebugTruthState::MissingExecutable:
        return "missing_executable";
    case BasicPitchDebugTruthState::MissingModelOrRuntime:
        return "missing_model_or_runtime";
    case BasicPitchDebugTruthState::PathChecksPassed:
        return "path_checks_passed";
    case BasicPitchDebugTruthState::RunningBackend:
        return "running_backend";
    case BasicPitchDebugTruthState::BackendFailed:
        return "backend_failed";
    case BasicPitchDebugTruthState::ArtifactsDiscovered:
        return "artifacts_discovered";
    case BasicPitchDebugTruthState::ResultJsonWritten:
        return "result_json_written";
    case BasicPitchDebugTruthState::UnifiedResultLoaded:
        return "unified_result_loaded";
    case BasicPitchDebugTruthState::ImportedIntoRealLayer:
        return "imported_into_real_layer";
    case BasicPitchDebugTruthState::InsertedIntoView:
        return "inserted_into_view";
    case BasicPitchDebugTruthState::EditProofPassed:
        return "edit_proof_passed";
    case BasicPitchDebugTruthState::SaveLoadProofPassed:
        return "save_load_proof_passed";
    case BasicPitchDebugTruthState::ExportProofPassed:
        return "export_proof_passed";
    case BasicPitchDebugTruthState::CompletedWithWarnings:
        return "completed_with_warnings";
    case BasicPitchDebugTruthState::Failed:
        return "failed";
    case BasicPitchDebugTruthState::Skipped:
        return "skipped";
    }
    return "unknown";
}

bool
BasicPitchDebugWorkflowReport::isValid() const
{
    if (wasSkipped()) {
        return report.isValid() &&
            !resultJsonWritten &&
            !unifiedResultLoaded &&
            !importedIntoTonyLayers &&
            !insertedIntoView &&
            !productionTranscription &&
            testOnlyDebugOnly &&
            !readyInstalledCompletedMutation;
    }

    return report.isValid() &&
        resultJsonWritten &&
        unifiedResultLoaded &&
        importedIntoTonyLayers &&
        insertedIntoView &&
        editProofPassed &&
        saveLoadProofPassed &&
        exportProofPassed &&
        !productionTranscription &&
        testOnlyDebugOnly &&
        !readyInstalledCompletedMutation;
}

bool
BasicPitchDebugWorkflowReport::wasSkipped() const
{
    return hasTruthState(BasicPitchDebugTruthState::Skipped) &&
        !skippedReason.trimmed().isEmpty();
}

bool
BasicPitchDebugWorkflowReport::hasTruthState(
    BasicPitchDebugTruthState state) const
{
    for (BasicPitchDebugTruthState existing: truthStates) {
        if (existing == state) {
            return true;
        }
    }
    return false;
}

QString
BasicPitchDebugWorkflowReport::debugSummaryString() const
{
    QStringList states;
    for (BasicPitchDebugTruthState state: truthStates) {
        states << basicPitchDebugTruthStateToString(state);
    }

    return QString("basic_pitch_debug_workflow mode=%1 states=%2 "
                   "skipped=%3 ran=%4 result_json=%5 loaded=%6 "
                   "notes=%7 imported=%8 inserted=%9 edit=%10 "
                   "save_load=%11 export=%12 production=%13 debug=%14 "
                   "ready_mutation=%15 valid=%16")
        .arg(basicPitchDebugWorkflowModeToString(mode))
        .arg(states.join(","))
        .arg(skippedReason.isEmpty() ? QString("none") : skippedReason)
        .arg(ranBasicPitch ? QString("true") : QString("false"))
        .arg(resultJsonWritten ? QString("true") : QString("false"))
        .arg(unifiedResultLoaded ? QString("true") : QString("false"))
        .arg(proofBundle.noteCount)
        .arg(importedIntoTonyLayers ? QString("true") : QString("false"))
        .arg(insertedIntoView ? QString("true") : QString("false"))
        .arg(editProofPassed ? QString("true") : QString("false"))
        .arg(saveLoadProofPassed ? QString("true") : QString("false"))
        .arg(exportProofPassed ? QString("true") : QString("false"))
        .arg(productionTranscription ? QString("true") : QString("false"))
        .arg(testOnlyDebugOnly ? QString("true") : QString("false"))
        .arg(readyInstalledCompletedMutation ? QString("true") :
                                               QString("false"))
        .arg(isValid() ? QString("true") : QString("false"));
}

BasicPitchDebugWorkflowReport
BasicPitchDebugWorkflow::run(
    const BasicPitchDebugWorkflowRequest &request) const
{
    BasicPitchDebugWorkflowReport result;
    result.mode = request.mode;
    result.productionTranscription = false;
    result.testOnlyDebugOnly = true;
    result.readyInstalledCompletedMutation = false;
    result.proofBundle.productionTranscription = false;
    result.proofBundle.testOnlyDebugOnly = true;

    addWarning(result.report,
               "basic_pitch_debug_workflow_test_only",
               "Basic Pitch debug workflow is manual/test-only and does "
               "not prove production transcription or user-facing UI.");

    if (request.mode == BasicPitchDebugWorkflowMode::RealBasicPitchManualOptIn) {
        if (!request.realRunConfig.discoveryConfig.explicitOptIn) {
            result.skippedReason = "explicit_opt_in_required";
            addState(result,
                     BasicPitchDebugTruthState::BackendNotConfigured,
                     "Real Basic Pitch workflow requires explicit opt-in.");
            addState(result,
                     BasicPitchDebugTruthState::Skipped,
                     "Skipped because manual real-run opt-in is disabled.");
            addWarning(result.report,
                       "basic_pitch_debug_workflow_explicit_opt_in_required",
                       "Real Basic Pitch debug workflow was skipped because "
                       "TONY_BASIC_PITCH_DISCOVERY_ENABLE was not enabled.");
            finishProofBundle(result);
            return result;
        }

        addState(result,
                 BasicPitchDebugTruthState::RunningBackend,
                 "Explicit manual Basic Pitch real-run proof started.");

        BasicPitchRealRunHandoffProof realRunProof;
        result.realRunResult = realRunProof.run(request.realRunConfig);
        appendIssues(result.report, result.realRunResult.report);
        result.ranBasicPitch = result.realRunResult.ranBasicPitch;
        result.skippedReason = result.realRunResult.skippedReason;
        result.handoffResult = result.realRunResult.handoffResult;
        result.discoveryResult = result.realRunResult.discoveryResult;

        result.proofBundle.commandUsed = result.realRunResult.commandUsed;
        result.proofBundle.inputAudioPath = result.realRunResult.inputAudioPath;
        result.proofBundle.outputDirectoryPath =
            result.realRunResult.outputDirectoryPath;
        result.proofBundle.discoveredArtifacts =
            result.realRunResult.discoveryResult.discoveredArtifacts;
        result.proofBundle.resultJsonPath = result.realRunResult.resultJsonPath;

        if (result.realRunResult.wasSkipped()) {
            addState(result,
                     BasicPitchDebugTruthState::Skipped,
                     result.realRunResult.skippedReason);
            finishProofBundle(result);
            return result;
        }
        if (!result.realRunResult.isValid()) {
            addState(result,
                     BasicPitchDebugTruthState::BackendFailed,
                     "Manual Basic Pitch real-run proof failed.");
            addState(result,
                     BasicPitchDebugTruthState::Failed,
                     "Workflow failed before result import proof.");
            finishProofBundle(result);
            return result;
        }
        if (hasNoteEventsArtifact(result.discoveryResult.discoveredArtifacts)) {
            addState(result,
                     BasicPitchDebugTruthState::ArtifactsDiscovered,
                     "Manual Basic Pitch run produced a recognized "
                     "note-events artifact.");
        }
    } else {
        QString outputDirectoryPath = trimmed(request.outputDirectoryPath);
        if (outputDirectoryPath.isEmpty()) {
            outputDirectoryPath =
                trimmed(request.discoveryConfig.outputDirectoryPath);
        }
        const QString inputAudioPath =
            trimmed(request.inputAudioPath).isEmpty() ?
                trimmed(request.discoveryConfig.inputAudioPath) :
                trimmed(request.inputAudioPath);

        result.proofBundle.inputAudioPath = inputAudioPath;
        result.proofBundle.outputDirectoryPath = outputDirectoryPath;
        result.proofBundle.commandUsed =
            trimmed(request.discoveryConfig.executablePath);

        if (inputAudioPath.isEmpty()) {
            addState(result,
                     BasicPitchDebugTruthState::NoAudio,
                     "No input audio path was provided.");
        }
        if (outputDirectoryPath.isEmpty()) {
            result.skippedReason = "output_directory_required";
            addState(result,
                     BasicPitchDebugTruthState::BackendNotConfigured,
                     "Basic Pitch debug workflow output directory is empty.");
            addState(result,
                     BasicPitchDebugTruthState::Skipped,
                     "Skipped because no artifact output directory was provided.");
            addWarning(result.report,
                       "basic_pitch_debug_workflow_output_directory_required",
                       "Synthetic/discovered Basic Pitch workflow requires "
                       "an explicit artifact output directory.");
            finishProofBundle(result);
            return result;
        }

        const QFileInfo outputInfo(outputDirectoryPath);
        if (outputInfo.exists() && outputInfo.isDir()) {
            addState(result,
                     BasicPitchDebugTruthState::PathChecksPassed,
                     "Artifact directory exists for debug workflow inspection.");
        }

        result.discoveryResult =
            discoveredArtifactsFromDirectory(request,
                                             outputDirectoryPath,
                                             inputAudioPath);
        appendIssues(result.report, result.discoveryResult.report);
        result.proofBundle.discoveredArtifacts =
            result.discoveryResult.discoveredArtifacts;

        if (!result.discoveryResult.isValid()) {
            addState(result,
                     BasicPitchDebugTruthState::Failed,
                     "Artifact directory inspection failed.");
            finishProofBundle(result);
            return result;
        }

        if (hasNoteEventsArtifact(result.discoveryResult.discoveredArtifacts)) {
            addState(result,
                     BasicPitchDebugTruthState::ArtifactsDiscovered,
                     "Recognized Basic Pitch note-events artifact discovered.");
        }
    }

    QString resultJsonPath = trimmed(request.resultJsonPath);
    if (resultJsonPath.isEmpty() &&
        !result.realRunResult.resultJsonPath.trimmed().isEmpty()) {
        resultJsonPath = result.realRunResult.resultJsonPath.trimmed();
    }
    if (resultJsonPath.isEmpty()) {
        resultJsonPath =
            defaultResultJsonPath(result.proofBundle.outputDirectoryPath);
    }
    result.proofBundle.resultJsonPath = resultJsonPath;

    QString exportCsvPath = trimmed(request.exportCsvPath);
    if (exportCsvPath.isEmpty()) {
        exportCsvPath =
            defaultExportCsvPath(result.proofBundle.outputDirectoryPath);
    }
    result.proofBundle.exportCsvPath = exportCsvPath;

    if (result.handoffResult.expectedUnifiedResultJsonPath.trimmed().isEmpty()) {
        BasicPitchUnifiedResultHandoffParameters handoffParameters;
        handoffParameters.expectedUnifiedResultJsonPath = resultJsonPath;
        handoffParameters.conversionParameters.requestId =
            trimmed(request.requestId).isEmpty() ?
                QString("basic_pitch_debug_workflow_request") :
                trimmed(request.requestId);
        handoffParameters.conversionParameters.resultId =
            trimmed(request.resultId).isEmpty() ?
                QString("basic_pitch_debug_workflow_result") :
                trimmed(request.resultId);
        handoffParameters.conversionParameters.inputAudioPath =
            result.proofBundle.inputAudioPath;

        BasicPitchUnifiedResultHandoff handoff;
        result.handoffResult =
            handoff.handoff(result.discoveryResult, handoffParameters);
        appendIssues(result.report, result.handoffResult.report);
    }

    result.resultJsonWritten = result.handoffResult.wroteResultJson;
    result.unifiedResultLoaded = result.handoffResult.loadedUnifiedResult;
    result.possiblePolyphony = result.handoffResult.possiblePolyphony;
    result.pitchBendMappingDeferred =
        result.handoffResult.pitchBendMappingDeferred;
    result.productionTranscription =
        result.handoffResult.productionTranscription;
    result.readyInstalledCompletedMutation =
        result.handoffResult.marksBackendReadyInstalledOrCompleted;

    if (result.resultJsonWritten) {
        addState(result,
                 BasicPitchDebugTruthState::ResultJsonWritten,
                 "UnifiedResult-compatible result.json was written.");
    }
    if (result.unifiedResultLoaded) {
        addState(result,
                 BasicPitchDebugTruthState::UnifiedResultLoaded,
                 "BackendRunResultLoader loaded the written result.json.");
    }

    if (!result.handoffResult.isValid()) {
        addState(result,
                 BasicPitchDebugTruthState::Failed,
                 "Basic Pitch debug workflow result.json handoff failed.");
        finishProofBundle(result);
        return result;
    }

    bool editImportCreatedLayer = false;
    bool editImportInsertedIntoView = false;
    result.editProofResult =
        proveCommandHistoryEditForHandoff(result.handoffResult,
                                          request.sampleRate,
                                          request.resolution,
                                          result.report,
                                          editImportCreatedLayer,
                                          editImportInsertedIntoView);
    result.editProofPassed = result.editProofResult.isValid() &&
        result.editProofResult.commandHistoryEditProof;
    if (result.editProofPassed) {
        addState(result,
                 BasicPitchDebugTruthState::EditProofPassed,
                 "CommandHistory-safe edit proof passed on a real note layer.");
    }

    BasicPitchLayerPersistenceExportProofOptions proofOptions;
    proofOptions.resultJsonPath = resultJsonPath;
    proofOptions.exportCsvPath = exportCsvPath;
    proofOptions.sampleRate = request.sampleRate;
    proofOptions.resolution = request.resolution;

    BasicPitchLayerPersistenceExportProof proof;
    result.persistenceExportProofResult =
        proof.proveHandoffResult(result.handoffResult, proofOptions);
    appendIssues(result.report, result.persistenceExportProofResult.report);

    result.importedIntoTonyLayers =
        result.persistenceExportProofResult.importedIntoTonyLayers ||
        editImportCreatedLayer;
    result.insertedIntoView =
        result.persistenceExportProofResult.insertedIntoView ||
        editImportInsertedIntoView;
    result.saveLoadProofPassed =
        result.persistenceExportProofResult.saveLoadProven;
    result.exportProofPassed =
        result.persistenceExportProofResult.exportProven;
    result.readyInstalledCompletedMutation =
        result.readyInstalledCompletedMutation ||
        result.persistenceExportProofResult.readyInstalledCompletedMutation;

    result.proofBundle.documentPaneLayerModelSnapshotSummary =
        result.persistenceExportProofResult.debugSummaryString();

    if (result.importedIntoTonyLayers) {
        addState(result,
                 BasicPitchDebugTruthState::ImportedIntoRealLayer,
                 "Basic Pitch-shaped result imported into a real Tony layer.");
    }
    if (result.insertedIntoView) {
        addState(result,
                 BasicPitchDebugTruthState::InsertedIntoView,
                 "Real imported layer was inserted into a real Pane/View.");
    }
    if (result.saveLoadProofPassed) {
        addState(result,
                 BasicPitchDebugTruthState::SaveLoadProofPassed,
                 "Real Tony/SV save/load proof passed.");
    }
    if (result.exportProofPassed) {
        addState(result,
                 BasicPitchDebugTruthState::ExportProofPassed,
                 "Real lower-level CSV export proof passed.");
    }

    if (hasError(result.report)) {
        addState(result,
                 BasicPitchDebugTruthState::Failed,
                 "Basic Pitch debug workflow ended with validation errors.");
    } else if (!result.report.issues.isEmpty()) {
        addState(result,
                 BasicPitchDebugTruthState::CompletedWithWarnings,
                 "Basic Pitch debug workflow completed with warnings.");
    }

    finishProofBundle(result);
    return result;
}

void
BasicPitchDebugWorkflow::appendIssues(ValidationReport &target,
                                      const ValidationReport &source)
{
    for (const ValidationIssue &issue: source.issues) {
        target.addIssue(issue.severity, issue.code, issue.message);
    }
}

void
BasicPitchDebugWorkflow::addState(BasicPitchDebugWorkflowReport &report,
                                  BasicPitchDebugTruthState state,
                                  const QString &message)
{
    if (!report.hasTruthState(state)) {
        report.truthStates.push_back(state);
    }

    BasicPitchDebugWorkflowEvent event;
    event.state = state;
    event.message = message;
    report.proofBundle.events.push_back(event);
}

}
}
