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
#include "../UnifiedResultFileLoader.h"
#include "../UnifiedResultParser.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QTemporaryDir>
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

    void parserAcceptsValidUnifiedResultFixture()
    {
        const QByteArray json = R"json(
{
  "contract_version": "0.1",
  "result_id": "res_001",
  "request_id": "req_001",
  "created_at": "2026-05-15T12:01:00Z",
  "engine": {
    "engine_id": "basic_pitch",
    "display_name": "Basic Pitch",
    "engine_version": null,
    "adapter_version": "0.1.0",
    "runtime_type": "python_cli",
    "device_used": "cpu"
  },
  "status": "completed",
  "audio": {
    "path": "C:/audio/input.wav",
    "duration_sec": 12.345,
    "sample_rate_hz": 44100,
    "channels": 1
  },
  "region": null,
  "summary": {
    "note_count": 1,
    "pitch_point_count": 1,
    "pitch_bend_count": 0,
    "technique_label_count": 0,
    "mean_confidence": 0.91,
    "low_confidence_count": 0,
    "duration_analyzed_sec": 12.345
  },
  "notes": [
    {
      "id": "note_0001",
      "start_sec": 1.24,
      "end_sec": 1.68,
      "midi_pitch": 64,
      "frequency_hz": 329.63,
      "velocity": 82,
      "confidence": 0.91,
      "source": { "engine_id": "basic_pitch" },
      "flags": []
    }
  ],
  "pitch_curve": [
    {
      "time_sec": 1.25,
      "frequency_hz": 330.1,
      "confidence": 0.88,
      "voiced": true
    }
  ],
  "pitch_bends": [],
  "technique_labels": [],
  "files": [],
  "warnings": [
    {
      "code": "LOW_CONFIDENCE_REGION",
      "severity": "warning",
      "message": "Low confidence in a short region.",
      "start_sec": 10.2,
      "end_sec": 11.0,
      "details": {}
    }
  ],
  "errors": [],
  "provenance": {
    "created_by": "unit_test_fixture"
  }
}
)json";

        const QJsonDocument document = QJsonDocument::fromJson(json);
        QVERIFY(document.isObject());

        UnifiedResultParser parser;
        const UnifiedResultParseResult parsed = parser.parse(document.object());

        QVERIFY(parsed.isValid());
        QCOMPARE(parsed.result.resultId, QString("res_001"));
        QCOMPARE(parsed.result.requestId, QString("req_001"));
        QCOMPARE(parsed.result.engine.engineId, QString("basic_pitch"));
        QVERIFY(parsed.result.status == BackendStatus::Completed);
        QCOMPARE(parsed.result.notes.size(), 1);
        QCOMPARE(parsed.result.pitchCurve.size(), 1);
        QCOMPARE(parsed.result.warnings.size(), 1);
        QVERIFY(parsed.result.notes.front().midiPitch.has_value());
        QCOMPARE(*parsed.result.notes.front().midiPitch, 64);
    }

    void parserRejectsInvalidUnifiedResultFixture()
    {
        const QByteArray json = R"json(
{
  "contract_version": "0.1",
  "request_id": "req_001",
  "created_at": "2026-05-15T12:01:00Z",
  "engine": {
    "engine_id": "basic_pitch",
    "display_name": "Basic Pitch",
    "engine_version": null,
    "adapter_version": "0.1.0",
    "runtime_type": "python_cli",
    "device_used": "cpu"
  },
  "status": "completed",
  "audio": {
    "path": "C:/audio/input.wav",
    "duration_sec": 12.345,
    "sample_rate_hz": 44100,
    "channels": 1
  },
  "region": null,
  "summary": {
    "note_count": 1,
    "pitch_point_count": 0,
    "pitch_bend_count": 0,
    "technique_label_count": 0,
    "mean_confidence": 0.5,
    "low_confidence_count": 0,
    "duration_analyzed_sec": 12.345
  },
  "notes": [
    {
      "id": "note_0001",
      "start_sec": 2.0,
      "end_sec": 1.0,
      "midi_pitch": 128,
      "frequency_hz": 0.0,
      "velocity": 82,
      "confidence": 1.5,
      "source": {},
      "flags": []
    }
  ],
  "pitch_curve": [],
  "pitch_bends": [],
  "technique_labels": [],
  "files": [],
  "warnings": [
    {
      "code": "BAD_WARNING_RANGE",
      "severity": "warning",
      "message": "Invalid warning timing.",
      "start_sec": 5.0,
      "end_sec": 4.0
    }
  ],
  "errors": [],
  "provenance": {}
}
)json";

        const QJsonDocument document = QJsonDocument::fromJson(json);
        QVERIFY(document.isObject());

        UnifiedResultParser parser;
        const UnifiedResultParseResult parsed = parser.parse(document.object());

        QVERIFY(!parsed.isValid());
        QVERIFY(parsed.report.issues.size() >= 5);
    }

    void fileLoaderLoadsValidUnifiedResultFile()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());

        const QString path = dir.filePath("valid-result.json");
        QVERIFY(writeTextFile(path, validUnifiedResultJson()));

        UnifiedResultFileLoader loader;
        const UnifiedResultFileLoadResult loaded = loader.load(path);

        QVERIFY(loaded.isValid());
        QCOMPARE(loaded.path, path);
        QCOMPARE(loaded.result.resultId, QString("res_file_001"));
        QCOMPARE(loaded.result.engine.engineId, QString("basic_pitch"));
        QCOMPARE(loaded.result.notes.size(), 1);
    }

    void fileLoaderMissingFileFailsCleanly()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());

        UnifiedResultFileLoader loader;
        const UnifiedResultFileLoadResult loaded =
            loader.load(dir.filePath("missing-result.json"));

        QVERIFY(!loaded.isValid());
        QCOMPARE(loaded.report.issues.size(), 1);
        QCOMPARE(loaded.report.issues.front().code, QString("file_open_failed"));
    }

    void fileLoaderInvalidJsonFailsCleanly()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());

        const QString path = dir.filePath("invalid-json.json");
        QVERIFY(writeTextFile(path, "{ invalid json"));

        UnifiedResultFileLoader loader;
        const UnifiedResultFileLoadResult loaded = loader.load(path);

        QVERIFY(!loaded.isValid());
        QCOMPARE(loaded.report.issues.size(), 1);
        QCOMPARE(loaded.report.issues.front().code, QString("invalid_json"));
    }

    void fileLoaderWrongTopLevelJsonFailsCleanly()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());

        const QString path = dir.filePath("array-result.json");
        QVERIFY(writeTextFile(path, "[]"));

        UnifiedResultFileLoader loader;
        const UnifiedResultFileLoadResult loaded = loader.load(path);

        QVERIFY(!loaded.isValid());
        QCOMPARE(loaded.report.issues.size(), 1);
        QCOMPARE(loaded.report.issues.front().code,
                 QString("invalid_top_level_json"));
    }

private:
    static bool writeTextFile(const QString &path, const QByteArray &content)
    {
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            return false;
        }
        return file.write(content) == content.size();
    }

    static QByteArray validUnifiedResultJson()
    {
        return R"json(
{
  "contract_version": "0.1",
  "result_id": "res_file_001",
  "request_id": "req_file_001",
  "created_at": "2026-05-15T12:01:00Z",
  "engine": {
    "engine_id": "basic_pitch",
    "display_name": "Basic Pitch",
    "engine_version": null,
    "adapter_version": "0.1.0",
    "runtime_type": "python_cli",
    "device_used": "cpu"
  },
  "status": "completed",
  "audio": {
    "path": "C:/audio/input.wav",
    "duration_sec": 12.345,
    "sample_rate_hz": 44100,
    "channels": 1
  },
  "region": null,
  "summary": {
    "note_count": 1,
    "pitch_point_count": 0,
    "pitch_bend_count": 0,
    "technique_label_count": 0,
    "mean_confidence": 0.91,
    "low_confidence_count": 0,
    "duration_analyzed_sec": 12.345
  },
  "notes": [
    {
      "id": "note_0001",
      "start_sec": 1.24,
      "end_sec": 1.68,
      "midi_pitch": 64,
      "frequency_hz": 329.63,
      "velocity": 82,
      "confidence": 0.91,
      "source": { "engine_id": "basic_pitch" },
      "flags": []
    }
  ],
  "pitch_curve": [],
  "pitch_bends": [],
  "technique_labels": [],
  "files": [],
  "warnings": [],
  "errors": [],
  "provenance": {
    "created_by": "unit_test_file_fixture"
  }
}
)json";
    }
};

#endif
