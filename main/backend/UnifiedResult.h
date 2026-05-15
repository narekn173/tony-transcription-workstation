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

#ifndef TONY_UNIFIED_RESULT_H
#define TONY_UNIFIED_RESULT_H

#include "BackendTypes.h"

#include <optional>

#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QVector>

namespace Tony {
namespace Backend {

enum class ResultDiagnosticSeverity {
    Info,
    Warning,
    Error,
    Fatal
};

struct BackendMetadata
{
    BackendId engineId;
    QString displayName;
    std::optional<QString> engineVersion;
    QString adapterVersion;
    BackendRuntimeType runtimeType = BackendRuntimeType::Unknown;
    std::optional<QString> deviceUsed;

    bool hasRequiredIdentity() const;
};

struct NoteEvent
{
    QString id;
    double startSec = 0.0;
    double endSec = 0.0;
    std::optional<int> midiPitch;
    std::optional<double> frequencyHz;
    std::optional<int> velocity;
    std::optional<double> confidence;
    std::optional<QString> label;
    std::optional<int> channel;
    std::optional<QString> pitchBendRef;
    std::optional<QString> techniqueRef;
    QVariantMap source;
    QStringList flags;

    bool isValidTimeRange() const;
    bool hasValidMidiPitch() const;
    bool hasValidVelocity() const;
    bool hasValidConfidence() const;
};

struct PitchPoint
{
    double timeSec = 0.0;
    std::optional<double> frequencyHz;
    std::optional<double> confidence;
    std::optional<bool> voiced;
    std::optional<double> midiPitchFloat;
    std::optional<double> centsDeviation;

    bool hasValidTime() const;
    bool hasValidFrequency() const;
    bool hasValidConfidence() const;
};

struct PitchBendPoint
{
    double timeSec = 0.0;
    double value = 0.0;

    bool hasValidTime() const;
};

struct PitchBend
{
    QString id;
    std::optional<QString> noteId;
    QString unit = "cents";
    QVector<PitchBendPoint> points;
    std::optional<double> confidence;

    bool hasValidConfidence() const;
};

struct TechniqueLabel
{
    QString id;
    std::optional<QString> noteId;
    double startSec = 0.0;
    double endSec = 0.0;
    QString technique = "unknown";
    std::optional<double> confidence;
    QVariantMap source;

    bool isValidTimeRange() const;
    bool hasValidConfidence() const;
};

struct ResultWarning
{
    QString code;
    ResultDiagnosticSeverity severity = ResultDiagnosticSeverity::Warning;
    QString message;
    std::optional<double> startSec;
    std::optional<double> endSec;
    QVariantMap details;

    bool hasValidTimeRange() const;
};

struct ResultError
{
    QString code;
    ResultDiagnosticSeverity severity = ResultDiagnosticSeverity::Error;
    QString message;
    bool recoverable = false;
    std::optional<double> startSec;
    std::optional<double> endSec;
    QVariantMap details;

    bool hasValidTimeRange() const;
};

struct ResultAudioMetadata
{
    QString path;
    double durationSec = 0.0;
    int sampleRateHz = 0;
    int channels = 0;
};

struct ResultRegion
{
    double startSec = 0.0;
    double endSec = 0.0;
    QString coordinateSystem = "original_audio_time";
    QString applyPolicy = "preview_only";

    bool isValidTimeRange() const;
};

struct ResultSummary
{
    int noteCount = 0;
    int pitchPointCount = 0;
    int pitchBendCount = 0;
    int techniqueLabelCount = 0;
    std::optional<double> meanConfidence;
    int lowConfidenceCount = 0;
    double durationAnalyzedSec = 0.0;
};

struct OutputFileReference
{
    QString kind;
    QString path;
    QString description;
    QString generatedBy;
    bool imported = false;
};

struct UnifiedResult
{
    QString contractVersion = "0.1";
    QString resultId;
    AnalysisRunId requestId;
    std::optional<AnalysisRunId> compareRunId;
    QDateTime createdAt;
    BackendMetadata engine;
    BackendStatus status = BackendStatus::Unknown;
    ResultAudioMetadata audio;
    std::optional<ResultRegion> region;
    ResultSummary summary;
    QVector<NoteEvent> notes;
    QVector<PitchPoint> pitchCurve;
    QVector<PitchBend> pitchBends;
    QVector<TechniqueLabel> techniqueLabels;
    QVector<OutputFileReference> files;
    QVector<ResultWarning> warnings;
    QVector<ResultError> errors;
    QVariantMap provenance;

    bool isEmpty() const;
    bool hasNotes() const;
    bool hasPitchCurve() const;
    bool hasPitchBends() const;
    bool hasTechniqueLabels() const;
    QString debugSummaryString() const;
};

using UnifiedNoteEvent = NoteEvent;
using UnifiedPitchPoint = PitchPoint;

bool isValidTimeRange(double startSec, double endSec);
bool isValidMidiPitch(int midiPitch);
bool isValidMidiPitch(const std::optional<int> &midiPitch);
bool isValidVelocity(int velocity);
bool isValidVelocity(const std::optional<int> &velocity);
bool isValidConfidence(double confidence);
bool isValidConfidence(const std::optional<double> &confidence);
QString resultSummaryString(const UnifiedResult &result);
QString toString(ResultDiagnosticSeverity severity);

}
}

#endif
