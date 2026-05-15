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

#ifndef TEST_BACKEND_TYPES_H
#define TEST_BACKEND_TYPES_H

#include "../BackendTypes.h"

#include <QObject>
#include <QtTest>

using namespace Tony::Backend;

class TestBackendTypes : public QObject
{
    Q_OBJECT

private slots:
    void statusStrings()
    {
        QCOMPARE(statusToString(BackendStatus::Unknown), QString("unknown"));
        QCOMPARE(statusToString(BackendStatus::NotConfigured),
                 QString("not_configured"));
        QCOMPARE(statusToString(BackendStatus::MissingExecutable),
                 QString("missing_executable"));
        QCOMPARE(statusToString(BackendStatus::MissingModel),
                 QString("missing_model"));
        QCOMPARE(statusToString(BackendStatus::Ready), QString("ready"));
        QCOMPARE(statusToString(BackendStatus::Running), QString("running"));
        QCOMPARE(statusToString(BackendStatus::Completed),
                 QString("completed"));
        QCOMPARE(statusToString(BackendStatus::CompletedWithWarnings),
                 QString("completed_with_warnings"));
        QCOMPARE(statusToString(BackendStatus::Failed), QString("failed"));
        QCOMPARE(statusToString(BackendStatus::Cancelled),
                 QString("cancelled"));
    }

    void capabilityDefaultsAreSafe()
    {
        BackendCapability capability;

        QVERIFY(!capability.supportsFullFile);
        QVERIFY(!capability.supportsSelectedRegion);
        QVERIFY(!capability.outputsNotes);
        QVERIFY(!capability.outputsPitchCurve);
        QVERIFY(!capability.outputsPitchBends);
        QVERIFY(!capability.outputsTechniqueLabels);
        QVERIFY(!capability.requiresPython);
        QVERIFY(!capability.requiresModelCheckpoint);
        QVERIFY(!capability.supportsCpu);
        QVERIFY(!capability.supportsGpuOptional);

        QVERIFY(!capability.supportsMode(AnalysisMode::FullFile));
        QVERIFY(!capability.supportsMode(AnalysisMode::Region));
        QCOMPARE(capability.summaryString(), QString("none"));
    }

    void capabilitySummaryReportsEnabledFields()
    {
        BackendCapability capability;
        capability.supportsFullFile = true;
        capability.supportsSelectedRegion = true;
        capability.outputsNotes = true;
        capability.outputsPitchCurve = true;
        capability.outputsPitchBends = true;
        capability.outputsTechniqueLabels = true;
        capability.requiresPython = true;
        capability.requiresModelCheckpoint = true;
        capability.supportsCpu = true;
        capability.supportsGpuOptional = true;

        const QString summary = capabilitySummaryString(capability);
        QVERIFY(summary.contains("full_file"));
        QVERIFY(summary.contains("selected_region"));
        QVERIFY(summary.contains("notes"));
        QVERIFY(summary.contains("pitch_curve"));
        QVERIFY(summary.contains("pitch_bends"));
        QVERIFY(summary.contains("technique_labels"));
        QVERIFY(summary.contains("requires_python"));
        QVERIFY(summary.contains("requires_model_checkpoint"));
        QVERIFY(summary.contains("cpu"));
        QVERIFY(summary.contains("gpu_optional"));
    }

    void manifestDefaultsDoNotAdvertiseAvailability()
    {
        BackendManifest manifest;

        QVERIFY(manifest.status == BackendStatus::NotConfigured);
        QVERIFY(manifest.backendId.isEmpty());
        QVERIFY(manifest.displayName.isEmpty());
        QVERIFY(manifest.description.isEmpty());
        QVERIFY(manifest.backendType == BackendRuntimeType::Unknown);
        QVERIFY(manifest.executablePath.isEmpty());
        QVERIFY(manifest.workingDirectory.isEmpty());
        QVERIFY(manifest.version.isEmpty());
        QVERIFY(!manifest.capabilities.supportsFullFile);
        QVERIFY(!manifest.capabilities.outputsNotes);
        QVERIFY(!manifest.capabilities.supportsCpu);
        QVERIFY(manifest.requiredFiles.isEmpty());
        QVERIFY(manifest.optionalFiles.isEmpty());
        QVERIFY(manifest.defaultSettings.isEmpty());
        QVERIFY(manifest.supportedInputFormats.isEmpty());
        QVERIFY(manifest.supportedOutputTypes.isEmpty());
        QVERIFY(!manifest.isValidBackendId());
        QVERIFY(!manifest.hasExecutablePath());
        QVERIFY(!manifest.supportsFullFile());
        QVERIFY(!manifest.supportsSelectedRegion());
        QVERIFY(!manifest.supportsNotes());
        QVERIFY(!manifest.supportsPitchCurve());
        QVERIFY(!manifest.hasRequiredIdentity());
    }

    void manifestAcceptsValidContractShape()
    {
        BackendManifest manifest;
        manifest.backendId = "test_backend";
        manifest.displayName = "Test Backend";
        manifest.description = "Contract-shape unit test backend.";
        manifest.backendType = BackendRuntimeType::AdapterCli;
        manifest.executablePath = "configured-backend.exe";
        manifest.workingDirectory = "configured-backend-workdir";
        manifest.version = "0.1.0-test";
        manifest.adapterVersion = "0.1.0";
        manifest.status = BackendStatus::NotConfigured;
        manifest.capabilities.supportsFullFile = true;
        manifest.capabilities.supportsSelectedRegion = true;
        manifest.capabilities.outputsNotes = true;
        manifest.capabilities.outputsPitchBends = true;
        manifest.capabilities.supportsCpu = true;
        manifest.capabilities.requiresPython = true;
        manifest.requiredFiles << "model-or-package";
        manifest.optionalFiles << "notes.mid";
        manifest.defaultSettings.insert("onset_threshold", 0.5);
        manifest.supportedInputFormats << "wav";
        manifest.supportedOutputTypes << "notes" << "midi" << "csv_notes";

        QVERIFY(manifest.isValidBackendId());
        QVERIFY(isValidBackendId(manifest.backendId));
        QVERIFY(manifest.hasRequiredIdentity());
        QVERIFY(manifest.hasExecutablePath());
        QVERIFY(manifest.supportsFullFile());
        QVERIFY(manifest.supportsSelectedRegion());
        QVERIFY(manifest.supportsNotes());
        QVERIFY(!manifest.supportsPitchCurve());
        QCOMPARE(manifest.id(), QString("test_backend"));

        const QString summary = manifestSummaryString(manifest);
        QVERIFY(summary.contains("test_backend"));
        QVERIFY(summary.contains("not_configured"));
        QVERIFY(summary.contains("adapter_cli"));
        QVERIFY(summary.contains("notes"));
    }

    void manifestRejectsInvalidBackendIds()
    {
        QVERIFY(!isValidBackendId(""));
        QVERIFY(!isValidBackendId("BasicPitch"));
        QVERIFY(!isValidBackendId("1basic_pitch"));
        QVERIFY(!isValidBackendId("basic-pitch"));
        QVERIFY(isValidBackendId("basic_pitch_2"));

        BackendManifest manifest;
        manifest.backendId = "BasicPitch";
        manifest.displayName = "Basic Pitch";

        QVERIFY(!manifest.isValidBackendId());
        QVERIFY(!manifest.hasRequiredIdentity());
    }

    void manifestCapabilityChecksReflectDeclaredCapabilitiesOnly()
    {
        BackendManifest manifest;

        QVERIFY(!manifest.supportsFullFile());
        QVERIFY(!manifest.supportsSelectedRegion());
        QVERIFY(!manifest.supportsNotes());
        QVERIFY(!manifest.supportsPitchCurve());

        manifest.capabilities.supportsFullFile = true;
        manifest.capabilities.outputsPitchCurve = true;

        QVERIFY(manifest.supportsFullFile());
        QVERIFY(!manifest.supportsSelectedRegion());
        QVERIFY(!manifest.supportsNotes());
        QVERIFY(manifest.supportsPitchCurve());
        QVERIFY(!manifest.capabilities.supportsMode(AnalysisMode::Region));
    }
};

#endif
