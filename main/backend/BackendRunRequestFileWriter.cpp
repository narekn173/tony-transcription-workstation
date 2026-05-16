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

#include "BackendRunRequestFileWriter.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>

namespace Tony {
namespace Backend {

namespace {

QString
cleanPath(const QString &path)
{
    return path.trimmed();
}

bool
parentDirectoryExists(const QFileInfo &fileInfo)
{
    const QString parent = fileInfo.path();
    if (parent.isEmpty() || parent == ".") {
        return true;
    }

    const QFileInfo parentInfo(QDir::cleanPath(parent));
    return parentInfo.exists() && parentInfo.isDir();
}

void
appendIssues(ValidationReport &target, const ValidationReport &source)
{
    for (const auto &issue: source.issues) {
        target.addIssue(issue.severity, issue.code, issue.message);
    }
}

}

bool
BackendRunRequestFileWriteResult::isValid() const
{
    return report.isValid();
}

BackendRunRequestFileWriteResult
BackendRunRequestFileWriter::write(
    const QString &path,
    const BackendRunRequestBuildResult &request) const
{
    BackendRunRequestFileWriteResult written;
    written.path = cleanPath(path);

    if (written.path.isEmpty()) {
        written.report.addError(
            "empty_request_file_path",
            "Backend run request file path is empty.");
        return written;
    }

    const QFileInfo fileInfo(written.path);
    if (!parentDirectoryExists(fileInfo)) {
        written.report.addError(
            "parent_directory_missing",
            "Backend run request file parent directory does not exist.");
        return written;
    }

    BackendRunRequestSerializer serializer;
    const BackendRunRequestSerializationResult serialized =
        serializer.serialize(request);
    appendIssues(written.report, serialized.report);

    if (!written.isValid()) {
        return written;
    }

    const QJsonDocument document(serialized.object);
    const QByteArray contents = document.toJson(QJsonDocument::Indented);

    QFile file(written.path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        written.report.addError(
            "file_write_failed",
            QString("Unable to open backend run request file for writing: %1")
                .arg(file.errorString()));
        return written;
    }

    written.bytesWritten = file.write(contents);
    if (written.bytesWritten != contents.size()) {
        written.report.addError(
            "file_write_failed",
            QString("Unable to write complete backend run request file: %1")
                .arg(file.errorString()));
        return written;
    }

    return written;
}

}
}
