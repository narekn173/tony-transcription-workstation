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

#include "ExternalProcessLogFileSink.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

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

QString
eventLine(const ExternalProcessEvent &event)
{
    return QString("[%1] type=%2 run_id=%3 exit_code=%4")
        .arg(event.timestampUtc.toUTC().toString(Qt::ISODateWithMs))
        .arg(event.typeName())
        .arg(event.runId)
        .arg(event.exitCode);
}

QString
logContents(const QVector<ExternalProcessEvent> &events,
            const std::optional<ExternalProcessResult> &result)
{
    QString contents;
    QTextStream stream(&contents);
    stream << "Tony External Process Log\n";
    stream << "generated_at="
           << QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)
           << "\n";
    stream << "event_count=" << events.size() << "\n";
    stream << "\n";

    if (result.has_value()) {
        stream << "Result\n";
        stream << result->debugSummaryString() << "\n";
        stream << "stdout_size=" << result->standardOutput.size() << "\n";
        stream << "stderr_size=" << result->standardError.size() << "\n";
        stream << "\n";
    }

    stream << "Events\n";
    if (events.isEmpty()) {
        stream << "No external process events were recorded.\n";
    }

    for (const auto &event: events) {
        stream << eventLine(event) << "\n";
        if (!event.message.isEmpty()) {
            stream << "message=" << event.message << "\n";
        }
        if (!event.data.isEmpty()) {
            stream << "data_begin\n";
            stream << event.data;
            if (!event.data.endsWith('\n')) {
                stream << "\n";
            }
            stream << "data_end\n";
        }
        stream << "\n";
    }

    return contents;
}

}

bool
ExternalProcessLogFileWriteResult::isValid() const
{
    return report.isValid();
}

ExternalProcessLogFileWriteResult
ExternalProcessLogFileSink::write(
    const QString &path,
    const QVector<ExternalProcessEvent> &events,
    const std::optional<ExternalProcessResult> &result) const
{
    ExternalProcessLogFileWriteResult written;
    written.path = cleanPath(path);
    written.eventCount = events.size();
    written.resultSummaryIncluded = result.has_value();

    if (written.path.isEmpty()) {
        written.report.addError(
            "empty_log_file_path",
            "External process log file path is empty.");
        return written;
    }

    const QFileInfo fileInfo(written.path);
    if (!parentDirectoryExists(fileInfo)) {
        written.report.addError(
            "parent_directory_missing",
            "External process log parent directory does not exist.");
        return written;
    }

    const QString contents = logContents(events, result);
    const QByteArray bytes = contents.toUtf8();

    QFile file(written.path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        written.report.addError(
            "file_write_failed",
            QString("Unable to open external process log file for writing: %1")
                .arg(file.errorString()));
        return written;
    }

    written.bytesWritten = file.write(bytes);
    if (written.bytesWritten != bytes.size()) {
        written.report.addError(
            "file_write_failed",
            QString("Unable to write complete external process log file: %1")
                .arg(file.errorString()));
        return written;
    }

    return written;
}

ExternalProcessLogFileWriteResult
ExternalProcessLogFileSink::write(
    const QString &path,
    const ExternalProcessEventCollector &collector,
    const std::optional<ExternalProcessResult> &result) const
{
    return write(path, collector.events(), result);
}

}
}
