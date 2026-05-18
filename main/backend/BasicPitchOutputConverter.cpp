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

#include "BasicPitchOutputConverter.h"

#include "BasicPitchAdapterContract.h"

#include <QDateTime>
#include <QIODevice>
#include <QLocale>
#include <QTextStream>
#include <QVariant>

#include <algorithm>

namespace Tony {
namespace Backend {

namespace {

constexpr const char *kStartTimeColumn = "start_time_s";
constexpr const char *kEndTimeColumn = "end_time_s";
constexpr const char *kPitchMidiColumn = "pitch_midi";
constexpr const char *kVelocityColumn = "velocity";
constexpr const char *kPitchBendColumn = "pitch_bend";

void
addWarning(ValidationReport &report,
           const QString &code,
           const QString &message)
{
    report.addIssue(ValidationSeverity::Warning, code, message);
}

void
appendIssues(ValidationReport &target, const ValidationReport &source)
{
    for (const auto &issue: source.issues) {
        target.addIssue(issue.severity, issue.code, issue.message);
    }
}

bool
parseDouble(const QString &value, double &parsed)
{
    bool ok = false;
    parsed = QLocale::c().toDouble(value.trimmed(), &ok);
    return ok;
}

bool
parseInt(const QString &value, int &parsed)
{
    bool ok = false;
    parsed = QLocale::c().toInt(value.trimmed(), &ok);
    return ok;
}

ResultWarning
makeResultWarning(const QString &code,
                  const QString &message,
                  const QVariantMap &details = {})
{
    ResultWarning warning;
    warning.code = code;
    warning.severity = ResultDiagnosticSeverity::Warning;
    warning.message = message;
    warning.details = details;
    return warning;
}

bool
hasOverlappingNotes(QVector<BasicPitchNoteEvent> notes)
{
    std::sort(notes.begin(),
              notes.end(),
              [](const BasicPitchNoteEvent &lhs,
                 const BasicPitchNoteEvent &rhs) {
                  if (lhs.startSec == rhs.startSec) {
                      return lhs.endSec < rhs.endSec;
                  }
                  return lhs.startSec < rhs.startSec;
              });

    double latestEnd = -1.0;
    for (const BasicPitchNoteEvent &note: notes) {
        if (latestEnd >= 0.0 && note.startSec < latestEnd) {
            return true;
        }
        latestEnd = std::max(latestEnd, note.endSec);
    }
    return false;
}

PitchBend
makePitchBend(const BasicPitchNoteEvent &source, const QString &noteId)
{
    PitchBend bend;
    bend.id = noteId + "_pitch_bend";
    bend.noteId = noteId;
    bend.unit = "midi_pitch_bend_units";

    const int count = source.pitchBendValues.size();
    for (int i = 0; i < count; ++i) {
        PitchBendPoint point;
        if (count <= 1) {
            point.timeSec = source.startSec;
        } else {
            const double fraction = double(i) / double(count - 1);
            point.timeSec =
                source.startSec + ((source.endSec - source.startSec) * fraction);
        }
        point.value = source.pitchBendValues.at(i);
        bend.points.push_back(point);
    }

    return bend;
}

}

bool
BasicPitchNoteEvent::hasPitchBend() const
{
    return !pitchBendValues.isEmpty();
}

bool
BasicPitchNoteEventsParseResult::isValid() const
{
    return report.isValid();
}

bool
BasicPitchOutputConversionResult::isValid() const
{
    return report.isValid();
}

QString
BasicPitchOutputConversionResult::debugSummaryString() const
{
    return QString("backend=%1 notes=%2 pitch_bends=%3 warnings=%4 "
                   "valid=%5 fixture_only=%6 imported=%7")
        .arg(result.engine.engineId.isEmpty() ? QString("unknown") :
                                                result.engine.engineId)
        .arg(result.notes.size())
        .arg(result.pitchBends.size())
        .arg(result.warnings.size())
        .arg(isValid() ? QString("true") : QString("false"))
        .arg(fixtureOnly ? QString("true") : QString("false"))
        .arg(importsIntoTonyLayers ? QString("true") : QString("false"));
}

BasicPitchNoteEventsParseResult
BasicPitchNoteEventsParser::parseCsvText(const QString &csvText) const
{
    BasicPitchNoteEventsParseResult result;

    if (csvText.trimmed().isEmpty()) {
        result.report.addError("empty_basic_pitch_note_events_csv",
                               "Basic Pitch note-events CSV fixture is empty.");
        return result;
    }

    QString text = csvText;
    QTextStream stream(&text, QIODevice::ReadOnly);

    QString headerLine;
    while (!stream.atEnd() && headerLine.trimmed().isEmpty()) {
        headerLine = stream.readLine();
    }

    result.headerColumns = splitCsvLine(headerLine);
    if (result.headerColumns.size() < 5 ||
        result.headerColumns.value(0) != kStartTimeColumn ||
        result.headerColumns.value(1) != kEndTimeColumn ||
        result.headerColumns.value(2) != kPitchMidiColumn ||
        result.headerColumns.value(3) != kVelocityColumn ||
        result.headerColumns.value(4) != kPitchBendColumn) {
        result.report.addError(
            "invalid_basic_pitch_note_events_header",
            "Basic Pitch note-events CSV must start with "
            "start_time_s,end_time_s,pitch_midi,velocity,pitch_bend.");
        return result;
    }

    int rowNumber = 1;
    while (!stream.atEnd()) {
        ++rowNumber;
        const QString line = stream.readLine();
        if (line.trimmed().isEmpty()) {
            continue;
        }

        const QStringList values = splitCsvLine(line);
        if (values.size() < 4) {
            result.report.addError(
                "invalid_basic_pitch_note_events_row",
                QString("Basic Pitch note-events row %1 has too few columns.")
                    .arg(rowNumber));
            continue;
        }

        BasicPitchNoteEvent event;
        if (!parseDouble(values.at(0), event.startSec) ||
            !parseDouble(values.at(1), event.endSec)) {
            result.report.addError(
                "invalid_basic_pitch_note_time",
                QString("Basic Pitch note-events row %1 has invalid timing.")
                    .arg(rowNumber));
            continue;
        }
        if (!parseInt(values.at(2), event.midiPitch)) {
            result.report.addError(
                "invalid_basic_pitch_midi_pitch",
                QString("Basic Pitch note-events row %1 has invalid MIDI pitch.")
                    .arg(rowNumber));
            continue;
        }
        if (!parseInt(values.at(3), event.velocity)) {
            result.report.addError(
                "invalid_basic_pitch_velocity",
                QString("Basic Pitch note-events row %1 has invalid velocity.")
                    .arg(rowNumber));
            continue;
        }

        if (!isValidTimeRange(event.startSec, event.endSec)) {
            result.report.addError(
                "invalid_basic_pitch_note_time_range",
                QString("Basic Pitch note-events row %1 has invalid start/end.")
                    .arg(rowNumber));
        }
        if (!isValidMidiPitch(event.midiPitch)) {
            result.report.addError(
                "invalid_basic_pitch_midi_pitch_range",
                QString("Basic Pitch note-events row %1 MIDI pitch is outside 0-127.")
                    .arg(rowNumber));
        }
        if (!isValidVelocity(event.velocity)) {
            result.report.addError(
                "invalid_basic_pitch_velocity_range",
                QString("Basic Pitch note-events row %1 velocity is outside 0-127.")
                    .arg(rowNumber));
        }

        for (int i = 4; i < values.size(); ++i) {
            if (values.at(i).trimmed().isEmpty()) {
                continue;
            }
            double bendValue = 0.0;
            if (!parseDouble(values.at(i), bendValue)) {
                result.report.addError(
                    "invalid_basic_pitch_pitch_bend_value",
                    QString("Basic Pitch note-events row %1 has invalid pitch-bend value.")
                        .arg(rowNumber));
                continue;
            }
            event.pitchBendValues.push_back(bendValue);
        }

        result.noteEvents.push_back(event);
    }

    if (result.noteEvents.isEmpty()) {
        result.report.addError("missing_basic_pitch_note_events",
                               "Basic Pitch note-events CSV contains no note rows.");
    }

    return result;
}

QStringList
BasicPitchNoteEventsParser::splitCsvLine(const QString &line)
{
    QStringList values;
    QString current;
    bool inQuotes = false;

    for (int i = 0; i < line.size(); ++i) {
        const QChar ch = line.at(i);
        if (ch == QLatin1Char('"')) {
            if (inQuotes &&
                i + 1 < line.size() &&
                line.at(i + 1) == QLatin1Char('"')) {
                current.append(ch);
                ++i;
            } else {
                inQuotes = !inQuotes;
            }
        } else if (ch == QLatin1Char(',') && !inQuotes) {
            values.push_back(current.trimmed());
            current.clear();
        } else {
            current.append(ch);
        }
    }
    values.push_back(current.trimmed());
    return values;
}

BasicPitchOutputConversionResult
BasicPitchOutputConverter::convertNoteEventsCsv(
    const QString &csvText,
    const BasicPitchOutputConversionParameters &parameters) const
{
    BasicPitchOutputConversionResult conversion;
    conversion.fixtureOnly = parameters.fixtureOnly;
    conversion.productionTranscription = parameters.productionTranscription;

    BasicPitchNoteEventsParser parser;
    conversion.parsedNoteEvents = parser.parseCsvText(csvText);
    appendIssues(conversion.report, conversion.parsedNoteEvents.report);

    UnifiedResult &result = conversion.result;
    result.contractVersion = "0.1";
    result.resultId = parameters.resultId.trimmed().isEmpty() ?
        QString("basic_pitch_fixture_result") :
        parameters.resultId.trimmed();
    result.requestId = parameters.requestId.trimmed().isEmpty() ?
        QString("basic_pitch_fixture_request") :
        parameters.requestId.trimmed();
    result.createdAt = QDateTime::currentDateTimeUtc();
    result.engine.engineId = BasicPitchAdapterContract::backendId();
    result.engine.displayName = "Basic Pitch";
    result.engine.adapterVersion = "fixture-converter-0.1";
    result.engine.runtimeType = BackendRuntimeType::PythonCli;
    result.status = BackendStatus::Unknown;
    result.audio.path = parameters.inputAudioPath.trimmed();
    result.provenance.insert("source_format",
                             parameters.fixtureOnly ?
                                 QString("basic_pitch_note_events_csv_fixture") :
                                 QString("basic_pitch_note_events_csv_artifact"));
    result.provenance.insert("source_artifact_path",
                             parameters.sourceArtifactPath.trimmed());
    result.provenance.insert("fixture_only_conversion",
                             parameters.fixtureOnly);
    result.provenance.insert("production_transcription",
                             parameters.productionTranscription);
    result.provenance.insert("created_result_json", false);
    result.provenance.insert("imported_into_tony_layers", false);

    if (!conversion.parsedNoteEvents.isValid()) {
        return conversion;
    }

    conversion.detectedPolyphony =
        hasOverlappingNotes(conversion.parsedNoteEvents.noteEvents);

    double latestEndSec = 0.0;
    int index = 0;
    for (const BasicPitchNoteEvent &parsed:
         conversion.parsedNoteEvents.noteEvents) {
        ++index;
        const QString noteId =
            QString("basic_pitch_fixture_note_%1")
                .arg(index, 4, 10, QLatin1Char('0'));

        NoteEvent note;
        note.id = noteId;
        note.startSec = parsed.startSec;
        note.endSec = parsed.endSec;
        note.midiPitch = parsed.midiPitch;
        note.velocity = parsed.velocity;
        note.source.insert("backend_id", BasicPitchAdapterContract::backendId());
        note.source.insert("source_format", "basic_pitch_note_events_csv");
        note.source.insert("fixture_only", parameters.fixtureOnly);
        note.source.insert("production_transcription",
                           parameters.productionTranscription);
        note.flags << "basic_pitch";
        if (parameters.fixtureOnly) {
            note.flags << "fixture_only";
        } else {
            note.flags << "real_artifact_manual_only";
        }
        if (!parameters.productionTranscription) {
            note.flags << "production_transcription_false";
        }

        if (parsed.hasPitchBend()) {
            const PitchBend bend = makePitchBend(parsed, noteId);
            note.pitchBendRef = bend.id;
            result.pitchBends.push_back(bend);
            conversion.pitchBendDataPreserved = true;
            conversion.pitchBendTonyMappingDeferred = true;
        }

        latestEndSec = std::max(latestEndSec, parsed.endSec);
        result.notes.push_back(note);
    }

    result.summary.noteCount = result.notes.size();
    result.summary.pitchBendCount = result.pitchBends.size();
    result.summary.durationAnalyzedSec = latestEndSec;

    QVariantMap polyphonyDetails;
    polyphonyDetails.insert("detected_polyphony", conversion.detectedPolyphony);
    polyphonyDetails.insert("monophonic_guaranteed", false);
    const QString polyphonyMessage = conversion.detectedPolyphony ?
        QString("Basic Pitch fixture contains overlapping notes; do not "
                "force into one monophonic Tony note layer.") :
        QString("Basic Pitch output can be polyphonic; this fixture alone "
                "does not prove monophonic behavior.");
    result.warnings.push_back(
        makeResultWarning("possible_polyphony",
                          polyphonyMessage,
                          polyphonyDetails));
    addWarning(conversion.report,
               "possible_polyphony",
               polyphonyMessage);

    if (conversion.pitchBendDataPreserved) {
        result.warnings.push_back(
            makeResultWarning(
                "pitch_bend_mapping_deferred",
                "Basic Pitch pitch-bend values were preserved in UnifiedResult, "
                "but Tony pitch-bend layer mapping is not proven."));
        addWarning(conversion.report,
                   "pitch_bend_mapping_deferred",
                   "Basic Pitch pitch-bend values were preserved in UnifiedResult, "
                   "but Tony pitch-bend layer mapping is not proven.");
    }

    if (parameters.fixtureOnly) {
        result.warnings.push_back(
            makeResultWarning("fixture_only_conversion",
                              "Converted from a fixture-backed Basic Pitch CSV; "
                              "this is not production transcription proof."));
        addWarning(conversion.report,
                   "fixture_only_conversion",
                   "Converted from a fixture-backed Basic Pitch CSV.");
    } else {
        result.warnings.push_back(
            makeResultWarning("real_artifact_manual_only",
                              "Converted from a discovered Basic Pitch CSV "
                              "artifact in a manual/test-only path; this is "
                              "not production transcription proof."));
        addWarning(conversion.report,
                   "real_artifact_manual_only",
                   "Converted from a discovered Basic Pitch CSV artifact in a "
                   "manual/test-only path.");
    }
    result.warnings.push_back(
        makeResultWarning("production_transcription_false",
                          "This Basic Pitch conversion is explicitly not a "
                          "production transcription result."));
    addWarning(conversion.report,
               "production_transcription_false",
               "This conversion is not production transcription proof.");

    return conversion;
}

}
}
