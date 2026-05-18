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

#include "BasicPitchAdapterContract.h"

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

}

bool
BasicPitchAdapterContractResult::isValid() const
{
    return report.isValid();
}

QString
BasicPitchAdapterContractResult::debugSummaryString() const
{
    return QString("backend=%1 executable=%2 input=%3 output_dir=%4 "
                   "result=%5 valid=%6 executes=%7 imports=%8")
        .arg(manifest.id().isEmpty() ? QString("unknown") : manifest.id())
        .arg(request.executablePath)
        .arg(request.arguments.size() >= 2 ? request.arguments.at(1) :
                                             QString())
        .arg(outputDirectoryPath)
        .arg(expectedUnifiedResultJsonPath)
        .arg(isValid() ? QString("true") : QString("false"))
        .arg(executesProcess ? QString("true") : QString("false"))
        .arg(importsIntoTonyLayers ? QString("true") : QString("false"));
}

BackendId
BasicPitchAdapterContract::backendId()
{
    return "basic_pitch";
}

QString
BasicPitchAdapterContract::cliCommandName()
{
    return "basic-pitch";
}

BackendManifest
BasicPitchAdapterContract::manifest() const
{
    BackendManifest manifest;
    manifest.contractVersion = "0.1";
    manifest.backendId = backendId();
    manifest.engineId = backendId();
    manifest.displayName = "Basic Pitch";
    manifest.description =
        "Conservative contract for the Spotify Basic Pitch Python CLI. "
        "This boundary does not execute Basic Pitch or import layers.";
    manifest.backendType = BackendRuntimeType::PythonCli;
    manifest.runtimeType = BackendRuntimeType::PythonCli;
    manifest.executablePath = cliCommandName();
    manifest.status = BackendStatus::NotConfigured;

    manifest.capabilities.supportsFullFile = true;
    manifest.capabilities.supportsSelectedRegion = false;
    manifest.capabilities.outputsNotes = true;
    manifest.capabilities.outputsPitchCurve = false;
    manifest.capabilities.outputsPitchBends = true;
    manifest.capabilities.outputsTechniqueLabels = false;
    manifest.capabilities.requiresPython = true;
    manifest.capabilities.requiresModelCheckpoint = false;
    manifest.capabilities.supportsCpu = true;
    manifest.capabilities.supportsGpuOptional = false;

    manifest.supportedInputFormats
        << "mp3" << "ogg" << "wav" << "flac" << "m4a";
    manifest.primaryOutputs << "midi" << "note_events_csv";
    manifest.optionalOutputs
        << "model_output_npz" << "pitch_bends" << "warnings";
    manifest.supportedOutputTypes =
        manifest.primaryOutputs + manifest.optionalOutputs;

    manifest.defaultSettings.insert("cli_command", cliCommandName());
    manifest.defaultSettings.insert("possible_polyphony", true);
    manifest.defaultSettings.insert("monophonic_guaranteed", false);
    manifest.defaultSettings.insert("pitch_bend_tony_mapping_proven", false);
    manifest.defaultSettings.insert("confidence_tony_mapping_proven", false);
    manifest.defaultSettings.insert("creates_unified_result_directly", false);
    manifest.defaultSettings.insert("supports_selected_region_directly", false);

    manifest.licenseName = "Apache-2.0";
    manifest.licenseSourceUrl = "https://github.com/spotify/basic-pitch";

    return manifest;
}

QStringList
BasicPitchAdapterContract::contractWarnings() const
{
    return {
        "Basic Pitch output must be treated as potentially polyphonic.",
        "Pitch bends require a future Tony mapping or explicit warning.",
        "Velocity/confidence fields require preservation or explicit warning.",
        "The Basic Pitch CLI does not directly emit UnifiedResult JSON.",
        "This contract never marks Basic Pitch Ready, Installed, or Completed."
    };
}

BasicPitchAdapterContractResult
BasicPitchAdapterContract::buildCliRequest(
    const BasicPitchAdapterContractParameters &parameters) const
{
    BasicPitchAdapterContractResult result;
    result.manifest = manifest();
    result.outputDirectoryPath = trimmed(parameters.outputDirectoryPath);
    result.expectedUnifiedResultJsonPath =
        trimmed(parameters.expectedUnifiedResultJsonPath);
    result.expectedOutputArtifacts
        << "midi"
        << "note_events_csv"
        << "model_output_npz";

    const QString executablePath = trimmed(parameters.executablePath);
    const QString inputAudioPath = trimmed(parameters.inputAudioPath);
    const QString modelSerialization = trimmed(parameters.modelSerialization);

    if (executablePath.isEmpty()) {
        result.report.addError("empty_basic_pitch_executable_path",
                               "Basic Pitch executable path is empty.");
    }
    if (inputAudioPath.isEmpty()) {
        result.report.addError("empty_basic_pitch_input_audio_path",
                               "Basic Pitch input audio path is empty.");
    }
    if (result.outputDirectoryPath.isEmpty()) {
        result.report.addError("empty_basic_pitch_output_directory",
                               "Basic Pitch output directory path is empty.");
    }
    if (result.expectedUnifiedResultJsonPath.isEmpty()) {
        result.report.addError(
            "empty_basic_pitch_unified_result_path",
            "Expected UnifiedResult JSON path is empty.");
    }
    if (parameters.timeoutMsec < 0) {
        result.report.addError("invalid_basic_pitch_timeout",
                               "Basic Pitch request timeout must not be negative.");
    }
    if (!modelSerialization.isEmpty() &&
        !isSupportedModelSerialization(modelSerialization)) {
        result.report.addError(
            "unsupported_basic_pitch_model_serialization",
            "Basic Pitch model serialization must be tf, coreml, tflite, or onnx.");
    }

    addWarning(result.report,
               "basic_pitch_possible_polyphony",
               "Basic Pitch output is potentially polyphonic and must not be "
               "blindly forced into one monophonic Tony note layer.");
    addWarning(result.report,
               "basic_pitch_pitch_bend_mapping_deferred",
               "Basic Pitch pitch bends are represented by the backend, but "
               "Tony pitch-bend layer mapping is not yet proven.");
    addWarning(result.report,
               "basic_pitch_unified_result_conversion_deferred",
               "Basic Pitch CLI artifacts still require a future converter "
               "before a UnifiedResult can be loaded.");

    result.request.executablePath = executablePath;
    result.request.workingDirectory = trimmed(parameters.workingDirectoryPath);
    result.request.environmentOverrides = parameters.environmentOverrides;
    result.request.timeoutMsec = parameters.timeoutMsec;

    if (!result.outputDirectoryPath.isEmpty()) {
        result.request.arguments << result.outputDirectoryPath;
    }
    if (!inputAudioPath.isEmpty()) {
        result.request.arguments << inputAudioPath;
    }
    if (parameters.saveNoteEvents) {
        result.request.arguments << "--save-note-events";
    }
    if (parameters.saveModelOutputs) {
        result.request.arguments << "--save-model-outputs";
    }
    if (parameters.requestMultiplePitchBends) {
        result.request.arguments << "--multiple-pitch-bends";
    }
    if (!trimmed(parameters.modelPath).isEmpty()) {
        result.request.arguments << "--model-path" << trimmed(parameters.modelPath);
    }
    if (!modelSerialization.isEmpty()) {
        result.request.arguments
            << "--model-serialization" << modelSerialization;
    }

    return result;
}

bool
BasicPitchAdapterContract::isSupportedModelSerialization(const QString &value)
{
    return value == "tf" ||
        value == "coreml" ||
        value == "tflite" ||
        value == "onnx";
}

}
}
