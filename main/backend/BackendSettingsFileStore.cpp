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

#include "BackendSettingsFileStore.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonParseError>

namespace Tony {
namespace Backend {

namespace {

void
appendIssues(ValidationReport &target, const ValidationReport &source)
{
    for (const auto &issue: source.issues) {
        target.addIssue(issue.severity, issue.code, issue.message);
    }
}

}

bool
BackendSettingsFileLoadResult::isValid() const
{
    return report.isValid();
}

bool
BackendSettingsFileSaveResult::isValid() const
{
    return report.isValid();
}

BackendSettingsFileLoadResult
BackendSettingsFileStore::load(const QString &path) const
{
    BackendSettingsFileLoadResult loaded;
    loaded.path = path;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        loaded.report.addError(
            "file_open_failed",
            QString("Unable to open BackendSettings file: %1")
                .arg(file.errorString()));
        return loaded;
    }

    QJsonParseError parseError;
    const QJsonDocument document =
        QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        loaded.report.addError(
            "invalid_json",
            QString("Unable to parse BackendSettings JSON: %1")
                .arg(parseError.errorString()));
        return loaded;
    }

    if (!document.isObject()) {
        loaded.report.addError(
            "invalid_top_level_json",
            "BackendSettings JSON must have an object at the top level.");
        return loaded;
    }

    BackendSettingsSerializer serializer;
    const BackendSettingsStoreSerializationResult parsed =
        serializer.storeFromJsonObject(document.object());

    loaded.store = parsed.store;
    loaded.loadedCount = parsed.loadedCount;
    loaded.rejectedCount = parsed.rejectedCount;
    appendIssues(loaded.report, parsed.report);
    return loaded;
}

BackendSettingsFileSaveResult
BackendSettingsFileStore::save(const QString &path,
                               const BackendSettingsStore &store) const
{
    BackendSettingsFileSaveResult saved;
    saved.path = path;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        saved.report.addError(
            "file_write_failed",
            QString("Unable to open BackendSettings file for writing: %1")
                .arg(file.errorString()));
        return saved;
    }

    BackendSettingsSerializer serializer;
    const QJsonDocument document(serializer.storeToJsonObject(store));
    const QByteArray contents = document.toJson(QJsonDocument::Indented);

    if (file.write(contents) != contents.size()) {
        saved.report.addError(
            "file_write_failed",
            QString("Unable to write complete BackendSettings file: %1")
                .arg(file.errorString()));
        return saved;
    }

    return saved;
}

}
}
