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

#include "BackendRunWorkspace.h"

#include <QDir>
#include <QFileInfo>

namespace Tony {
namespace Backend {

namespace {

QString
cleanPath(const QString &path)
{
    return QDir::cleanPath(path.trimmed());
}

QString
joinPath(const QString &base, const QString &child)
{
    return QDir(base).filePath(child);
}

bool
mkpathIfNeeded(const QString &path, bool &created)
{
    const QFileInfo info(path);
    if (info.exists() && info.isDir()) {
        created = false;
        return true;
    }
    if (info.exists() && !info.isDir()) {
        return false;
    }

    created = QDir().mkpath(path);
    return created;
}

void
addSanitizationWarning(ValidationReport &report,
                       const QString &code,
                       const QString &message)
{
    report.addIssue(ValidationSeverity::Warning, code, message);
}

}

bool
BackendRunWorkspacePreparationResult::isValid() const
{
    return report.isValid();
}

BackendRunWorkspace
BackendRunWorkspace::fromParts(const QString &baseWorkspaceDirectory,
                               const BackendId &backendId,
                               const AnalysisRunId &runId)
{
    BackendRunWorkspace workspace;
    workspace.baseWorkspaceDirectory = cleanPath(baseWorkspaceDirectory);
    workspace.backendId = backendId.trimmed();
    workspace.runId = runId.trimmed();
    workspace.safeBackendId = sanitizePathComponent(workspace.backendId);
    workspace.safeRunId = sanitizePathComponent(workspace.runId);

    if (!workspace.baseWorkspaceDirectory.isEmpty() &&
        !workspace.safeBackendId.isEmpty() &&
        !workspace.safeRunId.isEmpty()) {
        workspace.runDirectoryPath =
            joinPath(joinPath(workspace.baseWorkspaceDirectory,
                              workspace.safeBackendId),
                     workspace.safeRunId);
        workspace.logDirectoryPath =
            joinPath(workspace.runDirectoryPath, "logs");
        workspace.temporaryDirectoryPath =
            joinPath(workspace.runDirectoryPath, "temp");
        workspace.resultArtifactsDirectoryPath =
            joinPath(workspace.runDirectoryPath, "outputs");
        workspace.logFilePath =
            joinPath(workspace.logDirectoryPath, "process.log");
        workspace.stdoutLogPath =
            joinPath(workspace.logDirectoryPath, "stdout.log");
        workspace.stderrLogPath =
            joinPath(workspace.logDirectoryPath, "stderr.log");
        workspace.unifiedResultJsonPath =
            joinPath(workspace.runDirectoryPath, "result.json");
    }

    return workspace;
}

QString
BackendRunWorkspace::sanitizePathComponent(const QString &value)
{
    QString sanitized;

    for (const QChar ch: value.trimmed()) {
        const bool isLower = ch >= QLatin1Char('a') &&
            ch <= QLatin1Char('z');
        const bool isUpper = ch >= QLatin1Char('A') &&
            ch <= QLatin1Char('Z');
        const bool isDigit = ch >= QLatin1Char('0') &&
            ch <= QLatin1Char('9');

        if (isLower || isUpper || isDigit ||
            ch == QLatin1Char('_') || ch == QLatin1Char('-')) {
            sanitized.append(ch);
        } else {
            sanitized.append(QLatin1Char('_'));
        }
    }

    while (sanitized.contains("__")) {
        sanitized.replace("__", "_");
    }

    sanitized = sanitized.trimmed();
    while (sanitized.startsWith('_') || sanitized.startsWith('-')) {
        sanitized.remove(0, 1);
    }
    while (sanitized.endsWith('_') || sanitized.endsWith('-')) {
        sanitized.chop(1);
    }

    if (sanitized == "." || sanitized == "..") {
        return QString();
    }

    return sanitized;
}

bool
BackendRunWorkspace::isSafePathComponent(const QString &value)
{
    return !value.trimmed().isEmpty() &&
        sanitizePathComponent(value) == value.trimmed();
}

ValidationReport
BackendRunWorkspace::validate() const
{
    ValidationReport report;

    if (baseWorkspaceDirectory.trimmed().isEmpty()) {
        report.addError("empty_base_workspace_directory",
                        "Backend run workspace base directory is empty.");
    }
    if (backendId.trimmed().isEmpty()) {
        report.addError("empty_backend_id",
                        "Backend run workspace backend ID is empty.");
    }
    if (runId.trimmed().isEmpty()) {
        report.addError("empty_run_id",
                        "Backend run workspace run ID is empty.");
    }
    if (!backendId.trimmed().isEmpty() && safeBackendId.isEmpty()) {
        report.addError("invalid_backend_id_path_component",
                        "Backend ID cannot be converted to a safe path component.");
    }
    if (!runId.trimmed().isEmpty() && safeRunId.isEmpty()) {
        report.addError("invalid_run_id_path_component",
                        "Run ID cannot be converted to a safe path component.");
    }
    if (!backendId.trimmed().isEmpty() &&
        backendId.trimmed() != safeBackendId) {
        addSanitizationWarning(
            report,
            "backend_id_sanitized",
            "Backend ID was sanitized before being used in workspace paths.");
    }
    if (!runId.trimmed().isEmpty() && runId.trimmed() != safeRunId) {
        addSanitizationWarning(
            report,
            "run_id_sanitized",
            "Run ID was sanitized before being used in workspace paths.");
    }
    if (runDirectoryPath.trimmed().isEmpty()) {
        report.addError("empty_run_directory_path",
                        "Backend run workspace directory path is empty.");
    }
    if (logDirectoryPath.trimmed().isEmpty() ||
        temporaryDirectoryPath.trimmed().isEmpty() ||
        resultArtifactsDirectoryPath.trimmed().isEmpty() ||
        logFilePath.trimmed().isEmpty() ||
        stdoutLogPath.trimmed().isEmpty() ||
        stderrLogPath.trimmed().isEmpty() ||
        unifiedResultJsonPath.trimmed().isEmpty()) {
        report.addError("empty_workspace_output_path",
                        "One or more BackendRunWorkspace output paths are empty.");
    }

    return report;
}

bool
BackendRunWorkspace::hasValidShape() const
{
    return validate().isValid();
}

BackendRunWorkspacePreparationResult
BackendRunWorkspace::prepareWorkspace() const
{
    BackendRunWorkspacePreparationResult prepared;
    prepared.runDirectoryPath = runDirectoryPath;
    prepared.logDirectoryPath = logDirectoryPath;
    prepared.temporaryDirectoryPath = temporaryDirectoryPath;
    prepared.resultArtifactsDirectoryPath = resultArtifactsDirectoryPath;
    prepared.report = validate();

    if (!prepared.isValid()) {
        return prepared;
    }

    if (!mkpathIfNeeded(runDirectoryPath, prepared.runDirectoryCreated)) {
        prepared.report.addError("run_directory_create_failed",
                                 "Unable to create backend run workspace directory.");
        return prepared;
    }
    if (!mkpathIfNeeded(logDirectoryPath, prepared.logDirectoryCreated)) {
        prepared.report.addError("log_directory_create_failed",
                                 "Unable to create backend run log directory.");
        return prepared;
    }
    if (!mkpathIfNeeded(temporaryDirectoryPath,
                        prepared.temporaryDirectoryCreated)) {
        prepared.report.addError("temporary_directory_create_failed",
                                 "Unable to create backend run temporary directory.");
        return prepared;
    }
    if (!mkpathIfNeeded(resultArtifactsDirectoryPath,
                        prepared.resultArtifactsDirectoryCreated)) {
        prepared.report.addError("outputs_directory_create_failed",
                                 "Unable to create backend run output artifacts directory.");
        return prepared;
    }

    return prepared;
}

QString
BackendRunWorkspace::debugSummaryString() const
{
    return QString("backend=%1 run_id=%2 workspace=%3 logs=%4 temp=%5 "
                   "outputs=%6 result=%7")
        .arg(safeBackendId)
        .arg(safeRunId)
        .arg(runDirectoryPath)
        .arg(logDirectoryPath)
        .arg(temporaryDirectoryPath)
        .arg(resultArtifactsDirectoryPath)
        .arg(unifiedResultJsonPath);
}

}
}
