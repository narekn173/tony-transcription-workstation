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

#include "BackendAvailabilityProbe.h"

namespace Tony {
namespace Backend {

namespace {

void
appendIssues(ValidationReport &target, const ValidationReport &source)
{
    for (const auto &issue: source.issues) {
        target.addIssue(issue.severity, issue.code, issue.message);
    }
}

bool
reportHasIssue(const ValidationReport &report, const QString &code)
{
    for (const auto &issue: report.issues) {
        if (issue.code == code) {
            return true;
        }
    }
    return false;
}

bool
reportHasAnyDirectoryError(const ValidationReport &report)
{
    return reportHasIssue(report, "required_file_path_is_directory") ||
        reportHasIssue(report, "model_checkpoint_path_is_directory");
}

BackendAvailabilityProbeStatus
classify(const BackendExecutableProbeResult &executableProbe,
         const BackendRequiredFileProbeResult &requiredFileProbe,
         const ValidationReport &report)
{
    switch (executableProbe.status) {
    case BackendExecutableProbeStatus::EmptyPath:
        return BackendAvailabilityProbeStatus::NotConfigured;
    case BackendExecutableProbeStatus::Missing:
        return BackendAvailabilityProbeStatus::MissingExecutable;
    case BackendExecutableProbeStatus::Directory:
    case BackendExecutableProbeStatus::NotExecutable:
        return BackendAvailabilityProbeStatus::InvalidPath;
    case BackendExecutableProbeStatus::Present:
        break;
    }

    if (reportHasAnyDirectoryError(report)) {
        return BackendAvailabilityProbeStatus::InvalidPath;
    }

    if (requiredFileProbe.hasMissingRequiredFile() ||
        requiredFileProbe.hasMissingModelCheckpoint()) {
        return BackendAvailabilityProbeStatus::MissingModel;
    }

    if (!report.isValid()) {
        return BackendAvailabilityProbeStatus::InvalidPath;
    }

    return BackendAvailabilityProbeStatus::PathChecksPassed;
}

BackendAvailabilityReport
buildReport(const BackendManifest &manifest,
            const BackendExecutableProbeResult &executableProbe,
            const BackendRequiredFileProbeResult &requiredFileProbe)
{
    BackendAvailabilityReport availability;
    availability.backendId = manifest.id();
    availability.executableProbe = executableProbe;
    availability.requiredFileProbe = requiredFileProbe;

    appendIssues(availability.report, executableProbe.report);
    appendIssues(availability.report, requiredFileProbe.report);

    availability.status =
        classify(executableProbe, requiredFileProbe, availability.report);

    return availability;
}

}

bool
BackendAvailabilityReport::isValid() const
{
    return report.isValid();
}

bool
BackendAvailabilityReport::pathChecksPassed() const
{
    return status == BackendAvailabilityProbeStatus::PathChecksPassed;
}

QVector<ValidationIssue>
BackendAvailabilityReport::warnings() const
{
    QVector<ValidationIssue> result;
    for (const auto &issue: report.issues) {
        if (issue.severity == ValidationSeverity::Warning) {
            result.push_back(issue);
        }
    }
    return result;
}

QVector<ValidationIssue>
BackendAvailabilityReport::errors() const
{
    QVector<ValidationIssue> result;
    for (const auto &issue: report.issues) {
        if (issue.severity == ValidationSeverity::Error) {
            result.push_back(issue);
        }
    }
    return result;
}

int
BackendAvailabilityReport::warningCount() const
{
    return warnings().size();
}

int
BackendAvailabilityReport::errorCount() const
{
    return errors().size();
}

QString
BackendAvailabilityReport::statusName() const
{
    return toString(status);
}

QString
BackendAvailabilityReport::debugSummaryString() const
{
    return QString("backend=%1 availability=%2 executable=%3 "
                   "file_entries=%4 warnings=%5 errors=%6")
        .arg(backendId.isEmpty() ? QString("unknown") : backendId)
        .arg(statusName())
        .arg(executableProbe.statusName())
        .arg(requiredFileProbe.entries.size())
        .arg(warningCount())
        .arg(errorCount());
}

BackendAvailabilityReport
BackendAvailabilityProbe::probe(const BackendManifest &manifest) const
{
    BackendExecutableProbe executableProbe;
    BackendRequiredFileProbe requiredFileProbe;
    return buildReport(manifest,
                       executableProbe.probe(manifest),
                       requiredFileProbe.probe(manifest));
}

BackendAvailabilityReport
BackendAvailabilityProbe::probe(const BackendManifest &manifest,
                                const BackendSettings &settings) const
{
    BackendExecutableProbe executableProbe;
    BackendRequiredFileProbe requiredFileProbe;
    return buildReport(manifest,
                       executableProbe.probe(manifest, settings),
                       requiredFileProbe.probe(manifest, settings));
}

QString
toString(BackendAvailabilityProbeStatus status)
{
    switch (status) {
    case BackendAvailabilityProbeStatus::NotConfigured:
        return "not_configured";
    case BackendAvailabilityProbeStatus::MissingExecutable:
        return "missing_executable";
    case BackendAvailabilityProbeStatus::MissingModel:
        return "missing_model";
    case BackendAvailabilityProbeStatus::InvalidPath:
        return "invalid_path";
    case BackendAvailabilityProbeStatus::PathChecksPassed:
        return "path_checks_passed";
    }
    return "not_configured";
}

}
}
