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

#include "UnifiedResultFileWriter.h"

#include "UnifiedResultSerializer.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QJsonDocument>

namespace Tony {
namespace Backend {

bool
UnifiedResultFileWriteResult::isValid() const
{
    return report.isValid() && wroteFile;
}

QString
UnifiedResultFileWriteResult::debugSummaryString() const
{
    return QString("path=%1 wrote=%2 bytes=%3 created_parent=%4 valid=%5")
        .arg(path)
        .arg(wroteFile ? QString("true") : QString("false"))
        .arg(bytesWritten)
        .arg(createdParentDirectory ? QString("true") : QString("false"))
        .arg(isValid() ? QString("true") : QString("false"));
}

UnifiedResultFileWriteResult
UnifiedResultFileWriter::write(const QString &path,
                               const UnifiedResult &result) const
{
    UnifiedResultFileWriteResult writeResult;
    writeResult.path = path.trimmed();
    writeResult.createdParentDirectory = false;

    if (writeResult.path.isEmpty()) {
        writeResult.report.addError(
            "empty_unified_result_output_path",
            "UnifiedResult JSON output path is empty.");
        return writeResult;
    }

    const QFileInfo info(writeResult.path);
    if (info.exists() && info.isDir()) {
        writeResult.report.addError(
            "unified_result_output_path_is_directory",
            "UnifiedResult JSON output path is a directory.");
        return writeResult;
    }

    const QDir parentDirectory = info.absoluteDir();
    if (!parentDirectory.exists()) {
        writeResult.report.addError(
            "unified_result_output_parent_missing",
            "UnifiedResult JSON output parent directory does not exist.");
        return writeResult;
    }

    UnifiedResultSerializer serializer;
    const QJsonDocument document(serializer.toJsonObject(result));
    const QByteArray bytes = document.toJson(QJsonDocument::Indented);
    if (bytes.isEmpty()) {
        writeResult.report.addError(
            "empty_unified_result_serialization",
            "UnifiedResult JSON serialization produced no bytes.");
        return writeResult;
    }

    QFile file(writeResult.path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        writeResult.report.addError(
            "unified_result_file_open_failed",
            "UnifiedResult JSON output file could not be opened for writing.");
        return writeResult;
    }

    const qint64 written = file.write(bytes);
    if (written != bytes.size()) {
        writeResult.bytesWritten = written;
        writeResult.report.addError(
            "unified_result_file_write_failed",
            "UnifiedResult JSON output file could not be written fully.");
        return writeResult;
    }

    writeResult.bytesWritten = written;
    writeResult.wroteFile = true;
    return writeResult;
}

}
}
