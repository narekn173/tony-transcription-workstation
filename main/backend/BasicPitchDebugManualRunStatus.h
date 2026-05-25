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

#ifndef TONY_BASIC_PITCH_DEBUG_MANUAL_RUN_STATUS_H
#define TONY_BASIC_PITCH_DEBUG_MANUAL_RUN_STATUS_H

#include "BasicPitchRealRunHandoffProof.h"

#include <QStringList>

namespace Tony {
namespace Backend {

struct BasicPitchDebugManualRunStatusResult
{
    ValidationReport report;
    QStringList requiredEnvironmentKeys;
    QStringList optionalEnvironmentKeys;
    QStringList missingConfigurationKeys;
    QString commandPath;
    QString inputAudioPath;
    QString outputDirectoryPath;
    QString resultJsonPath;
    QString derivedResultJsonPath;
    QString skippedReason;
    bool explicitOptIn = false;
    bool commandConfigured = false;
    bool inputAudioConfigured = false;
    bool outputDirectoryConfigured = false;
    bool resultJsonConfigured = false;
    bool resultJsonWillBeDerived = false;
    bool manualRunAllowed = false;
    bool manualRunWouldBeSkipped = true;
    bool productionTranscription = false;
    bool testOnlyDebugOnly = true;
    bool readyInstalledCompletedMutation = false;

    bool isValid() const;
    QString debugSummaryString() const;
};

class BasicPitchDebugManualRunStatus
{
public:
    BasicPitchDebugManualRunStatusResult fromConfig(
        const BasicPitchRealRunHandoffProofConfig &config) const;

    static QStringList requiredEnvironmentKeys();
    static QStringList optionalEnvironmentKeys();

private:
    static QString defaultResultJsonPath(const QString &outputDirectoryPath);
};

}
}

#endif
