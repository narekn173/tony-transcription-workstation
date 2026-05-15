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

#include "UnifiedResult.h"

namespace Tony {
namespace Backend {

bool
BackendMetadata::hasRequiredIdentity() const
{
    return !engineId.isEmpty() &&
        !displayName.isEmpty() &&
        !adapterVersion.isEmpty();
}

bool
NoteEvent::isValidTimeRange() const
{
    return Tony::Backend::isValidTimeRange(startSec, endSec);
}

bool
NoteEvent::hasValidMidiPitch() const
{
    return isValidMidiPitch(midiPitch);
}

bool
NoteEvent::hasValidVelocity() const
{
    return isValidVelocity(velocity);
}

bool
NoteEvent::hasValidConfidence() const
{
    return isValidConfidence(confidence);
}

bool
PitchPoint::hasValidTime() const
{
    return timeSec >= 0.0;
}

bool
PitchPoint::hasValidFrequency() const
{
    return !frequencyHz.has_value() || *frequencyHz > 0.0;
}

bool
PitchPoint::hasValidConfidence() const
{
    return isValidConfidence(confidence);
}

bool
PitchBendPoint::hasValidTime() const
{
    return timeSec >= 0.0;
}

bool
PitchBend::hasValidConfidence() const
{
    return isValidConfidence(confidence);
}

bool
TechniqueLabel::isValidTimeRange() const
{
    return Tony::Backend::isValidTimeRange(startSec, endSec);
}

bool
TechniqueLabel::hasValidConfidence() const
{
    return isValidConfidence(confidence);
}

bool
ResultWarning::hasValidTimeRange() const
{
    if (!startSec.has_value() && !endSec.has_value()) {
        return true;
    }
    if (!startSec.has_value() || !endSec.has_value()) {
        return false;
    }
    return Tony::Backend::isValidTimeRange(*startSec, *endSec);
}

bool
ResultError::hasValidTimeRange() const
{
    if (!startSec.has_value() && !endSec.has_value()) {
        return true;
    }
    if (!startSec.has_value() || !endSec.has_value()) {
        return false;
    }
    return Tony::Backend::isValidTimeRange(*startSec, *endSec);
}

bool
ResultRegion::isValidTimeRange() const
{
    return Tony::Backend::isValidTimeRange(startSec, endSec);
}

bool
UnifiedResult::isEmpty() const
{
    return notes.isEmpty() &&
        pitchCurve.isEmpty() &&
        pitchBends.isEmpty() &&
        techniqueLabels.isEmpty() &&
        files.isEmpty() &&
        warnings.isEmpty() &&
        errors.isEmpty();
}

bool
UnifiedResult::hasNotes() const
{
    return !notes.isEmpty();
}

bool
UnifiedResult::hasPitchCurve() const
{
    return !pitchCurve.isEmpty();
}

bool
UnifiedResult::hasPitchBends() const
{
    return !pitchBends.isEmpty();
}

bool
UnifiedResult::hasTechniqueLabels() const
{
    return !techniqueLabels.isEmpty();
}

QString
UnifiedResult::debugSummaryString() const
{
    QStringList parts;
    parts << QString("result_id=%1").arg(resultId.isEmpty() ? "<empty>" : resultId);
    parts << QString("request_id=%1").arg(requestId.isEmpty() ? "<empty>" : requestId);
    parts << QString("engine_id=%1").arg(engine.engineId.isEmpty() ? "<empty>" : engine.engineId);
    parts << QString("status=%1").arg(statusToString(status));
    parts << QString("notes=%1").arg(notes.size());
    parts << QString("pitch_curve=%1").arg(pitchCurve.size());
    parts << QString("pitch_bends=%1").arg(pitchBends.size());
    parts << QString("technique_labels=%1").arg(techniqueLabels.size());
    parts << QString("warnings=%1").arg(warnings.size());
    parts << QString("errors=%1").arg(errors.size());
    return parts.join(", ");
}

bool
isValidTimeRange(double startSec, double endSec)
{
    return startSec >= 0.0 && endSec >= startSec;
}

bool
isValidMidiPitch(int midiPitch)
{
    return midiPitch >= 0 && midiPitch <= 127;
}

bool
isValidMidiPitch(const std::optional<int> &midiPitch)
{
    return !midiPitch.has_value() || isValidMidiPitch(*midiPitch);
}

bool
isValidVelocity(int velocity)
{
    return velocity >= 0 && velocity <= 127;
}

bool
isValidVelocity(const std::optional<int> &velocity)
{
    return !velocity.has_value() || isValidVelocity(*velocity);
}

bool
isValidConfidence(double confidence)
{
    return confidence >= 0.0 && confidence <= 1.0;
}

bool
isValidConfidence(const std::optional<double> &confidence)
{
    return !confidence.has_value() || isValidConfidence(*confidence);
}

QString
resultSummaryString(const UnifiedResult &result)
{
    return result.debugSummaryString();
}

QString
toString(ResultDiagnosticSeverity severity)
{
    switch (severity) {
    case ResultDiagnosticSeverity::Info: return "info";
    case ResultDiagnosticSeverity::Warning: return "warning";
    case ResultDiagnosticSeverity::Error: return "error";
    case ResultDiagnosticSeverity::Fatal: return "fatal";
    }
    return "error";
}

}
}
