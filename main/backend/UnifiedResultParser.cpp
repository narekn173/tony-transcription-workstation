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

#include "UnifiedResultParser.h"

#include <QJsonArray>
#include <QJsonValue>

namespace Tony {
namespace Backend {

namespace {

QStringList
requiredTopLevelFields()
{
    return {
        "contract_version",
        "result_id",
        "request_id",
        "created_at",
        "engine",
        "status",
        "audio",
        "region",
        "summary",
        "notes",
        "pitch_curve",
        "pitch_bends",
        "technique_labels",
        "files",
        "warnings",
        "errors",
        "provenance"
    };
}

std::optional<QString>
optionalString(const QJsonObject &object, const QString &key)
{
    const QJsonValue value = object.value(key);
    if (value.isUndefined() || value.isNull()) {
        return std::nullopt;
    }
    if (value.isString()) {
        return value.toString();
    }
    return std::nullopt;
}

std::optional<int>
optionalInt(const QJsonObject &object, const QString &key)
{
    const QJsonValue value = object.value(key);
    if (value.isUndefined() || value.isNull()) {
        return std::nullopt;
    }
    if (value.isDouble()) {
        return value.toInt();
    }
    return std::nullopt;
}

std::optional<double>
optionalDouble(const QJsonObject &object, const QString &key)
{
    const QJsonValue value = object.value(key);
    if (value.isUndefined() || value.isNull()) {
        return std::nullopt;
    }
    if (value.isDouble()) {
        return value.toDouble();
    }
    return std::nullopt;
}

std::optional<bool>
optionalBool(const QJsonObject &object, const QString &key)
{
    const QJsonValue value = object.value(key);
    if (value.isUndefined() || value.isNull()) {
        return std::nullopt;
    }
    if (value.isBool()) {
        return value.toBool();
    }
    return std::nullopt;
}

QStringList
stringListFromArray(const QJsonArray &array)
{
    QStringList values;
    for (const QJsonValue &value: array) {
        if (value.isString()) {
            values << value.toString();
        }
    }
    return values;
}

void
requireObject(const QJsonObject &parent,
              const QString &key,
              ValidationReport &report)
{
    if (!parent.value(key).isObject()) {
        report.addError(QString("invalid_%1").arg(key),
                        QString("UnifiedResult field '%1' must be an object.").arg(key));
    }
}

void
requireArray(const QJsonObject &parent,
             const QString &key,
             ValidationReport &report)
{
    if (!parent.value(key).isArray()) {
        report.addError(QString("invalid_%1").arg(key),
                        QString("UnifiedResult field '%1' must be an array.").arg(key));
    }
}

QVariantMap
variantMapFromObject(const QJsonObject &object)
{
    return object.toVariantMap();
}

}

bool
UnifiedResultParseResult::isValid() const
{
    return report.isValid();
}

UnifiedResultParseResult
UnifiedResultParser::parse(const QJsonObject &object) const
{
    UnifiedResultParseResult parsed;
    ValidationReport &report = parsed.report;
    UnifiedResult &result = parsed.result;

    for (const QString &field: requiredTopLevelFields()) {
        if (!object.contains(field)) {
            report.addError(QString("missing_%1").arg(field),
                            QString("UnifiedResult field '%1' is required.").arg(field));
        }
    }

    result.contractVersion = object.value("contract_version").toString();
    result.resultId = object.value("result_id").toString();
    result.requestId = object.value("request_id").toString();
    result.compareRunId = optionalString(object, "compare_run_id");
    result.createdAt = QDateTime::fromString(object.value("created_at").toString(),
                                             Qt::ISODate);
    result.status = parseStatus(object.value("status").toString(), report);

    if (object.contains("engine")) requireObject(object, "engine", report);
    const QJsonObject engine = object.value("engine").toObject();
    result.engine.engineId = engine.value("engine_id").toString();
    result.engine.displayName = engine.value("display_name").toString();
    result.engine.engineVersion = optionalString(engine, "engine_version");
    result.engine.adapterVersion = engine.value("adapter_version").toString();
    result.engine.runtimeType = parseRuntimeType(engine.value("runtime_type").toString());
    result.engine.deviceUsed = optionalString(engine, "device_used");

    if (object.contains("audio")) requireObject(object, "audio", report);
    const QJsonObject audio = object.value("audio").toObject();
    result.audio.path = audio.value("path").toString();
    result.audio.durationSec = audio.value("duration_sec").toDouble();
    result.audio.sampleRateHz = audio.value("sample_rate_hz").toInt();
    result.audio.channels = audio.value("channels").toInt();

    const QJsonValue regionValue = object.value("region");
    if (!regionValue.isUndefined() && !regionValue.isNull()) {
        if (!regionValue.isObject()) {
            report.addError("invalid_region",
                            "UnifiedResult field 'region' must be null or an object.");
        } else {
            const QJsonObject regionObject = regionValue.toObject();
            ResultRegion region;
            region.startSec = regionObject.value("start_sec").toDouble();
            region.endSec = regionObject.value("end_sec").toDouble();
            region.coordinateSystem = regionObject.value("coordinate_system").toString();
            region.applyPolicy = regionObject.value("apply_policy").toString();
            result.region = region;
        }
    }

    if (object.contains("summary")) requireObject(object, "summary", report);
    const QJsonObject summary = object.value("summary").toObject();
    result.summary.noteCount = summary.value("note_count").toInt();
    result.summary.pitchPointCount = summary.value("pitch_point_count").toInt();
    result.summary.pitchBendCount = summary.value("pitch_bend_count").toInt();
    result.summary.techniqueLabelCount = summary.value("technique_label_count").toInt();
    result.summary.meanConfidence = optionalDouble(summary, "mean_confidence");
    result.summary.lowConfidenceCount = summary.value("low_confidence_count").toInt();
    result.summary.durationAnalyzedSec =
        summary.value("duration_analyzed_sec").toDouble();

    if (object.contains("notes")) requireArray(object, "notes", report);
    for (const QJsonValue &value: object.value("notes").toArray()) {
        if (!value.isObject()) {
            report.addError("invalid_note", "UnifiedResult note entries must be objects.");
            continue;
        }
        const QJsonObject noteObject = value.toObject();
        NoteEvent note;
        note.id = noteObject.value("id").toString();
        note.startSec = noteObject.value("start_sec").toDouble();
        note.endSec = noteObject.value("end_sec").toDouble();
        note.midiPitch = optionalInt(noteObject, "midi_pitch");
        note.frequencyHz = optionalDouble(noteObject, "frequency_hz");
        note.velocity = optionalInt(noteObject, "velocity");
        note.confidence = optionalDouble(noteObject, "confidence");
        note.label = optionalString(noteObject, "label");
        note.channel = optionalInt(noteObject, "channel");
        note.pitchBendRef = optionalString(noteObject, "pitch_bend_ref");
        note.techniqueRef = optionalString(noteObject, "technique_ref");
        note.source = variantMapFromObject(noteObject.value("source").toObject());
        note.flags = stringListFromArray(noteObject.value("flags").toArray());
        result.notes.push_back(note);
    }

    if (object.contains("pitch_curve")) requireArray(object, "pitch_curve", report);
    for (const QJsonValue &value: object.value("pitch_curve").toArray()) {
        if (!value.isObject()) {
            report.addError("invalid_pitch_point",
                            "UnifiedResult pitch curve entries must be objects.");
            continue;
        }
        const QJsonObject pointObject = value.toObject();
        PitchPoint point;
        point.timeSec = pointObject.value("time_sec").toDouble();
        point.frequencyHz = optionalDouble(pointObject, "frequency_hz");
        point.confidence = optionalDouble(pointObject, "confidence");
        point.voiced = optionalBool(pointObject, "voiced");
        point.midiPitchFloat = optionalDouble(pointObject, "midi_pitch_float");
        point.centsDeviation = optionalDouble(pointObject, "cents_deviation");
        result.pitchCurve.push_back(point);
    }

    if (object.contains("pitch_bends")) requireArray(object, "pitch_bends", report);
    for (const QJsonValue &value: object.value("pitch_bends").toArray()) {
        if (!value.isObject()) {
            report.addError("invalid_pitch_bend",
                            "UnifiedResult pitch bend entries must be objects.");
            continue;
        }
        const QJsonObject bendObject = value.toObject();
        PitchBend bend;
        bend.id = bendObject.value("id").toString();
        bend.noteId = optionalString(bendObject, "note_id");
        bend.unit = bendObject.value("unit").toString("cents");
        bend.confidence = optionalDouble(bendObject, "confidence");
        for (const QJsonValue &pointValue: bendObject.value("points").toArray()) {
            if (!pointValue.isObject()) {
                report.addError("invalid_pitch_bend_point",
                                "UnifiedResult pitch bend points must be objects.");
                continue;
            }
            const QJsonObject pointObject = pointValue.toObject();
            PitchBendPoint point;
            point.timeSec = pointObject.value("time_sec").toDouble();
            point.value = pointObject.value("value").toDouble();
            bend.points.push_back(point);
        }
        result.pitchBends.push_back(bend);
    }

    if (object.contains("technique_labels")) {
        requireArray(object, "technique_labels", report);
    }
    for (const QJsonValue &value: object.value("technique_labels").toArray()) {
        if (!value.isObject()) {
            report.addError("invalid_technique_label",
                            "UnifiedResult technique label entries must be objects.");
            continue;
        }
        const QJsonObject labelObject = value.toObject();
        TechniqueLabel label;
        label.id = labelObject.value("id").toString();
        label.noteId = optionalString(labelObject, "note_id");
        label.startSec = labelObject.value("start_sec").toDouble();
        label.endSec = labelObject.value("end_sec").toDouble();
        label.technique = labelObject.value("technique").toString("unknown");
        label.confidence = optionalDouble(labelObject, "confidence");
        label.source = variantMapFromObject(labelObject.value("source").toObject());
        result.techniqueLabels.push_back(label);
    }

    if (object.contains("files")) requireArray(object, "files", report);
    for (const QJsonValue &value: object.value("files").toArray()) {
        if (!value.isObject()) {
            report.addError("invalid_file_reference",
                            "UnifiedResult file references must be objects.");
            continue;
        }
        const QJsonObject fileObject = value.toObject();
        OutputFileReference file;
        file.kind = fileObject.value("kind").toString();
        file.path = fileObject.value("path").toString();
        file.description = fileObject.value("description").toString();
        file.generatedBy = fileObject.value("generated_by").toString();
        file.imported = fileObject.value("imported").toBool();
        result.files.push_back(file);
    }

    if (object.contains("warnings")) requireArray(object, "warnings", report);
    for (const QJsonValue &value: object.value("warnings").toArray()) {
        if (!value.isObject()) {
            report.addError("invalid_warning",
                            "UnifiedResult warnings must be objects.");
            continue;
        }
        const QJsonObject warningObject = value.toObject();
        ResultWarning warning;
        warning.code = warningObject.value("code").toString();
        warning.severity = parseSeverity(warningObject.value("severity").toString(),
                                         ResultDiagnosticSeverity::Warning);
        warning.message = warningObject.value("message").toString();
        warning.startSec = optionalDouble(warningObject, "start_sec");
        warning.endSec = optionalDouble(warningObject, "end_sec");
        warning.details = variantMapFromObject(warningObject.value("details").toObject());
        result.warnings.push_back(warning);
    }

    if (object.contains("errors")) requireArray(object, "errors", report);
    for (const QJsonValue &value: object.value("errors").toArray()) {
        if (!value.isObject()) {
            report.addError("invalid_error",
                            "UnifiedResult errors must be objects.");
            continue;
        }
        const QJsonObject errorObject = value.toObject();
        ResultError error;
        error.code = errorObject.value("code").toString();
        error.severity = parseSeverity(errorObject.value("severity").toString(),
                                       ResultDiagnosticSeverity::Error);
        error.message = errorObject.value("message").toString();
        error.recoverable = errorObject.value("recoverable").toBool(false);
        error.startSec = optionalDouble(errorObject, "start_sec");
        error.endSec = optionalDouble(errorObject, "end_sec");
        error.details = variantMapFromObject(errorObject.value("details").toObject());
        result.errors.push_back(error);
    }

    if (object.contains("provenance")) {
        requireObject(object, "provenance", report);
        result.provenance = variantMapFromObject(object.value("provenance").toObject());
    }

    ResultValidator validator;
    const ValidationReport semanticReport = validator.validate(BackendRequest(), result);
    for (const ValidationIssue &issue: semanticReport.issues) {
        report.addIssue(issue.severity, issue.code, issue.message);
    }

    return parsed;
}

BackendRuntimeType
UnifiedResultParser::parseRuntimeType(const QString &value)
{
    if (value == "internal") return BackendRuntimeType::Internal;
    if (value == "vamp_plugin") return BackendRuntimeType::VampPlugin;
    if (value == "python_cli") return BackendRuntimeType::PythonCli;
    if (value == "native_cli") return BackendRuntimeType::NativeCli;
    if (value == "onnx_native") return BackendRuntimeType::OnnxNative;
    if (value == "adapter_cli") return BackendRuntimeType::AdapterCli;
    if (value == "development_test") return BackendRuntimeType::DevelopmentTest;
    return BackendRuntimeType::Unknown;
}

BackendStatus
UnifiedResultParser::parseStatus(const QString &value, ValidationReport &report)
{
    if (value == "completed") return BackendStatus::Completed;
    if (value == "completed_with_warnings") {
        return BackendStatus::CompletedWithWarnings;
    }
    if (value == "failed") return BackendStatus::Failed;
    if (value == "cancelled") return BackendStatus::Cancelled;
    if (value == "backend_missing") return BackendStatus::MissingExecutable;
    if (value == "model_missing") return BackendStatus::MissingModel;
    if (value == "unsupported_input" || value == "invalid_output") {
        return BackendStatus::Failed;
    }

    report.addError("invalid_status",
                    "UnifiedResult status is missing or not supported.");
    return BackendStatus::Unknown;
}

ResultDiagnosticSeverity
UnifiedResultParser::parseSeverity(const QString &value,
                                   ResultDiagnosticSeverity fallback)
{
    if (value == "info") return ResultDiagnosticSeverity::Info;
    if (value == "warning") return ResultDiagnosticSeverity::Warning;
    if (value == "error") return ResultDiagnosticSeverity::Error;
    if (value == "fatal") return ResultDiagnosticSeverity::Fatal;
    return fallback;
}

}
}
