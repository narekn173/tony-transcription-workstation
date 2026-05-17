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

#ifndef TONY_BACKEND_RUN_RESULT_REPORTER_H
#define TONY_BACKEND_RUN_RESULT_REPORTER_H

#include "BackendRunResultLoader.h"

#include <optional>

namespace Tony {
namespace Backend {

struct BackendRunResultReport
{
    BackendId backendId;
    QString expectedUnifiedResultJsonPath;
    std::optional<ExternalProcessResult> processResult;
    BackendRunResultLoadResult loadResult;
    ValidationReport report;
    QStringList warnings;
    QStringList errors;
    bool processResultAvailable = false;
    bool processSucceeded = false;
    bool processFailed = false;
    bool processTimedOut = false;
    bool processCancelled = false;
    bool outputFilePresent = false;
    bool outputFileMissing = false;
    bool outputFileEmpty = false;
    bool outputFileInvalid = false;
    bool unifiedResultLoaded = false;
    bool importedIntoTonyLayers = false;

    bool isValid() const;
    QString debugSummaryString() const;
};

class BackendRunResultReporter
{
public:
    BackendRunResultReport buildReport(
        const BackendId &backendId,
        const QString &expectedUnifiedResultJsonPath,
        const BackendRunResultLoadResult &loadResult) const;

    BackendRunResultReport buildReport(
        const BackendId &backendId,
        const std::optional<ExternalProcessResult> &processResult,
        const QString &expectedUnifiedResultJsonPath,
        const BackendRunResultLoadResult &loadResult) const;

private:
    static void appendIssues(ValidationReport &target,
                             const ValidationReport &source);
    static bool hasIssue(const ValidationReport &report,
                         const QString &code);
    static void populateIssueLists(BackendRunResultReport &result);
};

}
}

#endif
