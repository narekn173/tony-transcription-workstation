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

#ifndef TONY_BASIC_PITCH_OUTPUT_CONVERTER_H
#define TONY_BASIC_PITCH_OUTPUT_CONVERTER_H

#include "ResultValidator.h"
#include "UnifiedResult.h"

#include <QString>
#include <QStringList>
#include <QVector>

namespace Tony {
namespace Backend {

struct BasicPitchNoteEvent
{
    double startSec = 0.0;
    double endSec = 0.0;
    int midiPitch = -1;
    int velocity = -1;
    QVector<double> pitchBendValues;

    bool hasPitchBend() const;
};

struct BasicPitchNoteEventsParseResult
{
    ValidationReport report;
    QStringList headerColumns;
    QVector<BasicPitchNoteEvent> noteEvents;

    bool isValid() const;
};

struct BasicPitchOutputConversionParameters
{
    QString requestId = "basic_pitch_fixture_request";
    QString resultId = "basic_pitch_fixture_result";
    QString inputAudioPath;
    QString sourceArtifactPath;
    bool fixtureOnly = true;
    bool productionTranscription = false;
};

struct BasicPitchOutputConversionResult
{
    ValidationReport report;
    UnifiedResult result;
    BasicPitchNoteEventsParseResult parsedNoteEvents;
    bool fixtureOnly = true;
    bool productionTranscription = false;
    bool possiblePolyphony = true;
    bool detectedPolyphony = false;
    bool pitchBendDataPreserved = false;
    bool pitchBendTonyMappingDeferred = false;
    bool createdResultJson = false;
    bool importsIntoTonyLayers = false;
    bool marksBackendReadyInstalledOrCompleted = false;

    bool isValid() const;
    QString debugSummaryString() const;
};

class BasicPitchNoteEventsParser
{
public:
    BasicPitchNoteEventsParseResult parseCsvText(const QString &csvText) const;

private:
    static QStringList splitCsvLine(const QString &line);
};

class BasicPitchOutputConverter
{
public:
    BasicPitchOutputConversionResult convertNoteEventsCsv(
        const QString &csvText,
        const BasicPitchOutputConversionParameters &parameters) const;
};

}
}

#endif
