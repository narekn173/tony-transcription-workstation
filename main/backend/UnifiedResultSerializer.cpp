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

#include "UnifiedResultSerializer.h"

#include <QJsonArray>
#include <QJsonValue>

namespace Tony {
namespace Backend {

namespace {

QJsonValue
optionalString(const std::optional<QString> &value)
{
    if (!value.has_value()) {
        return QJsonValue::Null;
    }
    return *value;
}

QJsonValue
optionalInt(const std::optional<int> &value)
{
    if (!value.has_value()) {
        return QJsonValue::Null;
    }
    return *value;
}

QJsonValue
optionalDouble(const std::optional<double> &value)
{
    if (!value.has_value()) {
        return QJsonValue::Null;
    }
    return *value;
}

QJsonValue
optionalBool(const std::optional<bool> &value)
{
    if (!value.has_value()) {
        return QJsonValue::Null;
    }
    return *value;
}

QJsonArray
stringArray(const QStringList &values)
{
    QJsonArray array;
    for (const QString &value: values) {
        array.push_back(value);
    }
    return array;
}

QJsonObject
diagnosticDetails(const QVariantMap &details)
{
    return QJsonObject::fromVariantMap(details);
}

QJsonObject
warningObject(const ResultWarning &warning)
{
    QJsonObject object;
    object.insert("code", warning.code);
    object.insert("severity", toString(warning.severity));
    object.insert("message", warning.message);
    object.insert("start_sec", optionalDouble(warning.startSec));
    object.insert("end_sec", optionalDouble(warning.endSec));
    object.insert("details", diagnosticDetails(warning.details));
    return object;
}

QJsonObject
errorObject(const ResultError &error)
{
    QJsonObject object;
    object.insert("code", error.code);
    object.insert("severity", toString(error.severity));
    object.insert("message", error.message);
    object.insert("recoverable", error.recoverable);
    object.insert("start_sec", optionalDouble(error.startSec));
    object.insert("end_sec", optionalDouble(error.endSec));
    object.insert("details", diagnosticDetails(error.details));
    return object;
}

QString
unifiedResultStatusString(BackendStatus status)
{
    switch (status) {
    case BackendStatus::Completed: return "completed";
    case BackendStatus::CompletedWithWarnings:
        return "completed_with_warnings";
    case BackendStatus::Failed: return "failed";
    case BackendStatus::Cancelled: return "cancelled";
    case BackendStatus::MissingExecutable: return "backend_missing";
    case BackendStatus::MissingModel: return "model_missing";
    case BackendStatus::Unknown:
    case BackendStatus::NotConfigured:
    case BackendStatus::Ready:
    case BackendStatus::Running:
        return statusToString(status);
    }
    return statusToString(status);
}

}

QJsonObject
UnifiedResultSerializer::toJsonObject(const UnifiedResult &result) const
{
    QJsonObject object;
    object.insert("contract_version", result.contractVersion);
    object.insert("result_id", result.resultId);
    object.insert("request_id", result.requestId);
    if (result.compareRunId.has_value()) {
        object.insert("compare_run_id", *result.compareRunId);
    }
    object.insert("created_at", result.createdAt.toUTC().toString(Qt::ISODate));

    QJsonObject engine;
    engine.insert("engine_id", result.engine.engineId);
    engine.insert("display_name", result.engine.displayName);
    engine.insert("engine_version", optionalString(result.engine.engineVersion));
    engine.insert("adapter_version", result.engine.adapterVersion);
    engine.insert("runtime_type", toString(result.engine.runtimeType));
    engine.insert("device_used", optionalString(result.engine.deviceUsed));
    object.insert("engine", engine);

    object.insert("status", unifiedResultStatusString(result.status));

    QJsonObject audio;
    audio.insert("path", result.audio.path);
    audio.insert("duration_sec", result.audio.durationSec);
    audio.insert("sample_rate_hz", result.audio.sampleRateHz);
    audio.insert("channels", result.audio.channels);
    object.insert("audio", audio);

    if (result.region.has_value()) {
        QJsonObject region;
        region.insert("start_sec", result.region->startSec);
        region.insert("end_sec", result.region->endSec);
        region.insert("coordinate_system", result.region->coordinateSystem);
        region.insert("apply_policy", result.region->applyPolicy);
        object.insert("region", region);
    } else {
        object.insert("region", QJsonValue::Null);
    }

    QJsonObject summary;
    summary.insert("note_count", result.summary.noteCount);
    summary.insert("pitch_point_count", result.summary.pitchPointCount);
    summary.insert("pitch_bend_count", result.summary.pitchBendCount);
    summary.insert("technique_label_count",
                   result.summary.techniqueLabelCount);
    summary.insert("mean_confidence",
                   optionalDouble(result.summary.meanConfidence));
    summary.insert("low_confidence_count", result.summary.lowConfidenceCount);
    summary.insert("duration_analyzed_sec",
                   result.summary.durationAnalyzedSec);
    object.insert("summary", summary);

    QJsonArray notes;
    for (const NoteEvent &note: result.notes) {
        QJsonObject noteObject;
        noteObject.insert("id", note.id);
        noteObject.insert("start_sec", note.startSec);
        noteObject.insert("end_sec", note.endSec);
        noteObject.insert("midi_pitch", optionalInt(note.midiPitch));
        noteObject.insert("frequency_hz", optionalDouble(note.frequencyHz));
        noteObject.insert("velocity", optionalInt(note.velocity));
        noteObject.insert("confidence", optionalDouble(note.confidence));
        noteObject.insert("label", optionalString(note.label));
        noteObject.insert("channel", optionalInt(note.channel));
        noteObject.insert("pitch_bend_ref", optionalString(note.pitchBendRef));
        noteObject.insert("technique_ref", optionalString(note.techniqueRef));
        noteObject.insert("source",
                          QJsonObject::fromVariantMap(note.source));
        noteObject.insert("flags", stringArray(note.flags));
        notes.push_back(noteObject);
    }
    object.insert("notes", notes);

    QJsonArray pitchCurve;
    for (const PitchPoint &point: result.pitchCurve) {
        QJsonObject pointObject;
        pointObject.insert("time_sec", point.timeSec);
        pointObject.insert("frequency_hz", optionalDouble(point.frequencyHz));
        pointObject.insert("confidence", optionalDouble(point.confidence));
        pointObject.insert("voiced", optionalBool(point.voiced));
        pointObject.insert("midi_pitch_float",
                           optionalDouble(point.midiPitchFloat));
        pointObject.insert("cents_deviation",
                           optionalDouble(point.centsDeviation));
        pitchCurve.push_back(pointObject);
    }
    object.insert("pitch_curve", pitchCurve);

    QJsonArray pitchBends;
    for (const PitchBend &bend: result.pitchBends) {
        QJsonObject bendObject;
        bendObject.insert("id", bend.id);
        bendObject.insert("note_id", optionalString(bend.noteId));
        bendObject.insert("unit", bend.unit);
        bendObject.insert("confidence", optionalDouble(bend.confidence));

        QJsonArray points;
        for (const PitchBendPoint &point: bend.points) {
            QJsonObject pointObject;
            pointObject.insert("time_sec", point.timeSec);
            pointObject.insert("value", point.value);
            points.push_back(pointObject);
        }
        bendObject.insert("points", points);
        pitchBends.push_back(bendObject);
    }
    object.insert("pitch_bends", pitchBends);

    QJsonArray techniqueLabels;
    for (const TechniqueLabel &label: result.techniqueLabels) {
        QJsonObject labelObject;
        labelObject.insert("id", label.id);
        labelObject.insert("note_id", optionalString(label.noteId));
        labelObject.insert("start_sec", label.startSec);
        labelObject.insert("end_sec", label.endSec);
        labelObject.insert("technique", label.technique);
        labelObject.insert("confidence", optionalDouble(label.confidence));
        labelObject.insert("source",
                           QJsonObject::fromVariantMap(label.source));
        techniqueLabels.push_back(labelObject);
    }
    object.insert("technique_labels", techniqueLabels);

    QJsonArray files;
    for (const OutputFileReference &file: result.files) {
        QJsonObject fileObject;
        fileObject.insert("kind", file.kind);
        fileObject.insert("path", file.path);
        fileObject.insert("description", file.description);
        fileObject.insert("generated_by", file.generatedBy);
        fileObject.insert("imported", file.imported);
        files.push_back(fileObject);
    }
    object.insert("files", files);

    QJsonArray warnings;
    for (const ResultWarning &warning: result.warnings) {
        warnings.push_back(warningObject(warning));
    }
    object.insert("warnings", warnings);

    QJsonArray errors;
    for (const ResultError &error: result.errors) {
        errors.push_back(errorObject(error));
    }
    object.insert("errors", errors);

    object.insert("provenance",
                  QJsonObject::fromVariantMap(result.provenance));

    return object;
}

}
}
