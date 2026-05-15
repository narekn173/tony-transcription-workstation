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
    return !contractVersion.isEmpty() &&
        !engineId.isEmpty() &&
        !displayName.isEmpty();
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

bool
UnifiedResult::isEmpty() const
{
    return notes.isEmpty() &&
        pitchCurve.isEmpty() &&
        warnings.isEmpty() &&
        errors.isEmpty();
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
toString(BackendAvailabilityState state)
{
    switch (state) {
    case BackendAvailabilityState::NotConfigured: return "not_configured";
    case BackendAvailabilityState::Missing: return "missing";
    case BackendAvailabilityState::Installed: return "installed";
    case BackendAvailabilityState::Ready: return "ready";
    case BackendAvailabilityState::Broken: return "broken";
    case BackendAvailabilityState::Unsupported: return "unsupported";
    }
    return "unsupported";
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
