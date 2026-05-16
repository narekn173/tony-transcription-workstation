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

#include "ExternalProcessRunner.h"

#include <QProcessEnvironment>

namespace Tony {
namespace Backend {

namespace {

QString
processErrorMessage(QProcess::ProcessError error)
{
    switch (error) {
    case QProcess::FailedToStart:
        return "Process failed to start.";
    case QProcess::Crashed:
        return "Process crashed.";
    case QProcess::Timedout:
        return "Process operation timed out.";
    case QProcess::WriteError:
        return "Process write error.";
    case QProcess::ReadError:
        return "Process read error.";
    case QProcess::UnknownError:
        return "Unknown process error.";
    }
    return "Unknown process error.";
}

QString
exitStatusName(QProcess::ExitStatus status)
{
    switch (status) {
    case QProcess::NormalExit:
        return "normal_exit";
    case QProcess::CrashExit:
        return "crash_exit";
    }
    return "unknown_exit";
}

}

bool
ExternalProcessRequest::hasExecutable() const
{
    return !executablePath.trimmed().isEmpty();
}

bool
ExternalProcessRequest::hasTimeout() const
{
    return timeoutMsec > 0;
}

bool
ExternalProcessResult::succeeded() const
{
    return state == AnalysisRunState::Completed && started &&
        !startFailed && !timedOut && !cancelled &&
        exitStatus == QProcess::NormalExit && exitCode == 0;
}

QString
ExternalProcessResult::debugSummaryString() const
{
    return QString("state=%1 exit_code=%2 exit_status=%3 "
                   "started=%4 start_failed=%5 timed_out=%6")
        .arg(toString(state))
        .arg(exitCode)
        .arg(exitStatusName(exitStatus))
        .arg(started ? "true" : "false")
        .arg(startFailed ? "true" : "false")
        .arg(timedOut ? "true" : "false");
}

ExternalProcessResult
ExternalProcessRunner::run(const ExternalProcessRequest &request) const
{
    ExternalProcessResult result;

    if (!request.hasExecutable()) {
        result.state = AnalysisRunState::Failed;
        result.startFailed = true;
        result.error = {
            BackendErrorCode::BackendNotConfigured,
            "External process request has no executable path.",
            true
        };
        return result;
    }

    QProcess process;
    process.setProgram(request.executablePath);
    process.setArguments(request.arguments);
    process.setProcessChannelMode(QProcess::SeparateChannels);

    if (!request.workingDirectory.trimmed().isEmpty()) {
        process.setWorkingDirectory(request.workingDirectory);
    }

    if (!request.environmentOverrides.isEmpty()) {
        QProcessEnvironment environment =
            QProcessEnvironment::systemEnvironment();
        for (auto it = request.environmentOverrides.constBegin();
             it != request.environmentOverrides.constEnd(); ++it) {
            environment.insert(it.key(), it.value());
        }
        process.setProcessEnvironment(environment);
    }

    process.start();

    const int startTimeout =
        request.hasTimeout() ? qMin(request.timeoutMsec, 30000) : 30000;
    if (!process.waitForStarted(startTimeout)) {
        result.state = AnalysisRunState::Failed;
        result.startFailed = true;
        result.processError = process.error();
        result.standardOutput =
            QString::fromLocal8Bit(process.readAllStandardOutput());
        result.standardError =
            QString::fromLocal8Bit(process.readAllStandardError());
        result.error = {
            BackendErrorCode::BackendMissing,
            process.errorString().isEmpty() ?
                processErrorMessage(process.error()) : process.errorString(),
            true
        };
        return result;
    }

    result.started = true;
    result.state = AnalysisRunState::Running;

    const bool finished = request.hasTimeout() ?
        process.waitForFinished(request.timeoutMsec) :
        process.waitForFinished(-1);

    if (!finished) {
        result.timedOut = true;
        result.state = AnalysisRunState::Failed;
        result.processError = QProcess::Timedout;
        process.kill();
        process.waitForFinished(3000);
        result.exitCode = process.exitCode();
        result.exitStatus = process.exitStatus();
        result.standardOutput =
            QString::fromLocal8Bit(process.readAllStandardOutput());
        result.standardError =
            QString::fromLocal8Bit(process.readAllStandardError());
        result.error = {
            BackendErrorCode::TimedOut,
            "External process exceeded its timeout.",
            true
        };
        return result;
    }

    result.exitCode = process.exitCode();
    result.exitStatus = process.exitStatus();
    result.processError = process.error();
    result.standardOutput =
        QString::fromLocal8Bit(process.readAllStandardOutput());
    result.standardError =
        QString::fromLocal8Bit(process.readAllStandardError());

    if (result.exitStatus != QProcess::NormalExit) {
        result.state = AnalysisRunState::Failed;
        result.error = {
            BackendErrorCode::ExecutionFailed,
            "External process did not exit normally.",
            false
        };
        return result;
    }

    if (result.exitCode != 0) {
        result.state = AnalysisRunState::Failed;
        result.error = {
            BackendErrorCode::ExecutionFailed,
            QString("External process exited with code %1.")
                .arg(result.exitCode),
            false
        };
        return result;
    }

    result.state = AnalysisRunState::Completed;
    result.error = {
        BackendErrorCode::None,
        QString(),
        false
    };
    return result;
}

bool
ExternalProcessRunner::cancel(const AnalysisRunId &)
{
    return false;
}

}
}
