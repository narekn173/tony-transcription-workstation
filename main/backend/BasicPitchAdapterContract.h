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

#ifndef TONY_BASIC_PITCH_ADAPTER_CONTRACT_H
#define TONY_BASIC_PITCH_ADAPTER_CONTRACT_H

#include "BackendTypes.h"
#include "ExternalProcessRunner.h"
#include "ResultValidator.h"

namespace Tony {
namespace Backend {

struct BasicPitchAdapterContractParameters
{
    QString executablePath = "basic-pitch";
    QString inputAudioPath;
    QString outputDirectoryPath;
    QString workingDirectoryPath;
    QString expectedUnifiedResultJsonPath;
    QString modelPath;
    QString modelSerialization;
    bool saveNoteEvents = true;
    bool saveModelOutputs = true;
    bool requestMultiplePitchBends = true;
    int timeoutMsec = 0;
    QMap<QString, QString> environmentOverrides;
};

struct BasicPitchAdapterContractResult
{
    ValidationReport report;
    BackendManifest manifest;
    ExternalProcessRequest request;
    QString outputDirectoryPath;
    QString expectedUnifiedResultJsonPath;
    QStringList expectedOutputArtifacts;
    bool possiblePolyphony = true;
    bool monophonicGuaranteed = false;
    bool pitchBendTonyMappingProven = false;
    bool confidenceTonyMappingProven = false;
    bool executesProcess = false;
    bool createsFakeResultJson = false;
    bool importsIntoTonyLayers = false;

    bool isValid() const;
    QString debugSummaryString() const;
};

class BasicPitchAdapterContract
{
public:
    static BackendId backendId();
    static QString cliCommandName();

    BackendManifest manifest() const;
    QStringList contractWarnings() const;
    BasicPitchAdapterContractResult buildCliRequest(
        const BasicPitchAdapterContractParameters &parameters) const;

private:
    static bool isSupportedModelSerialization(const QString &value);
};

}
}

#endif
