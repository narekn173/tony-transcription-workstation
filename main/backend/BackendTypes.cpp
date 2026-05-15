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

#include "BackendTypes.h"

namespace Tony {
namespace Backend {

bool
AnalysisRegion::isValid() const
{
    return startSec >= 0.0 && endSec > startSec;
}

bool
BackendCapability::supportsMode(AnalysisMode mode) const
{
    switch (mode) {
    case AnalysisMode::FullFile:
        return supportsFullFile;
    case AnalysisMode::Region:
        return supportsSelectedRegion;
    case AnalysisMode::F0ThenSegmentation:
        return outputsPitchCurve;
    }
    return false;
}

QString
BackendCapability::summaryString() const
{
    QStringList parts;

    if (supportsFullFile) parts << "full_file";
    if (supportsSelectedRegion) parts << "selected_region";
    if (outputsNotes) parts << "notes";
    if (outputsPitchCurve) parts << "pitch_curve";
    if (outputsPitchBends) parts << "pitch_bends";
    if (outputsTechniqueLabels) parts << "technique_labels";
    if (requiresPython) parts << "requires_python";
    if (requiresModelCheckpoint) parts << "requires_model_checkpoint";
    if (supportsCpu) parts << "cpu";
    if (supportsGpuOptional) parts << "gpu_optional";

    if (parts.isEmpty()) {
        return "none";
    }
    return parts.join(", ");
}

AnalysisRunSummary
AnalysisRunSummary::notImplemented(const BackendId &engineId)
{
    AnalysisRunSummary summary;
    summary.engineId = engineId;
    summary.state = AnalysisRunState::NotImplemented;
    summary.errors.push_back({
        BackendErrorCode::NotImplemented,
        "Backend analysis execution is not implemented in the skeleton.",
        true
    });
    return summary;
}

bool
BackendManifest::hasRequiredIdentity() const
{
    const BackendId effectiveId = id();
    return !contractVersion.isEmpty() &&
        Tony::Backend::isValidBackendId(effectiveId) &&
        !displayName.isEmpty();
}

BackendId
BackendManifest::id() const
{
    if (!backendId.isEmpty()) {
        return backendId;
    }
    return engineId;
}

bool
BackendManifest::isValidBackendId() const
{
    return Tony::Backend::isValidBackendId(id());
}

bool
BackendManifest::hasExecutablePath() const
{
    return !executablePath.trimmed().isEmpty();
}

bool
BackendManifest::supportsFullFile() const
{
    return capabilities.supportsFullFile;
}

bool
BackendManifest::supportsSelectedRegion() const
{
    return capabilities.supportsSelectedRegion;
}

bool
BackendManifest::supportsNotes() const
{
    return capabilities.outputsNotes;
}

bool
BackendManifest::supportsPitchCurve() const
{
    return capabilities.outputsPitchCurve;
}

QString
BackendManifest::debugSummaryString() const
{
    return manifestSummaryString(*this);
}

bool
BackendRequest::isRunnableShape() const
{
    if (requestId.isEmpty() || engineId.isEmpty() || inputAudioPath.isEmpty()) {
        return false;
    }
    if (mode == AnalysisMode::Region && (!hasRegion || !region.isValid())) {
        return false;
    }
    return true;
}

QString
statusToString(BackendStatus status)
{
    switch (status) {
    case BackendStatus::Unknown: return "unknown";
    case BackendStatus::NotConfigured: return "not_configured";
    case BackendStatus::MissingExecutable: return "missing_executable";
    case BackendStatus::MissingModel: return "missing_model";
    case BackendStatus::Ready: return "ready";
    case BackendStatus::Running: return "running";
    case BackendStatus::Completed: return "completed";
    case BackendStatus::CompletedWithWarnings: return "completed_with_warnings";
    case BackendStatus::Failed: return "failed";
    case BackendStatus::Cancelled: return "cancelled";
    }
    return "unknown";
}

QString
capabilitySummaryString(const BackendCapability &capability)
{
    return capability.summaryString();
}

bool
isValidBackendId(const BackendId &backendId)
{
    if (backendId.isEmpty()) {
        return false;
    }

    const QChar first = backendId.front();
    if (first < QLatin1Char('a') || first > QLatin1Char('z')) {
        return false;
    }

    for (const QChar ch: backendId) {
        const bool isLower = ch >= QLatin1Char('a') && ch <= QLatin1Char('z');
        const bool isDigit = ch >= QLatin1Char('0') && ch <= QLatin1Char('9');
        if (!isLower && !isDigit && ch != QLatin1Char('_')) {
            return false;
        }
    }

    return true;
}

QString
manifestSummaryString(const BackendManifest &manifest)
{
    const QString manifestVersion =
        !manifest.version.isEmpty() ? manifest.version : manifest.engineVersion;
    const QString runtimeType =
        manifest.backendType != BackendRuntimeType::Unknown ?
            toString(manifest.backendType) : toString(manifest.runtimeType);
    const QStringList outputs =
        !manifest.supportedOutputTypes.isEmpty() ?
            manifest.supportedOutputTypes :
            manifest.primaryOutputs + manifest.optionalOutputs;

    return QString("backend=%1 display=\"%2\" status=%3 type=%4 version=%5 "
                   "capabilities=[%6] inputs=[%7] outputs=[%8]")
        .arg(manifest.id().isEmpty() ? QString("unknown") : manifest.id())
        .arg(manifest.displayName)
        .arg(statusToString(manifest.status))
        .arg(runtimeType)
        .arg(manifestVersion.isEmpty() ? QString("unknown") : manifestVersion)
        .arg(capabilitySummaryString(manifest.capabilities))
        .arg(manifest.supportedInputFormats.join(", "))
        .arg(outputs.join(", "));
}

QString
toString(BackendRuntimeType type)
{
    switch (type) {
    case BackendRuntimeType::Unknown: return "unknown";
    case BackendRuntimeType::Internal: return "internal";
    case BackendRuntimeType::VampPlugin: return "vamp_plugin";
    case BackendRuntimeType::PythonCli: return "python_cli";
    case BackendRuntimeType::NativeCli: return "native_cli";
    case BackendRuntimeType::OnnxNative: return "onnx_native";
    case BackendRuntimeType::AdapterCli: return "adapter_cli";
    case BackendRuntimeType::DevelopmentTest: return "development_test";
    }
    return "unknown";
}

QString
toString(BackendStatus status)
{
    return statusToString(status);
}

QString
toString(AnalysisRunState state)
{
    switch (state) {
    case AnalysisRunState::Idle: return "idle";
    case AnalysisRunState::Ready: return "ready";
    case AnalysisRunState::Queued: return "queued";
    case AnalysisRunState::Running: return "running";
    case AnalysisRunState::Cancelling: return "cancelling";
    case AnalysisRunState::Cancelled: return "cancelled";
    case AnalysisRunState::Completed: return "completed";
    case AnalysisRunState::CompletedWithWarnings: return "completed_with_warnings";
    case AnalysisRunState::Failed: return "failed";
    case AnalysisRunState::NotImplemented: return "not_implemented";
    }
    return "failed";
}

QString
toString(AnalysisMode mode)
{
    switch (mode) {
    case AnalysisMode::FullFile: return "full_file";
    case AnalysisMode::Region: return "region";
    case AnalysisMode::F0ThenSegmentation: return "f0_then_segmentation";
    }
    return "full_file";
}

QString
toString(BackendErrorCode code)
{
    switch (code) {
    case BackendErrorCode::None: return "none";
    case BackendErrorCode::NotImplemented: return "not_implemented";
    case BackendErrorCode::BackendNotConfigured: return "backend_not_configured";
    case BackendErrorCode::BackendMissing: return "backend_missing";
    case BackendErrorCode::DependencyMissing: return "dependency_missing";
    case BackendErrorCode::ModelMissing: return "model_missing";
    case BackendErrorCode::UnsupportedInput: return "unsupported_input";
    case BackendErrorCode::UnsupportedMode: return "unsupported_mode";
    case BackendErrorCode::ExecutionFailed: return "execution_failed";
    case BackendErrorCode::TimedOut: return "timed_out";
    case BackendErrorCode::Cancelled: return "cancelled";
    case BackendErrorCode::OutputMissing: return "output_missing";
    case BackendErrorCode::OutputInvalid: return "output_invalid";
    case BackendErrorCode::ValidationFailed: return "validation_failed";
    case BackendErrorCode::ImportFailed: return "import_failed";
    }
    return "none";
}

}
}
