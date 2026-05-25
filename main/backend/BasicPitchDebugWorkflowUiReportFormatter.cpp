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

#include "BasicPitchDebugWorkflowUiReportFormatter.h"

namespace Tony {
namespace Backend {

namespace {

QString
boolString(bool value)
{
    return value ? QString("true") : QString("false");
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

}

bool
BasicPitchDebugWorkflowUiReportText::isValid() const
{
    return !title.trimmed().isEmpty() &&
        !plainText.trimmed().isEmpty() &&
        !productionTranscription &&
        testOnlyDebugOnly &&
        !readyInstalledCompletedMutation;
}

BasicPitchDebugWorkflowUiReportText
BasicPitchDebugWorkflowUiReportFormatter::fromUiModel(
    const BasicPitchDebugWorkflowUiModelResult &uiModel) const
{
    BasicPitchDebugWorkflowUiReportText result;
    result.title = "Debug: Basic Pitch Workflow Proof";
    result.productionTranscription = uiModel.productionTranscription;
    result.testOnlyDebugOnly = uiModel.testOnlyDebugOnly;
    result.readyInstalledCompletedMutation =
        uiModel.readyInstalledCompletedMutation;

    QStringList lines;
    lines << "Basic Pitch Debug Workflow Proof";
    lines << "";
    appendBoolLine(lines, "Debug/test-only", uiModel.testOnlyDebugOnly);
    appendBoolLine(lines,
                   "Production transcription",
                   uiModel.productionTranscription);
    appendBoolLine(lines,
                   "Ready / Installed / Completed mutation",
                   uiModel.readyInstalledCompletedMutation);
    lines << "";
    appendLine(lines, "Primary state", uiModel.primaryState);
    appendLine(lines, "Secondary state", uiModel.secondaryState);
    appendLine(lines, "User message", uiModel.userMessage);
    appendLine(lines, "Technical message", uiModel.technicalMessage);
    lines << "";
    lines << "Progress: stage-based only; no percentage progress is reported.";
    lines << "";

    appendList(lines, "Warnings", uiModel.warnings);
    appendList(lines, "Errors", uiModel.errors);
    lines << "";

    lines << "Action gates:";
    appendBoolLine(lines, "  Can run", uiModel.canRun);
    appendBoolLine(lines, "  Can cancel", uiModel.canCancel);
    appendBoolLine(lines, "  Can import", uiModel.canImport);
    appendBoolLine(lines, "  Can edit", uiModel.canEdit);
    appendBoolLine(lines, "  Can save", uiModel.canSave);
    appendBoolLine(lines, "  Can export", uiModel.canExport);
    lines << "";

    lines << "Proof bundle:";
    appendLine(lines,
               "  Event states",
               uiModel.proofBundle.eventStates.join(","));
    appendLine(lines,
               "  Event count",
               QString::number(uiModel.proofBundle.eventCount));
    appendLine(lines,
               "  Artifact count",
               QString::number(uiModel.proofBundle.artifactCount));
    appendLine(lines, "  Result JSON", uiModel.proofBundle.resultJsonPath);
    appendLine(lines, "  Export CSV", uiModel.proofBundle.exportCsvPath);
    appendLine(lines,
               "  Note count",
               QString::number(uiModel.proofBundle.noteCount));
    appendBoolLine(lines,
                   "  Loaded result",
                   uiModel.proofBundle.loadedResult);
    appendBoolLine(lines,
                   "  Imported into Tony layers",
                   uiModel.proofBundle.importedIntoTonyLayers);
    appendBoolLine(lines,
                   "  Inserted into View/Pane",
                   uiModel.proofBundle.insertedIntoView);
    appendBoolLine(lines, "  Edit proof", uiModel.proofBundle.editProof);
    appendBoolLine(lines,
                   "  Save/load proof",
                   uiModel.proofBundle.saveLoadProof);
    appendBoolLine(lines,
                   "  Export proof",
                   uiModel.proofBundle.exportProof);
    appendBoolLine(lines,
                   "  Production transcription",
                   uiModel.proofBundle.productionTranscription);
    appendBoolLine(lines,
                   "  Test-only/debug-only",
                   uiModel.proofBundle.testOnlyDebugOnly);
    lines << "";
    appendLine(lines, "Proof summary", uiModel.proofBundleSummary);

    result.plainText = lines.join("\n");
    return result;
}

}
}
