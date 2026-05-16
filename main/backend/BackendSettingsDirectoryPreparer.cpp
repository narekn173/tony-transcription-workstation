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

#include "BackendSettingsDirectoryPreparer.h"

#include <QDir>
#include <QFileInfo>

namespace Tony {
namespace Backend {

namespace {

QString
cleanPath(const QString &path)
{
    return path.trimmed();
}

bool
hasNoExplicitParentDirectory(const QFileInfo &fileInfo)
{
    const QString parent = fileInfo.path();
    return parent.isEmpty() || parent == ".";
}

}

bool
BackendSettingsDirectoryPreparationResult::isValid() const
{
    return report.isValid();
}

BackendSettingsDirectoryPreparationResult
BackendSettingsDirectoryPreparer::inspectParentDirectory(
    const QString &settingsFilePath) const
{
    BackendSettingsDirectoryPreparationResult result;
    result.settingsFilePath = cleanPath(settingsFilePath);

    if (result.settingsFilePath.isEmpty()) {
        result.report.addError(
            "empty_settings_file_path",
            "Backend settings file path is empty.");
        return result;
    }

    const QFileInfo fileInfo(result.settingsFilePath);
    if (hasNoExplicitParentDirectory(fileInfo)) {
        result.report.addError(
            "missing_parent_directory",
            "Backend settings file path has no parent directory.");
        return result;
    }

    result.parentDirectoryPath = QDir::cleanPath(fileInfo.path());
    const QFileInfo parentInfo(result.parentDirectoryPath);
    result.directoryAlreadyExisted =
        parentInfo.exists() && parentInfo.isDir();

    if (parentInfo.exists() && !parentInfo.isDir()) {
        result.report.addError(
            "parent_path_not_directory",
            "Backend settings parent path exists but is not a directory.");
    }

    return result;
}

BackendSettingsDirectoryPreparationResult
BackendSettingsDirectoryPreparer::prepareParentDirectory(
    const QString &settingsFilePath) const
{
    BackendSettingsDirectoryPreparationResult result =
        inspectParentDirectory(settingsFilePath);

    if (!result.isValid() || result.directoryAlreadyExisted) {
        return result;
    }

    if (!QDir().mkpath(result.parentDirectoryPath)) {
        result.report.addError(
            "parent_directory_create_failed",
            "Unable to create BackendSettings parent directory.");
        return result;
    }

    result.directoryCreated = true;
    result.directoryAlreadyExisted = false;
    return result;
}

}
}
