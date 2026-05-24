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

#include "BasicPitchRealRunHandoffProof.h"

#include <QDir>

namespace Tony {
namespace Backend {

namespace {

QString
trimmed(const QString &value)
{
    return value.trimmed();
}

void
appendIssues(ValidationReport &target, const ValidationReport &source)
{
    for (const ValidationIssue &issue: source.issues) {
        target.addIssue(issue.severity, issue.code, issue.message);
    }
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
BasicPitchRealRunHandoffProofResult::isValid() const
{
    if (wasSkipped()) {
        return report.isValid() &&
            !productionTranscription &&
            !importedIntoTonyLayers &&
            !readyInstalledCompletedMutation;
    }

    return report.isValid() &&
        ranBasicPitch &&
        handoffResult.isValid() &&
        loadedResult &&
        !productionTranscription &&
        !importedIntoTonyLayers &&
        !readyInstalledCompletedMutation;
}

bool
BasicPitchRealRunHandoffProofResult::wasSkipped() const
{
    return !ranBasicPitch && !skippedReason.trimmed().isEmpty();
}

QString
BasicPitchRealRunHandoffProofResult::debugSummaryString() const
{
    return QString("basic_pitch_real_run ran=%1 skipped=%2 command=%3 "
                   "input=%4 output_dir=%5 result_json=%6 artifact=%7 "
                   "loaded=%8 notes=%9 imported=%10 production=%11 "
                   "ready_mutation=%12 valid=%13")
        .arg(ranBasicPitch ? QString("true") : QString("false"))
        .arg(skippedReason.isEmpty() ? QString("none") : skippedReason)
        .arg(commandUsed)
        .arg(inputAudioPath)
        .arg(outputDirectoryPath)
        .arg(resultJsonPath)
        .arg(selectedNoteEventsArtifactPath)
        .arg(loadedResult ? QString("true") : QString("false"))
        .arg(noteCount)
        .arg(importedIntoTonyLayers ? QString("true") : QString("false"))
        .arg(productionTranscription ? QString("true") : QString("false"))
        .arg(readyInstalledCompletedMutation ? QString("true") :
                                               QString("false"))
        .arg(isValid() ? QString("true") : QString("false"));
}

BasicPitchRealRunHandoffProofResult
BasicPitchRealRunHandoffProof::run(
    const BasicPitchRealRunHandoffProofConfig &config) const
{
    BasicPitchRealRunHandoffProofResult result;
    result.config = config;
    result.commandUsed = trimmed(config.discoveryConfig.executablePath);
    result.inputAudioPath = trimmed(config.discoveryConfig.inputAudioPath);
    result.outputDirectoryPath =
        trimmed(config.discoveryConfig.outputDirectoryPath);
    result.resultJsonPath = trimmed(config.resultJsonPath);
    result.productionTranscription = false;
    result.importedIntoTonyLayers = false;
    result.readyInstalledCompletedMutation = false;

    if (!config.discoveryConfig.explicitOptIn) {
        result.skippedReason = "explicit_opt_in_required";
        addWarning(result.report,
                   "basic_pitch_real_run_explicit_opt_in_required",
                   "Basic Pitch real-run handoff proof was skipped because "
                   "explicit opt-in was not enabled.");
        return result;
    }

    BasicPitchArtifactDiscovery discovery;
    result.discoveryResult = discovery.runDiscovery(config.discoveryConfig);
    appendIssues(result.report, result.discoveryResult.report);
    result.ranBasicPitch = result.discoveryResult.ranBasicPitch;
    result.skippedReason = result.discoveryResult.skippedReason;
    result.commandUsed = result.discoveryResult.commandUsed;
    result.outputDirectoryPath = result.discoveryResult.outputDirectoryPath;

    if (result.discoveryResult.wasSkipped() ||
        !result.discoveryResult.isValid()) {
        return result;
    }

    if (result.resultJsonPath.isEmpty()) {
        result.resultJsonPath =
            defaultResultJsonPath(result.outputDirectoryPath);
    }
    if (result.resultJsonPath.isEmpty()) {
        result.report.addError(
            "empty_basic_pitch_real_run_result_json_path",
            "Basic Pitch real-run handoff result JSON path is empty.");
        return result;
    }

    result.selectedNoteEventsArtifactPath =
        firstNoteEventsArtifactPath(result.discoveryResult);

    BasicPitchUnifiedResultHandoffParameters handoffParameters;
    handoffParameters.expectedUnifiedResultJsonPath = result.resultJsonPath;
    handoffParameters.conversionParameters.requestId =
        "basic_pitch_real_run_request";
    handoffParameters.conversionParameters.resultId =
        "basic_pitch_real_run_result";
    handoffParameters.conversionParameters.inputAudioPath =
        result.inputAudioPath;

    BasicPitchUnifiedResultHandoff handoff;
    result.handoffResult =
        handoff.handoff(result.discoveryResult, handoffParameters);
    appendIssues(result.report, result.handoffResult.report);

    result.loadedResult = result.handoffResult.loadedUnifiedResult;
    result.noteCount = result.handoffResult.noteCount;
    result.possiblePolyphony = result.handoffResult.possiblePolyphony;
    result.pitchBendMappingDeferred =
        result.handoffResult.pitchBendMappingDeferred;
    result.importedIntoTonyLayers = result.handoffResult.importedIntoTonyLayers;
    result.productionTranscription = result.handoffResult.productionTranscription;
    result.readyInstalledCompletedMutation =
        result.handoffResult.marksBackendReadyInstalledOrCompleted;

    return result;
}

BasicPitchRealRunHandoffProofConfig
BasicPitchRealRunHandoffProof::configFromEnvironment()
{
    return configFromEnvironment(QProcessEnvironment::systemEnvironment());
}

BasicPitchRealRunHandoffProofConfig
BasicPitchRealRunHandoffProof::configFromEnvironment(
    const QProcessEnvironment &environment)
{
    BasicPitchRealRunHandoffProofConfig config;
    config.discoveryConfig =
        BasicPitchArtifactDiscovery::configFromEnvironment(environment);
    config.resultJsonPath =
        environment.value("TONY_BASIC_PITCH_RESULT_JSON").trimmed();
    return config;
}

QString
BasicPitchRealRunHandoffProof::defaultResultJsonPath(
    const QString &outputDirectoryPath)
{
    const QString outputDirectory = trimmed(outputDirectoryPath);
    if (outputDirectory.isEmpty()) {
        return QString();
    }
    return QDir(outputDirectory).filePath("basic_pitch_result.json");
}

QString
BasicPitchRealRunHandoffProof::firstNoteEventsArtifactPath(
    const BasicPitchArtifactDiscoveryResult &discoveryResult)
{
    for (const BasicPitchDiscoveredArtifact &artifact:
         discoveryResult.discoveredArtifacts) {
        if (artifact.artifactType == "csv_note_events") {
            return artifact.path;
        }
    }
    return QString();
}

}
}
