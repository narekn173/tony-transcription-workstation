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

#include "BasicPitchDebugManualRunAction.h"

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
BasicPitchDebugManualRunActionReport::isValid() const
{
    if (wasSkipped()) {
        return report.isValid() &&
            !manualRunAttempted &&
            !ranBasicPitch &&
            !resultJsonWritten &&
            !loadedResult &&
            !productionTranscription &&
            testOnlyDebugOnly &&
            !importedIntoTonyLayers &&
            !readyInstalledCompletedMutation;
    }

    return report.isValid() &&
        preflightPassed &&
        manualRunAttempted &&
        ranBasicPitch &&
        realRunResult.isValid() &&
        resultJsonWritten &&
        loadedResult &&
        !productionTranscription &&
        testOnlyDebugOnly &&
        !importedIntoTonyLayers &&
        !readyInstalledCompletedMutation;
}

bool
BasicPitchDebugManualRunActionReport::wasSkipped() const
{
    return !preflightPassed &&
        preflightStatus.manualRunWouldBeSkipped &&
        !preflightStatus.skippedReason.trimmed().isEmpty();
}

QString
BasicPitchDebugManualRunActionReport::debugSummaryString() const
{
    return QString("basic_pitch_debug_manual_run_action stages=%1 "
                   "preflight=%2 attempted=%3 ran=%4 result_json=%5 "
                   "loaded=%6 notes=%7 imported=%8 production=%9 debug=%10 "
                   "ready_mutation=%11 valid=%12")
        .arg(stageStates.join(","))
        .arg(boolString(preflightPassed))
        .arg(boolString(manualRunAttempted))
        .arg(boolString(ranBasicPitch))
        .arg(boolString(resultJsonWritten))
        .arg(boolString(loadedResult))
        .arg(noteCount)
        .arg(boolString(importedIntoTonyLayers))
        .arg(boolString(productionTranscription))
        .arg(boolString(testOnlyDebugOnly))
        .arg(boolString(readyInstalledCompletedMutation))
        .arg(boolString(isValid()));
}

BasicPitchDebugManualRunActionReport
BasicPitchDebugManualRunAction::run(
    const BasicPitchRealRunHandoffProofConfig &config) const
{
    BasicPitchDebugManualRunActionReport result;
    result.productionTranscription = false;
    result.testOnlyDebugOnly = true;
    result.importedIntoTonyLayers = false;
    result.readyInstalledCompletedMutation = false;

    result.preflightStatus =
        BasicPitchDebugManualRunStatus().fromConfig(config);
    appendIssues(result.report, result.preflightStatus.report);

    result.commandUsed = result.preflightStatus.commandPath;
    result.inputAudioPath = result.preflightStatus.inputAudioPath;
    result.outputDirectoryPath = result.preflightStatus.outputDirectoryPath;
    result.resultJsonPath =
        result.preflightStatus.resultJsonConfigured ?
            result.preflightStatus.resultJsonPath :
            result.preflightStatus.derivedResultJsonPath;

    if (!result.preflightStatus.manualRunAllowed) {
        addStage(result, "not_configured");
        addStage(result, "skipped");
        addWarning(
            result.report,
            "basic_pitch_debug_manual_run_not_configured",
            "Basic Pitch manual-run action was skipped because required "
            "debug/manual configuration is missing.");
        collectIssueCodes(result.report,
                          result.warningCodes,
                          result.errorCodes);
        return result;
    }

    result.preflightPassed = true;
    addStage(result, "preflight_passed");
    addStage(result, "running_manual_execution_attempted");
    result.manualRunAttempted = true;

    BasicPitchRealRunHandoffProof proof;
    result.realRunResult = proof.run(config);
    appendIssues(result.report, result.realRunResult.report);

    result.ranBasicPitch = result.realRunResult.ranBasicPitch;
    result.commandUsed = trimmed(result.realRunResult.commandUsed).isEmpty() ?
        result.commandUsed : trimmed(result.realRunResult.commandUsed);
    result.inputAudioPath = trimmed(result.realRunResult.inputAudioPath)
        .isEmpty() ? result.inputAudioPath :
        trimmed(result.realRunResult.inputAudioPath);
    result.outputDirectoryPath =
        trimmed(result.realRunResult.outputDirectoryPath).isEmpty() ?
            result.outputDirectoryPath :
            trimmed(result.realRunResult.outputDirectoryPath);
    result.resultJsonPath = trimmed(result.realRunResult.resultJsonPath)
        .isEmpty() ? result.resultJsonPath :
        trimmed(result.realRunResult.resultJsonPath);
    result.selectedNoteEventsArtifactPath =
        result.realRunResult.selectedNoteEventsArtifactPath;
    result.discoveredArtifacts =
        result.realRunResult.discoveryResult.discoveredArtifacts;
    result.noteCount = result.realRunResult.noteCount;
    result.resultJsonWritten =
        result.realRunResult.handoffResult.wroteResultJson;
    result.loadedResult = result.realRunResult.loadedResult;
    result.possiblePolyphony = result.realRunResult.possiblePolyphony;
    result.pitchBendMappingDeferred =
        result.realRunResult.pitchBendMappingDeferred;
    result.productionTranscription =
        result.realRunResult.productionTranscription;
    result.importedIntoTonyLayers =
        result.realRunResult.importedIntoTonyLayers;
    result.readyInstalledCompletedMutation =
        result.realRunResult.readyInstalledCompletedMutation;

    if (result.realRunResult.wasSkipped()) {
        addStage(result, "skipped");
    }
    if (!result.realRunResult.isValid()) {
        addStage(result, "failed");
    }
    if (!result.discoveredArtifacts.isEmpty()) {
        addStage(result, "artifacts_discovered");
    }
    if (!result.selectedNoteEventsArtifactPath.trimmed().isEmpty()) {
        addStage(result, "selected_csv_note_events_artifact");
    }
    if (result.resultJsonWritten) {
        addStage(result, "result_json_written");
    }
    if (result.loadedResult) {
        addStage(result, "unified_result_loaded");
    }

    if (result.possiblePolyphony) {
        result.warningCodes << "possible_polyphony";
    }
    if (result.pitchBendMappingDeferred) {
        result.warningCodes << "pitch_bend_mapping_deferred";
    }
    collectIssueCodes(result.report,
                      result.warningCodes,
                      result.errorCodes);
    result.warningCodes.removeDuplicates();
    result.errorCodes.removeDuplicates();
    return result;
}

void
BasicPitchDebugManualRunAction::appendIssues(
    ValidationReport &target,
    const ValidationReport &source)
{
    for (const ValidationIssue &issue: source.issues) {
        target.addIssue(issue.severity, issue.code, issue.message);
    }
}

void
BasicPitchDebugManualRunAction::collectIssueCodes(
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

void
BasicPitchDebugManualRunAction::addStage(
    BasicPitchDebugManualRunActionReport &report,
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
