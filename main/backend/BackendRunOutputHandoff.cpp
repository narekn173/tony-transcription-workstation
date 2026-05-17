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

#include "BackendRunOutputHandoff.h"

#include <QFile>
#include <QFileInfo>

namespace Tony {
namespace Backend {

bool
BackendRunOutputHandoffResult::isValid() const
{
    return report.isValid();
}

QString
BackendRunOutputHandoffResult::debugSummaryString() const
{
    return QString("output_path=%1 exists=%2 file=%3 readable=%4 "
                   "non_empty=%5 imported=%6 valid=%7")
        .arg(outputPath)
        .arg(exists ? QString("true") : QString("false"))
        .arg(isFile ? QString("true") : QString("false"))
        .arg(readable ? QString("true") : QString("false"))
        .arg(nonEmpty ? QString("true") : QString("false"))
        .arg(importedIntoTonyLayers ? QString("true") : QString("false"))
        .arg(isValid() ? QString("true") : QString("false"));
}

BackendRunOutputHandoffResult
BackendRunOutputHandoff::inspect(
    const QString &expectedUnifiedResultJsonPath) const
{
    BackendRunOutputHandoffResult result;
    result.outputPath = expectedUnifiedResultJsonPath.trimmed();
    result.importedIntoTonyLayers = false;

    if (result.outputPath.isEmpty()) {
        result.report.addError(
            "empty_output_result_path",
            "Expected UnifiedResult JSON output path is empty.");
        return result;
    }

    const QFileInfo info(result.outputPath);
    result.exists = info.exists();

    if (!result.exists) {
        result.report.addError(
            "output_file_missing",
            QString("Expected UnifiedResult JSON output file is missing: %1")
                .arg(result.outputPath));
        return result;
    }

    if (info.isDir()) {
        result.report.addError(
            "output_path_is_directory",
            QString("Expected UnifiedResult JSON output path is a directory: "
                    "%1").arg(result.outputPath));
        return result;
    }

    result.isFile = info.isFile();
    if (!result.isFile) {
        result.report.addError(
            "output_path_not_file",
            QString("Expected UnifiedResult JSON output path is not a file: "
                    "%1").arg(result.outputPath));
        return result;
    }

    result.fileSizeBytes = info.size();
    result.nonEmpty = result.fileSizeBytes > 0;
    if (!result.nonEmpty) {
        result.report.addError(
            "empty_output_file",
            QString("Expected UnifiedResult JSON output file is empty: %1")
                .arg(result.outputPath));
        return result;
    }

    QFile file(result.outputPath);
    result.readable = file.open(QIODevice::ReadOnly);
    if (!result.readable) {
        result.report.addError(
            "output_file_not_readable",
            QString("Expected UnifiedResult JSON output file is not readable: "
                    "%1").arg(result.outputPath));
        return result;
    }

    return result;
}

BackendRunOutputHandoffResult
BackendRunOutputHandoff::inspect(
    const QString &expectedUnifiedResultJsonPath,
    const std::optional<ExternalProcessResult> &processResult) const
{
    BackendRunOutputHandoffResult result =
        inspect(expectedUnifiedResultJsonPath);

    if (processResult.has_value() && !processResult->succeeded()) {
        result.report.addError(
            "external_process_failed",
            processResult->error.message.isEmpty() ?
                QString("External backend process failed before output "
                        "handoff.") :
                processResult->error.message);
    }

    return result;
}

BackendRunOutputHandoffResult
BackendRunOutputHandoff::inspect(
    const BackendRunOrchestrationResult &runResult) const
{
    return inspect(runResult.expectedUnifiedResultJsonPath,
                   runResult.processResult);
}

}
}
