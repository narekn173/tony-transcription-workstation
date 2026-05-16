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

#ifndef TONY_BACKEND_RUN_WORKSPACE_H
#define TONY_BACKEND_RUN_WORKSPACE_H

#include "ResultValidator.h"

#include <QString>

namespace Tony {
namespace Backend {

struct BackendRunWorkspacePreparationResult
{
    ValidationReport report;
    QString runDirectoryPath;
    QString logDirectoryPath;
    QString temporaryDirectoryPath;
    QString resultArtifactsDirectoryPath;
    bool runDirectoryCreated = false;
    bool logDirectoryCreated = false;
    bool temporaryDirectoryCreated = false;
    bool resultArtifactsDirectoryCreated = false;

    bool isValid() const;
};

struct BackendRunWorkspace
{
    QString baseWorkspaceDirectory;
    BackendId backendId;
    AnalysisRunId runId;
    QString safeBackendId;
    QString safeRunId;
    QString runDirectoryPath;
    QString logDirectoryPath;
    QString temporaryDirectoryPath;
    QString resultArtifactsDirectoryPath;
    QString logFilePath;
    QString stdoutLogPath;
    QString stderrLogPath;
    QString unifiedResultJsonPath;

    static BackendRunWorkspace fromParts(const QString &baseWorkspaceDirectory,
                                         const BackendId &backendId,
                                         const AnalysisRunId &runId);

    static QString sanitizePathComponent(const QString &value);
    static bool isSafePathComponent(const QString &value);

    ValidationReport validate() const;
    bool hasValidShape() const;
    BackendRunWorkspacePreparationResult prepareWorkspace() const;
    QString debugSummaryString() const;
};

}
}

#endif
