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

#ifndef TONY_BACKEND_TYPES_H
#define TONY_BACKEND_TYPES_H

#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QVector>

namespace Tony {
namespace Backend {

using BackendId = QString;
using AnalysisRunId = QString;

enum class BackendRuntimeType {
    Unknown,
    Internal,
    VampPlugin,
    PythonCli,
    NativeCli,
    OnnxNative,
    AdapterCli,
    DevelopmentTest
};

enum class BackendAvailabilityState {
    NotConfigured,
    Missing,
    Installed,
    Ready,
    Broken,
    Unsupported
};

enum class AnalysisRunState {
    Idle,
    Ready,
    Queued,
    Running,
    Cancelling,
    Cancelled,
    Completed,
    CompletedWithWarnings,
    Failed,
    NotImplemented
};

enum class AnalysisMode {
    FullFile,
    Region,
    F0ThenSegmentation
};

enum class BackendErrorCode {
    None,
    NotImplemented,
    BackendNotConfigured,
    BackendMissing,
    DependencyMissing,
    ModelMissing,
    UnsupportedInput,
    UnsupportedMode,
    ExecutionFailed,
    TimedOut,
    Cancelled,
    OutputMissing,
    OutputInvalid,
    ValidationFailed,
    ImportFailed
};

struct AnalysisRegion
{
    double startSec = 0.0;
    double endSec = 0.0;

    bool isValid() const;
};

struct BackendCapabilityFlags
{
    bool supportsFullFile = false;
    bool supportsRegion = false;
    bool supportsBatch = false;
    bool outputsNotes = false;
    bool outputsPitchCurve = false;
    bool outputsPitchBends = false;
    bool outputsVelocity = false;
    bool outputsConfidence = false;
    bool outputsTechniqueLabels = false;
    bool outputsWarnings = false;
    bool canRunOffline = true;
};

struct BackendError
{
    BackendErrorCode code = BackendErrorCode::None;
    QString message;
    bool recoverable = false;
};

struct BackendWarning
{
    QString code;
    QString message;
};

struct AnalysisRunSummary
{
    AnalysisRunId runId;
    BackendId engineId;
    AnalysisRunState state = AnalysisRunState::Idle;
    QVector<BackendWarning> warnings;
    QVector<BackendError> errors;

    static AnalysisRunSummary notImplemented(const BackendId &engineId);
};

struct BackendManifest
{
    QString contractVersion = "0.1";
    BackendId engineId;
    QString displayName;
    QString engineVersion;
    QString adapterVersion;
    BackendRuntimeType runtimeType = BackendRuntimeType::Unknown;
    BackendAvailabilityState availability = BackendAvailabilityState::NotConfigured;
    BackendCapabilityFlags capabilities;
    QStringList primaryOutputs;
    QStringList optionalOutputs;
    QString licenseName;
    QString licenseSourceUrl;

    bool hasRequiredIdentity() const;
};

struct BackendRequest
{
    QString contractVersion = "0.1";
    AnalysisRunId requestId;
    BackendId engineId;
    AnalysisMode mode = AnalysisMode::FullFile;
    QString inputAudioPath;
    bool hasRegion = false;
    AnalysisRegion region;
    QString workspaceDir;
    QString resultJsonPath;
    QStringList requestedOutputs;
    QVariantMap settings;
    QString devicePreference = "cpu";
    int timeoutSec = 0;
    bool allowInternet = false;

    bool isRunnableShape() const;
};

struct UnifiedNoteEvent
{
    QString id;
    double startSec = 0.0;
    double endSec = 0.0;
    int midiPitch = -1;
    double frequencyHz = -1.0;
    int velocity = -1;
    double confidence = -1.0;
};

struct UnifiedPitchPoint
{
    double timeSec = 0.0;
    double frequencyHz = -1.0;
    double confidence = -1.0;
    bool voiced = false;
};

struct UnifiedResult
{
    QString contractVersion = "0.1";
    QString resultId;
    AnalysisRunId requestId;
    BackendId engineId;
    AnalysisRunState status = AnalysisRunState::Idle;
    double audioDurationSec = 0.0;
    QVector<UnifiedNoteEvent> notes;
    QVector<UnifiedPitchPoint> pitchCurve;
    QVector<BackendWarning> warnings;
    QVector<BackendError> errors;

    bool isEmpty() const;
};

QString toString(BackendRuntimeType type);
QString toString(BackendAvailabilityState state);
QString toString(AnalysisRunState state);
QString toString(AnalysisMode mode);
QString toString(BackendErrorCode code);

}
}

#endif
