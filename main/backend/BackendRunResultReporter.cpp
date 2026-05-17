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

#include "BackendRunResultReporter.h"

namespace Tony {
namespace Backend {

bool
BackendRunResultReport::isValid() const
{
    return report.isValid() && unifiedResultLoaded &&
        (!processResultAvailable || processSucceeded) &&
        !importedIntoTonyLayers;
}

QString
BackendRunResultReport::debugSummaryString() const
{
    return QString("backend_id=%1 process_available=%2 process_success=%3 "
                   "timed_out=%4 cancelled=%5 output_present=%6 "
                   "output_missing=%7 output_empty=%8 output_invalid=%9 "
                   "loaded=%10 imported=%11 errors=%12 warnings=%13 "
                   "valid=%14")
        .arg(backendId.isEmpty() ? QString("unknown") : backendId)
        .arg(processResultAvailable ? QString("true") : QString("false"))
        .arg(processSucceeded ? QString("true") : QString("false"))
        .arg(processTimedOut ? QString("true") : QString("false"))
        .arg(processCancelled ? QString("true") : QString("false"))
        .arg(outputFilePresent ? QString("true") : QString("false"))
        .arg(outputFileMissing ? QString("true") : QString("false"))
        .arg(outputFileEmpty ? QString("true") : QString("false"))
        .arg(outputFileInvalid ? QString("true") : QString("false"))
        .arg(unifiedResultLoaded ? QString("true") : QString("false"))
        .arg(importedIntoTonyLayers ? QString("true") : QString("false"))
        .arg(errors.size())
        .arg(warnings.size())
        .arg(isValid() ? QString("true") : QString("false"));
}

BackendRunResultReport
BackendRunResultReporter::buildReport(
    const BackendId &backendId,
    const QString &expectedUnifiedResultJsonPath,
    const BackendRunResultLoadResult &loadResult) const
{
    return buildReport(backendId,
                       std::nullopt,
                       expectedUnifiedResultJsonPath,
                       loadResult);
}

BackendRunResultReport
BackendRunResultReporter::buildReport(
    const BackendId &backendId,
    const std::optional<ExternalProcessResult> &processResult,
    const QString &expectedUnifiedResultJsonPath,
    const BackendRunResultLoadResult &loadResult) const
{
    BackendRunResultReport result;
    result.backendId = backendId.trimmed();
    result.expectedUnifiedResultJsonPath =
        expectedUnifiedResultJsonPath.trimmed();
    result.processResult = processResult;
    result.loadResult = loadResult;
    result.importedIntoTonyLayers = false;

    if (!Tony::Backend::isValidBackendId(result.backendId)) {
        result.report.addError("invalid_backend_id",
                               "Backend run report has an invalid backend ID.");
    }

    if (processResult.has_value()) {
        result.processResultAvailable = true;
        result.processSucceeded = processResult->succeeded();
        result.processTimedOut = processResult->timedOut;
        result.processCancelled = processResult->cancelled;
        result.processFailed =
            !result.processSucceeded &&
            !result.processTimedOut &&
            !result.processCancelled;

        if (result.processTimedOut) {
            result.report.addError(
                "process_timed_out",
                "External backend process timed out.");
        } else if (result.processCancelled) {
            result.report.addError(
                "process_cancelled",
                "External backend process was cancelled.");
        } else if (!result.processSucceeded) {
            result.report.addError(
                processResult->startFailed ? "process_start_failed" :
                                             "process_failed",
                processResult->error.message.isEmpty() ?
                    QString("External backend process did not complete "
                            "successfully.") :
                    processResult->error.message);
        }
    }

    appendIssues(result.report, loadResult.report);

    result.outputFilePresent = loadResult.handoffResult.exists;
    result.outputFileMissing =
        hasIssue(loadResult.report, "output_file_missing");
    result.outputFileEmpty =
        hasIssue(loadResult.report, "empty_output_file");
    result.unifiedResultLoaded =
        loadResult.fileLoaded && loadResult.loadedResult.has_value();
    result.outputFileInvalid =
        !result.unifiedResultLoaded &&
        (hasIssue(loadResult.report, "invalid_json") ||
         hasIssue(loadResult.report, "invalid_top_level_json") ||
         hasIssue(loadResult.report, "output_path_is_directory") ||
         hasIssue(loadResult.report, "output_path_not_file") ||
         hasIssue(loadResult.report, "output_file_not_readable"));

    if (!result.unifiedResultLoaded &&
        !result.outputFileMissing &&
        !result.outputFileEmpty &&
        loadResult.handoffAccepted) {
        result.outputFileInvalid = true;
    }

    populateIssueLists(result);
    return result;
}

BackendRunResultReport
BackendRunResultReporter::buildReport(
    const BackendId &backendId,
    const BackendRunOrchestrationResult &runResult,
    const QString &expectedUnifiedResultJsonPath,
    const BackendRunResultLoadResult &loadResult) const
{
    const QString outputPath =
        expectedUnifiedResultJsonPath.trimmed().isEmpty() ?
            runResult.expectedUnifiedResultJsonPath :
            expectedUnifiedResultJsonPath;

    BackendRunResultReport result =
        buildReport(backendId, runResult.processResult, outputPath, loadResult);

    appendIssues(result.report, runResult.report);
    populateIssueLists(result);
    return result;
}

void
BackendRunResultReporter::appendIssues(ValidationReport &target,
                                       const ValidationReport &source)
{
    for (const auto &issue: source.issues) {
        if (hasIssue(target, issue.code)) {
            continue;
        }
        target.addIssue(issue.severity, issue.code, issue.message);
    }
}

bool
BackendRunResultReporter::hasIssue(const ValidationReport &report,
                                   const QString &code)
{
    for (const auto &issue: report.issues) {
        if (issue.code == code) {
            return true;
        }
    }
    return false;
}

void
BackendRunResultReporter::populateIssueLists(BackendRunResultReport &result)
{
    result.warnings.clear();
    result.errors.clear();

    for (const auto &issue: result.report.issues) {
        if (issue.severity == ValidationSeverity::Error) {
            result.errors.push_back(issue.code);
        } else {
            result.warnings.push_back(issue.code);
        }
    }
}

}
}
