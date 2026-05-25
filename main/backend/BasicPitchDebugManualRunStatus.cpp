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

#include "BasicPitchDebugManualRunStatus.h"

#include <QDir>

namespace Tony {
namespace Backend {

namespace {

QString
trimmed(const QString &value)
{
    return value.trimmed();
}

QString
boolString(bool value)
{
    return value ? QString("true") : QString("false");
}

void
addIfMissing(QStringList &values, const QString &value)
{
    if (!values.contains(value)) {
        values << value;
    }
}

}

bool
BasicPitchDebugManualRunStatusResult::isValid() const
{
    return report.isValid() &&
        !productionTranscription &&
        testOnlyDebugOnly &&
        !readyInstalledCompletedMutation;
}

QString
BasicPitchDebugManualRunStatusResult::debugSummaryString() const
{
    return QString("basic_pitch_debug_manual_run_status opt_in=%1 "
                   "command_configured=%2 audio_configured=%3 "
                   "output_configured=%4 result_configured=%5 "
                   "result_derived=%6 allowed=%7 skipped=%8 reason=%9 "
                   "missing=%10 production=%11 debug=%12 "
                   "ready_mutation=%13 valid=%14")
        .arg(boolString(explicitOptIn))
        .arg(boolString(commandConfigured))
        .arg(boolString(inputAudioConfigured))
        .arg(boolString(outputDirectoryConfigured))
        .arg(boolString(resultJsonConfigured))
        .arg(boolString(resultJsonWillBeDerived))
        .arg(boolString(manualRunAllowed))
        .arg(boolString(manualRunWouldBeSkipped))
        .arg(skippedReason.isEmpty() ? QString("none") : skippedReason)
        .arg(missingConfigurationKeys.join(","))
        .arg(boolString(productionTranscription))
        .arg(boolString(testOnlyDebugOnly))
        .arg(boolString(readyInstalledCompletedMutation))
        .arg(boolString(isValid()));
}

BasicPitchDebugManualRunStatusResult
BasicPitchDebugManualRunStatus::fromConfig(
    const BasicPitchRealRunHandoffProofConfig &config) const
{
    BasicPitchDebugManualRunStatusResult result;
    result.requiredEnvironmentKeys = requiredEnvironmentKeys();
    result.optionalEnvironmentKeys = optionalEnvironmentKeys();
    result.explicitOptIn = config.discoveryConfig.explicitOptIn;
    result.commandPath = trimmed(config.discoveryConfig.executablePath);
    result.inputAudioPath = trimmed(config.discoveryConfig.inputAudioPath);
    result.outputDirectoryPath =
        trimmed(config.discoveryConfig.outputDirectoryPath);
    result.resultJsonPath = trimmed(config.resultJsonPath);

    result.commandConfigured = !result.commandPath.isEmpty();
    result.inputAudioConfigured = !result.inputAudioPath.isEmpty();
    result.outputDirectoryConfigured = !result.outputDirectoryPath.isEmpty();
    result.resultJsonConfigured = !result.resultJsonPath.isEmpty();
    result.derivedResultJsonPath =
        defaultResultJsonPath(result.outputDirectoryPath);
    result.resultJsonWillBeDerived =
        !result.resultJsonConfigured &&
        !result.derivedResultJsonPath.isEmpty();

    if (!result.explicitOptIn) {
        addIfMissing(result.missingConfigurationKeys,
                     "TONY_BASIC_PITCH_DISCOVERY_ENABLE=1");
    }
    if (!result.commandConfigured) {
        addIfMissing(result.missingConfigurationKeys,
                     "TONY_BASIC_PITCH_COMMAND");
    }
    if (!result.inputAudioConfigured) {
        addIfMissing(result.missingConfigurationKeys,
                     "TONY_BASIC_PITCH_TEST_AUDIO");
    }
    if (!result.outputDirectoryConfigured) {
        addIfMissing(result.missingConfigurationKeys,
                     "TONY_BASIC_PITCH_OUTPUT_DIR");
    }

    result.manualRunAllowed =
        result.explicitOptIn &&
        result.commandConfigured &&
        result.inputAudioConfigured &&
        result.outputDirectoryConfigured;
    result.manualRunWouldBeSkipped = !result.manualRunAllowed;

    if (!result.explicitOptIn) {
        result.skippedReason = "explicit_opt_in_required";
    } else if (!result.commandConfigured) {
        result.skippedReason = "missing_command";
    } else if (!result.inputAudioConfigured) {
        result.skippedReason = "missing_audio_path";
    } else if (!result.outputDirectoryConfigured) {
        result.skippedReason = "missing_output_directory";
    }

    result.productionTranscription = false;
    result.testOnlyDebugOnly = true;
    result.readyInstalledCompletedMutation = false;
    result.report.addIssue(
        ValidationSeverity::Warning,
        "basic_pitch_debug_manual_run_status_debug_only",
        "Basic Pitch manual-run configuration status is debug/test-only "
        "and does not prove production transcription.");

    return result;
}

QStringList
BasicPitchDebugManualRunStatus::requiredEnvironmentKeys()
{
    return QStringList()
        << "TONY_BASIC_PITCH_DISCOVERY_ENABLE=1"
        << "TONY_BASIC_PITCH_COMMAND"
        << "TONY_BASIC_PITCH_TEST_AUDIO"
        << "TONY_BASIC_PITCH_OUTPUT_DIR";
}

QStringList
BasicPitchDebugManualRunStatus::optionalEnvironmentKeys()
{
    return QStringList() << "TONY_BASIC_PITCH_RESULT_JSON";
}

QString
BasicPitchDebugManualRunStatus::defaultResultJsonPath(
    const QString &outputDirectoryPath)
{
    const QString outputDirectory = trimmed(outputDirectoryPath);
    if (outputDirectory.isEmpty()) {
        return QString();
    }
    return QDir(outputDirectory).filePath("basic_pitch_result.json");
}

}
}
