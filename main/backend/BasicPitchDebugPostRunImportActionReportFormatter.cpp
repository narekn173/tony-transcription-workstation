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

#include "BasicPitchDebugPostRunImportActionReportFormatter.h"

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
BasicPitchDebugPostRunImportActionReportText::isValid() const
{
    return !title.trimmed().isEmpty() &&
        !plainText.trimmed().isEmpty() &&
        !productionTranscription &&
        testOnlyDebugOnly &&
        !readyInstalledCompletedMutation;
}

BasicPitchDebugPostRunImportActionReportText
BasicPitchDebugPostRunImportActionReportFormatter::fromReport(
    const BasicPitchDebugPostRunImportActionReport &report) const
{
    BasicPitchDebugPostRunImportActionReportText result;
    result.title = "Debug: Import Basic Pitch Manual Result";
    result.productionTranscription = report.productionTranscription;
    result.testOnlyDebugOnly = report.testOnlyDebugOnly;
    result.importedIntoTonyLayers = report.importedIntoTonyLayers;
    result.insertedIntoView = report.insertedIntoView;
    result.readyInstalledCompletedMutation =
        report.readyInstalledCompletedMutation;

    QStringList lines;
    lines << "Basic Pitch Debug Post-Run Import";
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
                   "Result JSON path resolved",
                   report.resultJsonPathResolved);
    appendLine(lines,
               "Result JSON path",
               displayValue(report.resultJsonPath));
    appendBoolLine(lines, "UnifiedResult loaded", report.loadedResult);
    appendBoolLine(lines, "Basic Pitch-shaped result", report.basicPitchShaped);
    appendLine(lines, "Note count", QString::number(report.noteCount));
    appendBoolLine(lines,
                   "Imported into Tony layers",
                   report.importedIntoTonyLayers);
    appendBoolLine(lines,
                   "Inserted into View/Pane",
                   report.insertedIntoView);
    if (!report.importedIntoTonyLayers) {
        lines << "No Tony layer was imported.";
    }
    lines << "";

    lines << "Import Target:";
    appendBoolLine(lines, "Document provided", report.documentProvided);
    appendBoolLine(lines, "View/Pane provided", report.viewProvided);
    appendBoolLine(lines,
                   "View insertion requested",
                   report.viewInsertionRequested);
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

    lines << "Stages:";
    appendList(lines, "Stage states", report.stageStates);
    lines << "";

    lines << "Limitations:";
    lines << "  - This is debug/test-only UI.";
    lines << "  - This is not production Basic Pitch transcription.";
    lines << "  - This action imports only an existing result.json.";
    lines << "  - This action does not run Basic Pitch.";
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
