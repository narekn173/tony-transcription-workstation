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

#include "BasicPitchDebugCombinedRunImportActionReportFormatter.h"

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

QString
proofStatus(bool tested, bool passed)
{
    if (!tested) {
        return QString("not tested by this action");
    }
    return passed ? QString("passed") : QString("failed");
}

}

bool
BasicPitchDebugCombinedRunImportActionReportText::isValid() const
{
    return !title.trimmed().isEmpty() &&
        !plainText.trimmed().isEmpty() &&
        !productionTranscription &&
        testOnlyDebugOnly &&
        !readyInstalledCompletedMutation;
}

BasicPitchDebugCombinedRunImportActionReportText
BasicPitchDebugCombinedRunImportActionReportFormatter::fromReport(
    const BasicPitchDebugCombinedRunImportActionReport &report) const
{
    BasicPitchDebugCombinedRunImportActionReportText result;
    result.title = "Debug: Run Basic Pitch Manual Handoff and Import";
    result.productionTranscription = report.productionTranscription;
    result.testOnlyDebugOnly = report.testOnlyDebugOnly;
    result.importedIntoTonyLayers = report.importedIntoTonyLayers;
    result.insertedIntoView = report.insertedIntoView;
    result.readyInstalledCompletedMutation =
        report.readyInstalledCompletedMutation;

    QStringList lines;
    lines << "Basic Pitch Debug Manual Handoff and Import";
    lines << "";
    lines << "Summary:";
    appendBoolLine(lines, "Debug/test-only", report.testOnlyDebugOnly);
    appendBoolLine(lines,
                   "Production transcription",
                   report.productionTranscription);
    appendBoolLine(lines,
                   "Ready / Installed / Completed mutation",
                   report.readyInstalledCompletedMutation);
    appendBoolLine(lines, "Manual run allowed", report.manualRunAllowed);
    appendBoolLine(lines, "Manual run attempted", report.manualRunAttempted);
    appendBoolLine(lines, "Backend process ran", report.ranBasicPitch);
    appendBoolLine(lines, "Result JSON written", report.resultJsonWritten);
    appendBoolLine(lines, "UnifiedResult loaded", report.loadedResult);
    appendBoolLine(lines, "Import attempted", report.importAttempted);
    appendBoolLine(lines,
                   "Imported into Tony layers",
                   report.importedIntoTonyLayers);
    appendBoolLine(lines,
                   "Inserted into View/Pane",
                   report.insertedIntoView);
    if (!report.ranBasicPitch) {
        lines << "No backend was run.";
    }
    if (!report.importedIntoTonyLayers) {
        lines << "No Tony layer was imported.";
    }
    lines << "";

    lines << "Manual Handoff:";
    appendLine(lines, "Command", displayValue(report.commandUsed));
    appendLine(lines,
               "Input audio path",
               displayValue(report.inputAudioPath));
    appendLine(lines,
               "Output directory",
               displayValue(report.outputDirectoryPath));
    appendLine(lines,
               "Result JSON path",
               displayValue(report.resultJsonPath));
    appendLine(lines,
               "Selected csv_note_events artifact",
               displayValue(report.selectedNoteEventsArtifactPath));
    appendList(lines,
               "Discovered artifacts",
               artifactSummaries(report.discoveredArtifacts));
    lines << "";

    lines << "Import Target:";
    appendBoolLine(lines, "Document provided", report.documentProvided);
    appendBoolLine(lines, "View/Pane provided", report.viewProvided);
    appendBoolLine(lines,
                   "View insertion requested",
                   report.viewInsertionRequested);
    appendLine(lines, "Note count", QString::number(report.noteCount));
    lines << "";

    lines << "Proof Status:";
    appendLine(lines,
               "Edit proof",
               proofStatus(report.editProofTested,
                           report.editProofPassed));
    appendLine(lines,
               "Save/load proof",
               proofStatus(report.saveLoadProofTested,
                           report.saveLoadProofPassed));
    appendLine(lines,
               "Export proof",
               proofStatus(report.exportProofTested,
                           report.exportProofPassed));
    lines << "";

    lines << "Warnings and Policy:";
    appendBoolLine(lines, "Possible polyphony", report.possiblePolyphony);
    appendBoolLine(lines,
                   "Pitch bend mapping deferred",
                   report.pitchBendMappingDeferred);
    appendList(lines, "Warnings", report.warningCodes);
    appendList(lines, "Errors", report.errorCodes);
    lines << "";

    lines << "Workflow Stage Sequence:";
    appendList(lines, "Stage states", report.stageStates);
    lines << "";

    lines << "Limitations:";
    lines << "  - This is debug/test-only UI.";
    lines << "  - This is not production Basic Pitch transcription.";
    lines << "  - Basic Pitch runs only when explicit manual opt-in is configured.";
    lines << "  - Import uses only a real result.json and real Tony/Sonic Visualiser layer APIs.";
    lines << "  - Edit/save/load/export proofs are reported as not tested here.";
    lines << "  - No Ready, Installed, or Completed global state is changed.";
    lines << "  - No percentage progress is reported.";
    lines << "";
    appendLine(lines, "Debug summary", report.debugSummaryString());

    result.plainText = lines.join("\n");
    return result;
}

}
}
