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

#ifndef TONY_BASIC_PITCH_DEBUG_WORKFLOW_UI_MODEL_H
#define TONY_BASIC_PITCH_DEBUG_WORKFLOW_UI_MODEL_H

#include "BasicPitchDebugWorkflow.h"

namespace Tony {
namespace Backend {

struct BasicPitchDebugWorkflowUiProofBundleSummary
{
    QStringList eventStates;
    int eventCount = 0;
    int artifactCount = 0;
    QString resultJsonPath;
    QString exportCsvPath;
    int noteCount = 0;
    bool loadedResult = false;
    bool importedIntoTonyLayers = false;
    bool insertedIntoView = false;
    bool editProof = false;
    bool saveLoadProof = false;
    bool exportProof = false;
    bool productionTranscription = false;
    bool testOnlyDebugOnly = true;
};

struct BasicPitchDebugWorkflowUiModelResult
{
    ValidationReport report;
    QString primaryState;
    QString secondaryState;
    QString userMessage;
    QString technicalMessage;
    QString proofBundleSummary;
    BasicPitchDebugWorkflowUiProofBundleSummary proofBundle;
    QStringList warnings;
    QStringList errors;
    bool canRun = false;
    bool canCancel = false;
    bool canImport = false;
    bool canEdit = false;
    bool canSave = false;
    bool canExport = false;
    bool shouldShowWarnings = false;
    bool shouldShowProofBundle = false;
    bool productionTranscription = false;
    bool testOnlyDebugOnly = true;
    bool readyInstalledCompletedMutation = false;

    bool isValid() const;
    QString debugSummaryString() const;
};

class BasicPitchDebugWorkflowUiModel
{
public:
    BasicPitchDebugWorkflowUiModelResult fromReport(
        const BasicPitchDebugWorkflowReport &workflowReport) const;

private:
    static void appendIssues(ValidationReport &target,
                             const ValidationReport &source);
};

}
}

#endif
