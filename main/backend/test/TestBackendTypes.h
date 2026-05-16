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

#include "../BackendDiscoveryConfig.h"
#include "../BackendManifestDirectoryLoader.h"
#include "../BackendManifestFileLoader.h"
#include "../BackendManifestParser.h"
#include "../BackendManifestSchemaValidator.h"
#include "../BackendRegistry.h"
#include "../BackendTypes.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QTemporaryDir>
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

private:
    static bool writeFile(const QString &path, const QByteArray &contents)
    {
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            return false;
        }
        return file.write(contents) == contents.size();
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
