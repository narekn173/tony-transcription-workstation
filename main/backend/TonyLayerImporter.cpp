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
#include "framework/Document.h"
#include "layer/Layer.h"
#include "layer/LayerFactory.h"

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
                   "model=%3, layer=%4, notes=%5)")
        .arg(succeeded ? QString("true") : QString("false"))
        .arg(importedIntoTonyLayers ? QString("true") : QString("false"))
        .arg(createdModelType)
        .arg(createdLayerType)
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
        unifiedResult.provenance.value("test_only").toBool();

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
    model->setObjectName(QString("Backend notes - %1")
                         .arg(backendDisplayName(unifiedResult)));

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

        layer->setPresentationName(QString("Backend notes - %1")
                                   .arg(backendDisplayName(unifiedResult)));

        result.layer = layer;
        result.documentLayerCreated = true;
        result.importedIntoTonyLayers = true;
        result.createdLayerType =
            sv::LayerFactory::getInstance()->getLayerTypeName(
                sv::LayerFactory::getInstance()->getLayerType(layer));
    }

    result.succeeded = true;
    result.error = { BackendErrorCode::None, QString(), false };
    return result;
}

}
}
