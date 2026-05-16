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

#ifndef TONY_BACKEND_RUN_REQUEST_BUILDER_H
#define TONY_BACKEND_RUN_REQUEST_BUILDER_H

#include "BackendRunWorkspace.h"
#include "BackendSettingsStore.h"
#include "ExternalProcessRunner.h"

#include <optional>

namespace Tony {
namespace Backend {

struct BackendRunRequestParameters
{
    QString inputAudioFilePath;
    std::optional<AnalysisRegion> selectedRegion;
    QString expectedUnifiedResultJsonPath;
    int timeoutMsec = 0;
    QStringList additionalArguments;
};

struct BackendRunRequestBuildResult
{
    ExternalProcessRequest request;
    ValidationReport report;
    BackendId backendId;
    QString inputAudioFilePath;
    QString expectedUnifiedResultJsonPath;
    std::optional<AnalysisRegion> selectedRegion;
    bool usedSettingsExecutableOverride = false;
    bool usedSettingsWorkingDirectoryOverride = false;
    bool hasSelectedRegion = false;

    bool isValid() const;
    QString debugSummaryString() const;
};

class BackendRunRequestBuilder
{
public:
    BackendRunRequestBuildResult build(
        const BackendManifest &manifest,
        const BackendRunWorkspace &workspace,
        const BackendRunRequestParameters &parameters) const;

    BackendRunRequestBuildResult build(
        const BackendManifest &manifest,
        const std::optional<BackendSettings> &settings,
        const BackendRunWorkspace &workspace,
        const BackendRunRequestParameters &parameters) const;

private:
    static QString effectiveExecutablePath(
        const BackendManifest &manifest,
        const std::optional<BackendSettings> &settings,
        bool &usedSettingsOverride);

    static QString effectiveWorkingDirectory(
        const BackendManifest &manifest,
        const std::optional<BackendSettings> &settings,
        const BackendRunWorkspace &workspace,
        bool &usedSettingsOverride);
};

}
}

#endif
