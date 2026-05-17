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

#ifndef TONY_BACKEND_RUN_ORCHESTRATOR_H
#define TONY_BACKEND_RUN_ORCHESTRATOR_H

#include "BackendRunRequestPreparer.h"
#include "ExternalProcessLogFileSink.h"

#include <QVector>

#include <optional>

namespace Tony {
namespace Backend {

struct BackendRunOrchestrationParameters
{
    QString inputAudioFilePath;
    std::optional<AnalysisRegion> selectedRegion;
    QString expectedUnifiedResultJsonPath;
    QString requestJsonFilePath;
    QString requestJsonArgumentFlag = "--request";
    int timeoutMsec = 0;
    QStringList additionalArguments;
    bool prepareWorkspace = false;
    bool writeProcessLog = false;
    QString processLogFilePath;
};

struct BackendRunOrchestrationResult
{
    ValidationReport report;
    BackendRunWorkspacePreparationResult workspacePreparation;
    BackendRunRequestPreparationResult requestPreparation;
    ExternalProcessRequest processRequest;
    std::optional<ExternalProcessResult> processResult;
    ExternalProcessLogFileWriteResult logWriteResult;
    QVector<ExternalProcessEvent> processEvents;
    QString expectedUnifiedResultJsonPath;
    QString requestJsonFilePath;
    QString processLogFilePath;
    bool workspacePrepared = false;
    bool requestPrepared = false;
    bool processRunAttempted = false;
    bool processLogWritten = false;
    bool resultJsonExists = false;
    bool importedIntoTonyLayers = false;

    bool isValid() const;
    QString debugSummaryString() const;
};

class BackendRunOrchestrator
{
public:
    BackendRunOrchestrationResult prepareOnly(
        const BackendManifest &manifest,
        const BackendRunWorkspace &workspace,
        const BackendRunOrchestrationParameters &parameters) const;

    BackendRunOrchestrationResult prepareOnly(
        const BackendManifest &manifest,
        const std::optional<BackendSettings> &settings,
        const BackendRunWorkspace &workspace,
        const BackendRunOrchestrationParameters &parameters) const;

    BackendRunOrchestrationResult run(
        const BackendManifest &manifest,
        const BackendRunWorkspace &workspace,
        const BackendRunOrchestrationParameters &parameters) const;

    BackendRunOrchestrationResult run(
        const BackendManifest &manifest,
        const std::optional<BackendSettings> &settings,
        const BackendRunWorkspace &workspace,
        const BackendRunOrchestrationParameters &parameters) const;

private:
    static BackendRunRequestPreparationParameters toPreparationParameters(
        const BackendRunOrchestrationParameters &parameters);
};

}
}

#endif
