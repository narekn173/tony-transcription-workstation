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

#ifndef TONY_BASIC_PITCH_ARTIFACT_DISCOVERY_H
#define TONY_BASIC_PITCH_ARTIFACT_DISCOVERY_H

#include "ExternalProcessRunner.h"
#include "ResultValidator.h"

#include <QMap>
#include <QProcessEnvironment>
#include <QString>
#include <QStringList>
#include <QVector>

namespace Tony {
namespace Backend {

struct BasicPitchArtifactDiscoveryConfig
{
    bool explicitOptIn = false;
    QString executablePath = "basic-pitch";
    QString inputAudioPath;
    QString outputDirectoryPath;
    bool saveMidi = true;
    bool saveNoteEvents = true;
    bool saveModelOutputs = true;
    bool sonifyMidi = false;
    bool multiplePitchBends = true;
    int timeoutMsec = 0;
    QMap<QString, QString> environmentOverrides;
};

struct BasicPitchDiscoveredArtifact
{
    QString path;
    QString fileName;
    QString artifactType = "unknown";
    qint64 sizeBytes = 0;
};

struct BasicPitchArtifactDiscoveryResult
{
    ValidationReport report;
    bool ranBasicPitch = false;
    QString skippedReason;
    QString commandUsed;
    QString outputDirectoryPath;
    ExternalProcessRequest request;
    ExternalProcessResult processResult;
    QVector<BasicPitchDiscoveredArtifact> discoveredArtifacts;
    QString standardOutputSummary;
    QString standardErrorSummary;
    bool productionTranscription = false;
    bool importedIntoTonyLayers = false;
    bool readyInstalledCompletedMutation = false;

    bool isValid() const;
    bool wasSkipped() const;
    QString debugSummaryString() const;
};

class BasicPitchArtifactDiscovery
{
public:
    BasicPitchArtifactDiscoveryResult buildRequest(
        const BasicPitchArtifactDiscoveryConfig &config) const;

    BasicPitchArtifactDiscoveryResult inspectOutputDirectory(
        const QString &outputDirectoryPath) const;

    BasicPitchArtifactDiscoveryResult runDiscovery(
        const BasicPitchArtifactDiscoveryConfig &config) const;

    static BasicPitchArtifactDiscoveryConfig configFromEnvironment();
    static BasicPitchArtifactDiscoveryConfig configFromEnvironment(
        const QProcessEnvironment &environment);

private:
    static QString classifyArtifact(const QString &path);
    static QString summarizeOutput(const QString &text);
};

}
}

#endif
