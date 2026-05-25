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
    lines << "Summary:";
    appendBoolLine(lines, "Debug/test-only", uiModel.testOnlyDebugOnly);
    appendBoolLine(lines,
                   "Production transcription",
                   uiModel.productionTranscription);
    appendBoolLine(lines,
                   "Ready / Installed / Completed mutation",
                   uiModel.readyInstalledCompletedMutation);
    appendBoolLine(lines,
                   "Backend process ran",
                   uiModel.proofBundle.ranBasicPitch);
    if (!uiModel.proofBundle.ranBasicPitch) {
        lines << "No backend was run.";
    }
    lines << "";

    lines << "Configuration:";
    appendLine(lines, "Mode", uiModel.proofBundle.workflowMode);
    appendBoolLine(lines,
                   "Manual real-run opt-in",
                   uiModel.proofBundle.realRunExplicitOptIn);
    appendBoolLine(lines,
                   "Command configured",
                   uiModel.proofBundle.commandConfigured);
    appendLine(lines,
               "Command",
               displayValue(uiModel.proofBundle.commandUsed));
    appendBoolLine(lines,
                   "Input audio configured",
                   uiModel.proofBundle.inputAudioConfigured);
    appendLine(lines,
               "Input audio path",
               displayValue(uiModel.proofBundle.inputAudioPath));
    appendBoolLine(lines,
                   "Output directory configured",
                   uiModel.proofBundle.outputDirectoryConfigured);
    appendLine(lines,
               "Output directory",
               displayValue(uiModel.proofBundle.outputDirectoryPath));
    appendBoolLine(lines,
                   "Result JSON path configured",
                   uiModel.proofBundle.resultJsonConfigured);
    appendLine(lines,
               "Result JSON path",
               displayValue(uiModel.proofBundle.resultJsonPath));
    appendList(lines,
               "Required manual-run env/config keys",
               uiModel.proofBundle.requiredManualRunEnvironmentKeys);
    appendList(lines,
               "Optional manual-run env/config keys",
               uiModel.proofBundle.optionalManualRunEnvironmentKeys);
    appendList(lines,
               "Missing manual-run env/config keys",
               uiModel.proofBundle.missingManualRunConfigurationKeys);
    appendBoolLine(lines,
                   "Manual run allowed",
                   uiModel.proofBundle.manualRunAllowed);
    appendBoolLine(lines,
                   "Manual run would be skipped",
                   uiModel.proofBundle.manualRunWouldBeSkipped);
    appendLine(lines,
               "Manual run skipped reason",
               displayValue(uiModel.proofBundle.manualRunSkippedReason));
    appendBoolLine(lines,
                   "Result JSON will be derived",
                   uiModel.proofBundle.manualRunResultJsonWillBeDerived);
    appendLine(lines,
               "Derived result JSON path",
               displayValue(uiModel.proofBundle.manualRunDerivedResultJsonPath));
    if (uiModel.proofBundle.manualRunWouldBeSkipped) {
        lines << "Manual Basic Pitch run would be skipped from this configuration.";
    }
    if (!uiModel.proofBundle.manualRunAllowed) {
        lines << "Basic Pitch debug workflow is not configured.";
        lines << "No backend was run unless a separate proof report says otherwise.";
        lines << "This is not production transcription.";
    }
    appendLine(lines,
               "Skipped reason",
               displayValue(uiModel.proofBundle.skippedReason));
    if (uiModel.primaryState == "backend_not_configured" ||
        uiModel.primaryState == "skipped") {
        lines << "Basic Pitch debug workflow is not configured.";
    }
    if (uiModel.proofBundle.workflowMode == "synthetic_artifact_only") {
        lines << "Synthetic/test-only: true";
        lines << "Synthetic/test-only workflow data is not real audio transcription.";
    }
    lines << "";

    lines << "Workflow State:";
    appendLine(lines, "Primary state", uiModel.primaryState);
    appendLine(lines, "Secondary state", uiModel.secondaryState);
    appendLine(lines, "User message", uiModel.userMessage);
    appendLine(lines, "Technical message", uiModel.technicalMessage);
    lines << "";
    lines << "Progress: stage-based only; no percentage progress is reported.";
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
    appendBoolLine(lines,
                   "  Selected csv_note_events artifact found",
                   uiModel.proofBundle.selectedNoteEventsArtifactFound);
    appendLine(lines,
               "  Selected csv_note_events artifact",
               displayValue(uiModel.proofBundle.selectedNoteEventsArtifactPath));
    appendLine(lines,
               "  Result JSON",
               displayValue(uiModel.proofBundle.resultJsonPath));
    appendLine(lines,
               "  Export CSV",
               displayValue(uiModel.proofBundle.exportCsvPath));
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
    appendList(lines, "  Artifacts", uiModel.proofBundle.artifactSummaries);
    lines << "";
    appendLine(lines, "Proof summary", uiModel.proofBundleSummary);
    lines << "";

    appendList(lines, "Warnings", uiModel.warnings);
    appendList(lines, "Errors", uiModel.errors);
    lines << "";

    lines << "Limitations:";
    lines << "  - This is debug/test-only UI.";
    lines << "  - This is not production Basic Pitch transcription.";
    lines << "  - No Ready, Installed, or Completed global state is changed.";
    lines << "  - No percentage progress is reported.";

    result.plainText = lines.join("\n");
    return result;
}

}
}
