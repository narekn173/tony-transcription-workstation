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

#include "BasicPitchDebugPostImportProofActionReportFormatter.h"

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
        return QString("not tested");
    }
    return passed ? QString("passed") : QString("failed");
}

}

bool
BasicPitchDebugPostImportProofActionReportText::isValid() const
{
    return !title.trimmed().isEmpty() &&
        !plainText.trimmed().isEmpty() &&
        !productionTranscription &&
        testOnlyDebugOnly &&
        !readyInstalledCompletedMutation;
}

BasicPitchDebugPostImportProofActionReportText
BasicPitchDebugPostImportProofActionReportFormatter::fromReport(
    const BasicPitchDebugPostImportProofActionReport &report) const
{
    BasicPitchDebugPostImportProofActionReportText result;
    result.title = "Debug: Basic Pitch Post-Import Edit/Save/Export Proof";
    result.productionTranscription = report.productionTranscription;
    result.testOnlyDebugOnly = report.testOnlyDebugOnly;
    result.readyInstalledCompletedMutation =
        report.readyInstalledCompletedMutation;

    QStringList lines;
    lines << "Basic Pitch Debug Post-Import Edit/Save/Export Proof";
    lines << "";
    lines << "Summary:";
    appendBoolLine(lines, "Debug/test-only", report.testOnlyDebugOnly);
    appendBoolLine(lines,
                   "Production transcription",
                   report.productionTranscription);
    appendBoolLine(lines,
                   "Ready / Installed / Completed mutation",
                   report.readyInstalledCompletedMutation);
    appendLine(lines, "Result JSON path", displayValue(report.resultJsonPath));
    appendLine(lines, "Export CSV path", displayValue(report.exportCsvPath));
    appendBoolLine(lines, "UnifiedResult loaded", report.loadedResult);
    appendLine(lines, "Note count", QString::number(report.noteCount));
    appendLine(lines,
               "Exported note rows",
               QString::number(report.exportedNoteCount));
    lines << "";

    lines << "Real Tony/Sonic Visualiser Layer Proof:";
    appendBoolLine(lines, "Real NoteModel exists", report.realNoteModelExists);
    appendBoolLine(lines, "Real NoteLayer exists", report.realNoteLayerExists);
    appendBoolLine(lines, "Document-owned layer", report.documentOwnedLayer);
    appendBoolLine(lines, "Inserted into View/Pane", report.insertedIntoView);
    appendBoolLine(lines, "Layer editable", report.layerEditable);
    lines << "";

    lines << "Edit / Save / Export Proof:";
    appendLine(lines,
               "Edit proof",
               proofStatus(report.editProofTested,
                           report.editProofPassed));
    appendLine(lines,
               "Undo/redo proof",
               proofStatus(report.undoRedoProofTested,
                           report.undoRedoProofPassed));
    appendLine(lines,
               "Save/load proof",
               proofStatus(report.saveLoadProofTested,
                           report.saveLoadProofPassed));
    appendLine(lines,
               "CSV export proof",
               proofStatus(report.exportProofTested,
                           report.exportProofPassed));
    appendBoolLine(lines,
                   "Exported CSV non-empty",
                   report.exportedCsvNonEmpty);
    appendBoolLine(lines,
                   "Export preserved timing/duration/pitch/velocity",
                   report.exportedTimingDurationPitchVelocity);
    lines << "";

    lines << "Warnings and Policy:";
    appendBoolLine(lines, "Possible polyphony", report.possiblePolyphony);
    appendBoolLine(lines,
                   "Pitch bend mapping deferred",
                   report.pitchBendMappingDeferred);
    appendList(lines, "Warnings", report.warningCodes);
    appendList(lines, "Errors", report.errorCodes);
    lines << "";

    lines << "Limitations:";
    lines << "  - This is debug/test-only proof.";
    lines << "  - This is not production Basic Pitch transcription.";
    lines << "  - The proof reuses the real importer and real Tony/Sonic Visualiser persistence/export APIs.";
    lines << "  - No Ready, Installed, or Completed global state is changed.";
    lines << "  - No percentage progress is reported.";
    lines << "";
    appendLine(lines, "Debug summary", report.debugSummaryString());

    result.plainText = lines.join("\n");
    return result;
}

}
}
