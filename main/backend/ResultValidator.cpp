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
        !result.engine.engineId.isEmpty() &&
        request.engineId != result.engine.engineId) {
        report.addError("engine_id_mismatch",
                        "UnifiedResult engine ID does not match the request.");
    }
    if (result.status == BackendStatus::Failed && result.errors.isEmpty()) {
        report.addError("failed_result_without_error",
                        "A failed UnifiedResult must include at least one error.");
    }

    for (const auto &note: result.notes) {
        if (!note.isValidTimeRange()) {
            report.addError("invalid_note_timing",
                            "A note has invalid start/end timing.");
        }
        if (!note.hasValidMidiPitch()) {
            report.addError("invalid_midi_pitch",
                            "A note has a MIDI pitch outside the 0-127 range.");
        }
        if (!note.hasValidVelocity()) {
            report.addError("invalid_velocity",
                            "A note has velocity outside the 0-127 range.");
        }
        if (!note.hasValidConfidence()) {
            report.addError("invalid_confidence",
                            "A note has confidence outside the 0.0-1.0 range.");
        }
        if (note.frequencyHz.has_value() && *note.frequencyHz <= 0.0) {
            report.addError("invalid_frequency",
                            "A note has a non-positive frequency value.");
        }
    }

    double previousTime = -1.0;
    for (const auto &point: result.pitchCurve) {
        if (!point.hasValidTime()) {
            report.addError("invalid_pitch_time",
                            "A pitch point has negative time.");
        }
        if (point.timeSec < previousTime) {
            report.addError("pitch_curve_not_monotonic",
                            "Pitch curve points are not ordered by time.");
        }
        previousTime = point.timeSec;
        if (!point.hasValidFrequency()) {
            report.addError("invalid_pitch_frequency",
                            "A pitch point has a non-positive frequency value.");
        }
        if (!point.hasValidConfidence()) {
            report.addError("invalid_pitch_confidence",
                            "A pitch point has confidence outside the 0.0-1.0 range.");
        }
    }

    for (const auto &bend: result.pitchBends) {
        if (!bend.hasValidConfidence()) {
            report.addError("invalid_pitch_bend_confidence",
                            "A pitch bend has confidence outside the 0.0-1.0 range.");
        }
        double previousBendTime = -1.0;
        for (const auto &point: bend.points) {
            if (!point.hasValidTime()) {
                report.addError("invalid_pitch_bend_time",
                                "A pitch bend point has negative time.");
            }
            if (point.timeSec < previousBendTime) {
                report.addError("pitch_bend_not_monotonic",
                                "Pitch bend points are not ordered by time.");
            }
            previousBendTime = point.timeSec;
        }
    }

    for (const auto &label: result.techniqueLabels) {
        if (!label.isValidTimeRange()) {
            report.addError("invalid_technique_timing",
                            "A technique label has invalid start/end timing.");
        }
        if (!label.hasValidConfidence()) {
            report.addError("invalid_technique_confidence",
                            "A technique label has confidence outside the 0.0-1.0 range.");
        }
    }

    for (const auto &warning: result.warnings) {
        if (!warning.hasValidTimeRange()) {
            report.addError("invalid_warning_timing",
                            "A result warning has invalid start/end timing.");
        }
    }

    for (const auto &error: result.errors) {
        if (!error.hasValidTimeRange()) {
            report.addError("invalid_error_timing",
                            "A result error has invalid start/end timing.");
        }
    }

    return report;
}

}
}
