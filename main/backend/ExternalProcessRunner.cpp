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

#include <QElapsedTimer>
#include <QMutexLocker>
#include <QProcessEnvironment>
#include <QUuid>
#include <QtGlobal>

#include <chrono>
#include <future>

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

QString
eventTypeName(ExternalProcessEventType type)
{
    switch (type) {
    case ExternalProcessEventType::Started:
        return "started";
    case ExternalProcessEventType::StdoutChunk:
        return "stdout_chunk";
    case ExternalProcessEventType::StderrChunk:
        return "stderr_chunk";
    case ExternalProcessEventType::Finished:
        return "finished";
    case ExternalProcessEventType::FailedToStart:
        return "failed_to_start";
    case ExternalProcessEventType::TimedOut:
        return "timed_out";
    case ExternalProcessEventType::Cancelled:
        return "cancelled";
    }
    return "unknown";
}

void
appendEvent(const ExternalProcessRequest &request,
            ExternalProcessEventType type,
            const QString &message = QString(),
            const QString &data = QString(),
            int exitCode = -1)
{
    if (!request.eventCollector) {
        return;
    }

    ExternalProcessEvent event;
    event.type = type;
    event.runId = request.runId;
    event.message = message;
    event.data = data;
    event.exitCode = exitCode;
    request.eventCollector->append(event);
}

void
appendOutputEvents(const ExternalProcessRequest &request,
                   const QString &standardOutput,
                   const QString &standardError)
{
    if (!standardOutput.isEmpty()) {
        appendEvent(request,
                    ExternalProcessEventType::StdoutChunk,
                    "External process wrote stdout.",
                    standardOutput);
    }
    if (!standardError.isEmpty()) {
        appendEvent(request,
                    ExternalProcessEventType::StderrChunk,
                    "External process wrote stderr.",
                    standardError);
    }
}

}

class ExternalProcessAsyncRunState
{
public:
    AnalysisRunId runId;
    QSharedPointer<ExternalProcessCancellationToken> cancellationToken;
    std::future<ExternalProcessResult> future;
    std::optional<ExternalProcessResult> result;

    bool isReady() const
    {
        if (!future.valid()) {
            return result.has_value();
        }
        return future.wait_for(std::chrono::milliseconds(0)) ==
            std::future_status::ready;
    }

    bool isRunning() const
    {
        return future.valid() && !isReady();
    }

    std::optional<ExternalProcessResult> collect()
    {
        if (result.has_value()) {
            return result;
        }
        if (!future.valid() || !isReady()) {
            return std::nullopt;
        }
        result = future.get();
        return result;
    }
};

QString
ExternalProcessEvent::typeName() const
{
    return eventTypeName(type);
}

QString
ExternalProcessEvent::debugSummaryString() const
{
    return QString("type=%1 run_id=%2 exit_code=%3 data_size=%4")
        .arg(typeName())
        .arg(runId)
        .arg(exitCode)
        .arg(data.size());
}

void
ExternalProcessEventCollector::append(const ExternalProcessEvent &event)
{
    QMutexLocker locker(&m_mutex);
    m_events.push_back(event);
}

QVector<ExternalProcessEvent>
ExternalProcessEventCollector::events() const
{
    QMutexLocker locker(&m_mutex);
    return m_events;
}

bool
ExternalProcessEventCollector::hasEvent(
    ExternalProcessEventType type) const
{
    return count(type) > 0;
}

int
ExternalProcessEventCollector::count(
    ExternalProcessEventType type) const
{
    QMutexLocker locker(&m_mutex);
    int total = 0;
    for (const auto &event: m_events) {
        if (event.type == type) {
            ++total;
        }
    }
    return total;
}

void
ExternalProcessEventCollector::clear()
{
    QMutexLocker locker(&m_mutex);
    m_events.clear();
}

QString
ExternalProcessEventCollector::debugSummaryString() const
{
    QMutexLocker locker(&m_mutex);
    return QString("events=%1").arg(m_events.size());
}

void
ExternalProcessCancellationToken::requestCancellation()
{
    m_cancelled.store(true);
}

bool
ExternalProcessCancellationToken::isCancellationRequested() const
{
    return m_cancelled.load();
}

void
ExternalProcessCancellationToken::reset()
{
    m_cancelled.store(false);
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
ExternalProcessRequest::isCancellationRequested() const
{
    return cancellationToken &&
        cancellationToken->isCancellationRequested();
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
                   "started=%4 start_failed=%5 timed_out=%6 cancelled=%7")
        .arg(toString(state))
        .arg(exitCode)
        .arg(exitStatusName(exitStatus))
        .arg(started ? "true" : "false")
        .arg(startFailed ? "true" : "false")
        .arg(timedOut ? "true" : "false")
        .arg(cancelled ? "true" : "false");
}

bool
ExternalProcessRunHandle::isValid() const
{
    return !runId.trimmed().isEmpty();
}

QString
ExternalProcessRunHandle::debugSummaryString() const
{
    return QString("run_id=%1 valid=%2")
        .arg(runId)
        .arg(isValid() ? "true" : "false");
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
        appendEvent(request,
                    ExternalProcessEventType::FailedToStart,
                    result.error.message);
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
        appendOutputEvents(request,
                           result.standardOutput,
                           result.standardError);
        appendEvent(request,
                    ExternalProcessEventType::FailedToStart,
                    result.error.message);
        return result;
    }

    result.started = true;
    result.state = AnalysisRunState::Running;
    appendEvent(request,
                ExternalProcessEventType::Started,
                "External process started.");

    QElapsedTimer elapsed;
    elapsed.start();

    bool finished = false;
    while (!finished) {
        if (request.isCancellationRequested()) {
            result.cancelled = true;
            result.state = AnalysisRunState::Cancelled;
            result.processError = process.error();
            process.terminate();
            if (!process.waitForFinished(1000)) {
                process.kill();
                process.waitForFinished(3000);
            }
            result.exitCode = process.exitCode();
            result.exitStatus = process.exitStatus();
            result.standardOutput =
                QString::fromLocal8Bit(process.readAllStandardOutput());
            result.standardError =
                QString::fromLocal8Bit(process.readAllStandardError());
            result.error = {
                BackendErrorCode::Cancelled,
                "External process was cancelled.",
                true
            };
            appendOutputEvents(request,
                               result.standardOutput,
                               result.standardError);
            appendEvent(request,
                        ExternalProcessEventType::Cancelled,
                        result.error.message,
                        QString(),
                        result.exitCode);
            return result;
        }

        if (request.hasTimeout() && elapsed.elapsed() >= request.timeoutMsec) {
            break;
        }

        const int waitMsec = request.hasTimeout() ?
            qBound(1, int(request.timeoutMsec - elapsed.elapsed()), 50) : 50;
        finished = process.waitForFinished(waitMsec);
    }

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
        appendOutputEvents(request,
                           result.standardOutput,
                           result.standardError);
        appendEvent(request,
                    ExternalProcessEventType::TimedOut,
                    result.error.message,
                    QString(),
                    result.exitCode);
        return result;
    }

    result.exitCode = process.exitCode();
    result.exitStatus = process.exitStatus();
    result.processError = process.error();
    result.standardOutput =
        QString::fromLocal8Bit(process.readAllStandardOutput());
    result.standardError =
        QString::fromLocal8Bit(process.readAllStandardError());
    appendOutputEvents(request, result.standardOutput, result.standardError);

    if (result.exitStatus != QProcess::NormalExit) {
        result.state = AnalysisRunState::Failed;
        result.error = {
            BackendErrorCode::ExecutionFailed,
            "External process did not exit normally.",
            false
        };
        appendEvent(request,
                    ExternalProcessEventType::Finished,
                    result.error.message,
                    QString(),
                    result.exitCode);
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
        appendEvent(request,
                    ExternalProcessEventType::Finished,
                    result.error.message,
                    QString(),
                    result.exitCode);
        return result;
    }

    result.state = AnalysisRunState::Completed;
    result.error = {
        BackendErrorCode::None,
        QString(),
        false
    };
    appendEvent(request,
                ExternalProcessEventType::Finished,
                "External process finished.",
                QString(),
                result.exitCode);
    return result;
}

ExternalProcessRunHandle
ExternalProcessRunner::startAsync(const ExternalProcessRequest &request)
{
    ExternalProcessRequest asyncRequest = request;
    if (!asyncRequest.cancellationToken) {
        asyncRequest.cancellationToken =
            QSharedPointer<ExternalProcessCancellationToken>::create();
    }

    QSharedPointer<ExternalProcessAsyncRunState> state =
        QSharedPointer<ExternalProcessAsyncRunState>::create();
    state->runId =
        QUuid::createUuid().toString(QUuid::WithoutBraces);
    asyncRequest.runId = state->runId;
    state->cancellationToken = asyncRequest.cancellationToken;
    state->future = std::async(std::launch::async, [asyncRequest]() {
        ExternalProcessRunner runner;
        return runner.run(asyncRequest);
    });

    {
        QMutexLocker locker(&m_asyncRunsMutex);
        m_asyncRuns.insert(state->runId, state);
    }

    ExternalProcessRunHandle handle;
    handle.runId = state->runId;
    return handle;
}

bool
ExternalProcessRunner::isRunning(const ExternalProcessRunHandle &handle) const
{
    return isRunning(handle.runId);
}

bool
ExternalProcessRunner::isRunning(const AnalysisRunId &runId) const
{
    const QSharedPointer<ExternalProcessAsyncRunState> state =
        asyncRunById(runId);
    return state && state->isRunning();
}

bool
ExternalProcessRunner::hasAsyncRun(
    const ExternalProcessRunHandle &handle) const
{
    return bool(asyncRunById(handle.runId));
}

std::optional<ExternalProcessResult>
ExternalProcessRunner::collectResult(const ExternalProcessRunHandle &handle)
{
    const QSharedPointer<ExternalProcessAsyncRunState> state =
        asyncRunById(handle.runId);
    if (!state) {
        return std::nullopt;
    }
    return state->collect();
}

bool
ExternalProcessRunner::cleanup(const ExternalProcessRunHandle &handle)
{
    QMutexLocker locker(&m_asyncRunsMutex);
    auto it = m_asyncRuns.find(handle.runId);
    if (it == m_asyncRuns.end()) {
        return false;
    }
    if ((*it)->isRunning()) {
        return false;
    }
    m_asyncRuns.erase(it);
    return true;
}

bool
ExternalProcessRunner::cancel(const AnalysisRunId &runId)
{
    const QSharedPointer<ExternalProcessAsyncRunState> state =
        asyncRunById(runId);
    if (!state || !state->isRunning()) {
        return false;
    }
    return cancel(state->cancellationToken);
}

bool
ExternalProcessRunner::cancel(const ExternalProcessRunHandle &handle)
{
    return cancel(handle.runId);
}

bool
ExternalProcessRunner::cancel(
    const QSharedPointer<ExternalProcessCancellationToken> &token) const
{
    if (!token) {
        return false;
    }

    token->requestCancellation();
    return true;
}

QSharedPointer<ExternalProcessAsyncRunState>
ExternalProcessRunner::asyncRunById(const AnalysisRunId &runId) const
{
    if (runId.trimmed().isEmpty()) {
        return {};
    }

    QMutexLocker locker(&m_asyncRunsMutex);
    auto it = m_asyncRuns.constFind(runId);
    if (it == m_asyncRuns.constEnd()) {
        return {};
    }
    return *it;
}

}
}
