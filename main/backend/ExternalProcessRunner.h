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

#ifndef TONY_EXTERNAL_PROCESS_RUNNER_H
#define TONY_EXTERNAL_PROCESS_RUNNER_H

#include "BackendTypes.h"

#include <QMap>
#include <QProcess>

namespace Tony {
namespace Backend {

struct ExternalProcessRequest
{
    QString executablePath;
    QStringList arguments;
    QString workingDirectory;
    QMap<QString, QString> environmentOverrides;
    int timeoutMsec = 0;

    bool hasExecutable() const;
    bool hasTimeout() const;
};

struct ExternalProcessResult
{
    AnalysisRunState state = AnalysisRunState::Idle;
    int exitCode = -1;
    QProcess::ExitStatus exitStatus = QProcess::NormalExit;
    QProcess::ProcessError processError = QProcess::UnknownError;
    bool started = false;
    bool startFailed = false;
    bool timedOut = false;
    bool cancelled = false;
    QString standardOutput;
    QString standardError;
    BackendError error;

    bool succeeded() const;
    QString debugSummaryString() const;
};

class ExternalProcessRunner
{
public:
    ExternalProcessResult run(const ExternalProcessRequest &request) const;
    bool cancel(const AnalysisRunId &runId);
};

}
}

#endif
