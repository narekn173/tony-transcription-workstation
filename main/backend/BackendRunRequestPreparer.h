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

#ifndef TONY_BACKEND_RUN_REQUEST_PREPARER_H
#define TONY_BACKEND_RUN_REQUEST_PREPARER_H

#include "BackendRunRequestFileWriter.h"

#include <optional>

namespace Tony {
namespace Backend {

struct BackendRunRequestPreparationParameters
{
    QString inputAudioFilePath;
    std::optional<AnalysisRegion> selectedRegion;
    QString expectedUnifiedResultJsonPath;
    QString requestJsonFilePath;
    QString requestJsonArgumentFlag = "--request";
    int timeoutMsec = 0;
    QStringList additionalArguments;
};

struct BackendRunRequestPreparationResult
{
    ExternalProcessRequest processRequest;
    BackendRunRequestBuildResult builtRequest;
    BackendRunRequestFileWriteResult fileWriteResult;
    ValidationReport report;
    QString requestJsonFilePath;
    qint64 bytesWritten = 0;
    bool requestFileWritten = false;

    bool isValid() const;
    QString debugSummaryString() const;
};

class BackendRunRequestPreparer
{
public:
    BackendRunRequestPreparationResult prepare(
        const BackendManifest &manifest,
        const BackendRunWorkspace &workspace,
        const BackendRunRequestPreparationParameters &parameters) const;

    BackendRunRequestPreparationResult prepare(
        const BackendManifest &manifest,
        const std::optional<BackendSettings> &settings,
        const BackendRunWorkspace &workspace,
        const BackendRunRequestPreparationParameters &parameters) const;
};

}
}

#endif
