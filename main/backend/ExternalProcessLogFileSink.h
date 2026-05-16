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

#ifndef TONY_EXTERNAL_PROCESS_LOG_FILE_SINK_H
#define TONY_EXTERNAL_PROCESS_LOG_FILE_SINK_H

#include "ExternalProcessRunner.h"
#include "ResultValidator.h"

#include <QString>
#include <QVector>

#include <optional>

namespace Tony {
namespace Backend {

struct ExternalProcessLogFileWriteResult
{
    ValidationReport report;
    QString path;
    int eventCount = 0;
    qint64 bytesWritten = 0;
    bool resultSummaryIncluded = false;

    bool isValid() const;
};

class ExternalProcessLogFileSink
{
public:
    ExternalProcessLogFileWriteResult write(
        const QString &path,
        const QVector<ExternalProcessEvent> &events,
        const std::optional<ExternalProcessResult> &result = std::nullopt)
        const;

    ExternalProcessLogFileWriteResult write(
        const QString &path,
        const ExternalProcessEventCollector &collector,
        const std::optional<ExternalProcessResult> &result = std::nullopt)
        const;
};

}
}

#endif
