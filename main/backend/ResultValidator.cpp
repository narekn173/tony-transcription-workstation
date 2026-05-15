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

#include "ResultValidator.h"

namespace Tony {
namespace Backend {

bool
ValidationReport::isValid() const
{
    for (const auto &issue: issues) {
        if (issue.severity == ValidationSeverity::Error) {
            return false;
        }
    }
    return true;
}

void
ValidationReport::addError(const QString &code, const QString &message)
{
    issues.push_back({ ValidationSeverity::Error, code, message });
}

ValidationReport
ResultValidator::validate(const BackendRequest &request,
                          const UnifiedResult &result) const
{
    ValidationReport report;

    if (result.contractVersion.isEmpty()) {
        report.addError("missing_contract_version",
                        "UnifiedResult contract version is missing.");
    }
    if (!request.requestId.isEmpty() &&
        !result.requestId.isEmpty() &&
        request.requestId != result.requestId) {
        report.addError("request_id_mismatch",
                        "UnifiedResult request ID does not match the request.");
    }
    if (!request.engineId.isEmpty() &&
        !result.engineId.isEmpty() &&
        request.engineId != result.engineId) {
        report.addError("engine_id_mismatch",
                        "UnifiedResult engine ID does not match the request.");
    }

    for (const auto &note: result.notes) {
        if (note.startSec < 0.0 || note.endSec <= note.startSec) {
            report.addError("invalid_note_timing",
                            "A note has invalid start/end timing.");
        }
        if (note.midiPitch > 127) {
            report.addError("invalid_midi_pitch",
                            "A note has a MIDI pitch greater than 127.");
        }
        if (note.velocity > 127) {
            report.addError("invalid_velocity",
                            "A note has a velocity greater than 127.");
        }
        if (note.confidence > 1.0) {
            report.addError("invalid_confidence",
                            "A note has confidence greater than 1.0.");
        }
        if (note.frequencyHz == 0.0) {
            report.addError("invalid_frequency",
                            "A note has a zero frequency value.");
        }
    }

    double previousTime = -1.0;
    for (const auto &point: result.pitchCurve) {
        if (point.timeSec < previousTime) {
            report.addError("pitch_curve_not_monotonic",
                            "Pitch curve points are not ordered by time.");
        }
        previousTime = point.timeSec;
        if (point.frequencyHz == 0.0) {
            report.addError("invalid_pitch_frequency",
                            "A pitch point has a zero frequency value.");
        }
        if (point.confidence > 1.0) {
            report.addError("invalid_pitch_confidence",
                            "A pitch point has confidence greater than 1.0.");
        }
    }

    return report;
}

}
}
