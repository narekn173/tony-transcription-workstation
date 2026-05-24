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

#include "BasicPitchUnifiedResultHandoff.h"

#include "BasicPitchAdapterContract.h"

namespace Tony {
namespace Backend {

namespace {

void
appendIssues(ValidationReport &target, const ValidationReport &source)
{
    for (const ValidationIssue &issue: source.issues) {
        target.addIssue(issue.severity, issue.code, issue.message);
    }
}

void
addWarning(ValidationReport &report,
           const QString &code,
           const QString &message)
{
    report.addIssue(ValidationSeverity::Warning, code, message);
}

ResultWarning
makeWarning(const QString &code, const QString &message)
{
    ResultWarning warning;
    warning.code = code;
    warning.severity = ResultDiagnosticSeverity::Warning;
    warning.message = message;
    return warning;
}

}

bool
BasicPitchUnifiedResultHandoffResult::isValid() const
{
    return report.isValid() &&
        artifactConversion.isValid() &&
        fileWriteResult.isValid() &&
        outputHandoffAccepted &&
        loadedUnifiedResult &&
        reporterLoadedUnifiedResult &&
        runResultReport.isValid() &&
        !productionTranscription &&
        !importedIntoTonyLayers &&
        !marksBackendReadyInstalledOrCompleted;
}

QString
BasicPitchUnifiedResultHandoffResult::debugSummaryString() const
{
    return QString("path=%1 notes=%2 wrote=%3 handoff=%4 loaded=%5 "
                   "reported=%6 imported=%7 production=%8 ready_mutation=%9 "
                   "valid=%10")
        .arg(expectedUnifiedResultJsonPath)
        .arg(noteCount)
        .arg(wroteResultJson ? QString("true") : QString("false"))
        .arg(outputHandoffAccepted ? QString("true") : QString("false"))
        .arg(loadedUnifiedResult ? QString("true") : QString("false"))
        .arg(reporterLoadedUnifiedResult ? QString("true") : QString("false"))
        .arg(importedIntoTonyLayers ? QString("true") : QString("false"))
        .arg(productionTranscription ? QString("true") : QString("false"))
        .arg(marksBackendReadyInstalledOrCompleted ? QString("true") :
                                                     QString("false"))
        .arg(isValid() ? QString("true") : QString("false"));
}

BasicPitchUnifiedResultHandoffResult
BasicPitchUnifiedResultHandoff::handoff(
    const BasicPitchArtifactDiscoveryResult &discoveryResult,
    const BasicPitchUnifiedResultHandoffParameters &parameters) const
{
    BasicPitchUnifiedResultHandoffResult result;
    result.expectedUnifiedResultJsonPath =
        parameters.expectedUnifiedResultJsonPath.trimmed();
    result.productionTranscription = false;
    result.importedIntoTonyLayers = false;
    result.marksBackendReadyInstalledOrCompleted = false;

    BasicPitchArtifactToUnifiedResult converter;
    result.artifactConversion =
        converter.convert(discoveryResult, parameters.conversionParameters);
    appendIssues(result.report, result.artifactConversion.report);
    result.noteCount = result.artifactConversion.noteCount;
    result.possiblePolyphony = result.artifactConversion.possiblePolyphony;
    result.pitchBendMappingDeferred =
        result.artifactConversion.pitchBendMappingDeferred;

    if (!result.artifactConversion.isValid()) {
        return result;
    }

    UnifiedResult unifiedResult = result.artifactConversion.result;
    unifiedResult.status = BackendStatus::CompletedWithWarnings;
    unifiedResult.provenance.insert("created_result_json", true);
    unifiedResult.provenance.insert("result_json_path",
                                    result.expectedUnifiedResultJsonPath);
    unifiedResult.provenance.insert("unified_result_json_handoff", true);
    unifiedResult.provenance.insert("production_transcription", false);
    unifiedResult.provenance.insert("imported_into_tony_layers", false);
    unifiedResult.provenance.insert("basic_pitch_handoff_test_only", true);

    unifiedResult.warnings.push_back(makeWarning(
        "basic_pitch_result_json_handoff_test_only",
        "Basic Pitch UnifiedResult JSON handoff is a manual/test-only proof "
        "and is not production transcription support."));
    addWarning(result.report,
               "basic_pitch_result_json_handoff_test_only",
               "Basic Pitch UnifiedResult JSON handoff is manual/test-only.");

    UnifiedResultFileWriter writer;
    result.fileWriteResult =
        writer.write(result.expectedUnifiedResultJsonPath, unifiedResult);
    appendIssues(result.report, result.fileWriteResult.report);
    result.wroteResultJson = result.fileWriteResult.wroteFile;
    if (!result.fileWriteResult.isValid()) {
        return result;
    }

    BackendRunOutputHandoff handoff;
    result.outputHandoffResult =
        handoff.inspect(result.expectedUnifiedResultJsonPath);
    appendIssues(result.report, result.outputHandoffResult.report);
    result.outputHandoffAccepted = result.outputHandoffResult.isValid();
    if (!result.outputHandoffAccepted) {
        return result;
    }

    BackendRunResultLoader loader;
    result.loadResult = loader.load(result.expectedUnifiedResultJsonPath);
    appendIssues(result.report, result.loadResult.report);
    result.loadedUnifiedResult = result.loadResult.isValid();

    BackendRunResultReporter reporter;
    result.runResultReport =
        reporter.buildReport(BasicPitchAdapterContract::backendId(),
                             result.expectedUnifiedResultJsonPath,
                             result.loadResult);
    appendIssues(result.report, result.runResultReport.report);
    result.reporterLoadedUnifiedResult =
        result.runResultReport.unifiedResultLoaded;

    if (result.loadResult.loadedResult.has_value()) {
        result.noteCount = result.loadResult.loadedResult->notes.size();
    }

    return result;
}

}
}
