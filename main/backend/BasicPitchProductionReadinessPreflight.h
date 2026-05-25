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

#ifndef TONY_BASIC_PITCH_PRODUCTION_READINESS_PREFLIGHT_H
#define TONY_BASIC_PITCH_PRODUCTION_READINESS_PREFLIGHT_H

#include "BasicPitchDebugCombinedRunImportAction.h"
#include "BasicPitchDebugManualRunStatus.h"
#include "BasicPitchDebugPostImportProofAction.h"

#include <QVector>

namespace Tony {
namespace Backend {

enum class BasicPitchProductionReadinessGateStatus {
    Passed,
    Failed,
    Warning,
    NotTested,
    Blocked,
    DebugOnly
};

QString basicPitchProductionReadinessGateStatusToString(
    BasicPitchProductionReadinessGateStatus status);

struct BasicPitchProductionReadinessGate
{
    QString id;
    QString displayName;
    BasicPitchProductionReadinessGateStatus status =
        BasicPitchProductionReadinessGateStatus::NotTested;
    QString userMessage;
    QString technicalMessage;
    QString evidenceReference;
    bool blocksProductionReadiness = true;
    bool debugOnly = false;

    bool blocksProduction() const;
    QString statusString() const;
};

struct BasicPitchProductionReadinessPreflightInput
{
    BasicPitchRealRunHandoffProofConfig manualRunConfig;
    BasicPitchDebugCombinedRunImportActionReport combinedRunImportReport;
    BasicPitchDebugPostImportProofActionReport postImportProofReport;
    bool hasCombinedRunImportReport = false;
    bool hasPostImportProofReport = false;

    bool preflightBoundaryDebugOnly = true;
    bool debugWorkflowAvailable = true;
    bool debugWarningVisibilityAvailable = true;
    bool debugRegressionProtectionAvailable = true;

    bool productionConfigurationAvailable = false;
    bool productionCommandRuntimeValidated = false;
    bool productionAudioInputValidated = false;
    bool productionOutputDirectoryValidated = false;
    bool productionRunPermissionValidated = false;
    bool productionArtifactDiscoveryValidated = false;
    bool productionResultValidationValidated = false;
    bool productionLayerMappingValidated = false;
    bool productionLayerImportValidated = false;
    bool productionVisibilityValidated = false;
    bool productionEditValidated = false;
    bool productionUndoRedoValidated = false;
    bool productionSaveLoadValidated = false;
    bool productionExportValidated = false;
    bool productionWarningVisibilityValidated = false;
    bool productionProgressReportingValidated = false;
    bool productionCancellationValidated = false;
    bool productionErrorRecoveryValidated = false;
    bool productionUserDocumentationAvailable = false;
    bool productionRegressionProtectionValidated = false;
    bool pitchBendProductionPolicyValidated = false;
    bool polyphonyProductionPolicyValidated = false;
};

struct BasicPitchProductionReadinessReport
{
    ValidationReport report;
    QVector<BasicPitchProductionReadinessGate> gates;
    QStringList warnings;
    QStringList errors;
    QStringList missingProductionBlockers;
    bool productionReady = false;
    bool productionTranscription = false;
    bool testOnlyDebugOnly = true;
    bool debugWorkflowAvailable = false;
    bool manualRunAvailable = false;
    bool importProofAvailable = false;
    bool postImportProofAvailable = false;
    bool readyInstalledCompletedMutation = false;

    bool isValid() const;
    const BasicPitchProductionReadinessGate *gateById(
        const QString &id) const;
    bool hasGateStatus(
        const QString &id,
        BasicPitchProductionReadinessGateStatus status) const;
    QString debugSummaryString() const;
};

class BasicPitchProductionReadinessPreflight
{
public:
    BasicPitchProductionReadinessReport evaluate(
        const BasicPitchProductionReadinessPreflightInput &input) const;

private:
    static void addGate(BasicPitchProductionReadinessReport &report,
                        const BasicPitchProductionReadinessGate &gate);
    static void addIssueCodes(const ValidationReport &source,
                              QStringList &warnings,
                              QStringList &errors);
};

}
}

#endif
