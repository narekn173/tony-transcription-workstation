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

#include "BasicPitchArtifactToUnifiedResult.h"

#include "BasicPitchAdapterContract.h"

#include <QFile>
#include <QFileInfo>
#include <QIODevice>

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

void
appendIssues(ValidationReport &target, const ValidationReport &source)
{
    for (const auto &issue: source.issues) {
        target.addIssue(issue.severity, issue.code, issue.message);
    }
}

QString
fallbackInputAudioPath(const BasicPitchArtifactDiscoveryResult &discoveryResult,
                       const QString &configuredPath)
{
    const QString explicitPath = trimmed(configuredPath);
    if (!explicitPath.isEmpty()) {
        return explicitPath;
    }
    if (discoveryResult.request.arguments.size() >= 2) {
        return discoveryResult.request.arguments.at(1).trimmed();
    }
    return QString();
}

}

bool
BasicPitchArtifactToUnifiedResultResult::isValid() const
{
    return report.isValid() &&
        conversionSucceeded &&
        unifiedResultValidationPassed;
}

QString
BasicPitchArtifactToUnifiedResultResult::debugSummaryString() const
{
    return QString("artifact=%1 type=%2 notes=%3 converted=%4 validated=%5 "
                   "source=%6 imported=%7 production=%8")
        .arg(sourceArtifactPath)
        .arg(artifactType)
        .arg(noteCount)
        .arg(conversionSucceeded ? QString("true") : QString("false"))
        .arg(unifiedResultValidationPassed ? QString("true") :
                                             QString("false"))
        .arg(fixtureOrRealArtifact)
        .arg(importedIntoTonyLayers ? QString("true") : QString("false"))
        .arg(productionTranscription ? QString("true") : QString("false"));
}

BasicPitchArtifactToUnifiedResultResult
BasicPitchArtifactToUnifiedResult::convert(
    const BasicPitchArtifactDiscoveryResult &discoveryResult,
    const BasicPitchArtifactToUnifiedResultParameters &parameters) const
{
    BasicPitchArtifactToUnifiedResultResult result;
    result.fixtureOrRealArtifact = discoveryResult.ranBasicPitch ?
        QString("real_discovered_artifact_manual_only") :
        QString("fixture_or_synthetic_artifact");

    appendIssues(result.report, discoveryResult.report);

    const std::optional<BasicPitchDiscoveredArtifact> artifact =
        firstNoteEventsArtifact(discoveryResult);
    if (!artifact.has_value()) {
        result.report.addError(
            "missing_basic_pitch_note_events_artifact",
            "No recognized Basic Pitch note-events CSV artifact was discovered.");
        return result;
    }

    result.foundNoteEventsArtifact = true;
    result.sourceArtifactPath = artifact->path;
    result.artifactType = artifact->artifactType;

    const QFileInfo artifactInfo(result.sourceArtifactPath);
    if (!artifactInfo.exists()) {
        result.report.addError(
            "missing_basic_pitch_note_events_artifact_file",
            "Recognized Basic Pitch note-events CSV artifact is missing.");
        return result;
    }
    if (artifactInfo.isDir()) {
        result.report.addError(
            "invalid_basic_pitch_note_events_artifact_file",
            "Recognized Basic Pitch note-events CSV artifact is a directory.");
        return result;
    }

    QFile file(result.sourceArtifactPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        result.report.addError(
            "unreadable_basic_pitch_note_events_artifact",
            "Recognized Basic Pitch note-events CSV artifact cannot be read.");
        return result;
    }

    BasicPitchOutputConversionParameters conversionParameters;
    conversionParameters.requestId =
        trimmed(parameters.requestId).isEmpty() ?
            QString("basic_pitch_artifact_request") :
            trimmed(parameters.requestId);
    conversionParameters.resultId =
        trimmed(parameters.resultId).isEmpty() ?
            QString("basic_pitch_artifact_result") :
            trimmed(parameters.resultId);
    conversionParameters.inputAudioPath =
        fallbackInputAudioPath(discoveryResult, parameters.inputAudioPath);
    conversionParameters.sourceArtifactPath = result.sourceArtifactPath;
    conversionParameters.fixtureOnly = !discoveryResult.ranBasicPitch;
    conversionParameters.productionTranscription = false;

    BasicPitchOutputConverter converter;
    result.conversion =
        converter.convertNoteEventsCsv(QString::fromUtf8(file.readAll()),
                                       conversionParameters);
    appendIssues(result.report, result.conversion.report);
    result.result = result.conversion.result;
    result.noteCount = result.result.notes.size();
    result.possiblePolyphony = result.conversion.possiblePolyphony;
    result.pitchBendMappingDeferred =
        result.conversion.pitchBendTonyMappingDeferred;
    result.conversionSucceeded = result.conversion.isValid();

    result.result.provenance.insert("source_artifact_type",
                                    result.artifactType);
    result.result.provenance.insert("fixture_or_real_artifact",
                                    result.fixtureOrRealArtifact);
    result.result.provenance.insert("artifact_discovery_ran_basic_pitch",
                                    discoveryResult.ranBasicPitch);
    result.result.provenance.insert("production_transcription", false);
    result.result.provenance.insert("imported_into_tony_layers", false);
    result.result.provenance.insert("created_result_json", false);

    if (!result.conversionSucceeded) {
        return result;
    }

    BackendRequest request;
    request.requestId = result.result.requestId;
    request.engineId = BasicPitchAdapterContract::backendId();
    request.inputAudioPath = result.result.audio.path;

    ResultValidator validator;
    result.unifiedResultValidationReport =
        validator.validate(request, result.result);
    appendIssues(result.report, result.unifiedResultValidationReport);
    result.unifiedResultValidationPassed =
        result.unifiedResultValidationReport.isValid();

    addWarning(result.report,
               "basic_pitch_artifact_conversion_manual_only",
               "Basic Pitch artifact conversion is manual/test-only and "
               "does not prove production transcription support.");

    return result;
}

std::optional<BasicPitchDiscoveredArtifact>
BasicPitchArtifactToUnifiedResult::firstNoteEventsArtifact(
    const BasicPitchArtifactDiscoveryResult &discoveryResult)
{
    for (const BasicPitchDiscoveredArtifact &artifact:
         discoveryResult.discoveredArtifacts) {
        if (artifact.artifactType == "csv_note_events") {
            return artifact;
        }
    }
    return std::nullopt;
}

}
}
