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

#ifndef TEST_UNIFIED_RESULT_H
#define TEST_UNIFIED_RESULT_H

#include "../ResultValidator.h"
#include "../UnifiedResult.h"

#include <QObject>
#include <QtTest>

using namespace Tony::Backend;

class TestUnifiedResult : public QObject
{
    Q_OBJECT

private slots:
    void defaultResultHasNoPayload()
    {
        UnifiedResult result;

        QVERIFY(result.isEmpty());
        QVERIFY(!result.hasNotes());
        QVERIFY(!result.hasPitchCurve());
        QVERIFY(!result.hasPitchBends());
        QVERIFY(!result.hasTechniqueLabels());
        QVERIFY(result.debugSummaryString().contains("status=unknown"));
    }

    void noteValidationAllowsOptionalFields()
    {
        NoteEvent note;
        note.startSec = 0.0;
        note.endSec = 0.0;

        QVERIFY(note.isValidTimeRange());
        QVERIFY(note.hasValidMidiPitch());
        QVERIFY(note.hasValidVelocity());
        QVERIFY(note.hasValidConfidence());

        note.midiPitch = 127;
        note.velocity = 0;
        note.confidence = 1.0;

        QVERIFY(note.hasValidMidiPitch());
        QVERIFY(note.hasValidVelocity());
        QVERIFY(note.hasValidConfidence());
    }

    void noteValidationRejectsOutOfRangeValues()
    {
        NoteEvent note;
        note.startSec = -0.1;
        note.endSec = 0.2;
        note.midiPitch = 128;
        note.velocity = -1;
        note.confidence = 1.1;

        QVERIFY(!note.isValidTimeRange());
        QVERIFY(!note.hasValidMidiPitch());
        QVERIFY(!note.hasValidVelocity());
        QVERIFY(!note.hasValidConfidence());
    }

    void validatorRejectsInvalidUnifiedResultValues()
    {
        BackendRequest request;
        request.requestId = "req_001";
        request.engineId = "basic_pitch";

        UnifiedResult result;
        result.requestId = "req_001";
        result.engine.engineId = "basic_pitch";

        NoteEvent note;
        note.startSec = 2.0;
        note.endSec = 1.0;
        note.midiPitch = 200;
        note.confidence = -0.01;
        result.notes.push_back(note);

        PitchPoint point;
        point.timeSec = 0.0;
        point.frequencyHz = 0.0;
        result.pitchCurve.push_back(point);

        ResultValidator validator;
        const ValidationReport report = validator.validate(request, result);

        QVERIFY(!report.isValid());
        QVERIFY(report.issues.size() >= 3);
    }

    void resultSummaryReportsPayloadCounts()
    {
        UnifiedResult result;
        result.resultId = "res_001";
        result.requestId = "req_001";
        result.engine.engineId = "basic_pitch";
        result.status = BackendStatus::Completed;

        result.notes.push_back(NoteEvent());
        result.pitchCurve.push_back(PitchPoint());
        result.pitchBends.push_back(PitchBend());
        result.techniqueLabels.push_back(TechniqueLabel());

        const QString summary = resultSummaryString(result);

        QVERIFY(!result.isEmpty());
        QVERIFY(result.hasNotes());
        QVERIFY(result.hasPitchCurve());
        QVERIFY(result.hasPitchBends());
        QVERIFY(result.hasTechniqueLabels());
        QVERIFY(summary.contains("notes=1"));
        QVERIFY(summary.contains("pitch_curve=1"));
        QVERIFY(summary.contains("pitch_bends=1"));
        QVERIFY(summary.contains("technique_labels=1"));
    }
};

#endif
