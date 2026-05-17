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

#include "BackendRunResultLoader.h"

namespace Tony {
namespace Backend {

bool
BackendRunResultLoadResult::isValid() const
{
    return report.isValid() && loadedResult.has_value();
}

QString
BackendRunResultLoadResult::debugSummaryString() const
{
    return QString("output_path=%1 handoff=%2 file_loaded=%3 "
                   "has_result=%4 imported=%5 valid=%6")
        .arg(outputPath)
        .arg(handoffAccepted ? QString("true") : QString("false"))
        .arg(fileLoaded ? QString("true") : QString("false"))
        .arg(loadedResult.has_value() ? QString("true") : QString("false"))
        .arg(importedIntoTonyLayers ? QString("true") : QString("false"))
        .arg(isValid() ? QString("true") : QString("false"));
}

BackendRunResultLoadResult
BackendRunResultLoader::load(
    const QString &expectedUnifiedResultJsonPath) const
{
    return load(expectedUnifiedResultJsonPath, std::nullopt);
}

BackendRunResultLoadResult
BackendRunResultLoader::load(
    const QString &expectedUnifiedResultJsonPath,
    const std::optional<ExternalProcessResult> &processResult) const
{
    BackendRunResultLoadResult result;
    result.outputPath = expectedUnifiedResultJsonPath.trimmed();
    result.importedIntoTonyLayers = false;

    BackendRunOutputHandoff handoff;
    result.handoffResult = handoff.inspect(result.outputPath, processResult);
    appendIssues(result.report, result.handoffResult.report);
    result.handoffAccepted = result.handoffResult.isValid();

    if (!result.handoffAccepted) {
        return result;
    }

    UnifiedResultFileLoader loader;
    result.fileLoadResult = loader.load(result.outputPath);
    appendIssues(result.report, result.fileLoadResult.report);
    result.fileLoaded = result.fileLoadResult.isValid();

    if (result.fileLoaded) {
        result.loadedResult = result.fileLoadResult.result;
    }

    return result;
}

BackendRunResultLoadResult
BackendRunResultLoader::load(
    const BackendRunOrchestrationResult &runResult) const
{
    return load(runResult.expectedUnifiedResultJsonPath,
                runResult.processResult);
}

void
BackendRunResultLoader::appendIssues(ValidationReport &target,
                                     const ValidationReport &source)
{
    for (const ValidationIssue &issue: source.issues) {
        target.addIssue(issue.severity, issue.code, issue.message);
    }
}

}
}
