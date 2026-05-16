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

#include "BackendRequiredFileProbe.h"

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
roleIsRequired(BackendRequiredFileProbeRole role)
{
    return role == BackendRequiredFileProbeRole::RequiredFile ||
        role == BackendRequiredFileProbeRole::ModelCheckpoint;
}

QString
missingCode(BackendRequiredFileProbeRole role)
{
    switch (role) {
    case BackendRequiredFileProbeRole::RequiredFile:
        return "required_file_missing";
    case BackendRequiredFileProbeRole::OptionalFile:
        return "optional_file_missing";
    case BackendRequiredFileProbeRole::ModelCheckpoint:
        return "model_checkpoint_missing";
    }
    return "required_file_missing";
}

QString
directoryCode(BackendRequiredFileProbeRole role)
{
    switch (role) {
    case BackendRequiredFileProbeRole::RequiredFile:
        return "required_file_path_is_directory";
    case BackendRequiredFileProbeRole::OptionalFile:
        return "optional_file_path_is_directory";
    case BackendRequiredFileProbeRole::ModelCheckpoint:
        return "model_checkpoint_path_is_directory";
    }
    return "required_file_path_is_directory";
}

QString
emptyCode(BackendRequiredFileProbeRole role)
{
    switch (role) {
    case BackendRequiredFileProbeRole::RequiredFile:
        return "required_file_empty_path";
    case BackendRequiredFileProbeRole::OptionalFile:
        return "optional_file_empty_path";
    case BackendRequiredFileProbeRole::ModelCheckpoint:
        return "model_checkpoint_empty_path";
    }
    return "required_file_empty_path";
}

QString
messagePrefix(BackendRequiredFileProbeRole role)
{
    switch (role) {
    case BackendRequiredFileProbeRole::RequiredFile:
        return "Required backend file";
    case BackendRequiredFileProbeRole::OptionalFile:
        return "Optional backend file";
    case BackendRequiredFileProbeRole::ModelCheckpoint:
        return "Backend model/checkpoint";
    }
    return "Required backend file";
}

void
addIssue(ValidationReport &report,
         BackendRequiredFileProbeRole role,
         ValidationSeverity severity,
         const QString &code,
         const QString &message)
{
    if (roleIsRequired(role)) {
        report.addError(code, message);
    } else {
        report.addIssue(severity, code, message);
    }
}

BackendRequiredFileProbeEntry
probePath(ValidationReport &report,
          BackendRequiredFileProbeRole role,
          const QString &path)
{
    BackendRequiredFileProbeEntry entry;
    entry.role = role;
    entry.path = cleanPath(path);

    if (entry.path.isEmpty()) {
        entry.status = BackendRequiredFileProbeStatus::EmptyPath;
        addIssue(report,
                 role,
                 ValidationSeverity::Warning,
                 emptyCode(role),
                 QString("%1 path is empty.").arg(messagePrefix(role)));
        return entry;
    }

    const QFileInfo fileInfo(entry.path);
    entry.exists = fileInfo.exists();
    entry.isDirectory = fileInfo.exists() && fileInfo.isDir();
    entry.isFile = fileInfo.exists() && fileInfo.isFile();

    if (!entry.exists) {
        entry.status = BackendRequiredFileProbeStatus::Missing;
        addIssue(report,
                 role,
                 ValidationSeverity::Warning,
                 missingCode(role),
                 QString("%1 path does not exist.").arg(messagePrefix(role)));
        return entry;
    }

    if (entry.isDirectory) {
        entry.status = BackendRequiredFileProbeStatus::Directory;
        addIssue(report,
                 role,
                 ValidationSeverity::Warning,
                 directoryCode(role),
                 QString("%1 path points to a directory.").arg(messagePrefix(role)));
        return entry;
    }

    if (!entry.isFile) {
        entry.status = BackendRequiredFileProbeStatus::Missing;
        addIssue(report,
                 role,
                 ValidationSeverity::Warning,
                 missingCode(role),
                 QString("%1 path does not point to a regular file.")
                     .arg(messagePrefix(role)));
        return entry;
    }

    entry.status = BackendRequiredFileProbeStatus::Present;
    return entry;
}

BackendRequiredFileProbeResult
probeFiles(const BackendManifest &manifest,
           const QString &modelCheckpointPathOverride)
{
    BackendRequiredFileProbeResult result;
    result.backendId = manifest.id();

    for (const auto &path: manifest.requiredFiles) {
        result.entries.push_back(
            probePath(result.report,
                      BackendRequiredFileProbeRole::RequiredFile,
                      path));
    }

    for (const auto &path: manifest.optionalFiles) {
        result.entries.push_back(
            probePath(result.report,
                      BackendRequiredFileProbeRole::OptionalFile,
                      path));
    }

    const QString cleanModelCheckpointPath =
        cleanPath(modelCheckpointPathOverride);
    if (!cleanModelCheckpointPath.isEmpty()) {
        result.entries.push_back(
            probePath(result.report,
                      BackendRequiredFileProbeRole::ModelCheckpoint,
                      cleanModelCheckpointPath));
    }

    return result;
}

}

bool
BackendRequiredFileProbeEntry::isPresent() const
{
    return status == BackendRequiredFileProbeStatus::Present;
}

bool
BackendRequiredFileProbeEntry::isRequired() const
{
    return roleIsRequired(role);
}

QString
BackendRequiredFileProbeEntry::roleName() const
{
    return toString(role);
}

QString
BackendRequiredFileProbeEntry::statusName() const
{
    return toString(status);
}

bool
BackendRequiredFileProbeResult::isValid() const
{
    return report.isValid();
}

bool
BackendRequiredFileProbeResult::hasRequiredFiles() const
{
    for (const auto &entry: entries) {
        if (entry.role == BackendRequiredFileProbeRole::RequiredFile) {
            return true;
        }
    }
    return false;
}

bool
BackendRequiredFileProbeResult::hasModelCheckpointProbe() const
{
    for (const auto &entry: entries) {
        if (entry.role == BackendRequiredFileProbeRole::ModelCheckpoint) {
            return true;
        }
    }
    return false;
}

bool
BackendRequiredFileProbeResult::hasMissingRequiredFile() const
{
    for (const auto &entry: entries) {
        if (entry.role == BackendRequiredFileProbeRole::RequiredFile &&
            !entry.isPresent()) {
            return true;
        }
    }
    return false;
}

bool
BackendRequiredFileProbeResult::hasMissingModelCheckpoint() const
{
    for (const auto &entry: entries) {
        if (entry.role == BackendRequiredFileProbeRole::ModelCheckpoint &&
            !entry.isPresent()) {
            return true;
        }
    }
    return false;
}

BackendRequiredFileProbeResult
BackendRequiredFileProbe::probe(const BackendManifest &manifest) const
{
    return probeFiles(manifest, QString());
}

BackendRequiredFileProbeResult
BackendRequiredFileProbe::probe(const BackendManifest &manifest,
                                const BackendSettings &settings) const
{
    return probeFiles(manifest, settings.modelCheckpointPathOverride);
}

QString
toString(BackendRequiredFileProbeRole role)
{
    switch (role) {
    case BackendRequiredFileProbeRole::RequiredFile: return "required_file";
    case BackendRequiredFileProbeRole::OptionalFile: return "optional_file";
    case BackendRequiredFileProbeRole::ModelCheckpoint: return "model_checkpoint";
    }
    return "required_file";
}

QString
toString(BackendRequiredFileProbeStatus status)
{
    switch (status) {
    case BackendRequiredFileProbeStatus::EmptyPath: return "empty_path";
    case BackendRequiredFileProbeStatus::Missing: return "missing";
    case BackendRequiredFileProbeStatus::Directory: return "directory";
    case BackendRequiredFileProbeStatus::Present: return "present";
    }
    return "empty_path";
}

}
}
