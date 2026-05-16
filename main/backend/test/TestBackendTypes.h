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

#include "../BackendAvailabilityProbe.h"
#include "../BackendAvailabilityStore.h"
#include "../BackendDiscoveryConfig.h"
#include "../BackendDiscoveryService.h"
#include "../BackendExecutableProbe.h"
#include "../BackendManifestDirectoryLoader.h"
#include "../BackendManifestFileLoader.h"
#include "../BackendManifestParser.h"
#include "../BackendManifestSchemaValidator.h"
#include "../BackendRegistry.h"
#include "../BackendRequiredFileProbe.h"
#include "../BackendRunRequestFileWriter.h"
#include "../BackendRunRequestBuilder.h"
#include "../BackendRunRequestSerializer.h"
#include "../BackendRunWorkspace.h"
#include "../BackendSettingsDirectoryPreparer.h"
#include "../BackendSettingsFactory.h"
#include "../BackendSettingsFileStore.h"
#include "../BackendSettingsPathResolver.h"
#include "../BackendSettingsPersistenceConfig.h"
#include "../BackendSettingsPersistenceService.h"
#include "../BackendSettingsSerializer.h"
#include "../BackendSettingsStore.h"
#include "../BackendTypes.h"
#include "../ExternalProcessLogFileSink.h"
#include "../ExternalProcessRunner.h"

#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QTemporaryDir>
#include <QtTest>

#include <future>

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

    void manifestParserAcceptsBasicPitchExampleShape()
    {
        const QJsonDocument document =
            QJsonDocument::fromJson(basicPitchManifestJson());
        QVERIFY(document.isObject());

        BackendManifestParser parser;
        const BackendManifestParseResult parsed =
            parser.parse(document.object());

        QVERIFY(parsed.isValid());
        QCOMPARE(parsed.manifest.id(), QString("basic_pitch"));
        QCOMPARE(parsed.manifest.displayName, QString("Basic Pitch"));
        QVERIFY(parsed.manifest.backendType == BackendRuntimeType::PythonCli);
        QVERIFY(parsed.manifest.status == BackendStatus::NotConfigured);
        QVERIFY(parsed.manifest.capabilities.supportsFullFile);
        QVERIFY(parsed.manifest.capabilities.supportsSelectedRegion);
        QVERIFY(parsed.manifest.capabilities.outputsNotes);
        QVERIFY(!parsed.manifest.capabilities.outputsPitchCurve);
        QVERIFY(parsed.manifest.capabilities.outputsPitchBends);
        QVERIFY(parsed.manifest.capabilities.requiresPython);
        QVERIFY(parsed.manifest.capabilities.supportsCpu);
        QVERIFY(!parsed.manifest.hasExecutablePath());
        QCOMPARE(parsed.manifest.supportedInputFormats, QStringList({ "wav" }));
        QVERIFY(parsed.manifest.supportedOutputTypes.contains("notes"));
        QVERIFY(parsed.manifest.supportedOutputTypes.contains("midi"));
        QVERIFY(parsed.manifest.supportedOutputTypes.contains("csv_notes"));
    }

    void manifestParserRejectsMissingBackendId()
    {
        QJsonObject object = QJsonDocument::fromJson(basicPitchManifestJson()).object();
        object.remove("engine_id");

        BackendManifestParser parser;
        const BackendManifestParseResult parsed = parser.parse(object);

        QVERIFY(!parsed.isValid());
        QCOMPARE(parsed.report.issues.front().code, QString("missing_backend_id"));
    }

    void manifestParserRejectsInvalidBackendId()
    {
        QJsonObject object = QJsonDocument::fromJson(basicPitchManifestJson()).object();
        object.insert("engine_id", "BasicPitch");

        BackendManifestParser parser;
        const BackendManifestParseResult parsed = parser.parse(object);

        QVERIFY(!parsed.isValid());
        QCOMPARE(parsed.report.issues.front().code, QString("invalid_backend_id"));
    }

    void manifestParserRejectsMalformedCapabilities()
    {
        QJsonObject object = QJsonDocument::fromJson(basicPitchManifestJson()).object();
        QJsonObject capabilities = object.value("capabilities").toObject();
        capabilities.insert("supports_full_file", "yes");
        object.insert("capabilities", capabilities);

        BackendManifestParser parser;
        const BackendManifestParseResult parsed = parser.parse(object);

        QVERIFY(!parsed.isValid());
        QVERIFY(!parsed.manifest.capabilities.supportsFullFile);
        QVERIFY(!parsed.report.issues.isEmpty());
    }

    void manifestParserRejectsEmptyManifest()
    {
        BackendManifestParser parser;
        const BackendManifestParseResult parsed = parser.parse(QJsonObject());

        QVERIFY(!parsed.isValid());
        QVERIFY(parsed.report.issues.size() >= 2);
        QVERIFY(!parsed.manifest.isValidBackendId());
        QVERIFY(parsed.manifest.status == BackendStatus::NotConfigured);
    }

    void manifestFileLoaderLoadsValidManifestFile()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath("backend_manifest.json");
        QVERIFY(writeFile(path, basicPitchManifestJson()));

        BackendManifestFileLoader loader;
        const BackendManifestFileLoadResult loaded = loader.load(path);

        QVERIFY(loaded.isValid());
        QCOMPARE(loaded.path, path);
        QCOMPARE(loaded.manifest.id(), QString("basic_pitch"));
        QCOMPARE(loaded.manifest.displayName, QString("Basic Pitch"));
        QVERIFY(loaded.manifest.backendType == BackendRuntimeType::PythonCli);
        QVERIFY(loaded.manifest.status == BackendStatus::NotConfigured);
    }

    void manifestFileLoaderMissingFileFailsCleanly()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath("missing_manifest.json");

        BackendManifestFileLoader loader;
        const BackendManifestFileLoadResult loaded = loader.load(path);

        QVERIFY(!loaded.isValid());
        QCOMPARE(loaded.path, path);
        QVERIFY(!loaded.report.issues.isEmpty());
        QCOMPARE(loaded.report.issues.front().code, QString("file_open_failed"));
    }

    void manifestFileLoaderInvalidJsonFailsCleanly()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath("backend_manifest.json");
        QVERIFY(writeFile(path, "{ invalid json"));

        BackendManifestFileLoader loader;
        const BackendManifestFileLoadResult loaded = loader.load(path);

        QVERIFY(!loaded.isValid());
        QVERIFY(!loaded.report.issues.isEmpty());
        QCOMPARE(loaded.report.issues.front().code, QString("invalid_json"));
    }

    void manifestFileLoaderWrongTopLevelJsonFailsCleanly()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath("backend_manifest.json");
        QVERIFY(writeFile(path, "[]"));

        BackendManifestFileLoader loader;
        const BackendManifestFileLoadResult loaded = loader.load(path);

        QVERIFY(!loaded.isValid());
        QVERIFY(!loaded.report.issues.isEmpty());
        QCOMPARE(loaded.report.issues.front().code,
                 QString("invalid_top_level_json"));
    }

    void manifestFileLoaderDoesNotMarkManifestReady()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath("backend_manifest.json");
        QVERIFY(writeFile(path, basicPitchManifestJson()));

        BackendManifestFileLoader loader;
        const BackendManifestFileLoadResult loaded = loader.load(path);

        QVERIFY(loaded.isValid());
        QVERIFY(loaded.manifest.status == BackendStatus::NotConfigured);
        QVERIFY(!loaded.manifest.hasExecutablePath());
    }

    void manifestSchemaValidatorReportsDeferredFullSchemaBoundary()
    {
        BackendManifestSchemaValidator validator;

        QVERIFY(!validator.hasFullJsonSchemaValidator());
        QCOMPARE(validator.schemaReferencePath(),
                 QString("docs/schemas/backend_manifest.schema.json"));
    }

    void manifestSchemaValidatorAcceptsValidJsonThroughLightweightBoundary()
    {
        const QJsonDocument document =
            QJsonDocument::fromJson(basicPitchManifestJson());
        QVERIFY(document.isObject());

        BackendManifestSchemaValidator validator;
        const BackendManifestSchemaValidationResult validation =
            validator.validateLightweight(document.object());

        QVERIFY(validation.isValid());
        QVERIFY(!validation.fullJsonSchemaValidationApplied);
        QCOMPARE(validation.schemaReferencePath,
                 QString("docs/schemas/backend_manifest.schema.json"));
        QVERIFY(validation.debugSummaryString().contains("deferred"));
    }

    void manifestSchemaValidatorRejectsInvalidJsonThroughLightweightBoundary()
    {
        QJsonObject object = QJsonDocument::fromJson(basicPitchManifestJson()).object();
        object.insert("engine_id", "BasicPitch");

        BackendManifestSchemaValidator validator;
        const BackendManifestSchemaValidationResult validation =
            validator.validateLightweight(object);

        QVERIFY(!validation.isValid());
        QVERIFY(!validation.fullJsonSchemaValidationApplied);
        QCOMPARE(validation.report.issues.front().code,
                 QString("invalid_backend_id"));
    }

    void manifestSchemaValidatorKeepsParsedManifestValidationSeparate()
    {
        BackendManifest manifest;
        manifest.contractVersion = "0.1";
        manifest.backendId = "BasicPitch";
        manifest.displayName = "Basic Pitch";

        BackendManifestSchemaValidator validator;
        const BackendManifestSchemaValidationResult validation =
            validator.validateParsedManifest(manifest);

        QVERIFY(!validation.isValid());
        QVERIFY(!validation.fullJsonSchemaValidationApplied);
        QCOMPARE(validation.report.issues.front().code,
                 QString("invalid_backend_id"));
    }

    void manifestSchemaValidatorDoesNotChangeAvailabilityStatus()
    {
        BackendManifest manifest;
        manifest.backendId = "basic_pitch";
        manifest.displayName = "Basic Pitch";
        manifest.status = BackendStatus::NotConfigured;

        BackendManifestSchemaValidator validator;
        const BackendManifestSchemaValidationResult validation =
            validator.validateParsedManifest(manifest);

        QVERIFY(validation.isValid());
        QVERIFY(manifest.status == BackendStatus::NotConfigured);
    }

    void registryAddsValidManifest()
    {
        BackendRegistry registry;
        const BackendManifest manifest = parsedBasicPitchManifest();

        QVERIFY(registry.addManifest(manifest));
        QVERIFY(registry.hasBackend("basic_pitch"));
        QCOMPARE(registry.allManifests().size(), 1);
        QVERIFY(registry.engineIds().contains("basic_pitch"));
    }

    void registryRejectsDuplicateBackendId()
    {
        BackendRegistry registry;
        BackendManifest manifest = parsedBasicPitchManifest();

        QVERIFY(registry.addManifest(manifest));

        manifest.displayName = "Duplicate Basic Pitch";
        QVERIFY(!registry.addManifest(manifest));
        QCOMPARE(registry.allManifests().size(), 1);
    }

    void registryRejectsInvalidBackendId()
    {
        BackendRegistry registry;
        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.backendId = "BasicPitch";
        manifest.engineId = "BasicPitch";

        QVERIFY(!registry.addManifest(manifest));
        QVERIFY(!registry.hasBackend("BasicPitch"));
        QVERIFY(registry.allManifests().isEmpty());
    }

    void registryLooksUpManifestByBackendId()
    {
        BackendRegistry registry;
        const BackendManifest manifest = parsedBasicPitchManifest();

        QVERIFY(registry.addManifest(manifest));
        const std::optional<BackendManifest> found =
            registry.manifestById("basic_pitch");

        QVERIFY(found.has_value());
        QCOMPARE(found->id(), QString("basic_pitch"));
        QCOMPARE(found->displayName, QString("Basic Pitch"));
        QVERIFY(found->status == BackendStatus::NotConfigured);
        QVERIFY(!registry.manifestById("missing_backend").has_value());
    }

    void registryRemovesManifestByBackendId()
    {
        BackendRegistry registry;
        const BackendManifest manifest = parsedBasicPitchManifest();

        QVERIFY(registry.addManifest(manifest));
        QVERIFY(registry.removeManifest("basic_pitch"));
        QVERIFY(!registry.hasBackend("basic_pitch"));
        QVERIFY(registry.allManifests().isEmpty());
        QVERIFY(!registry.removeManifest("basic_pitch"));
    }

    void registryClearRemovesLoadedManifests()
    {
        BackendRegistry registry;
        QVERIFY(registry.addManifest(parsedBasicPitchManifest()));
        QVERIFY(registry.hasBackend("basic_pitch"));

        registry.clear();

        QVERIFY(!registry.hasBackend("basic_pitch"));
        QVERIFY(registry.allManifests().isEmpty());
        QVERIFY(registry.engineIds().isEmpty());
        QCOMPARE(registry.size(), 0);
    }

    void registryDoesNotMarkLoadedManifestReady()
    {
        BackendRegistry registry;
        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.status = BackendStatus::Ready;

        QVERIFY(registry.addManifest(manifest));

        const std::optional<BackendManifest> found =
            registry.manifestById("basic_pitch");
        QVERIFY(found.has_value());
        QVERIFY(found->status == BackendStatus::NotConfigured);
    }

    void manifestDirectoryLoaderLoadsDirectoryWithOneValidManifest()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath("backend_manifest.json");
        QVERIFY(writeFile(path, basicPitchManifestJson()));

        BackendRegistry registry;
        BackendManifestDirectoryLoader loader;
        const BackendManifestDirectoryLoadResult loaded =
            loader.load(directory.path(), registry);

        QVERIFY(loaded.isValid());
        QCOMPARE(loaded.directoryPath, directory.path());
        QCOMPARE(loaded.entries.size(), 1);
        QCOMPARE(loaded.loadedCount(), 1);
        QVERIFY(loaded.entries.front().isValid());
        QVERIFY(registry.hasBackend("basic_pitch"));

        const std::optional<BackendManifest> manifest =
            registry.manifestById("basic_pitch");
        QVERIFY(manifest.has_value());
        QCOMPARE(manifest->displayName, QString("Basic Pitch"));
        QVERIFY(manifest->status == BackendStatus::NotConfigured);
    }

    void manifestDirectoryLoaderReportsInvalidJsonFile()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath("backend_manifest.json");
        QVERIFY(writeFile(path, "{ invalid json"));

        BackendRegistry registry;
        BackendManifestDirectoryLoader loader;
        const BackendManifestDirectoryLoadResult loaded =
            loader.load(directory.path(), registry);

        QVERIFY(!loaded.isValid());
        QCOMPARE(loaded.entries.size(), 1);
        QCOMPARE(loaded.loadedCount(), 0);
        QVERIFY(!loaded.entries.front().isValid());
        QVERIFY(reportHasIssue(loaded.entries.front().report, "invalid_json"));
        QVERIFY(reportHasIssue(loaded.report, "invalid_json"));
        QVERIFY(registry.allManifests().isEmpty());
    }

    void manifestDirectoryLoaderMissingDirectoryFailsCleanly()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString missingPath = directory.filePath("missing");

        BackendRegistry registry;
        BackendManifestDirectoryLoader loader;
        const BackendManifestDirectoryLoadResult loaded =
            loader.load(missingPath, registry);

        QVERIFY(!loaded.isValid());
        QCOMPARE(loaded.directoryPath, missingPath);
        QCOMPARE(loaded.entries.size(), 0);
        QCOMPARE(loaded.loadedCount(), 0);
        QVERIFY(reportHasIssue(loaded.report, "directory_not_found"));
        QVERIFY(registry.allManifests().isEmpty());
    }

    void manifestDirectoryLoaderReportsDuplicateBackendIds()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        QVERIFY(writeFile(directory.filePath("a_manifest.json"),
                          basicPitchManifestJson()));
        QVERIFY(writeFile(directory.filePath("b_manifest.json"),
                          basicPitchManifestJson()));

        BackendRegistry registry;
        BackendManifestDirectoryLoader loader;
        const BackendManifestDirectoryLoadResult loaded =
            loader.load(directory.path(), registry);

        QVERIFY(!loaded.isValid());
        QCOMPARE(loaded.entries.size(), 2);
        QCOMPARE(loaded.loadedCount(), 1);
        QVERIFY(loaded.entries[0].addedToRegistry);
        QVERIFY(!loaded.entries[1].addedToRegistry);
        QVERIFY(reportHasIssue(loaded.entries[1].report, "registry_add_failed"));
        QVERIFY(reportHasIssue(loaded.report, "registry_add_failed"));
        QCOMPARE(registry.allManifests().size(), 1);
        QVERIFY(registry.hasBackend("basic_pitch"));
    }

    void manifestDirectoryLoaderDoesNotMarkBackendReadyOrInstalled()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath("backend_manifest.json");
        QVERIFY(writeFile(path, basicPitchManifestJson()));

        BackendRegistry registry;
        BackendManifestDirectoryLoader loader;
        const BackendManifestDirectoryLoadResult loaded =
            loader.load(directory.path(), registry);

        QVERIFY(loaded.isValid());

        const std::optional<BackendManifest> manifest =
            registry.manifestById("basic_pitch");
        QVERIFY(manifest.has_value());
        QVERIFY(manifest->status == BackendStatus::NotConfigured);
        QVERIFY(manifest->status != BackendStatus::Ready);
    }

    void discoveryConfigDefaultsAreSafe()
    {
        const BackendDiscoveryConfig config =
            BackendDiscoveryConfig::safeDefaults();

        QCOMPARE(config.defaultLocalManifestDirectoryPath,
                 QString("backends/manifests"));
        QVERIFY(config.userConfiguredManifestDirectoryPath.isEmpty());
        QVERIFY(config.testOnlyManifestDirectoryPath.isEmpty());
        QVERIFY(config.hasAnyUsableManifestDirectory());

        const QVector<BackendDiscoveryPath> candidates =
            config.manifestDirectoryCandidates();
        QCOMPARE(candidates.size(), 1);
        QVERIFY(candidates.front().source ==
                BackendDiscoveryPathSource::DefaultLocal);
        QCOMPARE(candidates.front().path, QString("backends/manifests"));
        QCOMPARE(candidates.front().sourceName(), QString("default_local"));
        QVERIFY(candidates.front().isUsable());
        QVERIFY(config.validate().isValid());
    }

    void discoveryConfigSupportsCustomManifestDirectory()
    {
        BackendDiscoveryConfig config = BackendDiscoveryConfig::safeDefaults();
        config.userConfiguredManifestDirectoryPath =
            "C:/Users/Test/AppData/Local/Tony/backends";

        const QVector<BackendDiscoveryPath> candidates =
            config.manifestDirectoryCandidates();

        QCOMPARE(candidates.size(), 2);
        QVERIFY(candidates[0].source ==
                BackendDiscoveryPathSource::DefaultLocal);
        QVERIFY(candidates[1].source ==
                BackendDiscoveryPathSource::UserConfigured);
        QCOMPARE(candidates[1].path,
                 QString("C:/Users/Test/AppData/Local/Tony/backends"));
        QCOMPARE(candidates[1].sourceName(), QString("user_configured"));
        QVERIFY(config.validate().isValid());
    }

    void discoveryConfigSupportsExplicitTestOnlyDirectory()
    {
        BackendDiscoveryConfig config;
        config.testOnlyManifestDirectoryPath = "C:/test/backend-manifests";

        const QVector<BackendDiscoveryPath> candidates =
            config.manifestDirectoryCandidates();

        QCOMPARE(candidates.size(), 1);
        QVERIFY(candidates.front().source == BackendDiscoveryPathSource::TestOnly);
        QCOMPARE(candidates.front().path,
                 QString("C:/test/backend-manifests"));
        QCOMPARE(candidates.front().sourceName(), QString("test_only"));
        QVERIFY(config.validate().isValid());
    }

    void discoveryConfigHandlesEmptyPathsWithoutCrashing()
    {
        BackendDiscoveryConfig config;
        config.defaultLocalManifestDirectoryPath = " ";
        config.userConfiguredManifestDirectoryPath = "  ";
        config.testOnlyManifestDirectoryPath = "";

        const QVector<BackendDiscoveryPath> candidates =
            config.manifestDirectoryCandidates();
        const ValidationReport report = config.validate();

        QVERIFY(candidates.isEmpty());
        QVERIFY(!config.hasAnyUsableManifestDirectory());
        QVERIFY(report.isValid());
        QVERIFY(reportHasIssue(report, "empty_default_manifest_directory"));
        QVERIFY(reportHasIssue(report, "empty_user_manifest_directory"));
        QVERIFY(reportHasIssue(report, "no_manifest_directories"));
    }

    void discoveryConfigDoesNotMarkAnyBackendReadyOrInstalled()
    {
        BackendRegistry registry;
        BackendDiscoveryConfig config = BackendDiscoveryConfig::safeDefaults();
        config.userConfiguredManifestDirectoryPath =
            "C:/Users/Test/AppData/Local/Tony/backends";

        QVERIFY(config.hasAnyUsableManifestDirectory());
        QVERIFY(registry.allManifests().isEmpty());
        QVERIFY(!registry.hasBackend("basic_pitch"));
        QVERIFY(!registry.manifestById("basic_pitch").has_value());
    }

    void discoveryServiceDiscoversFromTestDirectory()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        QVERIFY(writeFile(directory.filePath("backend_manifest.json"),
                          basicPitchManifestJson()));

        BackendDiscoveryConfig config;
        config.testOnlyManifestDirectoryPath = directory.path();
        BackendRegistry registry;
        BackendDiscoveryService service;
        const BackendDiscoveryResult discovered =
            service.discover(config, registry);

        QVERIFY(discovered.isValid());
        QCOMPARE(discovered.directoriesScanned(), 1);
        QCOMPARE(discovered.manifestsLoaded(), 1);
        QCOMPARE(discovered.manifestsRejected(), 0);
        QCOMPARE(discovered.duplicateIds(), 0);
        QCOMPARE(discovered.directories.front().directory.path,
                 directory.path());
        QVERIFY(discovered.directories.front().directory.source ==
                BackendDiscoveryPathSource::TestOnly);
        QVERIFY(registry.hasBackend("basic_pitch"));
    }

    void discoveryServiceMissingDirectoryHandledCleanly()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendDiscoveryConfig config;
        config.testOnlyManifestDirectoryPath = directory.filePath("missing");
        BackendRegistry registry;
        BackendDiscoveryService service;
        const BackendDiscoveryResult discovered =
            service.discover(config, registry);

        QVERIFY(!discovered.isValid());
        QCOMPARE(discovered.directoriesScanned(), 1);
        QCOMPARE(discovered.manifestsLoaded(), 0);
        QCOMPARE(discovered.manifestsRejected(), 0);
        QVERIFY(reportHasIssue(discovered.report, "directory_not_found"));
        QVERIFY(registry.allManifests().isEmpty());
    }

    void discoveryServiceRegistersValidManifest()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        QVERIFY(writeFile(directory.filePath("backend_manifest.json"),
                          basicPitchManifestJson()));

        BackendDiscoveryConfig config;
        config.testOnlyManifestDirectoryPath = directory.path();
        BackendRegistry registry;
        BackendDiscoveryService service;
        const BackendDiscoveryResult discovered =
            service.discover(config, registry);

        QVERIFY(discovered.isValid());

        const std::optional<BackendManifest> manifest =
            registry.manifestById("basic_pitch");
        QVERIFY(manifest.has_value());
        QCOMPARE(manifest->displayName, QString("Basic Pitch"));
        QVERIFY(manifest->status == BackendStatus::NotConfigured);
    }

    void discoveryServiceRejectsInvalidManifest()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        QVERIFY(writeFile(directory.filePath("backend_manifest.json"),
                          "{ invalid json"));

        BackendDiscoveryConfig config;
        config.testOnlyManifestDirectoryPath = directory.path();
        BackendRegistry registry;
        BackendDiscoveryService service;
        const BackendDiscoveryResult discovered =
            service.discover(config, registry);

        QVERIFY(!discovered.isValid());
        QCOMPARE(discovered.directoriesScanned(), 1);
        QCOMPARE(discovered.manifestsLoaded(), 0);
        QCOMPARE(discovered.manifestsRejected(), 1);
        QVERIFY(reportHasIssue(discovered.report, "invalid_json"));
        QVERIFY(registry.allManifests().isEmpty());
    }

    void discoveryServiceReportsDuplicateBackendIds()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        QVERIFY(writeFile(directory.filePath("a_manifest.json"),
                          basicPitchManifestJson()));
        QVERIFY(writeFile(directory.filePath("b_manifest.json"),
                          basicPitchManifestJson()));

        BackendDiscoveryConfig config;
        config.testOnlyManifestDirectoryPath = directory.path();
        BackendRegistry registry;
        BackendDiscoveryService service;
        const BackendDiscoveryResult discovered =
            service.discover(config, registry);

        QVERIFY(!discovered.isValid());
        QCOMPARE(discovered.directoriesScanned(), 1);
        QCOMPARE(discovered.manifestsLoaded(), 1);
        QCOMPARE(discovered.manifestsRejected(), 1);
        QCOMPARE(discovered.duplicateIds(), 1);
        QVERIFY(reportHasIssue(discovered.report, "registry_add_failed"));
        QCOMPARE(registry.allManifests().size(), 1);
        QVERIFY(registry.hasBackend("basic_pitch"));
    }

    void discoveryServiceDoesNotMarkManifestReadyOrInstalled()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        QVERIFY(writeFile(directory.filePath("backend_manifest.json"),
                          basicPitchManifestJson()));

        BackendDiscoveryConfig config;
        config.testOnlyManifestDirectoryPath = directory.path();
        BackendRegistry registry;
        BackendDiscoveryService service;
        const BackendDiscoveryResult discovered =
            service.discover(config, registry);

        QVERIFY(discovered.isValid());

        const std::optional<BackendManifest> manifest =
            registry.manifestById("basic_pitch");
        QVERIFY(manifest.has_value());
        QVERIFY(manifest->status == BackendStatus::NotConfigured);
        QVERIFY(manifest->status != BackendStatus::Ready);
    }

    void backendSettingsDefaultsAreSafe()
    {
        BackendSettings settings;

        QVERIFY(settings.backendId.isEmpty());
        QVERIFY(settings.executablePathOverride.isEmpty());
        QVERIFY(settings.workingDirectoryOverride.isEmpty());
        QVERIFY(settings.modelCheckpointPathOverride.isEmpty());
        QVERIFY(settings.pythonExecutablePathOverride.isEmpty());
        QVERIFY(settings.environmentVariables.isEmpty());
        QVERIFY(!settings.enabled);
        QVERIFY(!settings.isValidBackendId());
        QVERIFY(!settings.hasAnyPathOverride());
        QVERIFY(!settings.isConfigured());
        QVERIFY(settings.statusFromSettings() == BackendStatus::NotConfigured);
        QVERIFY(!settings.validate().isValid());
        QVERIFY(reportHasIssue(settings.validate(), "invalid_backend_id"));
    }

    void backendSettingsStoreAddsAndGetsSettings()
    {
        BackendSettings settings;
        settings.backendId = "basic_pitch";
        settings.executablePathOverride = "C:/Tools/basic_pitch/adapter.exe";
        settings.workingDirectoryOverride = "C:/Tools/basic_pitch";
        settings.modelCheckpointPathOverride = "C:/Models/basic_pitch/model";
        settings.pythonExecutablePathOverride = "C:/Python/python.exe";
        settings.environmentVariables.insert("TONY_BACKEND_MODE", "test");
        settings.enabled = true;
        settings.displayLabelOverride = "Local Basic Pitch";
        settings.userNotes = "Configured by user.";

        BackendSettingsStore store;

        QVERIFY(settings.validate().isValid());
        QVERIFY(settings.isConfigured());
        QVERIFY(settings.hasExecutablePathOverride());
        QVERIFY(settings.hasWorkingDirectoryOverride());
        QVERIFY(settings.hasModelCheckpointPathOverride());
        QVERIFY(settings.hasPythonExecutablePathOverride());
        QVERIFY(store.setSettings(settings));
        QCOMPARE(store.size(), 1);
        QCOMPARE(store.configuredBackendIds().size(), 1);
        QCOMPARE(store.configuredBackendIds().front(), QString("basic_pitch"));

        const std::optional<BackendSettings> found =
            store.settingsForBackend("basic_pitch");
        QVERIFY(found.has_value());
        QCOMPARE(found->backendId, QString("basic_pitch"));
        QCOMPARE(found->executablePathOverride,
                 QString("C:/Tools/basic_pitch/adapter.exe"));
        QCOMPARE(found->workingDirectoryOverride,
                 QString("C:/Tools/basic_pitch"));
        QCOMPARE(found->modelCheckpointPathOverride,
                 QString("C:/Models/basic_pitch/model"));
        QCOMPARE(found->pythonExecutablePathOverride,
                 QString("C:/Python/python.exe"));
        QCOMPARE(found->environmentVariables.value("TONY_BACKEND_MODE"),
                 QString("test"));
        QVERIFY(found->enabled);
        QCOMPARE(found->displayLabelOverride, QString("Local Basic Pitch"));
        QVERIFY(found->debugSummaryString().contains("not_configured"));
        QVERIFY(found->debugSummaryString().contains("executable_path"));
    }

    void backendSettingsStoreRejectsInvalidBackendId()
    {
        BackendSettings settings;
        settings.backendId = "BasicPitch";
        settings.executablePathOverride = "C:/Tools/basic_pitch/adapter.exe";

        BackendSettingsStore store;

        QVERIFY(!settings.validate().isValid());
        QVERIFY(!store.setSettings(settings));
        QVERIFY(store.configuredBackendIds().isEmpty());
        QVERIFY(!store.settingsForBackend("BasicPitch").has_value());
        QVERIFY(!store.removeSettings("BasicPitch"));
    }

    void backendSettingsEmptyPathsAreAllowedButUnconfigured()
    {
        BackendSettings settings;
        settings.backendId = "basic_pitch";
        settings.executablePathOverride = " ";
        settings.workingDirectoryOverride = "";
        settings.modelCheckpointPathOverride = "  ";
        settings.pythonExecutablePathOverride = "";

        BackendSettingsStore store;

        QVERIFY(settings.validate().isValid());
        QVERIFY(!settings.hasAnyPathOverride());
        QVERIFY(!settings.isConfigured());
        QVERIFY(store.setSettings(settings));
        QVERIFY(store.settingsForBackend("basic_pitch").has_value());
    }

    void backendSettingsStoreRemovesSettings()
    {
        BackendSettings settings;
        settings.backendId = "basic_pitch";
        settings.enabled = true;

        BackendSettingsStore store;

        QVERIFY(store.setSettings(settings));
        QVERIFY(store.settingsForBackend("basic_pitch").has_value());
        QVERIFY(store.removeSettings("basic_pitch"));
        QVERIFY(!store.settingsForBackend("basic_pitch").has_value());
        QVERIFY(!store.removeSettings("basic_pitch"));
        QCOMPARE(store.size(), 0);
    }

    void backendSettingsStoreClearsSettings()
    {
        BackendSettings first;
        first.backendId = "basic_pitch";
        BackendSettings second;
        second.backendId = "crepe_notes";

        BackendSettingsStore store;

        QVERIFY(store.setSettings(first));
        QVERIFY(store.setSettings(second));
        QCOMPARE(store.size(), 2);

        store.clear();

        QCOMPARE(store.size(), 0);
        QVERIFY(store.configuredBackendIds().isEmpty());
        QVERIFY(!store.settingsForBackend("basic_pitch").has_value());
        QVERIFY(!store.settingsForBackend("crepe_notes").has_value());
    }

    void backendSettingsAloneNeverMarkBackendReadyOrInstalled()
    {
        BackendSettings settings;
        settings.backendId = "basic_pitch";
        settings.executablePathOverride = "C:/Tools/basic_pitch/adapter.exe";
        settings.enabled = true;

        BackendSettingsStore store;

        QVERIFY(store.setSettings(settings));

        const std::optional<BackendSettings> found =
            store.settingsForBackend("basic_pitch");
        QVERIFY(found.has_value());
        QVERIFY(found->statusFromSettings() == BackendStatus::NotConfigured);
        QVERIFY(found->statusFromSettings() != BackendStatus::Ready);

        BackendRegistry registry;
        QVERIFY(registry.allManifests().isEmpty());
        QVERIFY(!registry.hasBackend("basic_pitch"));
    }

    void backendSettingsSerializerSerializesDefaultSettings()
    {
        BackendSettings settings;
        BackendSettingsSerializer serializer;
        const QJsonObject object = serializer.toJson(settings);

        QCOMPARE(object.value("backend_id").toString(), QString());
        QCOMPARE(object.value("executable_path_override").toString(), QString());
        QCOMPARE(object.value("working_directory_override").toString(), QString());
        QCOMPARE(object.value("model_checkpoint_path_override").toString(), QString());
        QCOMPARE(object.value("python_executable_path_override").toString(), QString());
        QVERIFY(object.value("environment_variables").isObject());
        QVERIFY(object.value("environment_variables").toObject().isEmpty());
        QVERIFY(!object.value("enabled").toBool());
        QCOMPARE(object.value("display_label_override").toString(), QString());
        QCOMPARE(object.value("user_notes").toString(), QString());
        QVERIFY(!object.contains("status"));
    }

    void backendSettingsSerializerDeserializesValidSettings()
    {
        QJsonObject environment;
        environment.insert("TONY_BACKEND_MODE", "test");

        QJsonObject object;
        object.insert("backend_id", "basic_pitch");
        object.insert("executable_path_override",
                      "C:/Tools/basic_pitch/adapter.exe");
        object.insert("working_directory_override",
                      "C:/Tools/basic_pitch");
        object.insert("model_checkpoint_path_override",
                      "C:/Models/basic_pitch/model");
        object.insert("python_executable_path_override",
                      "C:/Python/python.exe");
        object.insert("environment_variables", environment);
        object.insert("enabled", true);
        object.insert("display_label_override", "Local Basic Pitch");
        object.insert("user_notes", "Configured by user.");

        BackendSettingsSerializer serializer;
        const BackendSettingsSerializationResult parsed =
            serializer.fromJson(object);

        QVERIFY(parsed.isValid());
        QCOMPARE(parsed.settings.backendId, QString("basic_pitch"));
        QCOMPARE(parsed.settings.executablePathOverride,
                 QString("C:/Tools/basic_pitch/adapter.exe"));
        QCOMPARE(parsed.settings.workingDirectoryOverride,
                 QString("C:/Tools/basic_pitch"));
        QCOMPARE(parsed.settings.modelCheckpointPathOverride,
                 QString("C:/Models/basic_pitch/model"));
        QCOMPARE(parsed.settings.pythonExecutablePathOverride,
                 QString("C:/Python/python.exe"));
        QCOMPARE(parsed.settings.environmentVariables.value("TONY_BACKEND_MODE"),
                 QString("test"));
        QVERIFY(parsed.settings.enabled);
        QCOMPARE(parsed.settings.displayLabelOverride,
                 QString("Local Basic Pitch"));
        QCOMPARE(parsed.settings.userNotes, QString("Configured by user."));
        QVERIFY(parsed.settings.statusFromSettings() == BackendStatus::NotConfigured);
    }

    void backendSettingsSerializerRejectsInvalidBackendId()
    {
        QJsonObject object;
        object.insert("backend_id", "BasicPitch");
        object.insert("enabled", true);

        BackendSettingsSerializer serializer;
        const BackendSettingsSerializationResult parsed =
            serializer.fromJson(object);

        QVERIFY(!parsed.isValid());
        QVERIFY(reportHasIssue(parsed.report, "invalid_backend_id"));
        QVERIFY(parsed.settings.statusFromSettings() == BackendStatus::NotConfigured);
    }

    void backendSettingsSerializerRoundTripsSettings()
    {
        BackendSettings settings;
        settings.backendId = "basic_pitch";
        settings.executablePathOverride = "C:/Tools/basic_pitch/adapter.exe";
        settings.workingDirectoryOverride = "C:/Tools/basic_pitch";
        settings.modelCheckpointPathOverride = "C:/Models/basic_pitch/model";
        settings.pythonExecutablePathOverride = "C:/Python/python.exe";
        settings.environmentVariables.insert("TONY_BACKEND_MODE", "test");
        settings.environmentVariables.insert("TONY_BACKEND_TRACE", "1");
        settings.enabled = true;
        settings.displayLabelOverride = "Local Basic Pitch";
        settings.userNotes = "Configured by user.";

        BackendSettingsSerializer serializer;
        const QJsonObject object = serializer.toJson(settings);
        const BackendSettingsSerializationResult parsed =
            serializer.fromJson(object);

        QVERIFY(parsed.isValid());
        QCOMPARE(parsed.settings.backendId, settings.backendId);
        QCOMPARE(parsed.settings.executablePathOverride,
                 settings.executablePathOverride);
        QCOMPARE(parsed.settings.workingDirectoryOverride,
                 settings.workingDirectoryOverride);
        QCOMPARE(parsed.settings.modelCheckpointPathOverride,
                 settings.modelCheckpointPathOverride);
        QCOMPARE(parsed.settings.pythonExecutablePathOverride,
                 settings.pythonExecutablePathOverride);
        QCOMPARE(parsed.settings.environmentVariables,
                 settings.environmentVariables);
        QCOMPARE(parsed.settings.enabled, settings.enabled);
        QCOMPARE(parsed.settings.displayLabelOverride,
                 settings.displayLabelOverride);
        QCOMPARE(parsed.settings.userNotes, settings.userNotes);
        QVERIFY(parsed.settings.statusFromSettings() == BackendStatus::NotConfigured);
    }

    void backendSettingsSerializerSerializesStoreWithMultipleBackends()
    {
        BackendSettings first;
        first.backendId = "basic_pitch";
        first.executablePathOverride = "C:/Tools/basic_pitch/adapter.exe";
        first.enabled = true;
        BackendSettings second;
        second.backendId = "crepe_notes";
        second.pythonExecutablePathOverride = "C:/Python/python.exe";

        BackendSettingsStore store;
        QVERIFY(store.setSettings(first));
        QVERIFY(store.setSettings(second));

        BackendSettingsSerializer serializer;
        const QJsonArray array = serializer.storeToJsonArray(store);
        const QJsonObject object = serializer.storeToJsonObject(store);
        const BackendSettingsStoreSerializationResult parsed =
            serializer.storeFromJsonObject(object);

        QCOMPARE(array.size(), 2);
        QVERIFY(object.value("backend_settings").isArray());
        QVERIFY(parsed.isValid());
        QCOMPARE(parsed.loadedCount, 2);
        QCOMPARE(parsed.rejectedCount, 0);
        QCOMPARE(parsed.store.size(), 2);
        QVERIFY(parsed.store.settingsForBackend("basic_pitch").has_value());
        QVERIFY(parsed.store.settingsForBackend("crepe_notes").has_value());
        QVERIFY(parsed.store.configuredBackendIds().contains("basic_pitch"));
        QVERIFY(parsed.store.configuredBackendIds().contains("crepe_notes"));
    }

    void backendSettingsSerializerLoadsMultipleEntriesSafely()
    {
        QJsonObject first;
        first.insert("backend_id", "basic_pitch");
        first.insert("enabled", true);
        QJsonObject second;
        second.insert("backend_id", "BasicPitch");

        QJsonArray array;
        array.push_back(first);
        array.push_back(second);
        array.push_back(QString("not an object"));

        BackendSettingsSerializer serializer;
        const BackendSettingsStoreSerializationResult parsed =
            serializer.storeFromJsonArray(array);

        QVERIFY(!parsed.isValid());
        QCOMPARE(parsed.loadedCount, 1);
        QCOMPARE(parsed.rejectedCount, 2);
        QCOMPARE(parsed.store.size(), 1);
        QVERIFY(parsed.store.settingsForBackend("basic_pitch").has_value());
        QVERIFY(reportHasIssue(parsed.report, "invalid_backend_id"));
        QVERIFY(reportHasIssue(parsed.report, "invalid_settings_entry"));
    }

    void backendSettingsSerializerNeverMarksBackendReadyOrInstalled()
    {
        QJsonObject object;
        object.insert("backend_id", "basic_pitch");
        object.insert("executable_path_override",
                      "C:/Tools/basic_pitch/adapter.exe");
        object.insert("enabled", true);
        object.insert("status", "ready");

        BackendSettingsSerializer serializer;
        const BackendSettingsSerializationResult parsed =
            serializer.fromJson(object);

        QVERIFY(parsed.isValid());
        QVERIFY(parsed.settings.statusFromSettings() == BackendStatus::NotConfigured);
        QVERIFY(parsed.settings.statusFromSettings() != BackendStatus::Ready);
        QVERIFY(!serializer.toJson(parsed.settings).contains("status"));
    }

    void backendSettingsFileStoreSavesValidSettingsFile()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath("backend_settings.json");

        BackendSettings settings;
        settings.backendId = "basic_pitch";
        settings.executablePathOverride = "C:/Tools/basic_pitch/adapter.exe";
        settings.enabled = true;

        BackendSettingsStore store;
        QVERIFY(store.setSettings(settings));

        BackendSettingsFileStore fileStore;
        const BackendSettingsFileSaveResult saved =
            fileStore.save(path, store);

        QVERIFY(saved.isValid());
        QCOMPARE(saved.path, path);

        QFile file(path);
        QVERIFY(file.open(QIODevice::ReadOnly));
        const QJsonDocument document = QJsonDocument::fromJson(file.readAll());

        QVERIFY(document.isObject());
        QVERIFY(document.object().value("backend_settings").isArray());
    }

    void backendSettingsFileStoreLoadsValidSettingsFile()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath("backend_settings.json");

        BackendSettings settings;
        settings.backendId = "basic_pitch";
        settings.executablePathOverride = "C:/Tools/basic_pitch/adapter.exe";
        settings.enabled = true;

        BackendSettingsStore store;
        QVERIFY(store.setSettings(settings));

        BackendSettingsSerializer serializer;
        const QJsonDocument document(serializer.storeToJsonObject(store));
        QVERIFY(writeFile(path, document.toJson()));

        BackendSettingsFileStore fileStore;
        const BackendSettingsFileLoadResult loaded = fileStore.load(path);

        QVERIFY(loaded.isValid());
        QCOMPARE(loaded.path, path);
        QCOMPARE(loaded.loadedCount, 1);
        QCOMPARE(loaded.rejectedCount, 0);
        QVERIFY(loaded.store.settingsForBackend("basic_pitch").has_value());

        const std::optional<BackendSettings> found =
            loaded.store.settingsForBackend("basic_pitch");
        QVERIFY(found.has_value());
        QCOMPARE(found->executablePathOverride,
                 QString("C:/Tools/basic_pitch/adapter.exe"));
        QVERIFY(found->enabled);
        QVERIFY(found->statusFromSettings() == BackendStatus::NotConfigured);
    }

    void backendSettingsFileStoreMissingFileFailsCleanly()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath("missing_settings.json");

        BackendSettingsFileStore fileStore;
        const BackendSettingsFileLoadResult loaded = fileStore.load(path);

        QVERIFY(!loaded.isValid());
        QCOMPARE(loaded.path, path);
        QCOMPARE(loaded.loadedCount, 0);
        QCOMPARE(loaded.rejectedCount, 0);
        QCOMPARE(loaded.store.size(), 0);
        QVERIFY(reportHasIssue(loaded.report, "file_open_failed"));
    }

    void backendSettingsFileStoreInvalidJsonFailsCleanly()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath("backend_settings.json");
        QVERIFY(writeFile(path, "{ invalid json"));

        BackendSettingsFileStore fileStore;
        const BackendSettingsFileLoadResult loaded = fileStore.load(path);

        QVERIFY(!loaded.isValid());
        QCOMPARE(loaded.loadedCount, 0);
        QCOMPARE(loaded.rejectedCount, 0);
        QVERIFY(reportHasIssue(loaded.report, "invalid_json"));
    }

    void backendSettingsFileStoreWrongTopLevelJsonFailsCleanly()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath("backend_settings.json");
        QVERIFY(writeFile(path, "[]"));

        BackendSettingsFileStore fileStore;
        const BackendSettingsFileLoadResult loaded = fileStore.load(path);

        QVERIFY(!loaded.isValid());
        QVERIFY(reportHasIssue(loaded.report, "invalid_top_level_json"));
    }

    void backendSettingsFileStoreRejectsInvalidBackendId()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath("backend_settings.json");
        QVERIFY(writeFile(path, R"json(
{
  "backend_settings": [
    {
      "backend_id": "BasicPitch",
      "enabled": true
    }
  ]
}
)json"));

        BackendSettingsFileStore fileStore;
        const BackendSettingsFileLoadResult loaded = fileStore.load(path);

        QVERIFY(!loaded.isValid());
        QCOMPARE(loaded.loadedCount, 0);
        QCOMPARE(loaded.rejectedCount, 1);
        QCOMPARE(loaded.store.size(), 0);
        QVERIFY(reportHasIssue(loaded.report, "invalid_backend_id"));
    }

    void backendSettingsFileStoreReportsWriteFailure()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendSettings settings;
        settings.backendId = "basic_pitch";

        BackendSettingsStore store;
        QVERIFY(store.setSettings(settings));

        BackendSettingsFileStore fileStore;
        const BackendSettingsFileSaveResult saved =
            fileStore.save(directory.path(), store);

        QVERIFY(!saved.isValid());
        QCOMPARE(saved.path, directory.path());
        QVERIFY(reportHasIssue(saved.report, "file_write_failed"));
    }

    void backendSettingsFileStoreRoundTripsStore()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath("backend_settings.json");

        BackendSettings first;
        first.backendId = "basic_pitch";
        first.executablePathOverride = "C:/Tools/basic_pitch/adapter.exe";
        first.enabled = true;
        BackendSettings second;
        second.backendId = "crepe_notes";
        second.pythonExecutablePathOverride = "C:/Python/python.exe";
        second.environmentVariables.insert("TONY_BACKEND_TRACE", "1");

        BackendSettingsStore store;
        QVERIFY(store.setSettings(first));
        QVERIFY(store.setSettings(second));

        BackendSettingsFileStore fileStore;
        const BackendSettingsFileSaveResult saved =
            fileStore.save(path, store);
        QVERIFY(saved.isValid());

        const BackendSettingsFileLoadResult loaded = fileStore.load(path);

        QVERIFY(loaded.isValid());
        QCOMPARE(loaded.loadedCount, 2);
        QCOMPARE(loaded.rejectedCount, 0);
        QCOMPARE(loaded.store.size(), 2);
        QVERIFY(loaded.store.settingsForBackend("basic_pitch").has_value());
        QVERIFY(loaded.store.settingsForBackend("crepe_notes").has_value());

        const std::optional<BackendSettings> found =
            loaded.store.settingsForBackend("crepe_notes");
        QVERIFY(found.has_value());
        QCOMPARE(found->pythonExecutablePathOverride,
                 QString("C:/Python/python.exe"));
        QCOMPARE(found->environmentVariables.value("TONY_BACKEND_TRACE"),
                 QString("1"));
        QVERIFY(found->statusFromSettings() == BackendStatus::NotConfigured);
    }

    void backendSettingsFileStoreLoadingNeverMarksBackendReadyOrInstalled()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath("backend_settings.json");
        QVERIFY(writeFile(path, R"json(
{
  "backend_settings": [
    {
      "backend_id": "basic_pitch",
      "executable_path_override": "C:/Tools/basic_pitch/adapter.exe",
      "enabled": true,
      "status": "ready"
    }
  ]
}
)json"));

        BackendSettingsFileStore fileStore;
        const BackendSettingsFileLoadResult loaded = fileStore.load(path);

        QVERIFY(loaded.isValid());
        QCOMPARE(loaded.loadedCount, 1);
        QCOMPARE(loaded.rejectedCount, 0);

        const std::optional<BackendSettings> found =
            loaded.store.settingsForBackend("basic_pitch");
        QVERIFY(found.has_value());
        QVERIFY(found->statusFromSettings() == BackendStatus::NotConfigured);
        QVERIFY(found->statusFromSettings() != BackendStatus::Ready);

        BackendRegistry registry;
        QVERIFY(registry.allManifests().isEmpty());
        QVERIFY(!registry.hasBackend("basic_pitch"));
    }

    void backendSettingsPersistenceConfigDefaultsAreSafe()
    {
        const BackendSettingsPersistenceConfig config =
            BackendSettingsPersistenceConfig::safeDefaults();

        QCOMPARE(config.defaultSettingsFilePath,
                 QString("settings/backend_settings.json"));
        QVERIFY(config.userConfiguredSettingsFilePath.isEmpty());
        QVERIFY(config.testOnlySettingsFilePath.isEmpty());
        QVERIFY(config.hasAnyUsableSettingsFilePath());

        const QVector<BackendSettingsPersistencePath> candidates =
            config.settingsFilePathCandidates();
        QCOMPARE(candidates.size(), 1);
        QVERIFY(candidates.front().source ==
                BackendSettingsPersistencePathSource::DefaultLocal);
        QCOMPARE(candidates.front().path,
                 QString("settings/backend_settings.json"));
        QCOMPARE(candidates.front().sourceName(), QString("default_local"));
        QVERIFY(candidates.front().isUsable());

        const BackendSettingsPersistencePath preferred =
            config.preferredSettingsFilePath();
        QVERIFY(preferred.source ==
                BackendSettingsPersistencePathSource::DefaultLocal);
        QCOMPARE(preferred.path, QString("settings/backend_settings.json"));
        QVERIFY(config.validate().isValid());
    }

    void backendSettingsPersistenceConfigSupportsCustomSettingsFilePath()
    {
        BackendSettingsPersistenceConfig config =
            BackendSettingsPersistenceConfig::safeDefaults();
        config.userConfiguredSettingsFilePath =
            "C:/Users/Test/AppData/Local/Tony/backend_settings.json";

        const QVector<BackendSettingsPersistencePath> candidates =
            config.settingsFilePathCandidates();

        QCOMPARE(candidates.size(), 2);
        QVERIFY(candidates[0].source ==
                BackendSettingsPersistencePathSource::DefaultLocal);
        QVERIFY(candidates[1].source ==
                BackendSettingsPersistencePathSource::UserConfigured);
        QCOMPARE(candidates[1].path,
                 QString("C:/Users/Test/AppData/Local/Tony/backend_settings.json"));
        QCOMPARE(candidates[1].sourceName(), QString("user_configured"));

        const BackendSettingsPersistencePath preferred =
            config.preferredSettingsFilePath();
        QVERIFY(preferred.source ==
                BackendSettingsPersistencePathSource::UserConfigured);
        QCOMPARE(preferred.path,
                 QString("C:/Users/Test/AppData/Local/Tony/backend_settings.json"));
        QVERIFY(config.validate().isValid());
    }

    void backendSettingsPersistenceConfigSupportsExplicitTestOnlyPath()
    {
        BackendSettingsPersistenceConfig config;
        config.testOnlySettingsFilePath = "C:/test/backend_settings.json";

        const QVector<BackendSettingsPersistencePath> candidates =
            config.settingsFilePathCandidates();

        QCOMPARE(candidates.size(), 1);
        QVERIFY(candidates.front().source ==
                BackendSettingsPersistencePathSource::TestOnly);
        QCOMPARE(candidates.front().path,
                 QString("C:/test/backend_settings.json"));
        QCOMPARE(candidates.front().sourceName(), QString("test_only"));

        const BackendSettingsPersistencePath preferred =
            config.preferredSettingsFilePath();
        QVERIFY(preferred.source ==
                BackendSettingsPersistencePathSource::TestOnly);
        QCOMPARE(preferred.path, QString("C:/test/backend_settings.json"));
        QVERIFY(config.validate().isValid());
    }

    void backendSettingsPersistenceConfigHandlesEmptyPathsWithoutCrashing()
    {
        BackendSettingsPersistenceConfig config;
        config.defaultSettingsFilePath = " ";
        config.userConfiguredSettingsFilePath = "  ";
        config.testOnlySettingsFilePath = "";

        const QVector<BackendSettingsPersistencePath> candidates =
            config.settingsFilePathCandidates();
        const BackendSettingsPersistencePath preferred =
            config.preferredSettingsFilePath();
        const ValidationReport report = config.validate();

        QVERIFY(candidates.isEmpty());
        QVERIFY(!preferred.isUsable());
        QVERIFY(!config.hasAnyUsableSettingsFilePath());
        QVERIFY(report.isValid());
        QVERIFY(reportHasIssue(report, "empty_default_settings_file_path"));
        QVERIFY(reportHasIssue(report, "empty_user_settings_file_path"));
        QVERIFY(reportHasIssue(report, "no_settings_file_paths"));
    }

    void backendSettingsPersistenceConfigAloneNeverMarksBackendReadyOrInstalled()
    {
        BackendSettingsPersistenceConfig config =
            BackendSettingsPersistenceConfig::safeDefaults();
        config.userConfiguredSettingsFilePath =
            "C:/Users/Test/AppData/Local/Tony/backend_settings.json";

        QVERIFY(config.validate().isValid());
        QVERIFY(config.hasAnyUsableSettingsFilePath());

        BackendSettings settings;
        settings.backendId = "basic_pitch";
        QVERIFY(settings.statusFromSettings() == BackendStatus::NotConfigured);
        QVERIFY(settings.statusFromSettings() != BackendStatus::Ready);

        BackendRegistry registry;
        QVERIFY(registry.allManifests().isEmpty());
        QVERIFY(!registry.hasBackend("basic_pitch"));
    }

    void backendSettingsPathResolverDefaultPathIsNonCrashing()
    {
        BackendSettingsPathResolver resolver;
        const BackendSettingsPathResolutionResult resolved =
            resolver.resolveDefaultPath();

        if (resolved.isValid()) {
            QVERIFY(!resolved.path.isEmpty());
            QVERIFY(!resolved.baseDirectory.isEmpty());
            QCOMPARE(resolved.fileName, QString("backend_settings.json"));
            QVERIFY(resolved.path.endsWith("backend_settings.json"));
        } else {
            QVERIFY(reportHasIssue(resolved.report,
                                   "empty_platform_settings_directory"));
        }
        QVERIFY(!resolved.usedTestOverride);
    }

    void backendSettingsPathResolverSupportsCustomFileName()
    {
        BackendSettingsPathResolver resolver;
        const BackendSettingsPathResolutionResult resolved =
            resolver.resolveDefaultPathFromBaseDirectory(
                "C:/Users/Test/AppData/Local/Tony",
                "backend_settings_test.json");

        QVERIFY(resolved.isValid());
        QCOMPARE(resolved.baseDirectory,
                 QString("C:/Users/Test/AppData/Local/Tony"));
        QCOMPARE(resolved.fileName, QString("backend_settings_test.json"));
        QVERIFY(resolved.path.endsWith("backend_settings_test.json"));
        QVERIFY(!resolved.usedTestOverride);
    }

    void backendSettingsPathResolverRejectsEmptyOrInvalidFileName()
    {
        BackendSettingsPathResolver resolver;

        const BackendSettingsPathResolutionResult empty =
            resolver.resolveDefaultPathFromBaseDirectory(
                "C:/Users/Test/AppData/Local/Tony",
                " ");
        const BackendSettingsPathResolutionResult pathLike =
            resolver.resolveDefaultPathFromBaseDirectory(
                "C:/Users/Test/AppData/Local/Tony",
                "settings/backend_settings.json");
        const BackendSettingsPathResolutionResult parent =
            resolver.resolveDefaultPathFromBaseDirectory(
                "C:/Users/Test/AppData/Local/Tony",
                "..");

        QVERIFY(!empty.isValid());
        QVERIFY(!pathLike.isValid());
        QVERIFY(!parent.isValid());
        QVERIFY(reportHasIssue(empty.report, "empty_settings_file_name"));
        QVERIFY(reportHasIssue(pathLike.report,
                               "invalid_settings_file_name"));
        QVERIFY(reportHasIssue(parent.report,
                               "invalid_settings_file_name"));
        QVERIFY(empty.path.isEmpty());
        QVERIFY(pathLike.path.isEmpty());
        QVERIFY(parent.path.isEmpty());
    }

    void backendSettingsPathResolverHandlesEmptyPlatformPathCleanly()
    {
        BackendSettingsPathResolver resolver;
        const BackendSettingsPathResolutionResult resolved =
            resolver.resolveDefaultPathFromBaseDirectory(
                " ",
                "backend_settings.json");

        QVERIFY(!resolved.isValid());
        QVERIFY(resolved.path.isEmpty());
        QVERIFY(reportHasIssue(resolved.report,
                               "empty_platform_settings_directory"));
    }

    void backendSettingsPathResolverSupportsTestOverridePath()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath("backend_settings.json");

        BackendSettingsPathResolver resolver;
        const BackendSettingsPathResolutionResult resolved =
            resolver.resolveTestOverridePath(path);

        QVERIFY(resolved.isValid());
        QCOMPARE(resolved.path, path);
        QVERIFY(resolved.usedTestOverride);
        QVERIFY(resolved.baseDirectory.isEmpty());
        QVERIFY(resolved.fileName.isEmpty());
    }

    void backendSettingsPathResolverRejectsEmptyTestOverridePath()
    {
        BackendSettingsPathResolver resolver;
        const BackendSettingsPathResolutionResult resolved =
            resolver.resolveTestOverridePath(" ");

        QVERIFY(!resolved.isValid());
        QVERIFY(resolved.path.isEmpty());
        QVERIFY(resolved.usedTestOverride);
        QVERIFY(reportHasIssue(resolved.report, "empty_test_settings_path"));
    }

    void backendSettingsPathResolverAloneNeverMarksBackendReadyOrInstalled()
    {
        BackendSettingsPathResolver resolver;
        const BackendSettingsPathResolutionResult resolved =
            resolver.resolveDefaultPathFromBaseDirectory(
                "C:/Users/Test/AppData/Local/Tony",
                "backend_settings.json");

        QVERIFY(resolved.isValid());

        BackendSettings settings;
        settings.backendId = "basic_pitch";
        QVERIFY(settings.statusFromSettings() == BackendStatus::NotConfigured);
        QVERIFY(settings.statusFromSettings() != BackendStatus::Ready);

        BackendRegistry registry;
        QVERIFY(registry.allManifests().isEmpty());
        QVERIFY(!registry.hasBackend("basic_pitch"));
    }

    void backendSettingsPersistenceConfigAppliesResolvedDefaultPath()
    {
        BackendSettingsPathResolver resolver;
        const BackendSettingsPathResolutionResult resolved =
            resolver.resolveDefaultPathFromBaseDirectory(
                "C:/Users/Test/AppData/Local/Tony",
                "backend_settings.json");

        BackendSettingsPersistenceConfig config;
        const ValidationReport report =
            config.applyResolvedDefaultPath(resolved);

        QVERIFY(report.isValid());
        QCOMPARE(config.defaultSettingsFilePath,
                 QString("C:/Users/Test/AppData/Local/Tony/backend_settings.json"));
        QVERIFY(!config.hasExplicitSettingsFilePath());

        const BackendSettingsPersistencePath preferred =
            config.preferredSettingsFilePath();
        QVERIFY(preferred.source ==
                BackendSettingsPersistencePathSource::DefaultLocal);
        QCOMPARE(preferred.path,
                 QString("C:/Users/Test/AppData/Local/Tony/backend_settings.json"));
    }

    void backendSettingsPersistenceConfigKeepsUserPathPriority()
    {
        BackendSettingsPathResolver resolver;
        const BackendSettingsPathResolutionResult resolved =
            resolver.resolveDefaultPathFromBaseDirectory(
                "C:/Users/Test/AppData/Local/Tony",
                "backend_settings.json");

        BackendSettingsPersistenceConfig config;
        config.defaultSettingsFilePath = "C:/old/default/backend_settings.json";
        config.userConfiguredSettingsFilePath =
            "D:/Tony/backend_settings_user.json";

        const ValidationReport report =
            config.applyResolvedDefaultPath(resolved);

        QVERIFY(report.isValid());
        QVERIFY(config.hasExplicitSettingsFilePath());
        QCOMPARE(config.defaultSettingsFilePath,
                 QString("C:/old/default/backend_settings.json"));

        const BackendSettingsPersistencePath preferred =
            config.preferredSettingsFilePath();
        QVERIFY(preferred.source ==
                BackendSettingsPersistencePathSource::UserConfigured);
        QCOMPARE(preferred.path, QString("D:/Tony/backend_settings_user.json"));
    }

    void backendSettingsPersistenceConfigKeepsTestPathPriority()
    {
        BackendSettingsPathResolver resolver;
        const BackendSettingsPathResolutionResult resolved =
            resolver.resolveDefaultPathFromBaseDirectory(
                "C:/Users/Test/AppData/Local/Tony",
                "backend_settings.json");

        BackendSettingsPersistenceConfig config;
        config.defaultSettingsFilePath = "C:/old/default/backend_settings.json";
        config.userConfiguredSettingsFilePath =
            "D:/Tony/backend_settings_user.json";
        config.testOnlySettingsFilePath = "E:/tests/backend_settings.json";

        const ValidationReport report =
            config.applyResolvedDefaultPath(resolved);

        QVERIFY(report.isValid());
        QVERIFY(config.hasExplicitSettingsFilePath());
        QCOMPARE(config.defaultSettingsFilePath,
                 QString("C:/old/default/backend_settings.json"));

        const BackendSettingsPersistencePath preferred =
            config.preferredSettingsFilePath();
        QVERIFY(preferred.source ==
                BackendSettingsPersistencePathSource::TestOnly);
        QCOMPARE(preferred.path, QString("E:/tests/backend_settings.json"));
    }

    void backendSettingsPersistenceConfigReportsInvalidResolvedDefaultPath()
    {
        BackendSettingsPathResolver resolver;
        const BackendSettingsPathResolutionResult resolved =
            resolver.resolveDefaultPathFromBaseDirectory(
                " ",
                "backend_settings.json");

        BackendSettingsPersistenceConfig config;
        config.defaultSettingsFilePath = "C:/old/default/backend_settings.json";

        const ValidationReport report =
            config.applyResolvedDefaultPath(resolved);

        QVERIFY(!report.isValid());
        QVERIFY(reportHasIssue(report, "empty_platform_settings_directory"));
        QCOMPARE(config.defaultSettingsFilePath,
                 QString("C:/old/default/backend_settings.json"));
    }

    void backendSettingsPersistenceConfigResolverConnectionNeverMarksReady()
    {
        BackendSettingsPathResolver resolver;
        const BackendSettingsPathResolutionResult resolved =
            resolver.resolveDefaultPathFromBaseDirectory(
                "C:/Users/Test/AppData/Local/Tony",
                "backend_settings.json");

        BackendSettingsPersistenceConfig config;
        QVERIFY(config.applyResolvedDefaultPath(resolved).isValid());

        BackendSettings settings;
        settings.backendId = "basic_pitch";
        QVERIFY(settings.statusFromSettings() == BackendStatus::NotConfigured);
        QVERIFY(settings.statusFromSettings() != BackendStatus::Ready);

        BackendRegistry registry;
        QVERIFY(registry.allManifests().isEmpty());
        QVERIFY(!registry.hasBackend("basic_pitch"));
    }

    void backendSettingsFactoryCreatesDefaultConfigAndService()
    {
        BackendSettingsFactory factory;
        const BackendSettingsPersistenceComponents components =
            factory.createDefaultPersistenceComponents();

        QVERIFY(components.isValid());
        QVERIFY(components.config.hasAnyUsableSettingsFilePath());

        const BackendSettingsPersistencePath preferred =
            components.config.preferredSettingsFilePath();
        QVERIFY(preferred.isUsable());
        QVERIFY(preferred.source ==
                BackendSettingsPersistencePathSource::DefaultLocal);
        QVERIFY(preferred.path.endsWith(
                    BackendSettingsPathResolver::defaultFileName()));
    }

    void backendSettingsFactoryCreatesServiceFromConfig()
    {
        BackendSettingsPersistenceConfig config;
        config.userConfiguredSettingsFilePath =
            "D:/Tony/backend_settings_user.json";

        BackendSettingsFactory factory;
        const BackendSettingsPersistenceComponents components =
            factory.createPersistenceComponents(config);

        QVERIFY(components.isValid());

        const BackendSettingsPersistencePath preferred =
            components.config.preferredSettingsFilePath();
        QVERIFY(preferred.source ==
                BackendSettingsPersistencePathSource::UserConfigured);
        QCOMPARE(preferred.path,
                 QString("D:/Tony/backend_settings_user.json"));
    }

    void backendSettingsFactoryPreservesTestOverridePath()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath("backend_settings.json");

        BackendSettingsFactory factory;
        const BackendSettingsPersistenceComponents components =
            factory.createTestPersistenceComponents(path);

        QVERIFY(components.isValid());
        QCOMPARE(components.config.testOnlySettingsFilePath, path);

        const BackendSettingsPersistencePath preferred =
            components.config.preferredSettingsFilePath();
        QVERIFY(preferred.source ==
                BackendSettingsPersistencePathSource::TestOnly);
        QCOMPARE(preferred.path, path);
    }

    void backendSettingsFactoryReportsInvalidResolvedPathCleanly()
    {
        BackendSettingsPathResolver resolver;
        const BackendSettingsPathResolutionResult resolved =
            resolver.resolveDefaultPathFromBaseDirectory(
                " ",
                BackendSettingsPathResolver::defaultFileName());

        BackendSettingsFactory factory;
        const BackendSettingsPersistenceComponents components =
            factory.createPersistenceComponentsFromResolvedPath(resolved);

        QVERIFY(!components.isValid());
        QVERIFY(!components.config.hasAnyUsableSettingsFilePath());
        QVERIFY(reportHasIssue(components.report,
                               "empty_platform_settings_directory"));
        QVERIFY(reportHasIssue(components.report, "no_settings_file_path"));
    }

    void backendSettingsFactoryAloneNeverMarksBackendReadyOrInstalled()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendSettingsFactory factory;
        const BackendSettingsPersistenceComponents components =
            factory.createTestPersistenceComponents(
                directory.filePath("backend_settings.json"));
        QVERIFY(components.isValid());

        BackendSettings settings;
        settings.backendId = "basic_pitch";
        settings.enabled = true;
        QVERIFY(settings.statusFromSettings() == BackendStatus::NotConfigured);
        QVERIFY(settings.statusFromSettings() != BackendStatus::Ready);

        BackendRegistry registry;
        QVERIFY(registry.allManifests().isEmpty());
        QVERIFY(!registry.hasBackend("basic_pitch"));
    }

    void backendSettingsFactoryDoesNotLoadOrSaveAutomatically()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath("backend_settings.json");

        BackendSettingsFactory factory;
        const BackendSettingsPersistenceComponents components =
            factory.createTestPersistenceComponents(path);

        QVERIFY(components.isValid());
        QCOMPARE(components.config.preferredSettingsFilePath().path, path);
        QVERIFY(!QFile::exists(path));
    }

    void backendSettingsDirectoryPreparerEmptyPathFailsCleanly()
    {
        BackendSettingsDirectoryPreparer preparer;

        const BackendSettingsDirectoryPreparationResult inspected =
            preparer.inspectParentDirectory(" ");
        const BackendSettingsDirectoryPreparationResult prepared =
            preparer.prepareParentDirectory(" ");

        QVERIFY(!inspected.isValid());
        QVERIFY(!prepared.isValid());
        QVERIFY(reportHasIssue(inspected.report, "empty_settings_file_path"));
        QVERIFY(reportHasIssue(prepared.report, "empty_settings_file_path"));
    }

    void backendSettingsDirectoryPreparerRejectsPathWithoutParent()
    {
        BackendSettingsDirectoryPreparer preparer;
        const BackendSettingsDirectoryPreparationResult prepared =
            preparer.prepareParentDirectory("backend_settings.json");

        QVERIFY(!prepared.isValid());
        QVERIFY(prepared.parentDirectoryPath.isEmpty());
        QVERIFY(!prepared.directoryCreated);
        QVERIFY(reportHasIssue(prepared.report, "missing_parent_directory"));
    }

    void backendSettingsDirectoryPreparerExistingDirectorySucceeds()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath("backend_settings.json");

        BackendSettingsDirectoryPreparer preparer;
        const BackendSettingsDirectoryPreparationResult prepared =
            preparer.prepareParentDirectory(path);

        QVERIFY(prepared.isValid());
        QCOMPARE(prepared.settingsFilePath, path);
        QCOMPARE(prepared.parentDirectoryPath, QDir::cleanPath(directory.path()));
        QVERIFY(prepared.directoryAlreadyExisted);
        QVERIFY(!prepared.directoryCreated);
    }

    void backendSettingsDirectoryPreparerCreatesOnlyWhenExplicitlyRequested()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString parent =
            QDir(directory.path()).filePath("settings/nested");
        const QString path =
            QDir(parent).filePath("backend_settings.json");

        BackendSettingsDirectoryPreparer preparer;
        const BackendSettingsDirectoryPreparationResult inspected =
            preparer.inspectParentDirectory(path);

        QVERIFY(inspected.isValid());
        QCOMPARE(inspected.parentDirectoryPath, QDir::cleanPath(parent));
        QVERIFY(!inspected.directoryAlreadyExisted);
        QVERIFY(!inspected.directoryCreated);
        QVERIFY(!QDir(parent).exists());

        const BackendSettingsDirectoryPreparationResult prepared =
            preparer.prepareParentDirectory(path);

        QVERIFY(prepared.isValid());
        QCOMPARE(prepared.parentDirectoryPath, QDir::cleanPath(parent));
        QVERIFY(!prepared.directoryAlreadyExisted);
        QVERIFY(prepared.directoryCreated);
        QVERIFY(QDir(parent).exists());
    }

    void backendSettingsDirectoryPreparerInvalidParentFailsCleanly()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString blockingFilePath =
            directory.filePath("not_a_directory");
        QVERIFY(writeFile(blockingFilePath, "not a directory"));

        const QString path =
            QDir(blockingFilePath).filePath("backend_settings.json");

        BackendSettingsDirectoryPreparer preparer;
        const BackendSettingsDirectoryPreparationResult prepared =
            preparer.prepareParentDirectory(path);

        QVERIFY(!prepared.isValid());
        QVERIFY(!prepared.directoryCreated);
        QVERIFY(reportHasIssue(prepared.report,
                               "parent_path_not_directory"));
    }

    void backendSettingsDirectoryPreparerNeverMarksBackendReadyOrInstalled()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString parent =
            QDir(directory.path()).filePath("settings");
        const QString path =
            QDir(parent).filePath("backend_settings.json");

        BackendSettingsDirectoryPreparer preparer;
        const BackendSettingsDirectoryPreparationResult prepared =
            preparer.prepareParentDirectory(path);
        QVERIFY(prepared.isValid());

        BackendSettings settings;
        settings.backendId = "basic_pitch";
        settings.enabled = true;
        QVERIFY(settings.statusFromSettings() == BackendStatus::NotConfigured);
        QVERIFY(settings.statusFromSettings() != BackendStatus::Ready);

        BackendRegistry registry;
        QVERIFY(registry.allManifests().isEmpty());
        QVERIFY(!registry.hasBackend("basic_pitch"));
    }

    void backendSettingsPersistenceServiceLoadsFromTestPath()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath("backend_settings.json");

        BackendSettings settings;
        settings.backendId = "basic_pitch";
        settings.executablePathOverride = "C:/Tools/basic_pitch/adapter.exe";
        settings.enabled = true;

        BackendSettingsStore store;
        QVERIFY(store.setSettings(settings));

        BackendSettingsFileStore fileStore;
        QVERIFY(fileStore.save(path, store).isValid());

        BackendSettingsPersistenceConfig config;
        config.testOnlySettingsFilePath = path;

        BackendSettingsPersistenceService service;
        const BackendSettingsPersistenceLoadResult loaded =
            service.load(config);

        QVERIFY(loaded.isValid());
        QCOMPARE(loaded.path, path);
        QVERIFY(loaded.source ==
                BackendSettingsPersistencePathSource::TestOnly);
        QCOMPARE(loaded.loadedCount, 1);
        QCOMPARE(loaded.rejectedCount, 0);
        QVERIFY(loaded.store.settingsForBackend("basic_pitch").has_value());

        const std::optional<BackendSettings> found =
            loaded.store.settingsForBackend("basic_pitch");
        QVERIFY(found.has_value());
        QCOMPARE(found->executablePathOverride,
                 QString("C:/Tools/basic_pitch/adapter.exe"));
        QVERIFY(found->statusFromSettings() == BackendStatus::NotConfigured);
    }

    void backendSettingsPersistenceServiceSavesToTestPath()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath("backend_settings.json");

        BackendSettings settings;
        settings.backendId = "basic_pitch";
        settings.pythonExecutablePathOverride = "C:/Python/python.exe";
        settings.enabled = true;

        BackendSettingsStore store;
        QVERIFY(store.setSettings(settings));

        BackendSettingsPersistenceConfig config;
        config.testOnlySettingsFilePath = path;

        BackendSettingsPersistenceService service;
        const BackendSettingsPersistenceSaveResult saved =
            service.save(config, store);

        QVERIFY(saved.isValid());
        QCOMPARE(saved.path, path);
        QVERIFY(saved.source ==
                BackendSettingsPersistencePathSource::TestOnly);

        BackendSettingsFileStore fileStore;
        const BackendSettingsFileLoadResult loaded = fileStore.load(path);
        QVERIFY(loaded.isValid());
        QVERIFY(loaded.store.settingsForBackend("basic_pitch").has_value());
    }

    void backendSettingsPersistenceServiceNormalSaveDoesNotCreateDirectory()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString parent =
            QDir(directory.path()).filePath("settings/missing");
        const QString path =
            QDir(parent).filePath("backend_settings.json");

        BackendSettings settings;
        settings.backendId = "basic_pitch";

        BackendSettingsStore store;
        QVERIFY(store.setSettings(settings));

        BackendSettingsPersistenceConfig config;
        config.testOnlySettingsFilePath = path;

        BackendSettingsPersistenceService service;
        const BackendSettingsPersistenceSaveResult saved =
            service.save(config, store);

        QVERIFY(!saved.isValid());
        QCOMPARE(saved.path, path);
        QVERIFY(!saved.parentDirectoryPrepared);
        QVERIFY(!saved.parentDirectoryCreated);
        QVERIFY(!QDir(parent).exists());
        QVERIFY(reportHasIssue(saved.report, "file_write_failed"));
    }

    void backendSettingsPersistenceServiceExplicitPrepareAndSaveCreatesDirectory()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString parent =
            QDir(directory.path()).filePath("settings/missing");
        const QString path =
            QDir(parent).filePath("backend_settings.json");

        BackendSettings settings;
        settings.backendId = "basic_pitch";
        settings.enabled = true;

        BackendSettingsStore store;
        QVERIFY(store.setSettings(settings));

        BackendSettingsPersistenceConfig config;
        config.testOnlySettingsFilePath = path;

        BackendSettingsPersistenceService service;
        const BackendSettingsPersistenceSaveResult saved =
            service.saveWithPreparedDirectory(config, store);

        QVERIFY(saved.isValid());
        QCOMPARE(saved.path, path);
        QCOMPARE(saved.preparedParentDirectoryPath, QDir::cleanPath(parent));
        QVERIFY(saved.parentDirectoryPrepared);
        QVERIFY(saved.parentDirectoryCreated);
        QVERIFY(QDir(parent).exists());
        QVERIFY(QFile::exists(path));
    }

    void backendSettingsPersistenceServiceExplicitPrepareFailsForInvalidPath()
    {
        BackendSettings settings;
        settings.backendId = "basic_pitch";

        BackendSettingsStore store;
        QVERIFY(store.setSettings(settings));

        BackendSettingsPersistenceConfig config;
        config.testOnlySettingsFilePath = "backend_settings.json";

        BackendSettingsPersistenceService service;
        const BackendSettingsPersistenceSaveResult saved =
            service.saveWithPreparedDirectory(config, store);

        QVERIFY(!saved.isValid());
        QCOMPARE(saved.path, QString("backend_settings.json"));
        QVERIFY(!saved.parentDirectoryPrepared);
        QVERIFY(!saved.parentDirectoryCreated);
        QVERIFY(reportHasIssue(saved.report, "missing_parent_directory"));
    }

    void backendSettingsPersistenceServiceExplicitPrepareExistingDirectorySaves()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath("backend_settings.json");

        BackendSettings settings;
        settings.backendId = "crepe_notes";
        settings.modelCheckpointPathOverride = "C:/Models/crepe/model.bin";

        BackendSettingsStore store;
        QVERIFY(store.setSettings(settings));

        BackendSettingsPersistenceConfig config;
        config.testOnlySettingsFilePath = path;

        BackendSettingsPersistenceService service;
        const BackendSettingsPersistenceSaveResult saved =
            service.saveWithPreparedDirectory(config, store);

        QVERIFY(saved.isValid());
        QCOMPARE(saved.path, path);
        QCOMPARE(saved.preparedParentDirectoryPath,
                 QDir::cleanPath(directory.path()));
        QVERIFY(saved.parentDirectoryPrepared);
        QVERIFY(!saved.parentDirectoryCreated);
        QVERIFY(QFile::exists(path));

        BackendSettingsFileStore fileStore;
        const BackendSettingsFileLoadResult loaded = fileStore.load(path);
        QVERIFY(loaded.isValid());
        QVERIFY(loaded.store.settingsForBackend("crepe_notes").has_value());
    }

    void backendSettingsPersistenceServiceMissingPathFailsCleanly()
    {
        BackendSettingsPersistenceConfig config;
        config.defaultSettingsFilePath = " ";
        config.userConfiguredSettingsFilePath = "  ";

        BackendSettingsPersistenceService service;
        const BackendSettingsPersistenceLoadResult loaded =
            service.load(config);
        const BackendSettingsPersistenceSaveResult saved =
            service.save(config, BackendSettingsStore());

        QVERIFY(!loaded.isValid());
        QVERIFY(!saved.isValid());
        QVERIFY(loaded.path.isEmpty());
        QVERIFY(saved.path.isEmpty());
        QVERIFY(reportHasIssue(loaded.report, "no_settings_file_path"));
        QVERIFY(reportHasIssue(saved.report, "no_settings_file_path"));
    }

    void backendSettingsPersistenceServiceMissingFileFailsCleanly()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath("missing_settings.json");

        BackendSettingsPersistenceConfig config;
        config.testOnlySettingsFilePath = path;

        BackendSettingsPersistenceService service;
        const BackendSettingsPersistenceLoadResult loaded =
            service.load(config);

        QVERIFY(!loaded.isValid());
        QCOMPARE(loaded.path, path);
        QVERIFY(loaded.source ==
                BackendSettingsPersistencePathSource::TestOnly);
        QCOMPARE(loaded.loadedCount, 0);
        QCOMPARE(loaded.rejectedCount, 0);
        QCOMPARE(loaded.store.size(), 0);
        QVERIFY(reportHasIssue(loaded.report, "file_open_failed"));
    }

    void backendSettingsPersistenceServiceReportsInvalidJson()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath("backend_settings.json");
        QVERIFY(writeFile(path, "{ invalid json"));

        BackendSettingsPersistenceConfig config;
        config.testOnlySettingsFilePath = path;

        BackendSettingsPersistenceService service;
        const BackendSettingsPersistenceLoadResult loaded =
            service.load(config);

        QVERIFY(!loaded.isValid());
        QCOMPARE(loaded.path, path);
        QVERIFY(reportHasIssue(loaded.report, "invalid_json"));
    }

    void backendSettingsPersistenceServiceReportsWriteFailure()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendSettings settings;
        settings.backendId = "basic_pitch";

        BackendSettingsStore store;
        QVERIFY(store.setSettings(settings));

        BackendSettingsPersistenceConfig config;
        config.testOnlySettingsFilePath = directory.path();

        BackendSettingsPersistenceService service;
        const BackendSettingsPersistenceSaveResult saved =
            service.save(config, store);

        QVERIFY(!saved.isValid());
        QCOMPARE(saved.path, directory.path());
        QVERIFY(reportHasIssue(saved.report, "file_write_failed"));
    }

    void backendSettingsPersistenceServiceRoundTripsSettings()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath("backend_settings.json");

        BackendSettings first;
        first.backendId = "basic_pitch";
        first.executablePathOverride = "C:/Tools/basic_pitch/adapter.exe";
        first.enabled = true;

        BackendSettings second;
        second.backendId = "crepe_notes";
        second.modelCheckpointPathOverride = "C:/Models/crepe/model.bin";
        second.environmentVariables.insert("TONY_BACKEND_TRACE", "1");

        BackendSettingsStore store;
        QVERIFY(store.setSettings(first));
        QVERIFY(store.setSettings(second));

        BackendSettingsPersistenceConfig config;
        config.testOnlySettingsFilePath = path;

        BackendSettingsPersistenceService service;
        const BackendSettingsPersistenceSaveResult saved =
            service.save(config, store);
        QVERIFY(saved.isValid());

        const BackendSettingsPersistenceLoadResult loaded =
            service.load(config);

        QVERIFY(loaded.isValid());
        QCOMPARE(loaded.loadedCount, 2);
        QCOMPARE(loaded.rejectedCount, 0);
        QCOMPARE(loaded.store.size(), 2);
        QVERIFY(loaded.store.settingsForBackend("basic_pitch").has_value());
        QVERIFY(loaded.store.settingsForBackend("crepe_notes").has_value());

        const std::optional<BackendSettings> found =
            loaded.store.settingsForBackend("crepe_notes");
        QVERIFY(found.has_value());
        QCOMPARE(found->modelCheckpointPathOverride,
                 QString("C:/Models/crepe/model.bin"));
        QCOMPARE(found->environmentVariables.value("TONY_BACKEND_TRACE"),
                 QString("1"));
        QVERIFY(found->statusFromSettings() == BackendStatus::NotConfigured);
    }

    void backendSettingsPersistenceServiceNeverMarksBackendReadyOrInstalled()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath("backend_settings.json");
        QVERIFY(writeFile(path, R"json(
{
  "backend_settings": [
    {
      "backend_id": "basic_pitch",
      "executable_path_override": "C:/Tools/basic_pitch/adapter.exe",
      "enabled": true,
      "status": "ready"
    }
  ]
}
)json"));

        BackendSettingsPersistenceConfig config;
        config.testOnlySettingsFilePath = path;

        BackendSettingsPersistenceService service;
        const BackendSettingsPersistenceLoadResult loaded =
            service.load(config);

        QVERIFY(loaded.isValid());
        QCOMPARE(loaded.loadedCount, 1);

        const std::optional<BackendSettings> found =
            loaded.store.settingsForBackend("basic_pitch");
        QVERIFY(found.has_value());
        QVERIFY(found->statusFromSettings() == BackendStatus::NotConfigured);
        QVERIFY(found->statusFromSettings() != BackendStatus::Ready);

        BackendRegistry registry;
        QVERIFY(registry.allManifests().isEmpty());
        QVERIFY(!registry.hasBackend("basic_pitch"));
    }

    void backendSettingsPersistenceServicePreparedSaveNeverMarksReady()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString parent =
            QDir(directory.path()).filePath("settings");
        const QString path =
            QDir(parent).filePath("backend_settings.json");

        BackendSettings settings;
        settings.backendId = "basic_pitch";
        settings.enabled = true;

        BackendSettingsStore store;
        QVERIFY(store.setSettings(settings));

        BackendSettingsPersistenceConfig config;
        config.testOnlySettingsFilePath = path;

        BackendSettingsPersistenceService service;
        const BackendSettingsPersistenceSaveResult saved =
            service.saveWithPreparedDirectory(config, store);
        QVERIFY(saved.isValid());

        BackendSettingsFileStore fileStore;
        const BackendSettingsFileLoadResult loaded = fileStore.load(path);
        QVERIFY(loaded.isValid());

        const std::optional<BackendSettings> found =
            loaded.store.settingsForBackend("basic_pitch");
        QVERIFY(found.has_value());
        QVERIFY(found->statusFromSettings() == BackendStatus::NotConfigured);
        QVERIFY(found->statusFromSettings() != BackendStatus::Ready);

        BackendRegistry registry;
        QVERIFY(registry.allManifests().isEmpty());
        QVERIFY(!registry.hasBackend("basic_pitch"));
    }

    void backendExecutableProbeReportsEmptyExecutablePath()
    {
        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = " ";

        BackendExecutableProbe probe;
        const BackendExecutableProbeResult result = probe.probe(manifest);

        QVERIFY(!result.isValid());
        QVERIFY(!result.isPresent());
        QCOMPARE(result.backendId, QString("basic_pitch"));
        QVERIFY(result.status == BackendExecutableProbeStatus::EmptyPath);
        QCOMPARE(result.statusName(), QString("empty_path"));
        QVERIFY(result.effectiveExecutablePath.isEmpty());
        QVERIFY(!result.usedSettingsOverride);
        QVERIFY(reportHasIssue(result.report, "empty_executable_path"));
    }

    void backendExecutableProbeReportsMissingExecutablePath()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = directory.filePath("missing-backend.exe");

        BackendExecutableProbe probe;
        const BackendExecutableProbeResult result = probe.probe(manifest);

        QVERIFY(!result.isValid());
        QVERIFY(result.status == BackendExecutableProbeStatus::Missing);
        QCOMPARE(result.effectiveExecutablePath, manifest.executablePath);
        QVERIFY(!result.exists);
        QVERIFY(!result.isFile);
        QVERIFY(!result.isDirectory);
        QVERIFY(!result.isExecutable);
        QVERIFY(reportHasIssue(result.report, "executable_missing"));
    }

    void backendExecutableProbeRejectsDirectoryPath()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = directory.path();

        BackendExecutableProbe probe;
        const BackendExecutableProbeResult result = probe.probe(manifest);

        QVERIFY(!result.isValid());
        QVERIFY(result.status == BackendExecutableProbeStatus::Directory);
        QVERIFY(result.exists);
        QVERIFY(!result.isFile);
        QVERIFY(result.isDirectory);
        QVERIFY(!result.isPresent());
        QVERIFY(reportHasIssue(result.report, "executable_path_is_directory"));
    }

    void backendExecutableProbeReportsExistingExecutableFilePath()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath("probe-backend.exe");
        QVERIFY(writeFile(path, "this file is never executed"));
        QVERIFY(makeExecutable(path));

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = path;

        BackendExecutableProbe probe;
        const BackendExecutableProbeResult result = probe.probe(manifest);

        QVERIFY(result.isValid());
        QVERIFY(result.isPresent());
        QVERIFY(result.status == BackendExecutableProbeStatus::Present);
        QCOMPARE(result.statusName(), QString("present"));
        QCOMPARE(result.effectiveExecutablePath, path);
        QVERIFY(result.exists);
        QVERIFY(result.isFile);
        QVERIFY(!result.isDirectory);
        QVERIFY(result.isExecutable);
        QVERIFY(result.report.issues.isEmpty());
    }

    void backendExecutableProbeReportsNotExecutableWhenCheckable()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath("probe-backend.txt");
        QVERIFY(writeFile(path, "not executable"));
        QFile::setPermissions(path,
                              QFileDevice::ReadOwner |
                              QFileDevice::WriteOwner);

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = path;

        BackendExecutableProbe probe;
        const BackendExecutableProbeResult result = probe.probe(manifest);

        QVERIFY(!result.isValid());
        QVERIFY(result.status == BackendExecutableProbeStatus::NotExecutable);
        QVERIFY(result.exists);
        QVERIFY(result.isFile);
        QVERIFY(!result.isDirectory);
        QVERIFY(!result.isExecutable);
        QVERIFY(reportHasIssue(result.report, "executable_not_executable"));
    }

    void backendExecutableProbeSettingsOverrideTakesPriority()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString manifestPath = directory.filePath("manifest.exe");
        QVERIFY(writeFile(manifestPath, "manifest executable placeholder"));
        QVERIFY(makeExecutable(manifestPath));

        const QString overridePath = directory.filePath("missing-override.exe");

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = manifestPath;

        BackendSettings settings;
        settings.backendId = "basic_pitch";
        settings.executablePathOverride = overridePath;

        BackendExecutableProbe probe;
        const BackendExecutableProbeResult result =
            probe.probe(manifest, settings);

        QVERIFY(!result.isValid());
        QVERIFY(result.status == BackendExecutableProbeStatus::Missing);
        QVERIFY(result.usedSettingsOverride);
        QCOMPARE(result.manifestExecutablePath, manifestPath);
        QCOMPARE(result.settingsExecutablePathOverride, overridePath);
        QCOMPARE(result.effectiveExecutablePath, overridePath);
        QVERIFY(reportHasIssue(result.report, "executable_missing"));
    }

    void backendExecutableProbeNeverRunsExecutable()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString markerPath = directory.filePath("probe-ran.marker");
        const QString scriptPath = directory.filePath("probe-backend.bat");
        const QByteArray script =
            QByteArray("@echo off\r\n") +
            QByteArray("echo ran > \"") +
            QDir::toNativeSeparators(markerPath).toUtf8() +
            QByteArray("\"\r\n");
        QVERIFY(writeFile(scriptPath, script));
        QVERIFY(makeExecutable(scriptPath));

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = scriptPath;

        BackendExecutableProbe probe;
        const BackendExecutableProbeResult result = probe.probe(manifest);

        QVERIFY(result.exists);
        QVERIFY(!QFile::exists(markerPath));
    }

    void backendExecutableProbeNeverMarksBackendReadyOrInstalled()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath("probe-backend.exe");
        QVERIFY(writeFile(path, "this file is never executed"));
        QVERIFY(makeExecutable(path));

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = path;
        manifest.status = BackendStatus::NotConfigured;

        BackendSettings settings;
        settings.backendId = "basic_pitch";
        settings.executablePathOverride = path;
        settings.enabled = true;

        BackendExecutableProbe probe;
        const BackendExecutableProbeResult result =
            probe.probe(manifest, settings);

        QVERIFY(result.isPresent());
        QVERIFY(manifest.status == BackendStatus::NotConfigured);
        QVERIFY(manifest.status != BackendStatus::Ready);
        QVERIFY(settings.statusFromSettings() == BackendStatus::NotConfigured);
        QVERIFY(settings.statusFromSettings() != BackendStatus::Ready);

        BackendRegistry registry;
        QVERIFY(registry.allManifests().isEmpty());
        QVERIFY(!registry.hasBackend("basic_pitch"));
    }

    void backendRequiredFileProbeAcceptsNoRequiredFiles()
    {
        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.requiredFiles.clear();
        manifest.optionalFiles.clear();

        BackendRequiredFileProbe probe;
        const BackendRequiredFileProbeResult result = probe.probe(manifest);

        QVERIFY(result.isValid());
        QCOMPARE(result.backendId, QString("basic_pitch"));
        QVERIFY(result.entries.isEmpty());
        QVERIFY(!result.hasRequiredFiles());
        QVERIFY(!result.hasMissingRequiredFile());
        QVERIFY(!result.hasModelCheckpointProbe());
        QVERIFY(!result.hasMissingModelCheckpoint());
    }

    void backendRequiredFileProbeReportsPresentRequiredFile()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath("required-model.bin");
        QVERIFY(writeFile(path, "required file contents"));

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.requiredFiles.clear();
        manifest.optionalFiles.clear();
        manifest.requiredFiles << path;

        BackendRequiredFileProbe probe;
        const BackendRequiredFileProbeResult result = probe.probe(manifest);

        QVERIFY(result.isValid());
        QVERIFY(result.hasRequiredFiles());
        QVERIFY(!result.hasMissingRequiredFile());
        QCOMPARE(result.entries.size(), 1);
        const BackendRequiredFileProbeEntry entry = result.entries.front();
        QVERIFY(entry.role == BackendRequiredFileProbeRole::RequiredFile);
        QVERIFY(entry.status == BackendRequiredFileProbeStatus::Present);
        QCOMPARE(entry.roleName(), QString("required_file"));
        QCOMPARE(entry.statusName(), QString("present"));
        QCOMPARE(entry.path, path);
        QVERIFY(entry.exists);
        QVERIFY(entry.isFile);
        QVERIFY(!entry.isDirectory);
        QVERIFY(entry.isPresent());
        QVERIFY(entry.isRequired());
    }

    void backendRequiredFileProbeReportsMissingRequiredFile()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath("missing-required.bin");

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.requiredFiles.clear();
        manifest.optionalFiles.clear();
        manifest.requiredFiles << path;

        BackendRequiredFileProbe probe;
        const BackendRequiredFileProbeResult result = probe.probe(manifest);

        QVERIFY(!result.isValid());
        QVERIFY(result.hasRequiredFiles());
        QVERIFY(result.hasMissingRequiredFile());
        QCOMPARE(result.entries.size(), 1);
        const BackendRequiredFileProbeEntry entry = result.entries.front();
        QVERIFY(entry.role == BackendRequiredFileProbeRole::RequiredFile);
        QVERIFY(entry.status == BackendRequiredFileProbeStatus::Missing);
        QVERIFY(!entry.exists);
        QVERIFY(!entry.isFile);
        QVERIFY(!entry.isDirectory);
        QVERIFY(!QFile::exists(path));
        QVERIFY(reportHasIssue(result.report, "required_file_missing"));
    }

    void backendRequiredFileProbeOptionalMissingIsWarning()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath("missing-optional.bin");

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.requiredFiles.clear();
        manifest.optionalFiles.clear();
        manifest.optionalFiles << path;

        BackendRequiredFileProbe probe;
        const BackendRequiredFileProbeResult result = probe.probe(manifest);

        QVERIFY(result.isValid());
        QCOMPARE(result.entries.size(), 1);
        const BackendRequiredFileProbeEntry entry = result.entries.front();
        QVERIFY(entry.role == BackendRequiredFileProbeRole::OptionalFile);
        QVERIFY(entry.status == BackendRequiredFileProbeStatus::Missing);
        QVERIFY(!entry.isRequired());
        QVERIFY(reportHasIssue(result.report, "optional_file_missing"));
        QVERIFY(reportHasIssueWithSeverity(result.report,
                                           "optional_file_missing",
                                           ValidationSeverity::Warning));
        QVERIFY(!QFile::exists(path));
    }

    void backendRequiredFileProbeReportsPresentModelCheckpointOverride()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath("checkpoint.onnx");
        QVERIFY(writeFile(path, "checkpoint placeholder"));

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.requiredFiles.clear();
        manifest.optionalFiles.clear();

        BackendSettings settings;
        settings.backendId = "basic_pitch";
        settings.modelCheckpointPathOverride = path;

        BackendRequiredFileProbe probe;
        const BackendRequiredFileProbeResult result =
            probe.probe(manifest, settings);

        QVERIFY(result.isValid());
        QVERIFY(result.hasModelCheckpointProbe());
        QVERIFY(!result.hasMissingModelCheckpoint());
        QCOMPARE(result.entries.size(), 1);
        const BackendRequiredFileProbeEntry entry = result.entries.front();
        QVERIFY(entry.role == BackendRequiredFileProbeRole::ModelCheckpoint);
        QVERIFY(entry.status == BackendRequiredFileProbeStatus::Present);
        QCOMPARE(entry.roleName(), QString("model_checkpoint"));
        QCOMPARE(entry.path, path);
        QVERIFY(entry.exists);
        QVERIFY(entry.isFile);
        QVERIFY(!entry.isDirectory);
    }

    void backendRequiredFileProbeReportsMissingModelCheckpointOverride()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath("missing-checkpoint.onnx");

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.requiredFiles.clear();
        manifest.optionalFiles.clear();

        BackendSettings settings;
        settings.backendId = "basic_pitch";
        settings.modelCheckpointPathOverride = path;

        BackendRequiredFileProbe probe;
        const BackendRequiredFileProbeResult result =
            probe.probe(manifest, settings);

        QVERIFY(!result.isValid());
        QVERIFY(result.hasModelCheckpointProbe());
        QVERIFY(result.hasMissingModelCheckpoint());
        QCOMPARE(result.entries.size(), 1);
        const BackendRequiredFileProbeEntry entry = result.entries.front();
        QVERIFY(entry.role == BackendRequiredFileProbeRole::ModelCheckpoint);
        QVERIFY(entry.status == BackendRequiredFileProbeStatus::Missing);
        QVERIFY(!entry.exists);
        QVERIFY(!entry.isFile);
        QVERIFY(!entry.isDirectory);
        QVERIFY(reportHasIssue(result.report, "model_checkpoint_missing"));
        QVERIFY(!QFile::exists(path));
    }

    void backendRequiredFileProbeReportsDirectoryInsteadOfFile()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.requiredFiles.clear();
        manifest.optionalFiles.clear();
        manifest.requiredFiles << directory.path();

        BackendRequiredFileProbe probe;
        const BackendRequiredFileProbeResult result = probe.probe(manifest);

        QVERIFY(!result.isValid());
        QVERIFY(result.hasMissingRequiredFile());
        QCOMPARE(result.entries.size(), 1);
        const BackendRequiredFileProbeEntry entry = result.entries.front();
        QVERIFY(entry.role == BackendRequiredFileProbeRole::RequiredFile);
        QVERIFY(entry.status == BackendRequiredFileProbeStatus::Directory);
        QCOMPARE(entry.statusName(), QString("directory"));
        QVERIFY(entry.exists);
        QVERIFY(!entry.isFile);
        QVERIFY(entry.isDirectory);
        QVERIFY(reportHasIssue(result.report,
                               "required_file_path_is_directory"));
    }

    void backendRequiredFileProbeNeverMarksBackendReadyOrInstalled()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString requiredPath = directory.filePath("required-file.bin");
        const QString checkpointPath = directory.filePath("checkpoint.onnx");
        QVERIFY(writeFile(requiredPath, "required"));
        QVERIFY(writeFile(checkpointPath, "checkpoint"));

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.requiredFiles.clear();
        manifest.optionalFiles.clear();
        manifest.requiredFiles << requiredPath;
        manifest.status = BackendStatus::NotConfigured;

        BackendSettings settings;
        settings.backendId = "basic_pitch";
        settings.modelCheckpointPathOverride = checkpointPath;
        settings.enabled = true;

        BackendRequiredFileProbe probe;
        const BackendRequiredFileProbeResult result =
            probe.probe(manifest, settings);

        QVERIFY(result.isValid());
        QVERIFY(manifest.status == BackendStatus::NotConfigured);
        QVERIFY(manifest.status != BackendStatus::Ready);
        QVERIFY(settings.statusFromSettings() == BackendStatus::NotConfigured);
        QVERIFY(settings.statusFromSettings() != BackendStatus::Ready);

        BackendRegistry registry;
        QVERIFY(registry.allManifests().isEmpty());
        QVERIFY(!registry.hasBackend("basic_pitch"));
    }

    void backendAvailabilityProbeReportsMissingExecutable()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = directory.filePath("missing-backend.exe");
        manifest.requiredFiles.clear();
        manifest.optionalFiles.clear();

        BackendAvailabilityProbe probe;
        const BackendAvailabilityReport report = probe.probe(manifest);

        QCOMPARE(report.backendId, QString("basic_pitch"));
        QVERIFY(!report.isValid());
        QVERIFY(!report.pathChecksPassed());
        QVERIFY(report.status ==
                BackendAvailabilityProbeStatus::MissingExecutable);
        QCOMPARE(report.statusName(), QString("missing_executable"));
        QVERIFY(report.executableProbe.status ==
                BackendExecutableProbeStatus::Missing);
        QVERIFY(report.requiredFileProbe.entries.isEmpty());
        QVERIFY(reportHasIssue(report.report, "executable_missing"));
        QCOMPARE(report.errorCount(), 1);
        QCOMPARE(report.warningCount(), 0);
    }

    void backendAvailabilityProbeReportsExecutableButMissingRequiredFile()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString executablePath = directory.filePath("backend.exe");
        const QString requiredPath = directory.filePath("missing-model.bin");
        QVERIFY(writeFile(executablePath, "not executed"));
        QVERIFY(makeExecutable(executablePath));

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = executablePath;
        manifest.requiredFiles.clear();
        manifest.optionalFiles.clear();
        manifest.requiredFiles << requiredPath;

        BackendAvailabilityProbe probe;
        const BackendAvailabilityReport report = probe.probe(manifest);

        QVERIFY(!report.isValid());
        QVERIFY(report.status == BackendAvailabilityProbeStatus::MissingModel);
        QCOMPARE(report.statusName(), QString("missing_model"));
        QVERIFY(report.executableProbe.isPresent());
        QVERIFY(report.requiredFileProbe.hasMissingRequiredFile());
        QVERIFY(reportHasIssue(report.report, "required_file_missing"));
        QCOMPARE(report.errorCount(), 1);
    }

    void backendAvailabilityProbeReportsPathChecksPassed()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString executablePath = directory.filePath("backend.exe");
        const QString requiredPath = directory.filePath("model.bin");
        QVERIFY(writeFile(executablePath, "not executed"));
        QVERIFY(makeExecutable(executablePath));
        QVERIFY(writeFile(requiredPath, "model"));

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = executablePath;
        manifest.requiredFiles.clear();
        manifest.optionalFiles.clear();
        manifest.requiredFiles << requiredPath;

        BackendAvailabilityProbe probe;
        const BackendAvailabilityReport report = probe.probe(manifest);

        QVERIFY(report.isValid());
        QVERIFY(report.pathChecksPassed());
        QVERIFY(report.status ==
                BackendAvailabilityProbeStatus::PathChecksPassed);
        QCOMPARE(report.statusName(), QString("path_checks_passed"));
        QVERIFY(report.executableProbe.isPresent());
        QVERIFY(!report.requiredFileProbe.hasMissingRequiredFile());
        QCOMPARE(report.errorCount(), 0);
        QCOMPARE(report.warningCount(), 0);
        QVERIFY(report.debugSummaryString().contains("path_checks_passed"));
    }

    void backendAvailabilityProbeOptionalFileMissingCreatesWarningOnly()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString executablePath = directory.filePath("backend.exe");
        const QString optionalPath = directory.filePath("optional.log");
        QVERIFY(writeFile(executablePath, "not executed"));
        QVERIFY(makeExecutable(executablePath));

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = executablePath;
        manifest.requiredFiles.clear();
        manifest.optionalFiles.clear();
        manifest.optionalFiles << optionalPath;

        BackendAvailabilityProbe probe;
        const BackendAvailabilityReport report = probe.probe(manifest);

        QVERIFY(report.isValid());
        QVERIFY(report.pathChecksPassed());
        QVERIFY(report.status ==
                BackendAvailabilityProbeStatus::PathChecksPassed);
        QCOMPARE(report.errorCount(), 0);
        QCOMPARE(report.warningCount(), 1);
        QVERIFY(reportHasIssueWithSeverity(report.report,
                                           "optional_file_missing",
                                           ValidationSeverity::Warning));
        QVERIFY(!QFile::exists(optionalPath));
    }

    void backendAvailabilityProbeSettingsExecutableOverrideTakesPriority()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString manifestPath = directory.filePath("manifest.exe");
        const QString overridePath = directory.filePath("override.exe");
        QVERIFY(writeFile(manifestPath, "manifest executable placeholder"));
        QVERIFY(makeExecutable(manifestPath));
        QVERIFY(writeFile(overridePath, "override executable placeholder"));
        QVERIFY(makeExecutable(overridePath));

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = manifestPath;
        manifest.requiredFiles.clear();
        manifest.optionalFiles.clear();

        BackendSettings settings;
        settings.backendId = "basic_pitch";
        settings.executablePathOverride = overridePath;

        BackendAvailabilityProbe probe;
        const BackendAvailabilityReport report =
            probe.probe(manifest, settings);

        QVERIFY(report.isValid());
        QVERIFY(report.pathChecksPassed());
        QVERIFY(report.executableProbe.usedSettingsOverride);
        QCOMPARE(report.executableProbe.manifestExecutablePath, manifestPath);
        QCOMPARE(report.executableProbe.settingsExecutablePathOverride,
                 overridePath);
        QCOMPARE(report.executableProbe.effectiveExecutablePath, overridePath);
    }

    void backendAvailabilityProbeSettingsModelCheckpointOverrideIsChecked()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString executablePath = directory.filePath("backend.exe");
        const QString checkpointPath = directory.filePath("checkpoint.onnx");
        QVERIFY(writeFile(executablePath, "not executed"));
        QVERIFY(makeExecutable(executablePath));
        QVERIFY(writeFile(checkpointPath, "checkpoint"));

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = executablePath;
        manifest.requiredFiles.clear();
        manifest.optionalFiles.clear();

        BackendSettings settings;
        settings.backendId = "basic_pitch";
        settings.modelCheckpointPathOverride = checkpointPath;

        BackendAvailabilityProbe probe;
        const BackendAvailabilityReport report =
            probe.probe(manifest, settings);

        QVERIFY(report.isValid());
        QVERIFY(report.pathChecksPassed());
        QVERIFY(report.requiredFileProbe.hasModelCheckpointProbe());
        QCOMPARE(report.requiredFileProbe.entries.size(), 1);
        const BackendRequiredFileProbeEntry entry =
            report.requiredFileProbe.entries.front();
        QVERIFY(entry.role == BackendRequiredFileProbeRole::ModelCheckpoint);
        QVERIFY(entry.status == BackendRequiredFileProbeStatus::Present);
        QCOMPARE(entry.path, checkpointPath);
    }

    void backendAvailabilityProbeNeverRunsExecutable()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString markerPath = directory.filePath("availability-ran.marker");
        const QString scriptPath = directory.filePath("availability-backend.bat");
        const QByteArray script =
            QByteArray("@echo off\r\n") +
            QByteArray("echo ran > \"") +
            QDir::toNativeSeparators(markerPath).toUtf8() +
            QByteArray("\"\r\n");
        QVERIFY(writeFile(scriptPath, script));
        QVERIFY(makeExecutable(scriptPath));

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = scriptPath;
        manifest.requiredFiles.clear();
        manifest.optionalFiles.clear();

        BackendAvailabilityProbe probe;
        const BackendAvailabilityReport report = probe.probe(manifest);

        QVERIFY(report.executableProbe.exists);
        QVERIFY(!QFile::exists(markerPath));
    }

    void backendAvailabilityProbeNeverMarksBackendReadyOrInstalled()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString executablePath = directory.filePath("backend.exe");
        const QString requiredPath = directory.filePath("model.bin");
        QVERIFY(writeFile(executablePath, "not executed"));
        QVERIFY(makeExecutable(executablePath));
        QVERIFY(writeFile(requiredPath, "model"));

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = executablePath;
        manifest.requiredFiles.clear();
        manifest.optionalFiles.clear();
        manifest.requiredFiles << requiredPath;
        manifest.status = BackendStatus::NotConfigured;

        BackendSettings settings;
        settings.backendId = "basic_pitch";
        settings.enabled = true;

        BackendAvailabilityProbe probe;
        const BackendAvailabilityReport report =
            probe.probe(manifest, settings);

        QVERIFY(report.pathChecksPassed());
        QVERIFY(manifest.status == BackendStatus::NotConfigured);
        QVERIFY(manifest.status != BackendStatus::Ready);
        QVERIFY(settings.statusFromSettings() == BackendStatus::NotConfigured);
        QVERIFY(settings.statusFromSettings() != BackendStatus::Ready);

        BackendRegistry registry;
        QVERIFY(registry.allManifests().isEmpty());
        QVERIFY(!registry.hasBackend("basic_pitch"));
    }

    void backendAvailabilityStoreStoresValidReport()
    {
        BackendAvailabilityReport report =
            makeAvailabilityReport("basic_pitch",
                                   BackendAvailabilityProbeStatus::PathChecksPassed);

        BackendAvailabilityStore store;

        QVERIFY(store.setReport(report));
        QVERIFY(store.hasReport("basic_pitch"));
        QCOMPARE(store.size(), 1);
        QVERIFY(store.backendIds().contains("basic_pitch"));

        const std::optional<BackendAvailabilityReport> stored =
            store.reportById("basic_pitch");
        QVERIFY(stored.has_value());
        QVERIFY(stored->status ==
                BackendAvailabilityProbeStatus::PathChecksPassed);
        QCOMPARE(stored->statusName(), QString("path_checks_passed"));
        QVERIFY(store.debugSummaryString().contains("availability_reports=1"));
    }

    void backendAvailabilityStoreRejectsInvalidBackendId()
    {
        BackendAvailabilityReport report =
            makeAvailabilityReport("BasicPitch",
                                   BackendAvailabilityProbeStatus::PathChecksPassed);

        BackendAvailabilityStore store;

        QVERIFY(!store.setReport(report));
        QVERIFY(!store.hasReport("BasicPitch"));
        QVERIFY(!store.reportById("BasicPitch").has_value());
        QCOMPARE(store.size(), 0);
        QVERIFY(store.allReports().isEmpty());
    }

    void backendAvailabilityStoreReplacesExistingReport()
    {
        BackendAvailabilityStore store;

        QVERIFY(store.setReport(makeAvailabilityReport(
            "basic_pitch",
            BackendAvailabilityProbeStatus::MissingExecutable)));
        QVERIFY(store.setReport(makeAvailabilityReport(
            "basic_pitch",
            BackendAvailabilityProbeStatus::PathChecksPassed)));

        QCOMPARE(store.size(), 1);
        const std::optional<BackendAvailabilityReport> stored =
            store.reportById("basic_pitch");
        QVERIFY(stored.has_value());
        QVERIFY(stored->status ==
                BackendAvailabilityProbeStatus::PathChecksPassed);
    }

    void backendAvailabilityStoreRemovesReport()
    {
        BackendAvailabilityStore store;

        QVERIFY(store.setReport(makeAvailabilityReport(
            "basic_pitch",
            BackendAvailabilityProbeStatus::MissingExecutable)));

        QVERIFY(store.removeReport("basic_pitch"));
        QVERIFY(!store.hasReport("basic_pitch"));
        QCOMPARE(store.size(), 0);
        QVERIFY(!store.removeReport("basic_pitch"));
        QVERIFY(!store.removeReport("BasicPitch"));
    }

    void backendAvailabilityStoreClearsReports()
    {
        BackendAvailabilityStore store;

        QVERIFY(store.setReport(makeAvailabilityReport(
            "basic_pitch",
            BackendAvailabilityProbeStatus::MissingExecutable)));
        QVERIFY(store.setReport(makeAvailabilityReport(
            "crepe_notes",
            BackendAvailabilityProbeStatus::NotConfigured)));

        QCOMPARE(store.size(), 2);
        store.clear();
        QCOMPARE(store.size(), 0);
        QVERIFY(store.allReports().isEmpty());
        QVERIFY(store.backendIds().isEmpty());
    }

    void backendAvailabilityStoreReturnsAllReports()
    {
        BackendAvailabilityStore store;

        QVERIFY(store.setReport(makeAvailabilityReport(
            "basic_pitch",
            BackendAvailabilityProbeStatus::PathChecksPassed)));
        QVERIFY(store.setReport(makeAvailabilityReport(
            "crepe_notes",
            BackendAvailabilityProbeStatus::MissingModel)));

        const QVector<BackendAvailabilityReport> reports = store.allReports();

        QCOMPARE(reports.size(), 2);
        QVERIFY(store.hasReport("basic_pitch"));
        QVERIFY(store.hasReport("crepe_notes"));
        QVERIFY(store.backendIds().contains("basic_pitch"));
        QVERIFY(store.backendIds().contains("crepe_notes"));
    }

    void backendAvailabilityStoreNeverMarksBackendReadyOrInstalled()
    {
        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.status = BackendStatus::NotConfigured;

        BackendAvailabilityReport report =
            makeAvailabilityReport("basic_pitch",
                                   BackendAvailabilityProbeStatus::PathChecksPassed);

        BackendAvailabilityStore store;
        QVERIFY(store.setReport(report));

        const std::optional<BackendAvailabilityReport> stored =
            store.reportById("basic_pitch");
        QVERIFY(stored.has_value());
        QVERIFY(stored->pathChecksPassed());
        QVERIFY(manifest.status == BackendStatus::NotConfigured);
        QVERIFY(manifest.status != BackendStatus::Ready);

        BackendRegistry registry;
        QVERIFY(!registry.hasBackend("basic_pitch"));
        QVERIFY(registry.allManifests().isEmpty());
    }

    void backendRunWorkspaceGeneratesValidPaths()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const BackendRunWorkspace workspace =
            BackendRunWorkspace::fromParts(directory.path(),
                                           "basic_pitch",
                                           "run_001");

        QVERIFY(workspace.hasValidShape());
        QCOMPARE(workspace.safeBackendId, QString("basic_pitch"));
        QCOMPARE(workspace.safeRunId, QString("run_001"));
        QCOMPARE(workspace.runDirectoryPath,
                 QDir::cleanPath(QDir(directory.path())
                                     .filePath("basic_pitch/run_001")));
        QCOMPARE(workspace.logFilePath,
                 QDir::cleanPath(QDir(workspace.logDirectoryPath)
                                     .filePath("process.log")));
        QCOMPARE(workspace.stdoutLogPath,
                 QDir::cleanPath(QDir(workspace.logDirectoryPath)
                                     .filePath("stdout.log")));
        QCOMPARE(workspace.stderrLogPath,
                 QDir::cleanPath(QDir(workspace.logDirectoryPath)
                                     .filePath("stderr.log")));
        QCOMPARE(workspace.unifiedResultJsonPath,
                 QDir::cleanPath(QDir(workspace.runDirectoryPath)
                                     .filePath("result.json")));
        QVERIFY(workspace.debugSummaryString().contains("basic_pitch"));
        QVERIFY(workspace.debugSummaryString().contains("run_001"));
    }

    void backendRunWorkspaceRejectsEmptyBackendId()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const BackendRunWorkspace workspace =
            BackendRunWorkspace::fromParts(directory.path(), "", "run_001");
        const ValidationReport report = workspace.validate();

        QVERIFY(!report.isValid());
        QVERIFY(reportHasIssue(report, "empty_backend_id"));
        QVERIFY(!workspace.hasValidShape());
    }

    void backendRunWorkspaceRejectsEmptyRunId()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const BackendRunWorkspace workspace =
            BackendRunWorkspace::fromParts(directory.path(),
                                           "basic_pitch",
                                           "  ");
        const ValidationReport report = workspace.validate();

        QVERIFY(!report.isValid());
        QVERIFY(reportHasIssue(report, "empty_run_id"));
        QVERIFY(!workspace.hasValidShape());
    }

    void backendRunWorkspaceSanitizesTraversalLikeInput()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const BackendRunWorkspace workspace =
            BackendRunWorkspace::fromParts(directory.path(),
                                           "../basic_pitch",
                                           "run/../../001");
        const ValidationReport report = workspace.validate();

        QVERIFY(report.isValid());
        QVERIFY(reportHasIssueWithSeverity(report,
                                           "backend_id_sanitized",
                                           ValidationSeverity::Warning));
        QVERIFY(reportHasIssueWithSeverity(report,
                                           "run_id_sanitized",
                                           ValidationSeverity::Warning));
        QCOMPARE(workspace.safeBackendId, QString("basic_pitch"));
        QCOMPARE(workspace.safeRunId, QString("run_001"));
        QVERIFY(!workspace.runDirectoryPath.contains(".."));
        QVERIFY(!workspace.runDirectoryPath.contains("\\.."));
        QVERIFY(workspace.runDirectoryPath.startsWith(
            QDir::cleanPath(directory.path())));
    }

    void backendRunWorkspacePreparationIsExplicit()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const BackendRunWorkspace workspace =
            BackendRunWorkspace::fromParts(directory.path(),
                                           "basic_pitch",
                                           "run_001");

        QVERIFY(workspace.hasValidShape());
        QVERIFY(!QDir(workspace.runDirectoryPath).exists());
        QVERIFY(!QDir(workspace.logDirectoryPath).exists());
        QVERIFY(!QDir(workspace.temporaryDirectoryPath).exists());
        QVERIFY(!QDir(workspace.resultArtifactsDirectoryPath).exists());

        const BackendRunWorkspacePreparationResult prepared =
            workspace.prepareWorkspace();

        QVERIFY(prepared.isValid());
        QVERIFY(prepared.runDirectoryCreated);
        QVERIFY(prepared.logDirectoryCreated);
        QVERIFY(prepared.temporaryDirectoryCreated);
        QVERIFY(prepared.resultArtifactsDirectoryCreated);
        QVERIFY(QDir(workspace.runDirectoryPath).exists());
        QVERIFY(QDir(workspace.logDirectoryPath).exists());
        QVERIFY(QDir(workspace.temporaryDirectoryPath).exists());
        QVERIFY(QDir(workspace.resultArtifactsDirectoryPath).exists());
    }

    void backendRunWorkspaceGeneratedPathsAreStable()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const BackendRunWorkspace first =
            BackendRunWorkspace::fromParts(directory.path(),
                                           "basic_pitch",
                                           "run_001");
        const BackendRunWorkspace second =
            BackendRunWorkspace::fromParts(directory.path(),
                                           "basic_pitch",
                                           "run_001");

        QCOMPARE(first.runDirectoryPath, second.runDirectoryPath);
        QCOMPARE(first.logFilePath, second.logFilePath);
        QCOMPARE(first.stdoutLogPath, second.stdoutLogPath);
        QCOMPARE(first.stderrLogPath, second.stderrLogPath);
        QCOMPARE(first.unifiedResultJsonPath, second.unifiedResultJsonPath);
        QCOMPARE(first.temporaryDirectoryPath,
                 second.temporaryDirectoryPath);
        QCOMPARE(first.resultArtifactsDirectoryPath,
                 second.resultArtifactsDirectoryPath);
    }

    void backendRunWorkspaceModelDoesNotRunProcesses()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const BackendRunWorkspace workspace =
            BackendRunWorkspace::fromParts(directory.path(),
                                           "basic_pitch",
                                           "run_001");

        QVERIFY(workspace.hasValidShape());
        QVERIFY(!QFile::exists(workspace.logFilePath));
        QVERIFY(!QFile::exists(workspace.stdoutLogPath));
        QVERIFY(!QFile::exists(workspace.stderrLogPath));
        QVERIFY(!QFile::exists(workspace.unifiedResultJsonPath));
        QVERIFY(!workspace.debugSummaryString().contains(
            "--external-process-helper"));
    }

    void backendRunWorkspaceNeverMarksBackendReady()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.status = BackendStatus::NotConfigured;

        const BackendRunWorkspace workspace =
            BackendRunWorkspace::fromParts(directory.path(),
                                           manifest.id(),
                                           "run_001");
        QVERIFY(workspace.hasValidShape());

        const BackendRunWorkspacePreparationResult prepared =
            workspace.prepareWorkspace();
        QVERIFY(prepared.isValid());

        QVERIFY(manifest.status == BackendStatus::NotConfigured);
        QVERIFY(manifest.status != BackendStatus::Ready);

        BackendRegistry registry;
        QVERIFY(!registry.hasBackend("basic_pitch"));
        QVERIFY(registry.allManifests().isEmpty());
    }

    void backendRunRequestBuilderUsesManifestExecutablePath()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = directory.filePath("manifest-adapter.exe");

        const BackendRunWorkspace workspace =
            BackendRunWorkspace::fromParts(directory.path(),
                                           manifest.id(),
                                           "run_001");

        BackendRunRequestParameters parameters;
        parameters.inputAudioFilePath = directory.filePath("input.wav");
        parameters.expectedUnifiedResultJsonPath =
            workspace.unifiedResultJsonPath;
        parameters.timeoutMsec = 1234;

        BackendRunRequestBuilder builder;
        const BackendRunRequestBuildResult built =
            builder.build(manifest, workspace, parameters);

        QVERIFY(built.isValid());
        QCOMPARE(built.request.runId, QString("run_001"));
        QCOMPARE(built.request.executablePath, manifest.executablePath);
        QVERIFY(!built.usedSettingsExecutableOverride);
        QCOMPARE(built.request.workingDirectory, workspace.runDirectoryPath);
        QCOMPARE(built.request.timeoutMsec, 1234);
        QVERIFY(built.request.arguments.contains("--input"));
        QVERIFY(built.request.arguments.contains(parameters.inputAudioFilePath));
        QVERIFY(built.request.arguments.contains("--result"));
        QVERIFY(built.request.arguments.contains(
            parameters.expectedUnifiedResultJsonPath));
        QVERIFY(built.request.arguments.contains("--workspace"));
        QVERIFY(built.debugSummaryString().contains("basic_pitch"));
    }

    void backendRunRequestBuilderSettingsExecutableOverrideTakesPriority()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = directory.filePath("manifest-adapter.exe");

        BackendSettings settings;
        settings.backendId = "basic_pitch";
        settings.executablePathOverride =
            directory.filePath("settings-adapter.exe");

        const BackendRunWorkspace workspace =
            BackendRunWorkspace::fromParts(directory.path(),
                                           manifest.id(),
                                           "run_001");

        BackendRunRequestParameters parameters;
        parameters.inputAudioFilePath = directory.filePath("input.wav");
        parameters.expectedUnifiedResultJsonPath =
            workspace.unifiedResultJsonPath;

        BackendRunRequestBuilder builder;
        const BackendRunRequestBuildResult built =
            builder.build(manifest, settings, workspace, parameters);

        QVERIFY(built.isValid());
        QVERIFY(built.usedSettingsExecutableOverride);
        QCOMPARE(built.request.executablePath,
                 settings.executablePathOverride);
    }

    void backendRunRequestBuilderWorkingDirectoryOverrideWorks()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = directory.filePath("manifest-adapter.exe");
        manifest.workingDirectory = directory.filePath("manifest-workdir");

        BackendSettings settings;
        settings.backendId = "basic_pitch";
        settings.workingDirectoryOverride =
            directory.filePath("settings-workdir");

        const BackendRunWorkspace workspace =
            BackendRunWorkspace::fromParts(directory.path(),
                                           manifest.id(),
                                           "run_001");

        BackendRunRequestParameters parameters;
        parameters.inputAudioFilePath = directory.filePath("input.wav");
        parameters.expectedUnifiedResultJsonPath =
            workspace.unifiedResultJsonPath;

        BackendRunRequestBuilder builder;
        const BackendRunRequestBuildResult built =
            builder.build(manifest, settings, workspace, parameters);

        QVERIFY(built.isValid());
        QVERIFY(built.usedSettingsWorkingDirectoryOverride);
        QCOMPARE(built.request.workingDirectory,
                 settings.workingDirectoryOverride);
    }

    void backendRunRequestBuilderIncludesEnvironmentOverrides()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = directory.filePath("manifest-adapter.exe");

        BackendSettings settings;
        settings.backendId = "basic_pitch";
        settings.environmentVariables.insert("TONY_BACKEND_MODE", "test");
        settings.environmentVariables.insert("TONY_BACKEND_TRACE", "1");

        const BackendRunWorkspace workspace =
            BackendRunWorkspace::fromParts(directory.path(),
                                           manifest.id(),
                                           "run_001");

        BackendRunRequestParameters parameters;
        parameters.inputAudioFilePath = directory.filePath("input.wav");
        parameters.expectedUnifiedResultJsonPath =
            workspace.unifiedResultJsonPath;

        BackendRunRequestBuilder builder;
        const BackendRunRequestBuildResult built =
            builder.build(manifest, settings, workspace, parameters);

        QVERIFY(built.isValid());
        QCOMPARE(built.request.environmentOverrides.value("TONY_BACKEND_MODE"),
                 QString("test"));
        QCOMPARE(built.request.environmentOverrides.value("TONY_BACKEND_TRACE"),
                 QString("1"));
    }

    void backendRunRequestBuilderRejectsEmptyInputPath()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = directory.filePath("manifest-adapter.exe");

        const BackendRunWorkspace workspace =
            BackendRunWorkspace::fromParts(directory.path(),
                                           manifest.id(),
                                           "run_001");

        BackendRunRequestParameters parameters;
        parameters.inputAudioFilePath = "  ";
        parameters.expectedUnifiedResultJsonPath =
            workspace.unifiedResultJsonPath;

        BackendRunRequestBuilder builder;
        const BackendRunRequestBuildResult built =
            builder.build(manifest, workspace, parameters);

        QVERIFY(!built.isValid());
        QVERIFY(reportHasIssue(built.report, "empty_input_audio_path"));
    }

    void backendRunRequestBuilderRejectsInvalidSelectedRegion()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = directory.filePath("manifest-adapter.exe");

        const BackendRunWorkspace workspace =
            BackendRunWorkspace::fromParts(directory.path(),
                                           manifest.id(),
                                           "run_001");

        BackendRunRequestParameters parameters;
        parameters.inputAudioFilePath = directory.filePath("input.wav");
        parameters.expectedUnifiedResultJsonPath =
            workspace.unifiedResultJsonPath;
        parameters.selectedRegion = AnalysisRegion { 2.0, 1.0 };

        BackendRunRequestBuilder builder;
        const BackendRunRequestBuildResult built =
            builder.build(manifest, workspace, parameters);

        QVERIFY(!built.isValid());
        QVERIFY(built.hasSelectedRegion);
        QVERIFY(reportHasIssue(built.report, "invalid_selected_region"));
    }

    void backendRunRequestBuilderIncludesValidSelectedRegion()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = directory.filePath("manifest-adapter.exe");

        const BackendRunWorkspace workspace =
            BackendRunWorkspace::fromParts(directory.path(),
                                           manifest.id(),
                                           "run_001");

        BackendRunRequestParameters parameters;
        parameters.inputAudioFilePath = directory.filePath("input.wav");
        parameters.expectedUnifiedResultJsonPath =
            workspace.unifiedResultJsonPath;
        parameters.selectedRegion = AnalysisRegion { 1.25, 3.5 };

        BackendRunRequestBuilder builder;
        const BackendRunRequestBuildResult built =
            builder.build(manifest, workspace, parameters);

        QVERIFY(built.isValid());
        QVERIFY(built.hasSelectedRegion);
        const int startIndex =
            built.request.arguments.indexOf("--region-start");
        const int endIndex =
            built.request.arguments.indexOf("--region-end");
        QVERIFY(startIndex >= 0);
        QVERIFY(endIndex >= 0);
        QCOMPARE(built.request.arguments.at(startIndex + 1),
                 QString("1.250000"));
        QCOMPARE(built.request.arguments.at(endIndex + 1),
                 QString("3.500000"));
    }

    void backendRunRequestBuilderRejectsEmptyOutputPath()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = directory.filePath("manifest-adapter.exe");

        const BackendRunWorkspace workspace =
            BackendRunWorkspace::fromParts(directory.path(),
                                           manifest.id(),
                                           "run_001");

        BackendRunRequestParameters parameters;
        parameters.inputAudioFilePath = directory.filePath("input.wav");
        parameters.expectedUnifiedResultJsonPath = " ";

        BackendRunRequestBuilder builder;
        const BackendRunRequestBuildResult built =
            builder.build(manifest, workspace, parameters);

        QVERIFY(!built.isValid());
        QVERIFY(reportHasIssue(built.report, "empty_output_result_path"));
    }

    void backendRunRequestBuilderNeverRunsProcess()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString markerPath = directory.filePath("builder-ran.marker");
        const QString scriptPath = directory.filePath("builder-backend.bat");
        const QByteArray script =
            QByteArray("@echo off\r\n") +
            QByteArray("echo ran > \"") +
            QDir::toNativeSeparators(markerPath).toUtf8() +
            QByteArray("\"\r\n");
        QVERIFY(writeFile(scriptPath, script));
        QVERIFY(makeExecutable(scriptPath));

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = scriptPath;

        const BackendRunWorkspace workspace =
            BackendRunWorkspace::fromParts(directory.path(),
                                           manifest.id(),
                                           "run_001");

        BackendRunRequestParameters parameters;
        parameters.inputAudioFilePath = directory.filePath("input.wav");
        parameters.expectedUnifiedResultJsonPath =
            workspace.unifiedResultJsonPath;

        BackendRunRequestBuilder builder;
        const BackendRunRequestBuildResult built =
            builder.build(manifest, workspace, parameters);

        QVERIFY(built.isValid());
        QCOMPARE(built.request.executablePath, scriptPath);
        QVERIFY(!QFile::exists(markerPath));
    }

    void backendRunRequestBuilderNeverMarksBackendReady()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = directory.filePath("manifest-adapter.exe");
        manifest.status = BackendStatus::NotConfigured;

        BackendSettings settings;
        settings.backendId = "basic_pitch";
        settings.executablePathOverride =
            directory.filePath("settings-adapter.exe");
        settings.enabled = true;

        const BackendRunWorkspace workspace =
            BackendRunWorkspace::fromParts(directory.path(),
                                           manifest.id(),
                                           "run_001");

        BackendRunRequestParameters parameters;
        parameters.inputAudioFilePath = directory.filePath("input.wav");
        parameters.expectedUnifiedResultJsonPath =
            workspace.unifiedResultJsonPath;

        BackendRunRequestBuilder builder;
        const BackendRunRequestBuildResult built =
            builder.build(manifest, settings, workspace, parameters);

        QVERIFY(built.isValid());
        QVERIFY(manifest.status == BackendStatus::NotConfigured);
        QVERIFY(manifest.status != BackendStatus::Ready);
        QVERIFY(settings.statusFromSettings() == BackendStatus::NotConfigured);

        BackendRegistry registry;
        QVERIFY(!registry.hasBackend("basic_pitch"));
        QVERIFY(registry.allManifests().isEmpty());
    }

    void backendRunRequestBuilderAppendsRequestFileArgument()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = directory.filePath("manifest-adapter.exe");

        const BackendRunWorkspace workspace =
            BackendRunWorkspace::fromParts(directory.path(),
                                           manifest.id(),
                                           "run_001");
        const QString requestPath = directory.filePath("request.json");

        BackendRunRequestParameters parameters;
        parameters.inputAudioFilePath = directory.filePath("input.wav");
        parameters.expectedUnifiedResultJsonPath =
            workspace.unifiedResultJsonPath;
        parameters.includeRequestJsonFileArgument = true;
        parameters.requestJsonFilePath = requestPath;

        BackendRunRequestBuilder builder;
        const BackendRunRequestBuildResult built =
            builder.build(manifest, workspace, parameters);

        QVERIFY(built.isValid());
        const int requestIndex = built.request.arguments.indexOf("--request");
        QVERIFY(requestIndex >= 0);
        QVERIFY(requestIndex + 1 < built.request.arguments.size());
        QCOMPARE(built.request.arguments.at(requestIndex + 1), requestPath);
        QVERIFY(built.request.arguments.contains("--input"));
        QVERIFY(built.request.arguments.contains(parameters.inputAudioFilePath));
        QVERIFY(built.request.arguments.contains("--result"));
        QVERIFY(built.request.arguments.contains(
            parameters.expectedUnifiedResultJsonPath));
    }

    void backendRunRequestBuilderSupportsCustomRequestFileArgumentFlag()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = directory.filePath("manifest-adapter.exe");

        const BackendRunWorkspace workspace =
            BackendRunWorkspace::fromParts(directory.path(),
                                           manifest.id(),
                                           "run_001");
        const QString requestPath = directory.filePath("request.json");

        BackendRunRequestParameters parameters;
        parameters.inputAudioFilePath = directory.filePath("input.wav");
        parameters.expectedUnifiedResultJsonPath =
            workspace.unifiedResultJsonPath;
        parameters.includeRequestJsonFileArgument = true;
        parameters.requestJsonArgumentFlag = "--request-json";
        parameters.requestJsonFilePath = requestPath;

        BackendRunRequestBuilder builder;
        const BackendRunRequestBuildResult built =
            builder.build(manifest, workspace, parameters);

        QVERIFY(built.isValid());
        QVERIFY(!built.request.arguments.contains("--request"));
        const int requestIndex =
            built.request.arguments.indexOf("--request-json");
        QVERIFY(requestIndex >= 0);
        QCOMPARE(built.request.arguments.at(requestIndex + 1), requestPath);
    }

    void backendRunRequestBuilderRejectsEmptyRequestFilePath()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = directory.filePath("manifest-adapter.exe");

        const BackendRunWorkspace workspace =
            BackendRunWorkspace::fromParts(directory.path(),
                                           manifest.id(),
                                           "run_001");

        BackendRunRequestParameters parameters;
        parameters.inputAudioFilePath = directory.filePath("input.wav");
        parameters.expectedUnifiedResultJsonPath =
            workspace.unifiedResultJsonPath;
        parameters.includeRequestJsonFileArgument = true;
        parameters.requestJsonFilePath = " ";

        BackendRunRequestBuilder builder;
        const BackendRunRequestBuildResult built =
            builder.build(manifest, workspace, parameters);

        QVERIFY(!built.isValid());
        QVERIFY(reportHasIssue(built.report, "empty_request_json_file_path"));
        QVERIFY(!built.request.arguments.contains("--request"));
    }

    void backendRunRequestBuilderRejectsEmptyRequestFileArgumentFlag()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = directory.filePath("manifest-adapter.exe");

        const BackendRunWorkspace workspace =
            BackendRunWorkspace::fromParts(directory.path(),
                                           manifest.id(),
                                           "run_001");
        const QString requestPath = directory.filePath("request.json");

        BackendRunRequestParameters parameters;
        parameters.inputAudioFilePath = directory.filePath("input.wav");
        parameters.expectedUnifiedResultJsonPath =
            workspace.unifiedResultJsonPath;
        parameters.includeRequestJsonFileArgument = true;
        parameters.requestJsonArgumentFlag = " ";
        parameters.requestJsonFilePath = requestPath;

        BackendRunRequestBuilder builder;
        const BackendRunRequestBuildResult built =
            builder.build(manifest, workspace, parameters);

        QVERIFY(!built.isValid());
        QVERIFY(reportHasIssue(built.report, "empty_request_json_argument_flag"));
        QVERIFY(!built.request.arguments.contains(requestPath));
    }

    void backendRunRequestBuilderKeepsExistingBehaviorWithoutRequestFile()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const BackendRunRequestBuildResult built =
            validBackendRunRequest(directory.path());

        QVERIFY(built.isValid());
        QVERIFY(!built.request.arguments.contains("--request"));
        QVERIFY(built.request.arguments.contains("--input"));
        QVERIFY(built.request.arguments.contains("--result"));
        QVERIFY(built.request.arguments.contains("--workspace"));
    }

    void backendRunRequestBuilderRequestFileArgumentNeverRunsProcess()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString markerPath =
            directory.filePath("request-argument-ran.marker");
        const QString scriptPath =
            directory.filePath("request-argument-backend.bat");
        const QByteArray script =
            QByteArray("@echo off\r\n") +
            QByteArray("echo ran > \"") +
            QDir::toNativeSeparators(markerPath).toUtf8() +
            QByteArray("\"\r\n");
        QVERIFY(writeFile(scriptPath, script));
        QVERIFY(makeExecutable(scriptPath));

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = scriptPath;

        const BackendRunWorkspace workspace =
            BackendRunWorkspace::fromParts(directory.path(),
                                           manifest.id(),
                                           "run_001");

        BackendRunRequestParameters parameters;
        parameters.inputAudioFilePath = directory.filePath("input.wav");
        parameters.expectedUnifiedResultJsonPath =
            workspace.unifiedResultJsonPath;
        parameters.includeRequestJsonFileArgument = true;
        parameters.requestJsonFilePath = directory.filePath("request.json");

        BackendRunRequestBuilder builder;
        const BackendRunRequestBuildResult built =
            builder.build(manifest, workspace, parameters);

        QVERIFY(built.isValid());
        QVERIFY(built.request.arguments.contains("--request"));
        QVERIFY(!QFile::exists(markerPath));
    }

    void backendRunRequestBuilderRequestFileArgumentNeverMarksBackendReady()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = directory.filePath("manifest-adapter.exe");
        manifest.status = BackendStatus::NotConfigured;

        const BackendRunWorkspace workspace =
            BackendRunWorkspace::fromParts(directory.path(),
                                           manifest.id(),
                                           "run_001");

        BackendRunRequestParameters parameters;
        parameters.inputAudioFilePath = directory.filePath("input.wav");
        parameters.expectedUnifiedResultJsonPath =
            workspace.unifiedResultJsonPath;
        parameters.includeRequestJsonFileArgument = true;
        parameters.requestJsonFilePath = directory.filePath("request.json");

        BackendRunRequestBuilder builder;
        const BackendRunRequestBuildResult built =
            builder.build(manifest, workspace, parameters);

        QVERIFY(built.isValid());
        QVERIFY(manifest.status == BackendStatus::NotConfigured);
        QVERIFY(manifest.status != BackendStatus::Ready);

        BackendRegistry registry;
        QVERIFY(!registry.hasBackend("basic_pitch"));
        QVERIFY(registry.allManifests().isEmpty());
    }

    void backendRunRequestSerializerSerializesValidRequest()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const BackendRunRequestBuildResult request =
            validBackendRunRequest(directory.path());

        BackendRunRequestSerializer serializer;
        const BackendRunRequestSerializationResult serialized =
            serializer.serialize(request);

        QVERIFY(serialized.isValid());
        QCOMPARE(serialized.object.value("contract_version").toString(),
                 QString("0.1"));
        QCOMPARE(serialized.object.value("backend_id").toString(),
                 QString("basic_pitch"));
        QCOMPARE(serialized.object.value("run_id").toString(),
                 QString("run_001"));
        QCOMPARE(serialized.object.value("executable_path").toString(),
                 request.request.executablePath);
        QCOMPARE(serialized.object.value("working_directory").toString(),
                 request.request.workingDirectory);
        QCOMPARE(serialized.object.value("input_audio_path").toString(),
                 request.inputAudioFilePath);
        QCOMPARE(serialized.object.value("output_result_json_path").toString(),
                 request.expectedUnifiedResultJsonPath);
        QCOMPARE(serialized.object.value("timeout_msec").toInt(),
                 request.request.timeoutMsec);
        QVERIFY(serialized.object.value("arguments").isArray());
        QVERIFY(serialized.object.value("selected_region").isNull());
    }

    void backendRunRequestSerializerSerializesSelectedRegion()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const BackendRunRequestBuildResult request =
            validBackendRunRequest(directory.path(),
                                   std::optional<AnalysisRegion>(
                                       AnalysisRegion { 1.25, 3.5 }));

        BackendRunRequestSerializer serializer;
        const BackendRunRequestSerializationResult serialized =
            serializer.serialize(request);

        QVERIFY(serialized.isValid());
        QVERIFY(serialized.object.value("selected_region").isObject());

        const QJsonObject region =
            serialized.object.value("selected_region").toObject();
        QCOMPARE(region.value("start_sec").toDouble(), 1.25);
        QCOMPARE(region.value("end_sec").toDouble(), 3.5);
        QCOMPARE(region.value("coordinate_system").toString(),
                 QString("original_audio_time"));
        QCOMPARE(region.value("apply_policy").toString(),
                 QString("preview_only"));
    }

    void backendRunRequestSerializerSerializesEnvironmentOverrides()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendSettings settings;
        settings.backendId = "basic_pitch";
        settings.environmentVariables.insert("TONY_BACKEND_MODE", "test");
        settings.environmentVariables.insert("TONY_BACKEND_TRACE", "1");

        const BackendRunRequestBuildResult request =
            validBackendRunRequest(directory.path(), std::nullopt, settings);

        BackendRunRequestSerializer serializer;
        const BackendRunRequestSerializationResult serialized =
            serializer.serialize(request);

        QVERIFY(serialized.isValid());
        const QJsonObject environment =
            serialized.object.value("environment_overrides").toObject();
        QCOMPARE(environment.value("TONY_BACKEND_MODE").toString(),
                 QString("test"));
        QCOMPARE(environment.value("TONY_BACKEND_TRACE").toString(),
                 QString("1"));
    }

    void backendRunRequestFileWriterWritesValidRequestFile()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const BackendRunRequestBuildResult request =
            validBackendRunRequest(directory.path());
        const QString path = directory.filePath("request.json");

        BackendRunRequestFileWriter writer;
        const BackendRunRequestFileWriteResult written =
            writer.write(path, request);

        QVERIFY(written.isValid());
        QCOMPARE(written.path, path);
        QVERIFY(written.bytesWritten > 0);
        QVERIFY(QFile::exists(path));

        const QJsonDocument document =
            QJsonDocument::fromJson(readTextFile(path).toUtf8());
        QVERIFY(document.isObject());
        QCOMPARE(document.object().value("backend_id").toString(),
                 QString("basic_pitch"));
        QCOMPARE(document.object().value("input_audio_path").toString(),
                 request.inputAudioFilePath);
    }

    void backendRunRequestFileWriterRejectsEmptyPath()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendRunRequestFileWriter writer;
        const BackendRunRequestFileWriteResult written =
            writer.write(QString(), validBackendRunRequest(directory.path()));

        QVERIFY(!written.isValid());
        QVERIFY(reportHasIssue(written.report, "empty_request_file_path"));
    }

    void backendRunRequestFileWriterRejectsMissingParentDirectory()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString path = directory.filePath("missing-parent/request.json");
        BackendRunRequestFileWriter writer;
        const BackendRunRequestFileWriteResult written =
            writer.write(path, validBackendRunRequest(directory.path()));

        QVERIFY(!written.isValid());
        QVERIFY(reportHasIssue(written.report, "parent_directory_missing"));
        QVERIFY(!QFile::exists(path));
        QVERIFY(!QDir(directory.filePath("missing-parent")).exists());
    }

    void backendRunRequestFileWriterRejectsInvalidRequiredFields()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendRunRequestBuildResult request =
            validBackendRunRequest(directory.path());
        request.request.executablePath.clear();

        const QString path = directory.filePath("request.json");
        BackendRunRequestFileWriter writer;
        const BackendRunRequestFileWriteResult written =
            writer.write(path, request);

        QVERIFY(!written.isValid());
        QVERIFY(reportHasIssue(written.report, "empty_executable_path"));
        QVERIFY(!QFile::exists(path));
    }

    void backendRunRequestFileWriterRejectsWriteFailure()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendRunRequestFileWriter writer;
        const BackendRunRequestFileWriteResult written =
            writer.write(directory.path(),
                         validBackendRunRequest(directory.path()));

        QVERIFY(!written.isValid());
        QVERIFY(reportHasIssue(written.report, "file_write_failed"));
    }

    void backendRunRequestFileWriterNeverRunsProcess()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString markerPath = directory.filePath("writer-ran.marker");
        const QString scriptPath = directory.filePath("writer-backend.bat");
        const QByteArray script =
            QByteArray("@echo off\r\n") +
            QByteArray("echo ran > \"") +
            QDir::toNativeSeparators(markerPath).toUtf8() +
            QByteArray("\"\r\n");
        QVERIFY(writeFile(scriptPath, script));
        QVERIFY(makeExecutable(scriptPath));

        BackendRunRequestBuildResult request =
            validBackendRunRequest(directory.path());
        request.request.executablePath = scriptPath;

        BackendRunRequestFileWriter writer;
        const BackendRunRequestFileWriteResult written =
            writer.write(directory.filePath("request.json"), request);

        QVERIFY(written.isValid());
        QVERIFY(!QFile::exists(markerPath));
    }

    void backendRunRequestFileWriterNeverMarksBackendReady()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = directory.filePath("manifest-adapter.exe");
        manifest.status = BackendStatus::NotConfigured;

        const BackendRunWorkspace workspace =
            BackendRunWorkspace::fromParts(directory.path(),
                                           manifest.id(),
                                           "run_001");

        BackendRunRequestParameters parameters;
        parameters.inputAudioFilePath = directory.filePath("input.wav");
        parameters.expectedUnifiedResultJsonPath =
            workspace.unifiedResultJsonPath;

        BackendRunRequestBuilder builder;
        const BackendRunRequestBuildResult request =
            builder.build(manifest, workspace, parameters);

        BackendRunRequestFileWriter writer;
        const BackendRunRequestFileWriteResult written =
            writer.write(directory.filePath("request.json"), request);

        QVERIFY(written.isValid());
        QVERIFY(manifest.status == BackendStatus::NotConfigured);
        QVERIFY(manifest.status != BackendStatus::Ready);

        BackendRegistry registry;
        QVERIFY(!registry.hasBackend("basic_pitch"));
        QVERIFY(registry.allManifests().isEmpty());
    }

    void externalProcessRunnerRunsSuccessfulCommand()
    {
        ExternalProcessRunner runner;
        const ExternalProcessResult result =
            runner.run(externalProcessHelperRequest({ "success" }));

        QVERIFY(result.succeeded());
        QVERIFY(result.started);
        QVERIFY(!result.startFailed);
        QVERIFY(!result.timedOut);
        QVERIFY(result.state == AnalysisRunState::Completed);
        QCOMPARE(result.exitCode, 0);
        QVERIFY(result.exitStatus == QProcess::NormalExit);
        QVERIFY(result.error.code == BackendErrorCode::None);
        QVERIFY(result.debugSummaryString().contains("completed"));
    }

    void externalProcessRunnerReportsNonZeroExitCode()
    {
        ExternalProcessRunner runner;
        const ExternalProcessResult result =
            runner.run(externalProcessHelperRequest({ "failure" }));

        QVERIFY(!result.succeeded());
        QVERIFY(result.started);
        QVERIFY(!result.startFailed);
        QVERIFY(!result.timedOut);
        QVERIFY(result.state == AnalysisRunState::Failed);
        QCOMPARE(result.exitCode, 7);
        QVERIFY(result.exitStatus == QProcess::NormalExit);
        QVERIFY(result.error.code == BackendErrorCode::ExecutionFailed);
        QVERIFY(result.error.message.contains("code 7"));
    }

    void externalProcessRunnerReportsMissingExecutable()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        ExternalProcessRequest request;
        request.executablePath = directory.filePath("missing-runner.exe");
        request.timeoutMsec = 1000;

        ExternalProcessRunner runner;
        const ExternalProcessResult result = runner.run(request);

        QVERIFY(!result.succeeded());
        QVERIFY(!result.started);
        QVERIFY(result.startFailed);
        QVERIFY(!result.timedOut);
        QVERIFY(result.state == AnalysisRunState::Failed);
        QVERIFY(result.processError == QProcess::FailedToStart);
        QVERIFY(result.error.code == BackendErrorCode::BackendMissing);
    }

    void externalProcessRunnerTimesOutCommand()
    {
        ExternalProcessRunner runner;
        ExternalProcessRequest request =
            externalProcessHelperRequest({ "sleep", "1000" });
        request.timeoutMsec = 100;

        const ExternalProcessResult result = runner.run(request);

        QVERIFY(!result.succeeded());
        QVERIFY(result.started);
        QVERIFY(result.timedOut);
        QVERIFY(result.state == AnalysisRunState::Failed);
        QVERIFY(result.error.code == BackendErrorCode::TimedOut);
    }

    void externalProcessRunnerCancellationReportsCancelled()
    {
        ExternalProcessRequest request =
            externalProcessHelperRequest({ "sleep", "3000" });
        request.timeoutMsec = 5000;
        request.cancellationToken =
            QSharedPointer<ExternalProcessCancellationToken>::create();

        auto future = std::async(std::launch::async, [request]() {
            ExternalProcessRunner runner;
            return runner.run(request);
        });

        QTest::qWait(100);
        ExternalProcessRunner runner;
        QVERIFY(runner.cancel(request.cancellationToken));

        const ExternalProcessResult result = future.get();

        QVERIFY(!result.succeeded());
        QVERIFY(result.started);
        QVERIFY(!result.startFailed);
        QVERIFY(!result.timedOut);
        QVERIFY(result.cancelled);
        QVERIFY(result.state == AnalysisRunState::Cancelled);
        QVERIFY(result.error.code == BackendErrorCode::Cancelled);
        QVERIFY(result.debugSummaryString().contains("cancelled=true"));
    }

    void externalProcessRunnerCancelRejectsMissingToken()
    {
        ExternalProcessRunner runner;

        QVERIFY(!runner.cancel(
            QSharedPointer<ExternalProcessCancellationToken>()));
    }

    void externalProcessRunnerCapturesStdout()
    {
        ExternalProcessRunner runner;
        const ExternalProcessResult result =
            runner.run(externalProcessHelperRequest({ "stdout",
                                                      "runner_stdout" }));

        QVERIFY(result.succeeded());
        QVERIFY(result.standardOutput.contains("runner_stdout"));
        QVERIFY(result.standardError.isEmpty());
    }

    void externalProcessRunnerCapturesStderr()
    {
        ExternalProcessRunner runner;
        const ExternalProcessResult result =
            runner.run(externalProcessHelperRequest({ "stderr",
                                                      "runner_stderr" }));

        QVERIFY(result.succeeded());
        QVERIFY(result.standardOutput.isEmpty());
        QVERIFY(result.standardError.contains("runner_stderr"));
    }

    void externalProcessRunnerAsyncRunsSuccessfulCommand()
    {
        ExternalProcessRunner runner;
        const ExternalProcessRunHandle handle =
            runner.startAsync(externalProcessHelperRequest({ "success" }));

        QVERIFY(handle.isValid());
        QVERIFY(runner.hasAsyncRun(handle));
        QVERIFY(waitForAsyncRunToFinish(runner, handle));

        const std::optional<ExternalProcessResult> result =
            runner.collectResult(handle);
        QVERIFY(result.has_value());
        QVERIFY(result->succeeded());
        QVERIFY(result->state == AnalysisRunState::Completed);

        QVERIFY(runner.cleanup(handle));
        QVERIFY(!runner.hasAsyncRun(handle));
        QVERIFY(!runner.cleanup(handle));
    }

    void externalProcessRunnerAsyncCancelsByHandle()
    {
        ExternalProcessRunner runner;
        ExternalProcessRequest request =
            externalProcessHelperRequest({ "sleep", "3000" });
        request.timeoutMsec = 5000;

        const ExternalProcessRunHandle handle = runner.startAsync(request);

        QVERIFY(handle.isValid());
        QVERIFY(runner.hasAsyncRun(handle));
        QVERIFY(waitForAsyncRunToStart(runner, handle));
        QVERIFY(runner.cancel(handle));
        QVERIFY(waitForAsyncRunToFinish(runner, handle));

        const std::optional<ExternalProcessResult> result =
            runner.collectResult(handle);
        QVERIFY(result.has_value());
        QVERIFY(result->cancelled);
        QVERIFY(!result->timedOut);
        QVERIFY(result->state == AnalysisRunState::Cancelled);
        QVERIFY(result->error.code == BackendErrorCode::Cancelled);

        QVERIFY(runner.cleanup(handle));
        QVERIFY(!runner.hasAsyncRun(handle));
    }

    void externalProcessRunnerAsyncMissingExecutableReportsStartFailure()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        ExternalProcessRequest request;
        request.executablePath = directory.filePath("missing-runner.exe");
        request.timeoutMsec = 1000;

        ExternalProcessRunner runner;
        const ExternalProcessRunHandle handle = runner.startAsync(request);

        QVERIFY(handle.isValid());
        QVERIFY(waitForAsyncRunToFinish(runner, handle));

        const std::optional<ExternalProcessResult> result =
            runner.collectResult(handle);
        QVERIFY(result.has_value());
        QVERIFY(!result->succeeded());
        QVERIFY(result->startFailed);
        QVERIFY(result->state == AnalysisRunState::Failed);
        QVERIFY(result->error.code == BackendErrorCode::BackendMissing);
        QVERIFY(runner.cleanup(handle));
    }

    void externalProcessRunnerAsyncCapturesOutput()
    {
        ExternalProcessRunner runner;
        const ExternalProcessRunHandle stdoutHandle =
            runner.startAsync(externalProcessHelperRequest({ "stdout",
                                                             "async_stdout" }));
        const ExternalProcessRunHandle stderrHandle =
            runner.startAsync(externalProcessHelperRequest({ "stderr",
                                                             "async_stderr" }));

        QVERIFY(waitForAsyncRunToFinish(runner, stdoutHandle));
        QVERIFY(waitForAsyncRunToFinish(runner, stderrHandle));

        const std::optional<ExternalProcessResult> stdoutResult =
            runner.collectResult(stdoutHandle);
        const std::optional<ExternalProcessResult> stderrResult =
            runner.collectResult(stderrHandle);

        QVERIFY(stdoutResult.has_value());
        QVERIFY(stderrResult.has_value());
        QVERIFY(stdoutResult->succeeded());
        QVERIFY(stderrResult->succeeded());
        QVERIFY(stdoutResult->standardOutput.contains("async_stdout"));
        QVERIFY(stderrResult->standardError.contains("async_stderr"));

        QVERIFY(runner.cleanup(stdoutHandle));
        QVERIFY(runner.cleanup(stderrHandle));
    }

    void externalProcessRunnerAsyncCleanupRejectsRunningRun()
    {
        ExternalProcessRunner runner;
        ExternalProcessRequest request =
            externalProcessHelperRequest({ "sleep", "500" });
        request.timeoutMsec = 3000;

        const ExternalProcessRunHandle handle = runner.startAsync(request);

        QVERIFY(handle.isValid());
        QVERIFY(waitForAsyncRunToStart(runner, handle));
        QVERIFY(!runner.cleanup(handle));
        QVERIFY(runner.cancel(handle));
        QVERIFY(waitForAsyncRunToFinish(runner, handle));
        QVERIFY(runner.cleanup(handle));
        QVERIFY(!runner.hasAsyncRun(handle));
    }

    void externalProcessRunnerEventsRecordSuccessfulLifecycle()
    {
        ExternalProcessRunner runner;
        ExternalProcessRequest request =
            externalProcessHelperRequest({ "success" });
        request.eventCollector =
            QSharedPointer<ExternalProcessEventCollector>::create();

        const ExternalProcessResult result = runner.run(request);

        QVERIFY(result.succeeded());
        QVERIFY(request.eventCollector->hasEvent(
            ExternalProcessEventType::Started));
        QVERIFY(request.eventCollector->hasEvent(
            ExternalProcessEventType::Finished));
        QCOMPARE(request.eventCollector->count(
                     ExternalProcessEventType::Started), 1);
        QCOMPARE(request.eventCollector->count(
                     ExternalProcessEventType::Finished), 1);
        QVERIFY(request.eventCollector->debugSummaryString()
                    .contains("events=2"));

        const QVector<ExternalProcessEvent> events =
            request.eventCollector->events();
        QVERIFY(events.front().type == ExternalProcessEventType::Started);
        QVERIFY(events.back().type == ExternalProcessEventType::Finished);
        QCOMPARE(events.back().exitCode, 0);
        QCOMPARE(events.back().typeName(), QString("finished"));
        QVERIFY(events.back().debugSummaryString().contains("finished"));
    }

    void externalProcessRunnerEventsCaptureStdoutChunk()
    {
        ExternalProcessRunner runner;
        ExternalProcessRequest request =
            externalProcessHelperRequest({ "stdout", "event_stdout" });
        request.eventCollector =
            QSharedPointer<ExternalProcessEventCollector>::create();

        const ExternalProcessResult result = runner.run(request);

        QVERIFY(result.succeeded());
        QVERIFY(result.standardOutput.contains("event_stdout"));
        QVERIFY(request.eventCollector->hasEvent(
            ExternalProcessEventType::StdoutChunk));
        QVERIFY(eventDataContains(*request.eventCollector,
                                  ExternalProcessEventType::StdoutChunk,
                                  "event_stdout"));
    }

    void externalProcessRunnerEventsCaptureStderrChunk()
    {
        ExternalProcessRunner runner;
        ExternalProcessRequest request =
            externalProcessHelperRequest({ "stderr", "event_stderr" });
        request.eventCollector =
            QSharedPointer<ExternalProcessEventCollector>::create();

        const ExternalProcessResult result = runner.run(request);

        QVERIFY(result.succeeded());
        QVERIFY(result.standardError.contains("event_stderr"));
        QVERIFY(request.eventCollector->hasEvent(
            ExternalProcessEventType::StderrChunk));
        QVERIFY(eventDataContains(*request.eventCollector,
                                  ExternalProcessEventType::StderrChunk,
                                  "event_stderr"));
    }

    void externalProcessRunnerEventsRecordMissingExecutable()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        ExternalProcessRequest request;
        request.executablePath = directory.filePath("missing-runner.exe");
        request.timeoutMsec = 1000;
        request.eventCollector =
            QSharedPointer<ExternalProcessEventCollector>::create();

        ExternalProcessRunner runner;
        const ExternalProcessResult result = runner.run(request);

        QVERIFY(!result.succeeded());
        QVERIFY(result.startFailed);
        QVERIFY(request.eventCollector->hasEvent(
            ExternalProcessEventType::FailedToStart));
        QVERIFY(!request.eventCollector->hasEvent(
            ExternalProcessEventType::Started));
    }

    void externalProcessRunnerEventsRecordTimeout()
    {
        ExternalProcessRunner runner;
        ExternalProcessRequest request =
            externalProcessHelperRequest({ "sleep", "1000" });
        request.timeoutMsec = 100;
        request.eventCollector =
            QSharedPointer<ExternalProcessEventCollector>::create();

        const ExternalProcessResult result = runner.run(request);

        QVERIFY(!result.succeeded());
        QVERIFY(result.timedOut);
        QVERIFY(!result.cancelled);
        QVERIFY(request.eventCollector->hasEvent(
            ExternalProcessEventType::Started));
        QVERIFY(request.eventCollector->hasEvent(
            ExternalProcessEventType::TimedOut));
        QVERIFY(!request.eventCollector->hasEvent(
            ExternalProcessEventType::Cancelled));
    }

    void externalProcessRunnerEventsRecordAsyncCancellation()
    {
        ExternalProcessRunner runner;
        ExternalProcessRequest request =
            externalProcessHelperRequest({ "sleep", "3000" });
        request.timeoutMsec = 5000;
        request.eventCollector =
            QSharedPointer<ExternalProcessEventCollector>::create();

        const ExternalProcessRunHandle handle = runner.startAsync(request);

        QVERIFY(waitForAsyncRunToStart(runner, handle));
        QVERIFY(runner.cancel(handle));
        QVERIFY(waitForAsyncRunToFinish(runner, handle));

        const std::optional<ExternalProcessResult> result =
            runner.collectResult(handle);
        QVERIFY(result.has_value());
        QVERIFY(result->cancelled);
        QVERIFY(!result->timedOut);
        QVERIFY(result->state == AnalysisRunState::Cancelled);
        QVERIFY(request.eventCollector->hasEvent(
            ExternalProcessEventType::Started));
        QVERIFY(request.eventCollector->hasEvent(
            ExternalProcessEventType::Cancelled));
        QVERIFY(!request.eventCollector->hasEvent(
            ExternalProcessEventType::TimedOut));
        QVERIFY(runner.cleanup(handle));
    }

    void externalProcessRunnerEventCollectionPreservesResultSemantics()
    {
        ExternalProcessRunner runner;
        ExternalProcessRequest request =
            externalProcessHelperRequest({ "failure" });
        request.eventCollector =
            QSharedPointer<ExternalProcessEventCollector>::create();

        const ExternalProcessResult resultWithEvents = runner.run(request);
        const ExternalProcessResult resultWithoutEvents =
            runner.run(externalProcessHelperRequest({ "failure" }));

        QCOMPARE(resultWithEvents.exitCode, resultWithoutEvents.exitCode);
        QVERIFY(resultWithEvents.state == resultWithoutEvents.state);
        QVERIFY(resultWithEvents.error.code == resultWithoutEvents.error.code);
        QVERIFY(request.eventCollector->hasEvent(
            ExternalProcessEventType::Finished));
    }

    void externalProcessLogFileSinkWritesSuccessfulProcessEvents()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        ExternalProcessRunner runner;
        ExternalProcessRequest request =
            externalProcessHelperRequest({ "success" });
        request.eventCollector =
            QSharedPointer<ExternalProcessEventCollector>::create();

        const ExternalProcessResult result = runner.run(request);
        QVERIFY(result.succeeded());

        const QString path = directory.filePath("external-process.log");
        ExternalProcessLogFileSink sink;
        const ExternalProcessLogFileWriteResult written =
            sink.write(path, *request.eventCollector, result);

        QVERIFY(written.isValid());
        QCOMPARE(written.path, path);
        QCOMPARE(written.eventCount, request.eventCollector->events().size());
        QVERIFY(written.resultSummaryIncluded);
        QVERIFY(written.bytesWritten > 0);

        const QString contents = readTextFile(path);
        QVERIFY(contents.contains("Tony External Process Log"));
        QVERIFY(contents.contains("type=started"));
        QVERIFY(contents.contains("type=finished"));
        QVERIFY(contents.contains("exit_code=0"));
        QVERIFY(contents.contains("Result"));
    }

    void externalProcessLogFileSinkWritesStdoutAndStderrEvents()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        ExternalProcessRunner runner;
        ExternalProcessRequest stdoutRequest =
            externalProcessHelperRequest({ "stdout", "log_stdout" });
        stdoutRequest.eventCollector =
            QSharedPointer<ExternalProcessEventCollector>::create();
        const ExternalProcessResult stdoutResult = runner.run(stdoutRequest);
        QVERIFY(stdoutResult.succeeded());

        ExternalProcessRequest stderrRequest =
            externalProcessHelperRequest({ "stderr", "log_stderr" });
        stderrRequest.eventCollector =
            QSharedPointer<ExternalProcessEventCollector>::create();
        const ExternalProcessResult stderrResult = runner.run(stderrRequest);
        QVERIFY(stderrResult.succeeded());

        QVector<ExternalProcessEvent> events =
            stdoutRequest.eventCollector->events();
        events += stderrRequest.eventCollector->events();

        const QString path = directory.filePath("external-process.log");
        ExternalProcessLogFileSink sink;
        const ExternalProcessLogFileWriteResult written =
            sink.write(path, events);

        QVERIFY(written.isValid());
        QCOMPARE(written.eventCount, events.size());

        const QString contents = readTextFile(path);
        QVERIFY(contents.contains("type=stdout_chunk"));
        QVERIFY(contents.contains("log_stdout"));
        QVERIFY(contents.contains("type=stderr_chunk"));
        QVERIFY(contents.contains("log_stderr"));
        QVERIFY(contents.contains("data_begin"));
        QVERIFY(contents.contains("data_end"));
    }

    void externalProcessLogFileSinkRejectsEmptyPath()
    {
        ExternalProcessLogFileSink sink;
        const ExternalProcessLogFileWriteResult written =
            sink.write(QString(), QVector<ExternalProcessEvent>());

        QVERIFY(!written.isValid());
        QVERIFY(reportHasIssue(written.report, "empty_log_file_path"));
    }

    void externalProcessLogFileSinkRejectsMissingParentDirectory()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString path =
            directory.filePath("missing-parent/external-process.log");
        ExternalProcessLogFileSink sink;
        const ExternalProcessLogFileWriteResult written =
            sink.write(path, QVector<ExternalProcessEvent>());

        QVERIFY(!written.isValid());
        QVERIFY(reportHasIssue(written.report, "parent_directory_missing"));
        QVERIFY(!QFile::exists(path));
    }

    void externalProcessLogFileSinkWritesMinimalEmptyEventLog()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString path = directory.filePath("empty-events.log");
        ExternalProcessLogFileSink sink;
        const ExternalProcessLogFileWriteResult written =
            sink.write(path, QVector<ExternalProcessEvent>());

        QVERIFY(written.isValid());
        QCOMPARE(written.eventCount, 0);
        QVERIFY(written.bytesWritten > 0);

        const QString contents = readTextFile(path);
        QVERIFY(contents.contains("event_count=0"));
        QVERIFY(contents.contains(
            "No external process events were recorded."));
    }

    void externalProcessLogFileSinkRejectsWriteFailure()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        ExternalProcessLogFileSink sink;
        const ExternalProcessLogFileWriteResult written =
            sink.write(directory.path(), QVector<ExternalProcessEvent>());

        QVERIFY(!written.isValid());
        QVERIFY(reportHasIssue(written.report, "file_write_failed"));
    }

    void externalProcessLogFileSinkNeverRunsProcess()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        ExternalProcessEvent event;
        event.type = ExternalProcessEventType::StdoutChunk;
        event.runId = "manual_log_only";
        event.message = "Manual event for log sink test.";
        event.data = "manual_sink_data";

        const QString path = directory.filePath("manual-events.log");
        ExternalProcessLogFileSink sink;
        const ExternalProcessLogFileWriteResult written =
            sink.write(path, QVector<ExternalProcessEvent>({ event }));

        QVERIFY(written.isValid());
        const QString contents = readTextFile(path);
        QVERIFY(contents.contains("manual_log_only"));
        QVERIFY(contents.contains("manual_sink_data"));
        QVERIFY(!contents.contains("--external-process-helper"));
    }

    void externalProcessLogFileSinkNeverMarksBackendReady()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.status = BackendStatus::NotConfigured;

        ExternalProcessEvent event;
        event.type = ExternalProcessEventType::Finished;
        event.runId = "availability_unchanged";
        event.exitCode = 0;

        ExternalProcessLogFileSink sink;
        const ExternalProcessLogFileWriteResult written =
            sink.write(directory.filePath("availability.log"),
                       QVector<ExternalProcessEvent>({ event }));

        QVERIFY(written.isValid());
        QVERIFY(manifest.status == BackendStatus::NotConfigured);
        QVERIFY(manifest.status != BackendStatus::Ready);

        BackendRegistry registry;
        QVERIFY(!registry.hasBackend("basic_pitch"));
        QVERIFY(registry.allManifests().isEmpty());
    }

private:
    static bool writeFile(const QString &path, const QByteArray &contents)
    {
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            return false;
        }
        return file.write(contents) == contents.size();
    }

    static QString readTextFile(const QString &path)
    {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            return QString();
        }
        return QString::fromUtf8(file.readAll());
    }

    static bool makeExecutable(const QString &path)
    {
        return QFile::setPermissions(
            path,
            QFileDevice::ReadOwner |
            QFileDevice::WriteOwner |
            QFileDevice::ExeOwner |
            QFileDevice::ReadUser |
            QFileDevice::WriteUser |
            QFileDevice::ExeUser |
            QFileDevice::ReadGroup |
            QFileDevice::ExeGroup |
            QFileDevice::ReadOther |
            QFileDevice::ExeOther);
    }

    static bool reportHasIssue(const ValidationReport &report,
                               const QString &code)
    {
        for (const auto &issue: report.issues) {
            if (issue.code == code) {
                return true;
            }
        }
        return false;
    }

    static bool reportHasIssueWithSeverity(const ValidationReport &report,
                                           const QString &code,
                                           ValidationSeverity severity)
    {
        for (const auto &issue: report.issues) {
            if (issue.code == code && issue.severity == severity) {
                return true;
            }
        }
        return false;
    }

    static bool eventDataContains(
        const ExternalProcessEventCollector &collector,
        ExternalProcessEventType type,
        const QString &text)
    {
        for (const auto &event: collector.events()) {
            if (event.type == type && event.data.contains(text)) {
                return true;
            }
        }
        return false;
    }

    static ExternalProcessRequest externalProcessHelperRequest(
        const QStringList &helperArguments)
    {
        ExternalProcessRequest request;
        request.executablePath = QCoreApplication::applicationFilePath();
        request.arguments << "--external-process-helper";
        request.arguments << helperArguments;
        request.timeoutMsec = 3000;
        return request;
    }

    static bool waitForAsyncRunToStart(ExternalProcessRunner &runner,
                                       const ExternalProcessRunHandle &handle,
                                       int timeoutMsec = 1000)
    {
        QElapsedTimer elapsed;
        elapsed.start();
        while (elapsed.elapsed() < timeoutMsec) {
            if (runner.isRunning(handle)) {
                return true;
            }
            QTest::qWait(10);
        }
        return runner.isRunning(handle);
    }

    static bool waitForAsyncRunToFinish(ExternalProcessRunner &runner,
                                        const ExternalProcessRunHandle &handle,
                                        int timeoutMsec = 5000)
    {
        QElapsedTimer elapsed;
        elapsed.start();
        while (elapsed.elapsed() < timeoutMsec) {
            if (!runner.isRunning(handle)) {
                return true;
            }
            QTest::qWait(10);
        }
        return !runner.isRunning(handle);
    }

    static BackendAvailabilityReport makeAvailabilityReport(
        const BackendId &backendId,
        BackendAvailabilityProbeStatus status)
    {
        BackendAvailabilityReport report;
        report.backendId = backendId;
        report.status = status;
        return report;
    }

    static BackendRunRequestBuildResult validBackendRunRequest(
        const QString &baseDirectory,
        const std::optional<AnalysisRegion> &region = std::nullopt,
        const std::optional<BackendSettings> &settings = std::nullopt)
    {
        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath =
            QDir(baseDirectory).filePath("manifest-adapter.exe");

        const BackendRunWorkspace workspace =
            BackendRunWorkspace::fromParts(baseDirectory,
                                           manifest.id(),
                                           "run_001");

        BackendRunRequestParameters parameters;
        parameters.inputAudioFilePath =
            QDir(baseDirectory).filePath("input.wav");
        parameters.expectedUnifiedResultJsonPath =
            workspace.unifiedResultJsonPath;
        parameters.selectedRegion = region;
        parameters.timeoutMsec = 3000;

        BackendRunRequestBuilder builder;
        return builder.build(manifest, settings, workspace, parameters);
    }

    static BackendManifest parsedBasicPitchManifest()
    {
        const QJsonDocument document =
            QJsonDocument::fromJson(basicPitchManifestJson());
        BackendManifestParser parser;
        const BackendManifestParseResult parsed =
            parser.parse(document.object());
        return parsed.manifest;
    }

    static QByteArray basicPitchManifestJson()
    {
        return R"json(
{
  "contract_version": "0.1",
  "engine_id": "basic_pitch",
  "display_name": "Basic Pitch",
  "engine_version": null,
  "adapter_version": "0.1.0",
  "category": "note_transcription",
  "runtime": {
    "type": "python_cli",
    "requires_python": true,
    "requires_model_files": false,
    "supports_cpu": true,
    "supports_cuda": false,
    "supports_directml": false,
    "supports_rocm": false,
    "internet_required": false
  },
  "capabilities": {
    "supports_full_file": true,
    "supports_region": true,
    "supports_batch": false,
    "outputs_notes": true,
    "outputs_pitch_curve": false,
    "outputs_pitch_bends": true,
    "outputs_velocity": true,
    "outputs_confidence": true,
    "outputs_technique_labels": false,
    "outputs_warnings": true,
    "can_run_offline": true
  },
  "inputs": {
    "audio_formats": ["wav"],
    "preferred_formats": ["wav"],
    "mono_required": false,
    "max_channels": 2,
    "sample_rates_hz": [44100],
    "requires_resampling": "adapter_or_host"
  },
  "outputs": {
    "primary": ["notes", "midi", "csv_notes"],
    "optional": ["pitch_bends", "warnings", "logs"]
  },
  "settings_schema": {},
  "license": {
    "name": "Apache-2.0",
    "source_url": "https://github.com/spotify/basic-pitch",
    "redistribution_status": "allowed_with_notice",
    "notes": "Review before bundling."
  }
}
)json";
    }
};

#endif
