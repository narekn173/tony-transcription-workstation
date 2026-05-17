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

#include "TonyLayerImporter.h"

#include "base/Event.h"
#include "data/model/NoteModel.h"
#include "data/model/EventCommands.h"
#include "framework/Document.h"
#include "layer/Layer.h"
#include "layer/LayerFactory.h"
#include "view/View.h"
#include "widgets/CommandHistory.h"

#include <QFileInfo>
#include <QStringList>
#include <QtGlobal>

#include <cmath>
#include <memory>

namespace Tony {
namespace Backend {

namespace {

void
setFailure(TonyLayerImportResult &result,
           BackendErrorCode code,
           const QString &message,
           const QString &issueCode)
{
    result.succeeded = false;
    result.error = { code, message, true };
    result.report.addError(issueCode, message);
}

sv::sv_frame_t
secondsToFrame(double seconds, double sampleRate)
{
    return sv::sv_frame_t(std::llround(seconds * sampleRate));
}

bool
noteHasMidiPitch(const NoteEvent &note)
{
    return note.midiPitch.has_value();
}

bool
noteHasFrequency(const NoteEvent &note)
{
    return note.frequencyHz.has_value();
}

bool
resultUsesMidiPitch(const UnifiedResult &result,
                    TonyLayerImportResult &importResult)
{
    bool sawMidiPitch = false;
    bool sawFrequencyOnly = false;

    for (const auto &note: result.notes) {
        if (noteHasMidiPitch(note)) {
            sawMidiPitch = true;
        } else if (noteHasFrequency(note)) {
            sawFrequencyOnly = true;
        }
    }

    if (sawMidiPitch && sawFrequencyOnly) {
        setFailure(importResult,
                   BackendErrorCode::ValidationFailed,
                   "UnifiedResult notes mix MIDI-pitch and frequency-only "
                   "notes; a single Tony NoteModel needs one value unit.",
                   "mixed_note_pitch_units");
        return true;
    }

    return sawMidiPitch;
}

bool
validateImportableNote(const NoteEvent &note,
                       int index,
                       bool useMidiPitch,
                       TonyLayerImportResult &result)
{
    const QString prefix = QString("Note %1 ").arg(index);

    if (!note.isValidTimeRange() || note.startSec < 0.0) {
        setFailure(result,
                   BackendErrorCode::ValidationFailed,
                   prefix + "has an invalid time range.",
                   "invalid_note_time_range");
        return false;
    }

    if (useMidiPitch) {
        if (!note.hasValidMidiPitch()) {
            setFailure(result,
                       BackendErrorCode::ValidationFailed,
                       prefix + "has missing or invalid MIDI pitch.",
                       "invalid_note_midi_pitch");
            return false;
        }
    } else {
        if (!note.frequencyHz.has_value() || *note.frequencyHz <= 0.0) {
            setFailure(result,
                       BackendErrorCode::ValidationFailed,
                       prefix + "has no importable pitch value.",
                       "missing_note_pitch");
            return false;
        }
    }

    if (!note.hasValidVelocity()) {
        setFailure(result,
                   BackendErrorCode::ValidationFailed,
                   prefix + "has invalid velocity.",
                   "invalid_note_velocity");
        return false;
    }

    if (!note.hasValidConfidence()) {
        setFailure(result,
                   BackendErrorCode::ValidationFailed,
                   prefix + "has invalid confidence.",
                   "invalid_note_confidence");
        return false;
    }

    return true;
}

float
noteValue(const NoteEvent &note, bool useMidiPitch)
{
    return useMidiPitch ? float(*note.midiPitch) : float(*note.frequencyHz);
}

bool
noteLevel(const NoteEvent &note, float &level)
{
    if (note.velocity.has_value()) {
        level = qBound(0.0f, float(*note.velocity) / 127.0f, 1.0f);
        return true;
    }
    if (note.confidence.has_value()) {
        level = qBound(0.0f, float(*note.confidence), 1.0f);
        return true;
    }
    return false;
}

QString
noteLabel(const NoteEvent &note)
{
    return note.label.value_or(QString());
}

QString
cleanProvenanceValue(QString value)
{
    value = value.trimmed();
    value.replace('\n', ' ');
    value.replace('\r', ' ');
    value.replace(';', ',');
    value.replace('[', '(');
    value.replace(']', ')');
    return value.simplified();
}

QString
pathLeaf(const QString &path)
{
    const QString trimmed = path.trimmed();
    if (trimmed.isEmpty()) {
        return QString();
    }
    return cleanProvenanceValue(QFileInfo(trimmed).fileName());
}

void
appendProvenancePart(QStringList &parts,
                     const QString &key,
                     const QString &value)
{
    const QString cleaned = cleanProvenanceValue(value);
    if (!cleaned.isEmpty()) {
        parts << QString("%1=%2").arg(key).arg(cleaned);
    }
}

void
appendProvenanceFlag(QStringList &parts,
                     const QString &key,
                     bool value)
{
    if (value) {
        parts << QString("%1=true").arg(key);
    }
}

QString
formatRegionSeconds(double startSec, double endSec)
{
    return QString("%1-%2s")
        .arg(QString::number(startSec, 'f', 3))
        .arg(QString::number(endSec, 'f', 3));
}

QString
metadataValue(const QVariantMap &metadata, const QString &key)
{
    return metadata.value(key).toString().trimmed();
}

bool
viewContainsLayer(sv::View *view, sv::Layer *layer)
{
    if (!view || !layer) {
        return false;
    }

    for (int i = 0; i < view->getLayerCount(); ++i) {
        if (view->getLayer(i) == layer) {
            return true;
        }
    }

    return false;
}

QString
backendDisplayName(const UnifiedResult &result)
{
    if (!result.engine.displayName.trimmed().isEmpty()) {
        return result.engine.displayName.trimmed();
    }
    if (!result.engine.engineId.trimmed().isEmpty()) {
        return result.engine.engineId.trimmed();
    }
    return QString("Backend result");
}

QString
backendVersion(const UnifiedResult &result,
               const TonyLayerImportProvenance &provenance)
{
    if (!provenance.backendVersion.trimmed().isEmpty()) {
        return provenance.backendVersion.trimmed();
    }
    if (result.engine.engineVersion.has_value() &&
        !result.engine.engineVersion->trimmed().isEmpty()) {
        return result.engine.engineVersion->trimmed();
    }
    if (!result.engine.adapterVersion.trimmed().isEmpty()) {
        return result.engine.adapterVersion.trimmed();
    }
    return metadataValue(result.provenance, "backend_version");
}

QString
provenanceBackendId(const UnifiedResult &result,
                    const TonyLayerImportProvenance &provenance)
{
    if (!provenance.backendId.trimmed().isEmpty()) {
        return provenance.backendId.trimmed();
    }
    if (!result.engine.engineId.trimmed().isEmpty()) {
        return result.engine.engineId.trimmed();
    }
    return metadataValue(result.provenance, "backend_id");
}

QString
provenanceRunId(const UnifiedResult &result,
                const TonyLayerImportProvenance &provenance)
{
    if (!provenance.runId.trimmed().isEmpty()) {
        return provenance.runId.trimmed();
    }
    if (!result.requestId.trimmed().isEmpty()) {
        return result.requestId.trimmed();
    }
    return result.resultId.trimmed();
}

QString
provenanceDisplayName(const UnifiedResult &result,
                      const TonyLayerImportProvenance &provenance)
{
    if (!provenance.backendName.trimmed().isEmpty()) {
        return provenance.backendName.trimmed();
    }
    return backendDisplayName(result);
}

QString
durableProvenanceIdentity(const UnifiedResult &result,
                          const TonyLayerImportProvenance &provenance)
{
    const QString displayName =
        cleanProvenanceValue(provenanceDisplayName(result, provenance));

    QStringList parts;
    appendProvenancePart(parts, "backend",
                         provenanceBackendId(result, provenance));
    appendProvenancePart(parts, "version",
                         backendVersion(result, provenance));
    appendProvenancePart(parts, "run",
                         provenanceRunId(result, provenance));
    appendProvenancePart(parts, "result",
                         pathLeaf(provenance.resultJsonPath));
    appendProvenancePart(parts, "request",
                         pathLeaf(provenance.requestJsonPath));
    appendProvenancePart(parts, "input",
                         pathLeaf(provenance.inputAudioPath));

    if (provenance.selectedRegionStartSec.has_value() &&
        provenance.selectedRegionEndSec.has_value()) {
        appendProvenancePart(
            parts,
            "region",
            formatRegionSeconds(*provenance.selectedRegionStartSec,
                                *provenance.selectedRegionEndSec));
    }

    appendProvenancePart(parts, "warnings",
                         provenance.warningSummary);
    appendProvenancePart(parts, "confidence",
                         provenance.confidenceSummary);

    const bool testOnly =
        provenance.testOnly ||
        result.provenance.value("test_only").toBool();
    const bool devMock =
        provenance.devMock ||
        result.provenance.value("dev_mock").toBool();

    appendProvenanceFlag(parts, "test_only", testOnly);
    appendProvenanceFlag(parts, "dev_mock", devMock);

    QString identity = QString("Backend notes - %1")
        .arg(displayName.isEmpty() ? QString("Backend result") : displayName);
    if (!parts.isEmpty()) {
        identity += QString(" [%1]").arg(parts.join("; "));
    }
    return identity;
}

}

bool
TonyLayerImportProvenance::hasAnyField() const
{
    return !backendId.trimmed().isEmpty() ||
        !backendName.trimmed().isEmpty() ||
        !backendVersion.trimmed().isEmpty() ||
        !inputAudioPath.trimmed().isEmpty() ||
        selectedRegionStartSec.has_value() ||
        selectedRegionEndSec.has_value() ||
        !resultJsonPath.trimmed().isEmpty() ||
        !requestJsonPath.trimmed().isEmpty() ||
        !runId.trimmed().isEmpty() ||
        testOnly ||
        devMock ||
        !warningSummary.trimmed().isEmpty() ||
        !confidenceSummary.trimmed().isEmpty();
}

bool
TonyLayerImportResult::isValid() const
{
    return succeeded && report.isValid();
}

QString
TonyLayerImportResult::debugSummaryString() const
{
    return QString("TonyLayerImportResult(success=%1, imported=%2, "
                   "model=%3, layer=%4, notes=%5, provenance=%6)")
        .arg(succeeded ? QString("true") : QString("false"))
        .arg(importedIntoTonyLayers ? QString("true") : QString("false"))
        .arg(createdModelType)
        .arg(createdLayerType)
        .arg(noteCount)
        .arg(provenanceAttached ? QString("true") : QString("false"));
}

bool
TonyLayerCommandHistoryEditProofResult::isValid() const
{
    return succeeded && report.isValid();
}

QString
TonyLayerCommandHistoryEditProofResult::debugSummaryString() const
{
    return QString("TonyLayerCommandHistoryEditProofResult(success=%1, "
                   "proof=%2, notes=%3)")
        .arg(succeeded ? QString("true") : QString("false"))
        .arg(commandHistoryEditProof ? QString("true") : QString("false"))
        .arg(noteCount);
}

TonyLayerImportResult
TonyLayerImporter::importResult(const UnifiedResult &result) const
{
    TonyLayerImportOptions options;
    return importResult(result, options);
}

TonyLayerImportResult
TonyLayerImporter::importResult(
    const UnifiedResult &unifiedResult,
    const TonyLayerImportOptions &options) const
{
    TonyLayerImportResult result;
    result.sourceMarkedDevMock =
        unifiedResult.provenance.value("dev_mock").toBool() ||
        unifiedResult.provenance.value("test_only").toBool() ||
        options.provenance.devMock ||
        options.provenance.testOnly;

    if (options.sampleRate <= 0.0) {
        setFailure(result,
                   BackendErrorCode::ValidationFailed,
                   "Tony layer import requires a positive sample rate.",
                   "invalid_sample_rate");
        return result;
    }

    if (options.resolution <= 0) {
        setFailure(result,
                   BackendErrorCode::ValidationFailed,
                   "Tony layer import requires a positive model resolution.",
                   "invalid_resolution");
        return result;
    }

    if (options.insertLayerIntoView && !options.createDocumentLayer) {
        setFailure(result,
                   BackendErrorCode::ValidationFailed,
                   "View insertion requires a created Document layer.",
                   "missing_document_layer_for_view_insertion");
        return result;
    }

    if (options.insertLayerIntoView && !options.document) {
        setFailure(result,
                   BackendErrorCode::ValidationFailed,
                   "View insertion requires a Document pointer.",
                   "missing_document_for_layer_insertion");
        return result;
    }

    if (options.insertLayerIntoView && !options.view) {
        setFailure(result,
                   BackendErrorCode::ValidationFailed,
                   "View insertion requires a real View or Pane pointer.",
                   "missing_view_for_layer_insertion");
        return result;
    }

    if (unifiedResult.notes.isEmpty()) {
        setFailure(result,
                   BackendErrorCode::ValidationFailed,
                   "UnifiedResult contains no notes to import.",
                   "no_notes_to_import");
        return result;
    }

    const bool useMidiPitch = resultUsesMidiPitch(unifiedResult, result);
    if (!result.report.isValid()) {
        return result;
    }

    auto model = std::make_shared<sv::NoteModel>(
        options.sampleRate,
        options.resolution,
        false,
        options.useFlexiNoteLayer ?
            sv::NoteModel::FLEXI_NOTE :
            sv::NoteModel::NORMAL_NOTE);

    model->setScaleUnits(useMidiPitch ? "MIDI Pitch" : "Hz");
    const QString provenanceIdentity =
        durableProvenanceIdentity(unifiedResult, options.provenance);
    result.provenanceIdentity = provenanceIdentity;
    result.provenanceAttached = !provenanceIdentity.trimmed().isEmpty();

    model->setObjectName(provenanceIdentity);

    int importedNotes = 0;
    for (const auto &note: unifiedResult.notes) {
        if (!validateImportableNote(note, importedNotes, useMidiPitch, result)) {
            return result;
        }

        const sv::sv_frame_t startFrame =
            secondsToFrame(note.startSec, options.sampleRate);
        const sv::sv_frame_t endFrame =
            secondsToFrame(note.endSec, options.sampleRate);
        const sv::sv_frame_t duration =
            qMax<sv::sv_frame_t>(1, endFrame - startFrame);

        float level = 0.0f;
        const bool hasLevel = noteLevel(note, level);

        sv::Event event(startFrame,
                        noteValue(note, useMidiPitch),
                        duration,
                        noteLabel(note));

        if (hasLevel) {
            event = event.withLevel(level);
        }

        model->add(event);
        ++importedNotes;

        if (note.velocity.has_value() && note.confidence.has_value()) {
            result.report.addIssue(
                ValidationSeverity::Warning,
                "confidence_not_separately_imported",
                "Tony NoteModel stores one note level value; velocity was "
                "imported as level and confidence remains in UnifiedResult.");
        }
    }

    result.modelId = sv::ModelById::add(model);
    result.modelCreated = true;
    result.modelRegistered = true;
    result.createdModelType = options.useFlexiNoteLayer ?
        "NoteModel::FLEXI_NOTE" :
        "NoteModel::NORMAL_NOTE";
    result.noteCount = importedNotes;

    if (options.createDocumentLayer) {
        if (!options.document) {
            sv::ModelById::release(result.modelId);
            result.modelId = {};
            result.modelRegistered = false;
            setFailure(result,
                       BackendErrorCode::ValidationFailed,
                       "Document layer import requires a Document pointer.",
                       "missing_document");
            return result;
        }

        sv::Layer *layer = options.document->createImportedLayer(result.modelId);
        if (!layer) {
            sv::ModelById::release(result.modelId);
            result.modelId = {};
            result.modelRegistered = false;
            setFailure(result,
                       BackendErrorCode::ImportFailed,
                       "Document could not create an imported note layer.",
                       "document_layer_creation_failed");
            return result;
        }

        if (options.provenance.hasAnyField()) {
            layer->setObjectName(provenanceIdentity);
        }
        layer->setPresentationName(provenanceIdentity);

        result.layer = layer;
        result.documentLayerCreated = true;
        result.importedIntoTonyLayers = true;
        result.createdLayerType =
            sv::LayerFactory::getInstance()->getLayerTypeName(
                sv::LayerFactory::getInstance()->getLayerType(layer));
    }

    if (options.insertLayerIntoView) {
        if (!options.createDocumentLayer || !options.document || !result.layer) {
            setFailure(result,
                       BackendErrorCode::ValidationFailed,
                       "View insertion requires a created Document layer.",
                       "missing_document_layer_for_view_insertion");
            return result;
        }

        if (!options.view) {
            setFailure(result,
                       BackendErrorCode::ValidationFailed,
                       "View insertion requires a real View or Pane pointer.",
                       "missing_view_for_layer_insertion");
            return result;
        }

        options.document->addLayerToView(options.view, result.layer);
        if (!viewContainsLayer(options.view, result.layer)) {
            setFailure(result,
                       BackendErrorCode::ImportFailed,
                       "Document did not insert the imported layer into the "
                       "provided View.",
                       "view_layer_insertion_failed");
            return result;
        }

        result.insertedIntoView = true;
    }

    result.succeeded = true;
    result.error = { BackendErrorCode::None, QString(), false };
    return result;
}

TonyLayerCommandHistoryEditProofResult
TonyLayerImporter::proveCommandHistoryEdit(
    const TonyLayerImportResult &importResult,
    int noteIndex,
    float valueDelta) const
{
    TonyLayerCommandHistoryEditProofResult result;

    if (!importResult.isValid() || !importResult.modelRegistered) {
        result.error = {
            BackendErrorCode::ValidationFailed,
            "CommandHistory edit proof requires a valid registered import.",
            true
        };
        result.report.addError("invalid_import_for_edit_proof",
                               result.error.message);
        return result;
    }

    if (valueDelta == 0.0f) {
        result.error = {
            BackendErrorCode::ValidationFailed,
            "CommandHistory edit proof requires a non-zero value delta.",
            true
        };
        result.report.addError("invalid_edit_delta", result.error.message);
        return result;
    }

    auto model = sv::ModelById::getAs<sv::NoteModel>(importResult.modelId);
    if (!model) {
        result.error = {
            BackendErrorCode::ImportFailed,
            "Imported model is not an accessible NoteModel.",
            true
        };
        result.report.addError("missing_note_model_for_edit_proof",
                               result.error.message);
        return result;
    }

    const sv::EventVector events = model->getAllEvents();
    result.noteCount = int(events.size());
    if (noteIndex < 0 || noteIndex >= int(events.size())) {
        result.error = {
            BackendErrorCode::ValidationFailed,
            "Requested note index is outside the imported note model.",
            true
        };
        result.report.addError("invalid_note_index_for_edit_proof",
                               result.error.message);
        return result;
    }

    const sv::Event original = events[noteIndex];
    const sv::Event edited = original.withValue(original.getValue() +
                                               valueDelta);

    auto command = new sv::ChangeEventsCommand(importResult.modelId.untyped,
                                               "Backend Note Edit Proof");
    command->remove(original);
    command->add(edited);

    sv::ChangeEventsCommand *finished = command->finish();
    if (!finished) {
        result.error = {
            BackendErrorCode::ImportFailed,
            "CommandHistory edit proof produced no edit command.",
            true
        };
        result.report.addError("empty_edit_command", result.error.message);
        return result;
    }

    sv::CommandHistory *history = sv::CommandHistory::getInstance();
    history->addCommand(finished, false);

    if (!model->containsEvent(edited) || model->containsEvent(original)) {
        result.error = {
            BackendErrorCode::ImportFailed,
            "Edited note was not present after ChangeEventsCommand.",
            true
        };
        result.report.addError("edit_command_did_not_apply",
                               result.error.message);
        return result;
    }

    history->undo();
    if (!model->containsEvent(original) || model->containsEvent(edited)) {
        result.error = {
            BackendErrorCode::ImportFailed,
            "Undo did not restore the original imported note.",
            true
        };
        result.report.addError("edit_command_undo_failed",
                               result.error.message);
        return result;
    }

    history->redo();
    if (!model->containsEvent(edited) || model->containsEvent(original)) {
        result.error = {
            BackendErrorCode::ImportFailed,
            "Redo did not restore the edited imported note.",
            true
        };
        result.report.addError("edit_command_redo_failed",
                               result.error.message);
        return result;
    }

    history->undo();
    if (!model->containsEvent(original) || model->containsEvent(edited)) {
        result.error = {
            BackendErrorCode::ImportFailed,
            "Final undo did not leave the imported note model unchanged.",
            true
        };
        result.report.addError("edit_command_restore_failed",
                               result.error.message);
        return result;
    }

    result.succeeded = true;
    result.commandHistoryEditProof = true;
    result.error = { BackendErrorCode::None, QString(), false };
    return result;
}

}
}
