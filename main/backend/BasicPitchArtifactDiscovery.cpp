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

#include "BasicPitchArtifactDiscovery.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QTextStream>

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

}

bool
BasicPitchArtifactDiscoveryResult::isValid() const
{
    return report.isValid();
}

bool
BasicPitchArtifactDiscoveryResult::wasSkipped() const
{
    return !ranBasicPitch && !skippedReason.trimmed().isEmpty();
}

QString
BasicPitchArtifactDiscoveryResult::debugSummaryString() const
{
    return QString("basic_pitch_discovery ran=%1 skipped=%2 artifacts=%3 "
                   "valid=%4 production=%5 imported=%6")
        .arg(ranBasicPitch ? QString("true") : QString("false"))
        .arg(skippedReason.isEmpty() ? QString("none") : skippedReason)
        .arg(discoveredArtifacts.size())
        .arg(isValid() ? QString("true") : QString("false"))
        .arg(productionTranscription ? QString("true") : QString("false"))
        .arg(importedIntoTonyLayers ? QString("true") : QString("false"));
}

BasicPitchArtifactDiscoveryResult
BasicPitchArtifactDiscovery::buildRequest(
    const BasicPitchArtifactDiscoveryConfig &config) const
{
    BasicPitchArtifactDiscoveryResult result;
    result.commandUsed = trimmed(config.executablePath);
    result.outputDirectoryPath = trimmed(config.outputDirectoryPath);
    result.request.executablePath = result.commandUsed;
    result.request.timeoutMsec = config.timeoutMsec;
    result.request.environmentOverrides = config.environmentOverrides;

    const QString inputAudioPath = trimmed(config.inputAudioPath);

    if (result.commandUsed.isEmpty()) {
        result.report.addError("empty_basic_pitch_discovery_command",
                               "Basic Pitch discovery command is empty.");
    }
    if (inputAudioPath.isEmpty()) {
        result.report.addError("empty_basic_pitch_discovery_audio_path",
                               "Basic Pitch discovery input audio path is empty.");
    }
    if (result.outputDirectoryPath.isEmpty()) {
        result.report.addError("empty_basic_pitch_discovery_output_directory",
                               "Basic Pitch discovery output directory is empty.");
    }
    if (config.timeoutMsec < 0) {
        result.report.addError("invalid_basic_pitch_discovery_timeout",
                               "Basic Pitch discovery timeout must not be negative.");
    }

    if (!result.outputDirectoryPath.isEmpty()) {
        result.request.arguments << result.outputDirectoryPath;
    }
    if (!inputAudioPath.isEmpty()) {
        result.request.arguments << inputAudioPath;
    }
    if (config.saveMidi) {
        result.request.arguments << "--save-midi";
    }
    if (config.saveNoteEvents) {
        result.request.arguments << "--save-note-events";
    }
    if (config.saveModelOutputs) {
        result.request.arguments << "--save-model-outputs";
    }
    if (config.sonifyMidi) {
        result.request.arguments << "--sonify-midi";
    }
    if (config.multiplePitchBends) {
        result.request.arguments << "--multiple-pitch-bends";
    }

    addWarning(result.report,
               "basic_pitch_discovery_manual_only",
               "Basic Pitch artifact discovery is manual/test-only and "
               "does not prove production transcription support.");

    return result;
}

BasicPitchArtifactDiscoveryResult
BasicPitchArtifactDiscovery::inspectOutputDirectory(
    const QString &outputDirectoryPath) const
{
    BasicPitchArtifactDiscoveryResult result;
    result.outputDirectoryPath = trimmed(outputDirectoryPath);

    if (result.outputDirectoryPath.isEmpty()) {
        result.report.addError("empty_basic_pitch_discovery_output_directory",
                               "Basic Pitch discovery output directory is empty.");
        return result;
    }

    const QFileInfo info(result.outputDirectoryPath);
    if (!info.exists()) {
        result.report.addError("missing_basic_pitch_discovery_output_directory",
                               "Basic Pitch discovery output directory does not exist.");
        return result;
    }
    if (!info.isDir()) {
        result.report.addError("invalid_basic_pitch_discovery_output_directory",
                               "Basic Pitch discovery output path is not a directory.");
        return result;
    }

    const QFileInfoList files =
        QDir(result.outputDirectoryPath)
            .entryInfoList(QDir::Files | QDir::NoDotAndDotDot,
                           QDir::Name);

    if (files.isEmpty()) {
        addWarning(result.report,
                   "basic_pitch_discovery_no_artifacts",
                   "Basic Pitch discovery output directory contains no files.");
    }

    for (const QFileInfo &fileInfo: files) {
        BasicPitchDiscoveredArtifact artifact;
        artifact.path = fileInfo.absoluteFilePath();
        artifact.fileName = fileInfo.fileName();
        artifact.sizeBytes = fileInfo.size();
        artifact.artifactType = classifyArtifact(fileInfo.absoluteFilePath());
        result.discoveredArtifacts.push_back(artifact);
    }

    return result;
}

BasicPitchArtifactDiscoveryResult
BasicPitchArtifactDiscovery::runDiscovery(
    const BasicPitchArtifactDiscoveryConfig &config) const
{
    if (!config.explicitOptIn) {
        BasicPitchArtifactDiscoveryResult result;
        result.commandUsed = trimmed(config.executablePath);
        result.outputDirectoryPath = trimmed(config.outputDirectoryPath);
        result.skippedReason = "explicit_opt_in_required";
        addWarning(result.report,
                   "basic_pitch_discovery_explicit_opt_in_required",
                   "Basic Pitch discovery was skipped because explicit opt-in "
                   "was not enabled.");
        return result;
    }

    BasicPitchArtifactDiscoveryResult result = buildRequest(config);

    if (!result.report.isValid()) {
        result.skippedReason = "not_configured";
        return result;
    }

    const QFileInfo audioInfo(trimmed(config.inputAudioPath));
    if (!audioInfo.exists() || audioInfo.isDir()) {
        result.skippedReason = "missing_audio_file";
        result.report.addError(
            "missing_basic_pitch_input_audio_file",
            "Basic Pitch discovery input audio file is missing or invalid.");
        return result;
    }

    const QFileInfo outputInfo(result.outputDirectoryPath);
    if (!outputInfo.exists() || !outputInfo.isDir()) {
        result.skippedReason = "missing_output_directory";
        result.report.addError(
            "missing_basic_pitch_discovery_output_directory",
            "Basic Pitch discovery output directory must already exist.");
        return result;
    }

    ExternalProcessRunner runner;
    result.processResult = runner.run(result.request);
    result.ranBasicPitch = result.processResult.started;
    result.standardOutputSummary =
        summarizeOutput(result.processResult.standardOutput);
    result.standardErrorSummary =
        summarizeOutput(result.processResult.standardError);

    if (!result.processResult.succeeded()) {
        result.report.addError(
            "basic_pitch_discovery_process_failed",
            result.processResult.debugSummaryString());
    }

    const BasicPitchArtifactDiscoveryResult inspected =
        inspectOutputDirectory(result.outputDirectoryPath);
    appendIssues(result.report, inspected.report);
    result.discoveredArtifacts = inspected.discoveredArtifacts;

    return result;
}

BasicPitchArtifactDiscoveryConfig
BasicPitchArtifactDiscovery::configFromEnvironment()
{
    return configFromEnvironment(QProcessEnvironment::systemEnvironment());
}

BasicPitchArtifactDiscoveryConfig
BasicPitchArtifactDiscovery::configFromEnvironment(
    const QProcessEnvironment &environment)
{
    BasicPitchArtifactDiscoveryConfig config;
    config.explicitOptIn =
        environment.value("TONY_BASIC_PITCH_DISCOVERY_ENABLE").trimmed() == "1";

    const QString command =
        environment.value("TONY_BASIC_PITCH_COMMAND").trimmed();
    if (!command.isEmpty()) {
        config.executablePath = command;
    }
    config.inputAudioPath =
        environment.value("TONY_BASIC_PITCH_TEST_AUDIO").trimmed();
    config.outputDirectoryPath =
        environment.value("TONY_BASIC_PITCH_OUTPUT_DIR").trimmed();

    return config;
}

QString
BasicPitchArtifactDiscovery::classifyArtifact(const QString &path)
{
    const QFileInfo info(path);
    const QString suffix = info.suffix().toLower();

    if (suffix == "mid" || suffix == "midi") {
        return "midi";
    }
    if (suffix == "npz") {
        return "model_output_npz";
    }
    if (suffix == "wav") {
        return "sonified_midi_wav";
    }
    if (suffix == "log" || suffix == "txt") {
        return "log";
    }
    if (suffix == "csv") {
        QFile file(path);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            const QString header = stream.readLine().trimmed();
            if (header.startsWith(
                    "start_time_s,end_time_s,pitch_midi,velocity,pitch_bend")) {
                return "csv_note_events";
            }
        }
        return "csv";
    }

    return "unknown";
}

QString
BasicPitchArtifactDiscovery::summarizeOutput(const QString &text)
{
    const QString compact = text.trimmed();
    constexpr int maxLength = 512;
    if (compact.size() <= maxLength) {
        return compact;
    }
    return compact.left(maxLength) + "...";
}

}
}
