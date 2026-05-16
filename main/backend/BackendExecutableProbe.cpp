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

#include "BackendExecutableProbe.h"

#include <QFileInfo>

namespace Tony {
namespace Backend {

namespace {

QString
cleanPath(const QString &path)
{
    return path.trimmed();
}

BackendExecutableProbeResult
probePath(const BackendManifest &manifest,
          const QString &settingsExecutablePathOverride)
{
    BackendExecutableProbeResult result;
    result.backendId = manifest.id();
    result.manifestExecutablePath = cleanPath(manifest.executablePath);
    result.settingsExecutablePathOverride =
        cleanPath(settingsExecutablePathOverride);
    result.usedSettingsOverride =
        !result.settingsExecutablePathOverride.isEmpty();
    result.effectiveExecutablePath =
        result.usedSettingsOverride ?
            result.settingsExecutablePathOverride :
            result.manifestExecutablePath;

    if (result.effectiveExecutablePath.isEmpty()) {
        result.status = BackendExecutableProbeStatus::EmptyPath;
        result.report.addError(
            "empty_executable_path",
            "Backend executable path is empty.");
        return result;
    }

    const QFileInfo fileInfo(result.effectiveExecutablePath);
    result.exists = fileInfo.exists();
    result.isDirectory = fileInfo.exists() && fileInfo.isDir();
    result.isFile = fileInfo.exists() && fileInfo.isFile();
    result.isExecutable = fileInfo.exists() && fileInfo.isExecutable();

    if (!result.exists) {
        result.status = BackendExecutableProbeStatus::Missing;
        result.report.addError(
            "executable_missing",
            "Backend executable path does not exist.");
        return result;
    }

    if (result.isDirectory) {
        result.status = BackendExecutableProbeStatus::Directory;
        result.report.addError(
            "executable_path_is_directory",
            "Backend executable path points to a directory.");
        return result;
    }

    if (!result.isFile) {
        result.status = BackendExecutableProbeStatus::Missing;
        result.report.addError(
            "executable_not_file",
            "Backend executable path does not point to a regular file.");
        return result;
    }

    if (!result.isExecutable) {
        result.status = BackendExecutableProbeStatus::NotExecutable;
        result.report.addError(
            "executable_not_executable",
            "Backend executable path exists but is not executable.");
        return result;
    }

    result.status = BackendExecutableProbeStatus::Present;
    return result;
}

}

bool
BackendExecutableProbeResult::isValid() const
{
    return report.isValid();
}

bool
BackendExecutableProbeResult::isPresent() const
{
    return status == BackendExecutableProbeStatus::Present;
}

QString
BackendExecutableProbeResult::statusName() const
{
    return toString(status);
}

BackendExecutableProbeResult
BackendExecutableProbe::probe(const BackendManifest &manifest) const
{
    return probePath(manifest, QString());
}

BackendExecutableProbeResult
BackendExecutableProbe::probe(const BackendManifest &manifest,
                              const BackendSettings &settings) const
{
    return probePath(manifest, settings.executablePathOverride);
}

QString
toString(BackendExecutableProbeStatus status)
{
    switch (status) {
    case BackendExecutableProbeStatus::EmptyPath: return "empty_path";
    case BackendExecutableProbeStatus::Missing: return "missing";
    case BackendExecutableProbeStatus::Directory: return "directory";
    case BackendExecutableProbeStatus::NotExecutable: return "not_executable";
    case BackendExecutableProbeStatus::Present: return "present";
    }
    return "empty_path";
}

}
}
