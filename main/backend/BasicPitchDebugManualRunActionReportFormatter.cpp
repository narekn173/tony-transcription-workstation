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

#include "BasicPitchDebugManualRunActionReportFormatter.h"

namespace Tony {
namespace Backend {

namespace {

QString
boolString(bool value)
{
    return value ? QString("true") : QString("false");
}

QString
displayValue(const QString &value)
{
    const QString trimmed = value.trimmed();
    return trimmed.isEmpty() ? QString("(not provided)") : trimmed;
}

void
appendLine(QStringList &lines, const QString &label, const QString &value)
{
    lines << QString("%1: %2").arg(label, value);
}

void
appendBoolLine(QStringList &lines, const QString &label, bool value)
{
    appendLine(lines, label, boolString(value));
}

void
appendList(QStringList &lines,
           const QString &label,
           const QStringList &values)
{
    lines << QString("%1:").arg(label);
    if (values.isEmpty()) {
        lines << "  (none)";
        return;
    }
    for (const QString &value: values) {
        lines << QString("  - %1").arg(value);
    }
}

QStringList
artifactSummaries(const QVector<BasicPitchDiscoveredArtifact> &artifacts)
{
    QStringList values;
    for (const BasicPitchDiscoveredArtifact &artifact: artifacts) {
        values << QString("%1 type=%2 bytes=%3 path=%4")
            .arg(artifact.fileName)
            .arg(artifact.artifactType)
            .arg(artifact.sizeBytes)
            .arg(artifact.path);
    }
    return values;
}

}

bool
BasicPitchDebugManualRunActionReportText::isValid() const
{
    return !title.trimmed().isEmpty() &&
        !plainText.trimmed().isEmpty() &&
        !productionTranscription &&
        testOnlyDebugOnly &&
        !importedIntoTonyLayers &&
        !readyInstalledCompletedMutation;
}

BasicPitchDebugManualRunActionReportText
BasicPitchDebugManualRunActionReportFormatter::fromReport(
    const BasicPitchDebugManualRunActionReport &report) const
{
    BasicPitchDebugManualRunActionReportText result;
    result.title = "Debug: Run Basic Pitch Manual Handoff Proof";
    result.productionTranscription = report.productionTranscription;
    result.testOnlyDebugOnly = report.testOnlyDebugOnly;
    result.importedIntoTonyLayers = report.importedIntoTonyLayers;
    result.readyInstalledCompletedMutation =
        report.readyInstalledCompletedMutation;

    QStringList lines;
    lines << "Basic Pitch Debug Manual Handoff Proof";
    lines << "";
    lines << "Summary:";
    appendBoolLine(lines, "Debug/test-only", report.testOnlyDebugOnly);
    appendBoolLine(lines,
                   "Production transcription",
                   report.productionTranscription);
    appendBoolLine(lines,
                   "Ready / Installed / Completed mutation",
                   report.readyInstalledCompletedMutation);
    appendBoolLine(lines,
                   "Imported into Tony layers",
                   report.importedIntoTonyLayers);
    appendBoolLine(lines,
                   "Manual run attempted",
                   report.manualRunAttempted);
    appendBoolLine(lines,
                   "Backend process ran",
                   report.ranBasicPitch);
    if (!report.ranBasicPitch) {
        lines << "No backend was run.";
    }
    lines << "";

    lines << "Preflight:";
    appendBoolLine(lines,
                   "Manual real-run opt-in",
                   report.preflightStatus.explicitOptIn);
    appendBoolLine(lines,
                   "Command configured",
                   report.preflightStatus.commandConfigured);
    appendLine(lines,
               "Command",
               displayValue(report.commandUsed));
    appendBoolLine(lines,
                   "Input audio configured",
                   report.preflightStatus.inputAudioConfigured);
    appendLine(lines,
               "Input audio path",
               displayValue(report.inputAudioPath));
    appendBoolLine(lines,
                   "Output directory configured",
                   report.preflightStatus.outputDirectoryConfigured);
    appendLine(lines,
               "Output directory",
               displayValue(report.outputDirectoryPath));
    appendBoolLine(lines,
                   "Result JSON path configured",
                   report.preflightStatus.resultJsonConfigured);
    appendBoolLine(lines,
                   "Result JSON will be derived",
                   report.preflightStatus.resultJsonWillBeDerived);
    appendLine(lines,
               "Result JSON path",
               displayValue(report.resultJsonPath));
    appendList(lines,
               "Required manual-run env/config keys",
               report.preflightStatus.requiredEnvironmentKeys);
    appendList(lines,
               "Optional manual-run env/config keys",
               report.preflightStatus.optionalEnvironmentKeys);
    appendList(lines,
               "Missing manual-run env/config keys",
               report.preflightStatus.missingConfigurationKeys);
    appendBoolLine(lines,
                   "Manual run allowed",
                   report.preflightStatus.manualRunAllowed);
    appendBoolLine(lines,
                   "Manual run would be skipped",
                   report.preflightStatus.manualRunWouldBeSkipped);
    appendLine(lines,
               "Skipped reason",
               displayValue(report.preflightStatus.skippedReason));
    if (report.preflightStatus.manualRunWouldBeSkipped) {
        lines << "Manual Basic Pitch run would be skipped from this configuration.";
    }
    lines << "";

    lines << "Run Result:";
    appendLine(lines, "Stages", report.stageStates.join(","));
    appendBoolLine(lines, "Preflight passed", report.preflightPassed);
    appendBoolLine(lines, "Result JSON written", report.resultJsonWritten);
    appendBoolLine(lines, "UnifiedResult loaded", report.loadedResult);
    appendLine(lines,
               "Selected csv_note_events artifact",
               displayValue(report.selectedNoteEventsArtifactPath));
    appendLine(lines, "Note count", QString::number(report.noteCount));
    appendBoolLine(lines, "Possible polyphony", report.possiblePolyphony);
    appendBoolLine(lines,
                   "Pitch bend mapping deferred",
                   report.pitchBendMappingDeferred);
    appendList(lines, "Discovered artifacts",
               artifactSummaries(report.discoveredArtifacts));
    lines << "";

    appendList(lines, "Warnings", report.warningCodes);
    appendList(lines, "Errors", report.errorCodes);
    lines << "";

    lines << "Limitations:";
    lines << "  - This is debug/test-only UI.";
    lines << "  - This is not production Basic Pitch transcription.";
    lines << "  - This action stops at result handoff and loading.";
    lines << "  - No Tony layer import is performed by this action.";
    lines << "  - No Ready, Installed, or Completed global state is changed.";
    lines << "  - No percentage progress is reported.";
    lines << "";
    appendLine(lines, "Debug summary", report.debugSummaryString());

    result.plainText = lines.join("\n");
    return result;
}

}
}
