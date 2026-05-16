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

#include "BackendSettingsPathResolver.h"

#include <QDir>
#include <QStandardPaths>

namespace Tony {
namespace Backend {

namespace {

QString
cleanPathText(const QString &text)
{
    return text.trimmed();
}

void
appendIssues(ValidationReport &target, const ValidationReport &source)
{
    for (const auto &issue: source.issues) {
        target.addIssue(issue.severity, issue.code, issue.message);
    }
}

bool
containsInvalidFileNameCharacter(const QString &fileName)
{
    static const QString invalidCharacters = "/\\:*?\"<>|";
    for (const QChar character: fileName) {
        if (invalidCharacters.contains(character)) {
            return true;
        }
    }
    return false;
}

}

bool
BackendSettingsPathResolutionResult::isValid() const
{
    return report.isValid();
}

QString
BackendSettingsPathResolver::defaultFileName()
{
    return "backend_settings.json";
}

ValidationReport
BackendSettingsPathResolver::validateFileName(const QString &fileName)
{
    ValidationReport report;
    const QString cleaned = cleanPathText(fileName);

    if (cleaned.isEmpty()) {
        report.addError("empty_settings_file_name",
                        "Backend settings file name is empty.");
        return report;
    }

    if (cleaned == "." || cleaned == ".." ||
        containsInvalidFileNameCharacter(cleaned)) {
        report.addError(
            "invalid_settings_file_name",
            "Backend settings file name must be a file name, not a path.");
    }

    return report;
}

BackendSettingsPathResolutionResult
BackendSettingsPathResolver::resolveDefaultPath(const QString &fileName) const
{
    QString baseDirectory =
        QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);

    if (cleanPathText(baseDirectory).isEmpty()) {
        baseDirectory =
            QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    }

    return resolveDefaultPathFromBaseDirectory(baseDirectory, fileName);
}

BackendSettingsPathResolutionResult
BackendSettingsPathResolver::resolveDefaultPathFromBaseDirectory(
    const QString &baseDirectory,
    const QString &fileName) const
{
    BackendSettingsPathResolutionResult resolved;
    resolved.baseDirectory = cleanPathText(baseDirectory);
    resolved.fileName = cleanPathText(fileName);

    appendIssues(resolved.report, validateFileName(resolved.fileName));

    if (resolved.baseDirectory.isEmpty()) {
        resolved.report.addError(
            "empty_platform_settings_directory",
            "Qt did not provide a usable application settings directory.");
        return resolved;
    }

    if (!resolved.report.isValid()) {
        return resolved;
    }

    resolved.path = QDir(resolved.baseDirectory).filePath(resolved.fileName);
    return resolved;
}

BackendSettingsPathResolutionResult
BackendSettingsPathResolver::resolveTestOverridePath(const QString &path) const
{
    BackendSettingsPathResolutionResult resolved;
    resolved.path = cleanPathText(path);
    resolved.usedTestOverride = true;

    if (resolved.path.isEmpty()) {
        resolved.report.addError("empty_test_settings_path",
                                 "Test backend settings path is empty.");
    }

    return resolved;
}

}
}
