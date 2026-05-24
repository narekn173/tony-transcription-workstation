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
#include "../BasicPitchAdapterContract.h"
#include "../BasicPitchArtifactDiscovery.h"
#include "../BasicPitchArtifactToUnifiedResult.h"
#include "../BasicPitchDebugWorkflow.h"
#include "../BasicPitchDebugWorkflowUiModel.h"
#include "../BasicPitchLayerPersistenceExportProof.h"
#include "../BasicPitchOutputConverter.h"
#include "../BasicPitchRealRunHandoffProof.h"
#include "../BasicPitchResultToTonyLayerProof.h"
#include "../BasicPitchUnifiedResultHandoff.h"
#include "../BackendDiscoveryConfig.h"
#include "../BackendDiscoveryService.h"
#include "../BackendExecutableProbe.h"
#include "../BackendManifestDirectoryLoader.h"
#include "../BackendManifestFileLoader.h"
#include "../BackendManifestParser.h"
#include "../BackendManifestSchemaValidator.h"
#include "../BackendRegistry.h"
#include "../BackendRequiredFileProbe.h"
#include "../BackendRunOrchestrator.h"
#include "../BackendRunOutputHandoff.h"
#include "../BackendRunResultLoader.h"
#include "../BackendRunResultReporter.h"
#include "../BackendRunRequestFileWriter.h"
#include "../BackendRunRequestBuilder.h"
#include "../BackendRunRequestPreparer.h"
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
#include "../TonyLayerImporter.h"
#include "../UnifiedResultFileWriter.h"
#include "../UnifiedResultSerializer.h"

#include "data/fileio/CSVFileWriter.h"
#include "data/model/EventCommands.h"
#include "data/model/NoteModel.h"
#include "framework/Document.h"
#include "framework/SVFileReader.h"
#include "layer/FlexiNoteLayer.h"
#include "layer/NoteLayer.h"
#include "view/Pane.h"
#include "widgets/CommandHistory.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QSize>
#include <QTemporaryDir>
#include <QTextStream>
#include <QtTest>

#include <future>
#include <memory>
#include <set>
#include <vector>

using namespace Tony::Backend;

namespace {

class TestSVFileReaderPaneCallback : public sv::SVFileReaderPaneCallback
{
public:
    std::vector<std::unique_ptr<sv::Pane>> panes;
    std::vector<QPair<sv::sv_frame_t, sv::sv_frame_t>> selections;

    sv::Pane *addPane() override
    {
        panes.push_back(std::make_unique<sv::Pane>());
        return panes.back().get();
    }

    void setWindowSize(int width, int height) override
    {
        windowSize = QSize(width, height);
    }

    void addSelection(sv::sv_frame_t start, sv::sv_frame_t end) override
    {
        selections.push_back(qMakePair(start, end));
    }

    QSize windowSize;
};

QString serializeDocumentPaneSessionXml(const sv::Document &document,
                                        const sv::Pane &pane)
{
    QString xml;
    QTextStream stream(&xml);

    stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    stream << "<!DOCTYPE sonic-visualiser>\n";
    stream << "<sv>\n";
    document.toXml(stream, "", "");
    stream << "<display>\n";
    stream << "  <window width=\"800\" height=\"600\"/>\n";
    pane.toXml(stream, "  ");
    stream << "</display>\n";
    stream << "<selections/>\n";
    stream << "</sv>\n";

    return xml;
}

}

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

    void backendRunRequestPreparerPreparesValidRequestFile()
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

        BackendRunRequestPreparationParameters parameters;
        parameters.inputAudioFilePath = directory.filePath("input.wav");
        parameters.expectedUnifiedResultJsonPath =
            workspace.unifiedResultJsonPath;
        parameters.requestJsonFilePath = requestPath;
        parameters.timeoutMsec = 3000;

        BackendRunRequestPreparer preparer;
        const BackendRunRequestPreparationResult prepared =
            preparer.prepare(manifest, workspace, parameters);

        QVERIFY(prepared.isValid());
        QVERIFY(prepared.requestFileWritten);
        QCOMPARE(prepared.requestJsonFilePath, requestPath);
        QVERIFY(prepared.bytesWritten > 0);
        QVERIFY(QFile::exists(requestPath));
        QCOMPARE(prepared.processRequest.runId, QString("run_001"));
        QVERIFY(prepared.debugSummaryString().contains("basic_pitch"));
    }

    void backendRunRequestPreparerReturnsRequestWithRequestFileArgument()
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

        BackendRunRequestPreparationParameters parameters;
        parameters.inputAudioFilePath = directory.filePath("input.wav");
        parameters.expectedUnifiedResultJsonPath =
            workspace.unifiedResultJsonPath;
        parameters.requestJsonFilePath = requestPath;

        BackendRunRequestPreparer preparer;
        const BackendRunRequestPreparationResult prepared =
            preparer.prepare(manifest, workspace, parameters);

        QVERIFY(prepared.isValid());
        const int requestIndex =
            prepared.processRequest.arguments.indexOf("--request");
        QVERIFY(requestIndex >= 0);
        QVERIFY(requestIndex + 1 < prepared.processRequest.arguments.size());
        QCOMPARE(prepared.processRequest.arguments.at(requestIndex + 1),
                 requestPath);

        const QJsonDocument document =
            QJsonDocument::fromJson(readTextFile(requestPath).toUtf8());
        QVERIFY(document.isObject());
        const QJsonArray arguments =
            document.object().value("arguments").toArray();
        QVERIFY(arguments.contains(QJsonValue(QString("--request"))));
        QVERIFY(arguments.contains(QJsonValue(requestPath)));
    }

    void backendRunRequestPreparerPreservesSelectedRegion()
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

        BackendRunRequestPreparationParameters parameters;
        parameters.inputAudioFilePath = directory.filePath("input.wav");
        parameters.expectedUnifiedResultJsonPath =
            workspace.unifiedResultJsonPath;
        parameters.requestJsonFilePath = requestPath;
        parameters.selectedRegion = AnalysisRegion { 1.25, 3.5 };

        BackendRunRequestPreparer preparer;
        const BackendRunRequestPreparationResult prepared =
            preparer.prepare(manifest, workspace, parameters);

        QVERIFY(prepared.isValid());
        QVERIFY(prepared.builtRequest.hasSelectedRegion);
        QVERIFY(prepared.processRequest.arguments.contains("--region-start"));
        QVERIFY(prepared.processRequest.arguments.contains("--region-end"));

        const QJsonDocument document =
            QJsonDocument::fromJson(readTextFile(requestPath).toUtf8());
        QVERIFY(document.isObject());
        const QJsonObject region =
            document.object().value("selected_region").toObject();
        QCOMPARE(region.value("start_sec").toDouble(), 1.25);
        QCOMPARE(region.value("end_sec").toDouble(), 3.5);
    }

    void backendRunRequestPreparerPreservesEnvironmentOverrides()
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
        const QString requestPath = directory.filePath("request.json");

        BackendRunRequestPreparationParameters parameters;
        parameters.inputAudioFilePath = directory.filePath("input.wav");
        parameters.expectedUnifiedResultJsonPath =
            workspace.unifiedResultJsonPath;
        parameters.requestJsonFilePath = requestPath;

        BackendRunRequestPreparer preparer;
        const BackendRunRequestPreparationResult prepared =
            preparer.prepare(manifest, settings, workspace, parameters);

        QVERIFY(prepared.isValid());
        QCOMPARE(prepared.processRequest.environmentOverrides.value(
                     "TONY_BACKEND_MODE"),
                 QString("test"));

        const QJsonDocument document =
            QJsonDocument::fromJson(readTextFile(requestPath).toUtf8());
        QVERIFY(document.isObject());
        const QJsonObject environment =
            document.object().value("environment_overrides").toObject();
        QCOMPARE(environment.value("TONY_BACKEND_MODE").toString(),
                 QString("test"));
        QCOMPARE(environment.value("TONY_BACKEND_TRACE").toString(),
                 QString("1"));
    }

    void backendRunRequestPreparerRejectsEmptyRequestFilePath()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = directory.filePath("manifest-adapter.exe");

        const BackendRunWorkspace workspace =
            BackendRunWorkspace::fromParts(directory.path(),
                                           manifest.id(),
                                           "run_001");

        BackendRunRequestPreparationParameters parameters;
        parameters.inputAudioFilePath = directory.filePath("input.wav");
        parameters.expectedUnifiedResultJsonPath =
            workspace.unifiedResultJsonPath;
        parameters.requestJsonFilePath = " ";

        BackendRunRequestPreparer preparer;
        const BackendRunRequestPreparationResult prepared =
            preparer.prepare(manifest, workspace, parameters);

        QVERIFY(!prepared.isValid());
        QVERIFY(!prepared.requestFileWritten);
        QVERIFY(reportHasIssue(prepared.report,
                               "empty_request_json_file_path"));
    }

    void backendRunRequestPreparerRejectsMissingParentDirectory()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = directory.filePath("manifest-adapter.exe");

        const BackendRunWorkspace workspace =
            BackendRunWorkspace::fromParts(directory.path(),
                                           manifest.id(),
                                           "run_001");
        const QString requestPath =
            directory.filePath("missing-parent/request.json");

        BackendRunRequestPreparationParameters parameters;
        parameters.inputAudioFilePath = directory.filePath("input.wav");
        parameters.expectedUnifiedResultJsonPath =
            workspace.unifiedResultJsonPath;
        parameters.requestJsonFilePath = requestPath;

        BackendRunRequestPreparer preparer;
        const BackendRunRequestPreparationResult prepared =
            preparer.prepare(manifest, workspace, parameters);

        QVERIFY(!prepared.isValid());
        QVERIFY(!prepared.requestFileWritten);
        QVERIFY(reportHasIssue(prepared.report, "parent_directory_missing"));
        QVERIFY(!QFile::exists(requestPath));
        QVERIFY(!QDir(directory.filePath("missing-parent")).exists());
    }

    void backendRunRequestPreparerNeverRunsProcess()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString markerPath = directory.filePath("preparer-ran.marker");
        const QString scriptPath = directory.filePath("preparer-backend.bat");
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

        BackendRunRequestPreparationParameters parameters;
        parameters.inputAudioFilePath = directory.filePath("input.wav");
        parameters.expectedUnifiedResultJsonPath =
            workspace.unifiedResultJsonPath;
        parameters.requestJsonFilePath = directory.filePath("request.json");

        BackendRunRequestPreparer preparer;
        const BackendRunRequestPreparationResult prepared =
            preparer.prepare(manifest, workspace, parameters);

        QVERIFY(prepared.isValid());
        QCOMPARE(prepared.processRequest.executablePath, scriptPath);
        QVERIFY(!QFile::exists(markerPath));
    }

    void backendRunRequestPreparerNeverMarksBackendReady()
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

        BackendRunRequestPreparationParameters parameters;
        parameters.inputAudioFilePath = directory.filePath("input.wav");
        parameters.expectedUnifiedResultJsonPath =
            workspace.unifiedResultJsonPath;
        parameters.requestJsonFilePath = directory.filePath("request.json");

        BackendRunRequestPreparer preparer;
        const BackendRunRequestPreparationResult prepared =
            preparer.prepare(manifest, workspace, parameters);

        QVERIFY(prepared.isValid());
        QVERIFY(manifest.status == BackendStatus::NotConfigured);
        QVERIFY(manifest.status != BackendStatus::Ready);

        BackendRegistry registry;
        QVERIFY(!registry.hasBackend("basic_pitch"));
        QVERIFY(registry.allManifests().isEmpty());
    }

    void backendRunOrchestratorPrepareOnlyCreatesRequestWithoutRunning()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString markerPath = directory.filePath("orchestrator.marker");

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = QCoreApplication::applicationFilePath();

        const BackendRunWorkspace workspace =
            BackendRunWorkspace::fromParts(directory.path(),
                                           manifest.id(),
                                           "run_001");

        BackendRunOrchestrationParameters parameters;
        parameters.inputAudioFilePath = directory.filePath("input.wav");
        parameters.expectedUnifiedResultJsonPath =
            workspace.unifiedResultJsonPath;
        parameters.requestJsonFilePath = directory.filePath("request.json");
        parameters.additionalArguments
            << "--external-process-helper" << "stdout" << markerPath;

        BackendRunOrchestrator orchestrator;
        const BackendRunOrchestrationResult result =
            orchestrator.prepareOnly(manifest, workspace, parameters);

        QVERIFY(result.isValid());
        QVERIFY(result.requestPrepared);
        QVERIFY(!result.processRunAttempted);
        QVERIFY(!result.processResult.has_value());
        QVERIFY(QFile::exists(parameters.requestJsonFilePath));
        QVERIFY(!QFile::exists(markerPath));
        QVERIFY(result.debugSummaryString().contains("prepared=true"));
    }

    void backendRunOrchestratorExplicitRunUsesSafeLocalCommand()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = QCoreApplication::applicationFilePath();

        const BackendRunWorkspace workspace =
            BackendRunWorkspace::fromParts(directory.path(),
                                           manifest.id(),
                                           "run_001");

        BackendRunOrchestrationParameters parameters;
        parameters.prepareWorkspace = true;
        parameters.inputAudioFilePath = directory.filePath("input.wav");
        parameters.expectedUnifiedResultJsonPath =
            workspace.unifiedResultJsonPath;
        parameters.requestJsonFilePath =
            QDir(workspace.runDirectoryPath).filePath("request.json");
        parameters.timeoutMsec = 3000;
        parameters.additionalArguments
            << "--external-process-helper" << "stdout"
            << "orchestrator_stdout";

        BackendRunOrchestrator orchestrator;
        const BackendRunOrchestrationResult result =
            orchestrator.run(manifest, workspace, parameters);

        QVERIFY(result.isValid());
        QVERIFY(result.requestPrepared);
        QVERIFY(result.processRunAttempted);
        QVERIFY(result.processResult.has_value());
        QVERIFY(result.processResult->succeeded());
        QVERIFY(result.processResult->standardOutput
                    .contains("orchestrator_stdout"));
        QVERIFY(result.processRequest.arguments.contains("--request"));
    }

    void backendRunOrchestratorMissingParentRequiresExplicitPrepare()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = QCoreApplication::applicationFilePath();

        const BackendRunWorkspace workspace =
            BackendRunWorkspace::fromParts(directory.path(),
                                           manifest.id(),
                                           "run_001");

        BackendRunOrchestrationParameters parameters;
        parameters.inputAudioFilePath = directory.filePath("input.wav");
        parameters.expectedUnifiedResultJsonPath =
            workspace.unifiedResultJsonPath;
        parameters.requestJsonFilePath =
            QDir(workspace.runDirectoryPath).filePath("request.json");

        BackendRunOrchestrator orchestrator;
        const BackendRunOrchestrationResult missingParent =
            orchestrator.prepareOnly(manifest, workspace, parameters);

        QVERIFY(!missingParent.isValid());
        QVERIFY(reportHasIssue(missingParent.report,
                               "parent_directory_missing"));
        QVERIFY(!QFile::exists(parameters.requestJsonFilePath));

        parameters.prepareWorkspace = true;
        const BackendRunOrchestrationResult prepared =
            orchestrator.prepareOnly(manifest, workspace, parameters);

        QVERIFY(prepared.isValid());
        QVERIFY(prepared.workspacePrepared);
        QVERIFY(prepared.requestPrepared);
        QVERIFY(QFile::exists(parameters.requestJsonFilePath));
    }

    void backendRunOrchestratorPreservesSelectedRegionInRequestJson()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = QCoreApplication::applicationFilePath();

        const BackendRunWorkspace workspace =
            BackendRunWorkspace::fromParts(directory.path(),
                                           manifest.id(),
                                           "run_001");
        const QString requestPath = directory.filePath("request.json");

        BackendRunOrchestrationParameters parameters;
        parameters.inputAudioFilePath = directory.filePath("input.wav");
        parameters.expectedUnifiedResultJsonPath =
            workspace.unifiedResultJsonPath;
        parameters.requestJsonFilePath = requestPath;
        parameters.selectedRegion = AnalysisRegion { 2.5, 4.75 };

        BackendRunOrchestrator orchestrator;
        const BackendRunOrchestrationResult result =
            orchestrator.prepareOnly(manifest, workspace, parameters);

        QVERIFY(result.isValid());

        const QJsonDocument document =
            QJsonDocument::fromJson(readTextFile(requestPath).toUtf8());
        QVERIFY(document.isObject());
        const QJsonObject region =
            document.object().value("selected_region").toObject();
        QCOMPARE(region.value("start_sec").toDouble(), 2.5);
        QCOMPARE(region.value("end_sec").toDouble(), 4.75);
    }

    void backendRunOrchestratorPreservesProcessOutputEventsAndLog()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = QCoreApplication::applicationFilePath();

        const BackendRunWorkspace workspace =
            BackendRunWorkspace::fromParts(directory.path(),
                                           manifest.id(),
                                           "run_001");

        BackendRunOrchestrationParameters parameters;
        parameters.prepareWorkspace = true;
        parameters.writeProcessLog = true;
        parameters.inputAudioFilePath = directory.filePath("input.wav");
        parameters.expectedUnifiedResultJsonPath =
            workspace.unifiedResultJsonPath;
        parameters.requestJsonFilePath =
            QDir(workspace.runDirectoryPath).filePath("request.json");
        parameters.additionalArguments
            << "--external-process-helper" << "stdout-stderr"
            << "orchestrator_stdout" << "orchestrator_stderr";

        BackendRunOrchestrator orchestrator;
        const BackendRunOrchestrationResult result =
            orchestrator.run(manifest, workspace, parameters);

        QVERIFY(result.isValid());
        QVERIFY(result.processResult.has_value());
        QVERIFY(result.processResult->standardOutput
                    .contains("orchestrator_stdout"));
        QVERIFY(result.processResult->standardError
                    .contains("orchestrator_stderr"));
        QVERIFY(!result.processEvents.isEmpty());
        bool sawStdoutEvent = false;
        bool sawStderrEvent = false;
        for (const auto &event: result.processEvents) {
            sawStdoutEvent = sawStdoutEvent ||
                (event.type == ExternalProcessEventType::StdoutChunk &&
                 event.data.contains("orchestrator_stdout"));
            sawStderrEvent = sawStderrEvent ||
                (event.type == ExternalProcessEventType::StderrChunk &&
                 event.data.contains("orchestrator_stderr"));
        }
        QVERIFY(sawStdoutEvent);
        QVERIFY(sawStderrEvent);
        QVERIFY(result.processLogWritten);
        QVERIFY(QFile::exists(workspace.logFilePath));

        const QString logContents = readTextFile(workspace.logFilePath);
        QVERIFY(logContents.contains("orchestrator_stdout"));
        QVERIFY(logContents.contains("orchestrator_stderr"));
    }

    void backendRunOrchestratorReportsNonZeroExit()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = QCoreApplication::applicationFilePath();

        const BackendRunWorkspace workspace =
            BackendRunWorkspace::fromParts(directory.path(),
                                           manifest.id(),
                                           "run_001");

        BackendRunOrchestrationParameters parameters;
        parameters.prepareWorkspace = true;
        parameters.inputAudioFilePath = directory.filePath("input.wav");
        parameters.expectedUnifiedResultJsonPath =
            workspace.unifiedResultJsonPath;
        parameters.requestJsonFilePath =
            QDir(workspace.runDirectoryPath).filePath("request.json");
        parameters.additionalArguments
            << "--external-process-helper" << "failure";

        BackendRunOrchestrator orchestrator;
        const BackendRunOrchestrationResult result =
            orchestrator.run(manifest, workspace, parameters);

        QVERIFY(!result.isValid());
        QVERIFY(result.processRunAttempted);
        QVERIFY(result.processResult.has_value());
        QCOMPARE(result.processResult->exitCode, 7);
        QVERIFY(result.processResult->error.code ==
                BackendErrorCode::ExecutionFailed);
        QVERIFY(reportHasIssue(result.report, "external_process_failed"));
    }

    void backendRunOrchestratorNeverCreatesFakeAnalysisResults()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = QCoreApplication::applicationFilePath();

        const BackendRunWorkspace workspace =
            BackendRunWorkspace::fromParts(directory.path(),
                                           manifest.id(),
                                           "run_001");

        BackendRunOrchestrationParameters parameters;
        parameters.prepareWorkspace = true;
        parameters.inputAudioFilePath = directory.filePath("input.wav");
        parameters.expectedUnifiedResultJsonPath =
            workspace.unifiedResultJsonPath;
        parameters.requestJsonFilePath =
            QDir(workspace.runDirectoryPath).filePath("request.json");
        parameters.additionalArguments
            << "--external-process-helper" << "success";

        BackendRunOrchestrator orchestrator;
        const BackendRunOrchestrationResult result =
            orchestrator.run(manifest, workspace, parameters);

        QVERIFY(result.isValid());
        QVERIFY(result.processResult.has_value());
        QVERIFY(result.processResult->succeeded());
        QVERIFY(!result.resultJsonExists);
        QVERIFY(!QFile::exists(workspace.unifiedResultJsonPath));
    }

    void backendRunOrchestratorNeverImportsIntoTonyLayers()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = QCoreApplication::applicationFilePath();

        const BackendRunWorkspace workspace =
            BackendRunWorkspace::fromParts(directory.path(),
                                           manifest.id(),
                                           "run_001");

        BackendRunOrchestrationParameters parameters;
        parameters.inputAudioFilePath = directory.filePath("input.wav");
        parameters.expectedUnifiedResultJsonPath =
            workspace.unifiedResultJsonPath;
        parameters.requestJsonFilePath = directory.filePath("request.json");

        BackendRunOrchestrator orchestrator;
        const BackendRunOrchestrationResult result =
            orchestrator.prepareOnly(manifest, workspace, parameters);

        QVERIFY(result.isValid());
        QVERIFY(!result.importedIntoTonyLayers);
    }

    void backendRunOrchestratorNeverMarksBackendReady()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = QCoreApplication::applicationFilePath();
        manifest.status = BackendStatus::NotConfigured;

        const BackendRunWorkspace workspace =
            BackendRunWorkspace::fromParts(directory.path(),
                                           manifest.id(),
                                           "run_001");

        BackendRunOrchestrationParameters parameters;
        parameters.inputAudioFilePath = directory.filePath("input.wav");
        parameters.expectedUnifiedResultJsonPath =
            workspace.unifiedResultJsonPath;
        parameters.requestJsonFilePath = directory.filePath("request.json");

        BackendRunOrchestrator orchestrator;
        const BackendRunOrchestrationResult result =
            orchestrator.prepareOnly(manifest, workspace, parameters);

        QVERIFY(result.isValid());
        QVERIFY(manifest.status == BackendStatus::NotConfigured);
        QVERIFY(manifest.status != BackendStatus::Ready);

        BackendRegistry registry;
        QVERIFY(!registry.hasBackend("basic_pitch"));
        QVERIFY(registry.allManifests().isEmpty());
    }

    void backendRunOutputHandoffAcceptsReadableResultFile()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString resultPath = directory.filePath("result.json");
        QVERIFY(writeFile(resultPath,
                          QByteArray("{\"contract_version\":\"0.1\"}\n")));

        BackendRunOutputHandoff handoff;
        const BackendRunOutputHandoffResult result =
            handoff.inspect(resultPath);

        QVERIFY(result.isValid());
        QCOMPARE(result.outputPath, resultPath);
        QVERIFY(result.exists);
        QVERIFY(result.isFile);
        QVERIFY(result.readable);
        QVERIFY(result.nonEmpty);
        QVERIFY(result.fileSizeBytes > 0);
        QVERIFY(!result.importedIntoTonyLayers);
        QVERIFY(result.debugSummaryString().contains("valid=true"));
    }

    void backendRunOutputHandoffReportsMissingResultFile()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString resultPath = directory.filePath("missing-result.json");
        QVERIFY(!QFile::exists(resultPath));

        BackendRunOutputHandoff handoff;
        const BackendRunOutputHandoffResult result =
            handoff.inspect(resultPath);

        QVERIFY(!result.isValid());
        QVERIFY(!result.exists);
        QVERIFY(reportHasIssue(result.report, "output_file_missing"));
    }

    void backendRunOutputHandoffRejectsEmptyResultPath()
    {
        BackendRunOutputHandoff handoff;
        const BackendRunOutputHandoffResult result = handoff.inspect(" ");

        QVERIFY(!result.isValid());
        QVERIFY(result.outputPath.isEmpty());
        QVERIFY(reportHasIssue(result.report, "empty_output_result_path"));
    }

    void backendRunOutputHandoffRejectsDirectoryPath()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendRunOutputHandoff handoff;
        const BackendRunOutputHandoffResult result =
            handoff.inspect(directory.path());

        QVERIFY(!result.isValid());
        QVERIFY(result.exists);
        QVERIFY(!result.isFile);
        QVERIFY(reportHasIssue(result.report, "output_path_is_directory"));
    }

    void backendRunOutputHandoffRejectsEmptyResultFile()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString resultPath = directory.filePath("empty-result.json");
        QVERIFY(writeFile(resultPath, QByteArray()));

        BackendRunOutputHandoff handoff;
        const BackendRunOutputHandoffResult result =
            handoff.inspect(resultPath);

        QVERIFY(!result.isValid());
        QVERIFY(result.exists);
        QVERIFY(result.isFile);
        QVERIFY(!result.nonEmpty);
        QCOMPARE(result.fileSizeBytes, qint64(0));
        QVERIFY(reportHasIssue(result.report, "empty_output_file"));
    }

    void backendRunOutputHandoffNeverCreatesFakeResultFiles()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString resultPath = directory.filePath("not-created.json");
        QVERIFY(!QFile::exists(resultPath));

        BackendRunOutputHandoff handoff;
        const BackendRunOutputHandoffResult result =
            handoff.inspect(resultPath);

        QVERIFY(!result.isValid());
        QVERIFY(!QFile::exists(resultPath));
        QVERIFY(reportHasIssue(result.report, "output_file_missing"));
    }

    void backendRunOutputHandoffNeverImportsIntoTonyLayers()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString resultPath = directory.filePath("result.json");
        QVERIFY(writeFile(resultPath,
                          QByteArray("{\"contract_version\":\"0.1\"}\n")));

        BackendRunOutputHandoff handoff;
        const BackendRunOutputHandoffResult result =
            handoff.inspect(resultPath);

        QVERIFY(result.isValid());
        QVERIFY(!result.importedIntoTonyLayers);
    }

    void backendRunOutputHandoffNeverMarksBackendReady()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.status = BackendStatus::NotConfigured;

        const QString resultPath = directory.filePath("result.json");
        QVERIFY(writeFile(resultPath,
                          QByteArray("{\"contract_version\":\"0.1\"}\n")));

        BackendRunOutputHandoff handoff;
        const BackendRunOutputHandoffResult result =
            handoff.inspect(resultPath);

        QVERIFY(result.isValid());
        QVERIFY(manifest.status == BackendStatus::NotConfigured);
        QVERIFY(manifest.status != BackendStatus::Ready);

        BackendRegistry registry;
        QVERIFY(!registry.hasBackend("basic_pitch"));
        QVERIFY(registry.allManifests().isEmpty());
    }

    void backendRunResultLoaderLoadsValidResultJson()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString resultPath = directory.filePath("valid-result.json");
        QVERIFY(writeFile(resultPath, validBackendRunResultJson()));

        BackendRunResultLoader loader;
        const BackendRunResultLoadResult loaded = loader.load(resultPath);

        QVERIFY(loaded.isValid());
        QVERIFY(loaded.handoffAccepted);
        QVERIFY(loaded.fileLoaded);
        QVERIFY(loaded.loadedResult.has_value());
        QCOMPARE(loaded.loadedResult->resultId,
                 QString("res_backend_run_loader_001"));
        QCOMPARE(loaded.loadedResult->engine.engineId,
                 QString("basic_pitch"));
        QCOMPARE(loaded.loadedResult->notes.size(), 1);
        QVERIFY(!loaded.importedIntoTonyLayers);
        QVERIFY(loaded.debugSummaryString().contains("valid=true"));
    }

    void backendRunResultLoaderReportsMissingResultJson()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString resultPath = directory.filePath("missing-result.json");
        QVERIFY(!QFile::exists(resultPath));

        BackendRunResultLoader loader;
        const BackendRunResultLoadResult loaded = loader.load(resultPath);

        QVERIFY(!loaded.isValid());
        QVERIFY(!loaded.handoffAccepted);
        QVERIFY(!loaded.fileLoaded);
        QVERIFY(!loaded.loadedResult.has_value());
        QVERIFY(reportHasIssue(loaded.report, "output_file_missing"));
    }

    void backendRunResultLoaderReportsEmptyResultJson()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString resultPath = directory.filePath("empty-result.json");
        QVERIFY(writeFile(resultPath, QByteArray()));

        BackendRunResultLoader loader;
        const BackendRunResultLoadResult loaded = loader.load(resultPath);

        QVERIFY(!loaded.isValid());
        QVERIFY(!loaded.handoffAccepted);
        QVERIFY(!loaded.fileLoaded);
        QVERIFY(!loaded.loadedResult.has_value());
        QVERIFY(reportHasIssue(loaded.report, "empty_output_file"));
    }

    void backendRunResultLoaderReportsInvalidJson()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString resultPath = directory.filePath("invalid-result.json");
        QVERIFY(writeFile(resultPath, QByteArray("{ invalid json")));

        BackendRunResultLoader loader;
        const BackendRunResultLoadResult loaded = loader.load(resultPath);

        QVERIFY(!loaded.isValid());
        QVERIFY(loaded.handoffAccepted);
        QVERIFY(!loaded.fileLoaded);
        QVERIFY(!loaded.loadedResult.has_value());
        QVERIFY(reportHasIssue(loaded.report, "invalid_json"));
    }

    void backendRunResultLoaderReportsInvalidUnifiedResultStructure()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString resultPath =
            directory.filePath("invalid-structure-result.json");
        QVERIFY(writeFile(resultPath, QByteArray("{}")));

        BackendRunResultLoader loader;
        const BackendRunResultLoadResult loaded = loader.load(resultPath);

        QVERIFY(!loaded.isValid());
        QVERIFY(loaded.handoffAccepted);
        QVERIFY(!loaded.fileLoaded);
        QVERIFY(!loaded.loadedResult.has_value());
        QVERIFY(reportHasIssue(loaded.report, "missing_result_id"));
        QVERIFY(reportHasIssue(loaded.report, "invalid_status"));
    }

    void backendRunResultLoaderExposesResultOnlyOnSuccess()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString validPath = directory.filePath("valid-result.json");
        QVERIFY(writeFile(validPath, validBackendRunResultJson()));

        const QString invalidPath = directory.filePath("invalid-result.json");
        QVERIFY(writeFile(invalidPath, QByteArray("{ invalid json")));

        BackendRunResultLoader loader;
        const BackendRunResultLoadResult validLoad = loader.load(validPath);
        const BackendRunResultLoadResult invalidLoad = loader.load(invalidPath);

        QVERIFY(validLoad.isValid());
        QVERIFY(validLoad.loadedResult.has_value());
        QVERIFY(!invalidLoad.isValid());
        QVERIFY(!invalidLoad.loadedResult.has_value());
    }

    void backendRunResultLoaderNeverCreatesFakeResultFiles()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString resultPath = directory.filePath("not-created.json");
        QVERIFY(!QFile::exists(resultPath));

        BackendRunResultLoader loader;
        const BackendRunResultLoadResult loaded = loader.load(resultPath);

        QVERIFY(!loaded.isValid());
        QVERIFY(!QFile::exists(resultPath));
        QVERIFY(!loaded.loadedResult.has_value());
    }

    void backendRunResultLoaderNeverImportsIntoTonyLayers()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString resultPath = directory.filePath("valid-result.json");
        QVERIFY(writeFile(resultPath, validBackendRunResultJson()));

        BackendRunResultLoader loader;
        const BackendRunResultLoadResult loaded = loader.load(resultPath);

        QVERIFY(loaded.isValid());
        QVERIFY(!loaded.importedIntoTonyLayers);
    }

    void backendRunResultLoaderNeverMarksBackendReady()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.status = BackendStatus::NotConfigured;

        const QString resultPath = directory.filePath("valid-result.json");
        QVERIFY(writeFile(resultPath, validBackendRunResultJson()));

        BackendRunResultLoader loader;
        const BackendRunResultLoadResult loaded = loader.load(resultPath);

        QVERIFY(loaded.isValid());
        QVERIFY(manifest.status == BackendStatus::NotConfigured);
        QVERIFY(manifest.status != BackendStatus::Ready);

        BackendRegistry registry;
        QVERIFY(!registry.hasBackend("basic_pitch"));
        QVERIFY(registry.allManifests().isEmpty());
    }

    void backendRunResultReporterReportsSuccessfulLoadedResult()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString resultPath = directory.filePath("valid-result.json");
        QVERIFY(writeFile(resultPath, validBackendRunResultJson()));

        const ExternalProcessResult process = successfulProcessResult();
        BackendRunResultLoader loader;
        const BackendRunResultLoadResult loaded =
            loader.load(resultPath, process);

        BackendRunResultReporter reporter;
        const BackendRunResultReport report =
            reporter.buildReport("basic_pitch", process, resultPath, loaded);

        QVERIFY(report.isValid());
        QCOMPARE(report.backendId, QString("basic_pitch"));
        QVERIFY(report.processResultAvailable);
        QVERIFY(report.processSucceeded);
        QVERIFY(!report.processFailed);
        QVERIFY(report.outputFilePresent);
        QVERIFY(!report.outputFileMissing);
        QVERIFY(!report.outputFileEmpty);
        QVERIFY(!report.outputFileInvalid);
        QVERIFY(report.unifiedResultLoaded);
        QVERIFY(!report.importedIntoTonyLayers);
        QVERIFY(report.errors.isEmpty());
        QVERIFY(report.debugSummaryString().contains("valid=true"));
    }

    void backendRunResultReporterReportsMissingResultAfterSuccessfulProcess()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString resultPath = directory.filePath("missing-result.json");
        QVERIFY(!QFile::exists(resultPath));

        const ExternalProcessResult process = successfulProcessResult();
        BackendRunResultLoader loader;
        const BackendRunResultLoadResult loaded =
            loader.load(resultPath, process);

        BackendRunResultReporter reporter;
        const BackendRunResultReport report =
            reporter.buildReport("basic_pitch", process, resultPath, loaded);

        QVERIFY(!report.isValid());
        QVERIFY(report.processSucceeded);
        QVERIFY(!report.outputFilePresent);
        QVERIFY(report.outputFileMissing);
        QVERIFY(!report.unifiedResultLoaded);
        QVERIFY(report.errors.contains("output_file_missing"));
        QVERIFY(!QFile::exists(resultPath));
    }

    void backendRunResultReporterReportsNonZeroProcessExit()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString resultPath = directory.filePath("valid-result.json");
        QVERIFY(writeFile(resultPath, validBackendRunResultJson()));

        const ExternalProcessResult process = failedProcessResult();
        BackendRunResultLoader loader;
        const BackendRunResultLoadResult loaded =
            loader.load(resultPath, process);

        BackendRunResultReporter reporter;
        const BackendRunResultReport report =
            reporter.buildReport("basic_pitch", process, resultPath, loaded);

        QVERIFY(!report.isValid());
        QVERIFY(report.processResultAvailable);
        QVERIFY(!report.processSucceeded);
        QVERIFY(report.processFailed);
        QVERIFY(!report.processTimedOut);
        QVERIFY(!report.processCancelled);
        QVERIFY(report.outputFilePresent);
        QVERIFY(!report.unifiedResultLoaded);
        QVERIFY(report.errors.contains("process_failed"));
        QVERIFY(report.errors.contains("external_process_failed"));
    }

    void backendRunResultReporterReportsTimeoutAndCancellationStatus()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString timeoutPath = directory.filePath("timeout-result.json");
        const QString cancelPath = directory.filePath("cancel-result.json");
        QVERIFY(writeFile(timeoutPath, validBackendRunResultJson()));
        QVERIFY(writeFile(cancelPath, validBackendRunResultJson()));

        BackendRunResultLoader loader;
        BackendRunResultReporter reporter;

        const ExternalProcessResult timedOut = timedOutProcessResult();
        const BackendRunResultLoadResult timeoutLoad =
            loader.load(timeoutPath, timedOut);
        const BackendRunResultReport timeoutReport =
            reporter.buildReport("basic_pitch",
                                 timedOut,
                                 timeoutPath,
                                 timeoutLoad);

        QVERIFY(!timeoutReport.isValid());
        QVERIFY(timeoutReport.processTimedOut);
        QVERIFY(!timeoutReport.processCancelled);
        QVERIFY(timeoutReport.errors.contains("process_timed_out"));

        const ExternalProcessResult cancelled = cancelledProcessResult();
        const BackendRunResultLoadResult cancelLoad =
            loader.load(cancelPath, cancelled);
        const BackendRunResultReport cancelReport =
            reporter.buildReport("basic_pitch",
                                 cancelled,
                                 cancelPath,
                                 cancelLoad);

        QVERIFY(!cancelReport.isValid());
        QVERIFY(cancelReport.processCancelled);
        QVERIFY(!cancelReport.processTimedOut);
        QVERIFY(cancelReport.errors.contains("process_cancelled"));
    }

    void backendRunResultReporterReportsInvalidUnifiedResult()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString resultPath = directory.filePath("invalid-result.json");
        QVERIFY(writeFile(resultPath, QByteArray("{}")));

        const ExternalProcessResult process = successfulProcessResult();
        BackendRunResultLoader loader;
        const BackendRunResultLoadResult loaded =
            loader.load(resultPath, process);

        BackendRunResultReporter reporter;
        const BackendRunResultReport report =
            reporter.buildReport("basic_pitch", process, resultPath, loaded);

        QVERIFY(!report.isValid());
        QVERIFY(report.processSucceeded);
        QVERIFY(report.outputFilePresent);
        QVERIFY(report.outputFileInvalid);
        QVERIFY(!report.unifiedResultLoaded);
        QVERIFY(report.errors.contains("missing_result_id"));
        QVERIFY(report.errors.contains("invalid_status"));
    }

    void backendRunResultReporterNeverCreatesFakeResultFiles()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString resultPath = directory.filePath("not-created.json");
        QVERIFY(!QFile::exists(resultPath));

        BackendRunResultLoader loader;
        const BackendRunResultLoadResult loaded =
            loader.load(resultPath, successfulProcessResult());

        BackendRunResultReporter reporter;
        const BackendRunResultReport report =
            reporter.buildReport("basic_pitch",
                                 successfulProcessResult(),
                                 resultPath,
                                 loaded);

        QVERIFY(!report.isValid());
        QVERIFY(!report.unifiedResultLoaded);
        QVERIFY(!QFile::exists(resultPath));
        QVERIFY(report.errors.contains("output_file_missing"));
    }

    void backendRunResultReporterNeverImportsIntoTonyLayers()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString resultPath = directory.filePath("valid-result.json");
        QVERIFY(writeFile(resultPath, validBackendRunResultJson()));

        BackendRunResultLoader loader;
        const BackendRunResultLoadResult loaded =
            loader.load(resultPath, successfulProcessResult());

        BackendRunResultReporter reporter;
        const BackendRunResultReport report =
            reporter.buildReport("basic_pitch",
                                 successfulProcessResult(),
                                 resultPath,
                                 loaded);

        QVERIFY(report.isValid());
        QVERIFY(!report.importedIntoTonyLayers);
        QVERIFY(!report.loadResult.importedIntoTonyLayers);
    }

    void backendRunResultReporterNeverMarksBackendReady()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.status = BackendStatus::NotConfigured;

        const QString resultPath = directory.filePath("valid-result.json");
        QVERIFY(writeFile(resultPath, validBackendRunResultJson()));

        BackendRunResultLoader loader;
        const BackendRunResultLoadResult loaded =
            loader.load(resultPath, successfulProcessResult());

        BackendRunResultReporter reporter;
        const BackendRunResultReport report =
            reporter.buildReport(manifest.id(),
                                 successfulProcessResult(),
                                 resultPath,
                                 loaded);

        QVERIFY(report.isValid());
        QVERIFY(manifest.status == BackendStatus::NotConfigured);
        QVERIFY(manifest.status != BackendStatus::Ready);

        BackendRegistry registry;
        QVERIFY(!registry.hasBackend("basic_pitch"));
        QVERIFY(registry.allManifests().isEmpty());
    }

    void backendRunResultReporterConnectsPrepareOnlyOrchestratorResult()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = QCoreApplication::applicationFilePath();

        const BackendRunWorkspace workspace =
            BackendRunWorkspace::fromParts(directory.path(),
                                           manifest.id(),
                                           "run_001");

        BackendRunOrchestrationParameters parameters;
        parameters.inputAudioFilePath = directory.filePath("input.wav");
        parameters.expectedUnifiedResultJsonPath =
            workspace.unifiedResultJsonPath;
        parameters.requestJsonFilePath = directory.filePath("request.json");

        BackendRunOrchestrator orchestrator;
        const BackendRunOrchestrationResult runResult =
            orchestrator.prepareOnly(manifest, workspace, parameters);

        BackendRunResultLoader loader;
        const BackendRunResultLoadResult loaded =
            loader.load(runResult.expectedUnifiedResultJsonPath,
                        runResult.processResult);

        BackendRunResultReporter reporter;
        const BackendRunResultReport report =
            reporter.buildReport(manifest.id(),
                                 runResult,
                                 runResult.expectedUnifiedResultJsonPath,
                                 loaded);

        QVERIFY(runResult.isValid());
        QVERIFY(runResult.requestPrepared);
        QVERIFY(!runResult.processRunAttempted);
        QVERIFY(!report.processResultAvailable);
        QVERIFY(!report.processSucceeded);
        QVERIFY(report.outputFileMissing);
        QVERIFY(!report.unifiedResultLoaded);
        QVERIFY(!report.importedIntoTonyLayers);
        QVERIFY(report.errors.contains("output_file_missing"));
    }

    void backendRunResultReporterConnectsExplicitRunWithLoadedResult()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = QCoreApplication::applicationFilePath();

        const BackendRunWorkspace workspace =
            BackendRunWorkspace::fromParts(directory.path(),
                                           manifest.id(),
                                           "run_001");

        BackendRunOrchestrationParameters parameters;
        parameters.prepareWorkspace = true;
        parameters.inputAudioFilePath = directory.filePath("input.wav");
        parameters.expectedUnifiedResultJsonPath =
            workspace.unifiedResultJsonPath;
        parameters.requestJsonFilePath =
            QDir(workspace.runDirectoryPath).filePath("request.json");
        parameters.timeoutMsec = 3000;
        parameters.additionalArguments
            << "--external-process-helper" << "success";

        BackendRunOrchestrator orchestrator;
        const BackendRunOrchestrationResult runResult =
            orchestrator.run(manifest, workspace, parameters);
        QVERIFY(runResult.isValid());
        QVERIFY(runResult.processResult.has_value());
        QVERIFY(runResult.processResult->succeeded());

        QVERIFY(writeFile(runResult.expectedUnifiedResultJsonPath,
                          validBackendRunResultJson()));

        BackendRunResultLoader loader;
        const BackendRunResultLoadResult loaded =
            loader.load(runResult.expectedUnifiedResultJsonPath,
                        runResult.processResult);

        BackendRunResultReporter reporter;
        const BackendRunResultReport report =
            reporter.buildReport(manifest.id(),
                                 runResult,
                                 runResult.expectedUnifiedResultJsonPath,
                                 loaded);

        QVERIFY(report.isValid());
        QVERIFY(report.processResultAvailable);
        QVERIFY(report.processSucceeded);
        QVERIFY(report.outputFilePresent);
        QVERIFY(!report.outputFileMissing);
        QVERIFY(report.unifiedResultLoaded);
        QVERIFY(!report.importedIntoTonyLayers);
        QVERIFY(report.errors.isEmpty());
    }

    void backendRunResultReporterConnectsExplicitRunWithMissingResult()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = QCoreApplication::applicationFilePath();

        const BackendRunWorkspace workspace =
            BackendRunWorkspace::fromParts(directory.path(),
                                           manifest.id(),
                                           "run_001");

        BackendRunOrchestrationParameters parameters;
        parameters.prepareWorkspace = true;
        parameters.inputAudioFilePath = directory.filePath("input.wav");
        parameters.expectedUnifiedResultJsonPath =
            workspace.unifiedResultJsonPath;
        parameters.requestJsonFilePath =
            QDir(workspace.runDirectoryPath).filePath("request.json");
        parameters.timeoutMsec = 3000;
        parameters.additionalArguments
            << "--external-process-helper" << "success";

        BackendRunOrchestrator orchestrator;
        const BackendRunOrchestrationResult runResult =
            orchestrator.run(manifest, workspace, parameters);
        QVERIFY(runResult.isValid());
        QVERIFY(runResult.processResult.has_value());
        QVERIFY(runResult.processResult->succeeded());
        QVERIFY(!QFile::exists(runResult.expectedUnifiedResultJsonPath));

        BackendRunResultLoader loader;
        const BackendRunResultLoadResult loaded =
            loader.load(runResult.expectedUnifiedResultJsonPath,
                        runResult.processResult);

        BackendRunResultReporter reporter;
        const BackendRunResultReport report =
            reporter.buildReport(manifest.id(),
                                 runResult,
                                 runResult.expectedUnifiedResultJsonPath,
                                 loaded);

        QVERIFY(!report.isValid());
        QVERIFY(report.processSucceeded);
        QVERIFY(report.outputFileMissing);
        QVERIFY(!report.unifiedResultLoaded);
        QVERIFY(report.errors.contains("output_file_missing"));
    }

    void backendRunResultReporterConnectsNonZeroOrchestratorExit()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.executablePath = QCoreApplication::applicationFilePath();

        const BackendRunWorkspace workspace =
            BackendRunWorkspace::fromParts(directory.path(),
                                           manifest.id(),
                                           "run_001");

        BackendRunOrchestrationParameters parameters;
        parameters.prepareWorkspace = true;
        parameters.inputAudioFilePath = directory.filePath("input.wav");
        parameters.expectedUnifiedResultJsonPath =
            workspace.unifiedResultJsonPath;
        parameters.requestJsonFilePath =
            QDir(workspace.runDirectoryPath).filePath("request.json");
        parameters.timeoutMsec = 3000;
        parameters.additionalArguments
            << "--external-process-helper" << "failure";

        BackendRunOrchestrator orchestrator;
        const BackendRunOrchestrationResult runResult =
            orchestrator.run(manifest, workspace, parameters);
        QVERIFY(!runResult.isValid());
        QVERIFY(runResult.processResult.has_value());
        QCOMPARE(runResult.processResult->exitCode, 7);

        BackendRunResultLoader loader;
        const BackendRunResultLoadResult loaded =
            loader.load(runResult.expectedUnifiedResultJsonPath,
                        runResult.processResult);

        BackendRunResultReporter reporter;
        const BackendRunResultReport report =
            reporter.buildReport(manifest.id(),
                                 runResult,
                                 runResult.expectedUnifiedResultJsonPath,
                                 loaded);

        QVERIFY(!report.isValid());
        QVERIFY(report.processResultAvailable);
        QVERIFY(report.processFailed);
        QVERIFY(!report.processTimedOut);
        QVERIFY(!report.processCancelled);
        QVERIFY(report.errors.contains("process_failed"));
        QVERIFY(report.errors.contains("external_process_failed"));
    }

    void backendRunResultReporterKeepsTimeoutAndCancelDistinctFromOrchestrator()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString timeoutPath = directory.filePath("timeout-result.json");
        const QString cancelPath = directory.filePath("cancel-result.json");
        QVERIFY(writeFile(timeoutPath, validBackendRunResultJson()));
        QVERIFY(writeFile(cancelPath, validBackendRunResultJson()));

        BackendRunOrchestrationResult timeoutRun;
        timeoutRun.expectedUnifiedResultJsonPath = timeoutPath;
        timeoutRun.processRunAttempted = true;
        timeoutRun.processResult = timedOutProcessResult();

        BackendRunOrchestrationResult cancelRun;
        cancelRun.expectedUnifiedResultJsonPath = cancelPath;
        cancelRun.processRunAttempted = true;
        cancelRun.processResult = cancelledProcessResult();

        BackendRunResultLoader loader;
        BackendRunResultReporter reporter;

        const BackendRunResultLoadResult timeoutLoad =
            loader.load(timeoutPath, timeoutRun.processResult);
        const BackendRunResultReport timeoutReport =
            reporter.buildReport("basic_pitch",
                                 timeoutRun,
                                 timeoutPath,
                                 timeoutLoad);

        QVERIFY(!timeoutReport.isValid());
        QVERIFY(timeoutReport.processTimedOut);
        QVERIFY(!timeoutReport.processCancelled);
        QVERIFY(timeoutReport.errors.contains("process_timed_out"));

        const BackendRunResultLoadResult cancelLoad =
            loader.load(cancelPath, cancelRun.processResult);
        const BackendRunResultReport cancelReport =
            reporter.buildReport("basic_pitch",
                                 cancelRun,
                                 cancelPath,
                                 cancelLoad);

        QVERIFY(!cancelReport.isValid());
        QVERIFY(cancelReport.processCancelled);
        QVERIFY(!cancelReport.processTimedOut);
        QVERIFY(cancelReport.errors.contains("process_cancelled"));
    }

    void backendRunResultReporterOrchestratorBridgeNeverCreatesResultFiles()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendRunOrchestrationResult runResult;
        runResult.expectedUnifiedResultJsonPath =
            directory.filePath("not-created-result.json");
        QVERIFY(!QFile::exists(runResult.expectedUnifiedResultJsonPath));

        BackendRunResultLoader loader;
        const BackendRunResultLoadResult loaded =
            loader.load(runResult.expectedUnifiedResultJsonPath,
                        runResult.processResult);

        BackendRunResultReporter reporter;
        const BackendRunResultReport report =
            reporter.buildReport("basic_pitch",
                                 runResult,
                                 runResult.expectedUnifiedResultJsonPath,
                                 loaded);

        QVERIFY(!report.isValid());
        QVERIFY(report.outputFileMissing);
        QVERIFY(!QFile::exists(runResult.expectedUnifiedResultJsonPath));
    }

    void backendRunResultReporterOrchestratorBridgeNeverImportsIntoTonyLayers()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendRunOrchestrationResult runResult;
        runResult.expectedUnifiedResultJsonPath =
            directory.filePath("valid-result.json");
        runResult.processResult = successfulProcessResult();
        QVERIFY(writeFile(runResult.expectedUnifiedResultJsonPath,
                          validBackendRunResultJson()));

        BackendRunResultLoader loader;
        const BackendRunResultLoadResult loaded =
            loader.load(runResult.expectedUnifiedResultJsonPath,
                        runResult.processResult);

        BackendRunResultReporter reporter;
        const BackendRunResultReport report =
            reporter.buildReport("basic_pitch",
                                 runResult,
                                 runResult.expectedUnifiedResultJsonPath,
                                 loaded);

        QVERIFY(report.isValid());
        QVERIFY(!report.importedIntoTonyLayers);
        QVERIFY(!report.loadResult.importedIntoTonyLayers);
    }

    void backendRunResultReporterOrchestratorBridgeNeverMarksBackendReady()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.status = BackendStatus::NotConfigured;

        BackendRunOrchestrationResult runResult;
        runResult.expectedUnifiedResultJsonPath =
            directory.filePath("valid-result.json");
        runResult.processResult = successfulProcessResult();
        QVERIFY(writeFile(runResult.expectedUnifiedResultJsonPath,
                          validBackendRunResultJson()));

        BackendRunResultLoader loader;
        const BackendRunResultLoadResult loaded =
            loader.load(runResult.expectedUnifiedResultJsonPath,
                        runResult.processResult);

        BackendRunResultReporter reporter;
        const BackendRunResultReport report =
            reporter.buildReport(manifest.id(),
                                 runResult,
                                 runResult.expectedUnifiedResultJsonPath,
                                 loaded);

        QVERIFY(report.isValid());
        QVERIFY(manifest.status == BackendStatus::NotConfigured);
        QVERIFY(manifest.status != BackendStatus::Ready);

        BackendRegistry registry;
        QVERIFY(!registry.hasBackend("basic_pitch"));
        QVERIFY(registry.allManifests().isEmpty());
    }

    void devMockBackendEndToEndProofCreatesLoadsAndReportsResult()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendManifest manifest = devMockBackendManifest();
        BackendSettings settings;
        settings.backendId = manifest.id();
        settings.enabled = true;
        settings.environmentVariables.insert("TONY_DEV_MOCK_TEST", "1");

        const BackendRunWorkspace workspace =
            BackendRunWorkspace::fromParts(directory.path(),
                                           manifest.id(),
                                           "dev_mock_run_001");

        BackendRunOrchestrationParameters parameters;
        parameters.inputAudioFilePath =
            QDir(directory.path()).filePath("dev-mock-input.wav");
        parameters.selectedRegion = AnalysisRegion{ 0.5, 1.25 };
        parameters.expectedUnifiedResultJsonPath =
            workspace.unifiedResultJsonPath;
        parameters.requestJsonFilePath =
            QDir(workspace.runDirectoryPath).filePath("request.json");
        parameters.timeoutMsec = 3000;
        parameters.prepareWorkspace = true;
        parameters.additionalArguments << "--dev-mock-backend";

        BackendRunOrchestrator orchestrator;
        const BackendRunOrchestrationResult runResult =
            orchestrator.run(manifest, settings, workspace, parameters);

        QVERIFY(runResult.isValid());
        QVERIFY(runResult.workspacePrepared);
        QVERIFY(runResult.requestPrepared);
        QVERIFY(runResult.processRunAttempted);
        QVERIFY(runResult.processResult.has_value());
        QVERIFY(runResult.processResult->succeeded());
        QVERIFY(runResult.processResult->standardOutput.contains(
            "wrote test-only result"));
        QVERIFY(QFile::exists(parameters.requestJsonFilePath));
        QVERIFY(QFile::exists(parameters.expectedUnifiedResultJsonPath));
        QVERIFY(QFileInfo(parameters.expectedUnifiedResultJsonPath).size() > 0);
        QVERIFY(runResult.resultJsonExists);

        const QString requestText = readTextFile(parameters.requestJsonFilePath);
        const QJsonDocument requestDocument =
            QJsonDocument::fromJson(requestText.toUtf8());
        QVERIFY(requestDocument.isObject());
        const QJsonObject requestObject = requestDocument.object();
        QCOMPARE(requestObject.value("backend_id").toString(), manifest.id());
        QCOMPARE(requestObject.value("selected_region")
                     .toObject()
                     .value("start_sec")
                     .toDouble(),
                 0.5);
        QVERIFY(requestObject.value("environment_overrides")
                    .toObject()
                    .contains("TONY_DEV_MOCK_TEST"));

        BackendRunOutputHandoff handoff;
        const BackendRunOutputHandoffResult handoffResult =
            handoff.inspect(parameters.expectedUnifiedResultJsonPath,
                            runResult.processResult);
        QVERIFY(handoffResult.isValid());
        QVERIFY(handoffResult.exists);
        QVERIFY(handoffResult.readable);
        QVERIFY(handoffResult.nonEmpty);
        QVERIFY(!handoffResult.importedIntoTonyLayers);

        BackendRunResultLoader loader;
        const BackendRunResultLoadResult loaded = loader.load(runResult);
        QVERIFY(loaded.isValid());
        QVERIFY(loaded.handoffAccepted);
        QVERIFY(loaded.fileLoaded);
        QVERIFY(loaded.loadedResult.has_value());
        QVERIFY(!loaded.importedIntoTonyLayers);
        QCOMPARE(loaded.loadedResult->engine.engineId, manifest.id());
        QVERIFY(loaded.loadedResult->engine.runtimeType ==
                BackendRuntimeType::DevelopmentTest);
        QVERIFY(loaded.loadedResult->hasNotes());
        QCOMPARE(loaded.loadedResult->notes.size(), 1);
        QCOMPARE(loaded.loadedResult->notes.first().label.value_or(QString()),
                 QString("dev-mock-test-only"));
        QVERIFY(loaded.loadedResult->provenance.value("test_only").toBool());
        QVERIFY(loaded.loadedResult->provenance.value("dev_mock").toBool());
        QVERIFY(!loaded.loadedResult->provenance
                    .value("production_transcription")
                    .toBool());

        BackendRunResultReporter reporter;
        const BackendRunResultReport report =
            reporter.buildReport(manifest.id(),
                                 runResult,
                                 parameters.expectedUnifiedResultJsonPath,
                                 loaded);
        QVERIFY(report.isValid());
        QVERIFY(report.processResultAvailable);
        QVERIFY(report.processSucceeded);
        QVERIFY(report.outputFilePresent);
        QVERIFY(report.unifiedResultLoaded);
        QVERIFY(!report.importedIntoTonyLayers);
        QVERIFY(report.errors.isEmpty());

        QVERIFY(manifest.status == BackendStatus::NotConfigured);
        QVERIFY(manifest.status != BackendStatus::Ready);
        QVERIFY(manifest.status != BackendStatus::Completed);

        BackendRegistry registry;
        QVERIFY(!registry.hasBackend(manifest.id()));
        QVERIFY(registry.allManifests().isEmpty());
    }

    void devMockBackendMissingRequestFileFailsCleanly()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        ExternalProcessRequest request;
        request.executablePath = QCoreApplication::applicationFilePath();
        request.arguments << "--dev-mock-backend"
                          << "--request"
                          << directory.filePath("missing-request.json");
        request.timeoutMsec = 3000;

        ExternalProcessRunner runner;
        const ExternalProcessResult result = runner.run(request);

        QVERIFY(!result.succeeded());
        QVERIFY(result.started);
        QVERIFY(!result.startFailed);
        QCOMPARE(result.exitCode, 41);
        QVERIFY(result.standardError.contains("request file is missing"));
    }

    void devMockBackendInvalidRequestFileFailsCleanly()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString requestPath = directory.filePath("invalid-request.json");
        QVERIFY(writeFile(requestPath, "{ invalid request json\n"));

        ExternalProcessRequest request;
        request.executablePath = QCoreApplication::applicationFilePath();
        request.arguments << "--dev-mock-backend"
                          << "--request"
                          << requestPath;
        request.timeoutMsec = 3000;

        ExternalProcessRunner runner;
        const ExternalProcessResult result = runner.run(request);

        QVERIFY(!result.succeeded());
        QCOMPARE(result.exitCode, 42);
        QVERIFY(result.standardError.contains("invalid request JSON"));
    }

    void devMockBackendNonZeroExitIsReportedThroughPipeline()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendManifest manifest = devMockBackendManifest();
        const BackendRunWorkspace workspace =
            BackendRunWorkspace::fromParts(directory.path(),
                                           manifest.id(),
                                           "dev_mock_run_nonzero");

        BackendRunOrchestrationParameters parameters;
        parameters.inputAudioFilePath =
            QDir(directory.path()).filePath("dev-mock-input.wav");
        parameters.expectedUnifiedResultJsonPath =
            workspace.unifiedResultJsonPath;
        parameters.requestJsonFilePath =
            QDir(workspace.runDirectoryPath).filePath("request.json");
        parameters.timeoutMsec = 3000;
        parameters.prepareWorkspace = true;
        parameters.additionalArguments << "--dev-mock-backend"
                                       << "--dev-mock-mode"
                                       << "nonzero";

        BackendRunOrchestrator orchestrator;
        const BackendRunOrchestrationResult runResult =
            orchestrator.run(manifest, workspace, parameters);

        QVERIFY(!runResult.isValid());
        QVERIFY(runResult.processRunAttempted);
        QVERIFY(runResult.processResult.has_value());
        QVERIFY(!runResult.processResult->succeeded());
        QCOMPARE(runResult.processResult->exitCode, 45);
        QVERIFY(!QFile::exists(parameters.expectedUnifiedResultJsonPath));

        BackendRunResultLoader loader;
        const BackendRunResultLoadResult loaded = loader.load(runResult);

        BackendRunResultReporter reporter;
        const BackendRunResultReport report =
            reporter.buildReport(manifest.id(),
                                 runResult,
                                 parameters.expectedUnifiedResultJsonPath,
                                 loaded);

        QVERIFY(!report.isValid());
        QVERIFY(report.processFailed);
        QVERIFY(report.outputFileMissing);
        QVERIFY(!report.unifiedResultLoaded);
        QVERIFY(report.errors.contains("process_failed"));
        QVERIFY(report.errors.contains("output_file_missing"));
        QVERIFY(!report.importedIntoTonyLayers);
        QVERIFY(manifest.status == BackendStatus::NotConfigured);
    }

    void devMockBackendMissingResultIsReportedThroughPipeline()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BackendManifest manifest = devMockBackendManifest();
        const BackendRunWorkspace workspace =
            BackendRunWorkspace::fromParts(directory.path(),
                                           manifest.id(),
                                           "dev_mock_run_missing_result");

        BackendRunOrchestrationParameters parameters;
        parameters.inputAudioFilePath =
            QDir(directory.path()).filePath("dev-mock-input.wav");
        parameters.expectedUnifiedResultJsonPath =
            workspace.unifiedResultJsonPath;
        parameters.requestJsonFilePath =
            QDir(workspace.runDirectoryPath).filePath("request.json");
        parameters.timeoutMsec = 3000;
        parameters.prepareWorkspace = true;
        parameters.additionalArguments << "--dev-mock-backend"
                                       << "--dev-mock-mode"
                                       << "missing-result";

        BackendRunOrchestrator orchestrator;
        const BackendRunOrchestrationResult runResult =
            orchestrator.run(manifest, workspace, parameters);

        QVERIFY(runResult.isValid());
        QVERIFY(runResult.processRunAttempted);
        QVERIFY(runResult.processResult.has_value());
        QVERIFY(runResult.processResult->succeeded());
        QVERIFY(QFile::exists(parameters.requestJsonFilePath));
        QVERIFY(!QFile::exists(parameters.expectedUnifiedResultJsonPath));
        QVERIFY(!runResult.resultJsonExists);

        BackendRunResultLoader loader;
        const BackendRunResultLoadResult loaded = loader.load(runResult);
        QVERIFY(!loaded.isValid());
        QVERIFY(!loaded.fileLoaded);
        QVERIFY(!loaded.loadedResult.has_value());

        BackendRunResultReporter reporter;
        const BackendRunResultReport report =
            reporter.buildReport(manifest.id(),
                                 runResult,
                                 parameters.expectedUnifiedResultJsonPath,
                                 loaded);

        QVERIFY(!report.isValid());
        QVERIFY(report.processSucceeded);
        QVERIFY(report.outputFileMissing);
        QVERIFY(!report.unifiedResultLoaded);
        QVERIFY(report.errors.contains("output_file_missing"));
        QVERIFY(!report.importedIntoTonyLayers);
        QVERIFY(manifest.status == BackendStatus::NotConfigured);
    }

    void tonyLayerImporterCreatesRealDocumentNoteLayerProof()
    {
        UnifiedResult unifiedResult = validTonyLayerImportUnifiedResult();

        sv::Document document;
        TonyLayerImportOptions options;
        options.sampleRate = 44100.0;
        options.resolution = 1;
        options.document = &document;
        options.createDocumentLayer = true;

        TonyLayerImporter importer;
        const TonyLayerImportResult imported =
            importer.importResult(unifiedResult, options);

        QVERIFY(imported.isValid());
        QVERIFY(imported.modelCreated);
        QVERIFY(imported.modelRegistered);
        QVERIFY(imported.documentLayerCreated);
        QVERIFY(imported.importedIntoTonyLayers);
        QVERIFY(!imported.insertedIntoView);
        QVERIFY(imported.sourceMarkedDevMock);
        QCOMPARE(imported.noteCount, 2);
        QCOMPARE(imported.createdModelType,
                 QString("NoteModel::NORMAL_NOTE"));
        QCOMPARE(imported.createdLayerType, QString("notes"));

        auto model = sv::ModelById::getAs<sv::NoteModel>(imported.modelId);
        QVERIFY(model);
        QCOMPARE(model->getSubtype(), sv::NoteModel::NORMAL_NOTE);
        QCOMPARE(model->getScaleUnits(), QString("MIDI Pitch"));
        QVERIFY(model->isEditable());
        QCOMPARE(model->getEventCount(), 2);

        QVERIFY(imported.layer);
        QVERIFY(dynamic_cast<sv::NoteLayer *>(imported.layer) != nullptr);
        QVERIFY(dynamic_cast<sv::FlexiNoteLayer *>(imported.layer) == nullptr);
        QVERIFY(imported.layer->isLayerEditable());

        const std::set<sv::Layer *> documentLayers = document.getLayers();
        QVERIFY(documentLayers.find(imported.layer) != documentLayers.end());

        const sv::EventVector events = model->getAllEvents();
        QCOMPARE(int(events.size()), 2);
        QCOMPARE(events[0].getFrame(), sv::sv_frame_t(11025));
        QCOMPARE(events[0].getDuration(), sv::sv_frame_t(22050));
        QVERIFY(qAbs(events[0].getValue() - 60.0f) < 0.001f);
        QVERIFY(qAbs(events[0].getLevel() - (100.0f / 127.0f)) < 0.001f);
        QCOMPARE(events[0].getLabel(), QString("dev-mock-note-a"));

        QCOMPARE(events[1].getFrame(), sv::sv_frame_t(44100));
        QCOMPARE(events[1].getDuration(), sv::sv_frame_t(11025));
        QVERIFY(qAbs(events[1].getValue() - 64.0f) < 0.001f);
        QCOMPARE(events[1].getLabel(), QString("dev-mock-note-b"));

        BackendManifest manifest = devMockBackendManifest();
        QVERIFY(manifest.status == BackendStatus::NotConfigured);
        QVERIFY(manifest.status != BackendStatus::Ready);
        QVERIFY(manifest.status != BackendStatus::Completed);
    }

    void tonyLayerImporterInsertsRealLayerIntoPaneAndCommandHistoryEditWorks()
    {
        sv::CommandHistory::getInstance()->clear();

        UnifiedResult unifiedResult = validTonyLayerImportUnifiedResult();

        sv::Pane pane;
        sv::Document document;
        TonyLayerImportOptions options;
        options.sampleRate = 44100.0;
        options.resolution = 1;
        options.document = &document;
        options.createDocumentLayer = true;
        options.view = &pane;
        options.insertLayerIntoView = true;

        TonyLayerImporter importer;
        const TonyLayerImportResult imported =
            importer.importResult(unifiedResult, options);

        QVERIFY(imported.isValid());
        QVERIFY(imported.importedIntoTonyLayers);
        QVERIFY(imported.documentLayerCreated);
        QVERIFY(imported.insertedIntoView);
        QVERIFY(!imported.commandHistoryEditProof);
        QVERIFY(imported.layer);
        QCOMPARE(pane.getLayerCount(), 1);
        QCOMPARE(pane.getLayer(0), imported.layer);
        QVERIFY(imported.layer->isLayerEditable());

        auto model = sv::ModelById::getAs<sv::NoteModel>(imported.modelId);
        QVERIFY(model);
        const sv::EventVector originalEvents = model->getAllEvents();
        QCOMPARE(int(originalEvents.size()), 2);
        QCOMPARE(originalEvents[0].getFrame(), sv::sv_frame_t(11025));
        QCOMPARE(originalEvents[0].getDuration(), sv::sv_frame_t(22050));
        QVERIFY(qAbs(originalEvents[0].getValue() - 60.0f) < 0.001f);

        const TonyLayerCommandHistoryEditProofResult editProof =
            importer.proveCommandHistoryEdit(imported, 0, 2.0f);

        QVERIFY(editProof.isValid());
        QVERIFY(editProof.commandHistoryEditProof);
        QCOMPARE(editProof.noteCount, 2);

        const sv::EventVector restoredEvents = model->getAllEvents();
        QCOMPARE(int(restoredEvents.size()), 2);
        QCOMPARE(restoredEvents[0].getFrame(), originalEvents[0].getFrame());
        QCOMPARE(restoredEvents[0].getDuration(),
                 originalEvents[0].getDuration());
        QVERIFY(qAbs(restoredEvents[0].getValue() -
                     originalEvents[0].getValue()) < 0.001f);
        QCOMPARE(restoredEvents[0].getLabel(), originalEvents[0].getLabel());

        sv::CommandHistory::getInstance()->undo();
        QCOMPARE(pane.getLayerCount(), 0);

        sv::CommandHistory::getInstance()->redo();
        QCOMPARE(pane.getLayerCount(), 1);
        QCOMPARE(pane.getLayer(0), imported.layer);

        sv::CommandHistory::getInstance()->undo();
        QCOMPARE(pane.getLayerCount(), 0);
        sv::CommandHistory::getInstance()->clear();

        BackendManifest manifest = devMockBackendManifest();
        QVERIFY(manifest.status == BackendStatus::NotConfigured);
        QVERIFY(manifest.status != BackendStatus::Ready);
        QVERIFY(manifest.status != BackendStatus::Completed);
    }

    void tonyLayerImporterViewInsertionRequiresARealView()
    {
        sv::CommandHistory::getInstance()->clear();

        UnifiedResult unifiedResult = validTonyLayerImportUnifiedResult();

        sv::Document document;
        TonyLayerImportOptions options;
        options.sampleRate = 44100.0;
        options.resolution = 1;
        options.document = &document;
        options.createDocumentLayer = true;
        options.insertLayerIntoView = true;

        TonyLayerImporter importer;
        const TonyLayerImportResult imported =
            importer.importResult(unifiedResult, options);

        QVERIFY(!imported.isValid());
        QVERIFY(!imported.importedIntoTonyLayers);
        QVERIFY(!imported.documentLayerCreated);
        QVERIFY(!imported.modelCreated);
        QVERIFY(!imported.insertedIntoView);
        QVERIFY(reportHasIssue(imported.report,
                               "missing_view_for_layer_insertion"));

        sv::CommandHistory::getInstance()->clear();
    }

    void tonyLayerImporterImportedNoteLayerSurvivesSessionSaveLoad()
    {
        sv::CommandHistory::getInstance()->clear();

        UnifiedResult unifiedResult = validTonyLayerImportUnifiedResult();

        sv::Pane pane;
        sv::Document document;
        TonyLayerImportOptions options;
        options.sampleRate = 44100.0;
        options.resolution = 1;
        options.document = &document;
        options.createDocumentLayer = true;
        options.view = &pane;
        options.insertLayerIntoView = true;

        TonyLayerImporter importer;
        const TonyLayerImportResult imported =
            importer.importResult(unifiedResult, options);

        QVERIFY(imported.isValid());
        QVERIFY(imported.importedIntoTonyLayers);
        QVERIFY(imported.insertedIntoView);
        QVERIFY(imported.layer);
        QVERIFY(dynamic_cast<sv::NoteLayer *>(imported.layer) != nullptr);

        const QString sessionXml =
            serializeDocumentPaneSessionXml(document, pane);
        QVERIFY(!sessionXml.isEmpty());
        QVERIFY(sessionXml.contains("<sv>"));
        QVERIFY(sessionXml.contains("<data>"));
        QVERIFY(sessionXml.contains("<display>"));
        QVERIFY(sessionXml.contains("type=\"sparse\""));
        QVERIFY(sessionXml.contains("dimensions=\"3\""));
        QVERIFY(sessionXml.contains("subtype=\"note\""));
        QVERIFY(sessionXml.contains("type=\"notes\""));
        QVERIFY(sessionXml.contains("dev-mock-note-a"));
        QVERIFY(sessionXml.contains("dev-mock-note-b"));

        TestSVFileReaderPaneCallback callback;
        sv::Document reloadedDocument;
        sv::SVFileReader reader(&reloadedDocument, callback,
                                "backend-import-save-load-test");
        reader.parseXml(sessionXml);

        QVERIFY2(reader.isOK(), qPrintable(reader.getErrorString()));
        QCOMPARE(int(callback.panes.size()), 1);

        sv::Pane *reloadedPane = callback.panes.front().get();
        QVERIFY(reloadedPane);
        QCOMPARE(reloadedPane->getLayerCount(), 1);

        sv::Layer *reloadedLayer = reloadedPane->getLayer(0);
        QVERIFY(reloadedLayer);
        QVERIFY(dynamic_cast<sv::NoteLayer *>(reloadedLayer) != nullptr);
        QVERIFY(dynamic_cast<sv::FlexiNoteLayer *>(reloadedLayer) == nullptr);
        QVERIFY(reloadedLayer->isLayerEditable());

        const std::set<sv::Layer *> reloadedLayers =
            reloadedDocument.getLayers();
        QVERIFY(reloadedLayers.find(reloadedLayer) != reloadedLayers.end());

        auto reloadedModel =
            sv::ModelById::getAs<sv::NoteModel>(reloadedLayer->getModel());
        QVERIFY(reloadedModel);
        QCOMPARE(reloadedModel->getSubtype(), sv::NoteModel::NORMAL_NOTE);
        QCOMPARE(reloadedModel->getScaleUnits(), QString("MIDI Pitch"));
        QVERIFY(reloadedModel->isEditable());
        QCOMPARE(reloadedModel->getEventCount(), 2);

        const sv::EventVector events = reloadedModel->getAllEvents();
        QCOMPARE(int(events.size()), 2);
        QCOMPARE(events[0].getFrame(), sv::sv_frame_t(11025));
        QCOMPARE(events[0].getDuration(), sv::sv_frame_t(22050));
        QVERIFY(qAbs(events[0].getValue() - 60.0f) < 0.001f);
        QVERIFY(qAbs(events[0].getLevel() - (100.0f / 127.0f)) < 0.001f);
        QCOMPARE(events[0].getLabel(), QString("dev-mock-note-a"));

        QCOMPARE(events[1].getFrame(), sv::sv_frame_t(44100));
        QCOMPARE(events[1].getDuration(), sv::sv_frame_t(11025));
        QVERIFY(qAbs(events[1].getValue() - 64.0f) < 0.001f);
        QCOMPARE(events[1].getLabel(), QString("dev-mock-note-b"));

        BackendManifest manifest = devMockBackendManifest();
        QVERIFY(manifest.status == BackendStatus::NotConfigured);
        QVERIFY(manifest.status != BackendStatus::Ready);
        QVERIFY(manifest.status != BackendStatus::Completed);

        sv::CommandHistory::getInstance()->clear();
    }

    void tonyLayerImporterProvenanceIdentitySurvivesSessionSaveLoad()
    {
        sv::CommandHistory::getInstance()->clear();

        UnifiedResult unifiedResult = validTonyLayerImportUnifiedResult();

        sv::Pane pane;
        sv::Document document;
        TonyLayerImportOptions options;
        options.sampleRate = 44100.0;
        options.resolution = 1;
        options.document = &document;
        options.createDocumentLayer = true;
        options.view = &pane;
        options.insertLayerIntoView = true;
        options.provenance.backendId = "dev_mock_backend";
        options.provenance.backendName = "Dev Mock Backend (test only)";
        options.provenance.backendVersion = "0.1.0-test";
        options.provenance.inputAudioPath =
            "C:/codex093/codex093-audio.wav";
        options.provenance.selectedRegionStartSec = 0.25;
        options.provenance.selectedRegionEndSec = 1.25;
        options.provenance.resultJsonPath =
            "C:/codex093/codex093-result.json";
        options.provenance.requestJsonPath =
            "C:/codex093/codex093-request.json";
        options.provenance.runId = "codex093_run_001";
        options.provenance.testOnly = true;
        options.provenance.devMock = true;
        options.provenance.warningSummary = "completed_with_warnings";
        options.provenance.confidenceSummary = "mean=0.775";

        TonyLayerImporter importer;
        const TonyLayerImportResult imported =
            importer.importResult(unifiedResult, options);

        QVERIFY(imported.isValid());
        QVERIFY(imported.importedIntoTonyLayers);
        QVERIFY(imported.insertedIntoView);
        QVERIFY(imported.provenanceAttached);
        QVERIFY(imported.durableIdentityPersisted);
        QVERIFY(!imported.structuredProvenancePersisted);
        QVERIFY(reportHasIssue(imported.report,
                               "structured_provenance_persistence_deferred"));
        QVERIFY(imported.layer);

        const QString identity = imported.provenanceIdentity;
        QVERIFY(identity.contains("backend=dev_mock_backend"));
        QVERIFY(identity.contains("version=0.1.0-test"));
        QVERIFY(identity.contains("run=codex093_run_001"));
        QVERIFY(identity.contains("result=codex093-result.json"));
        QVERIFY(identity.contains("request=codex093-request.json"));
        QVERIFY(identity.contains("input=codex093-audio.wav"));
        QVERIFY(identity.contains("region=0.250-1.250s"));
        QVERIFY(identity.contains("test_only=true"));
        QVERIFY(identity.contains("dev_mock=true"));

        auto model = sv::ModelById::getAs<sv::NoteModel>(imported.modelId);
        QVERIFY(model);
        QCOMPARE(model->objectName(), identity);
        QCOMPARE(imported.layer->objectName(), identity);
        QCOMPARE(imported.layer->getLayerPresentationName(), identity);

        const QString sessionXml =
            serializeDocumentPaneSessionXml(document, pane);
        QVERIFY(!sessionXml.isEmpty());
        QVERIFY(sessionXml.contains("presentationName=\""));
        QVERIFY(sessionXml.contains("backend=dev_mock_backend"));
        QVERIFY(sessionXml.contains("run=codex093_run_001"));
        QVERIFY(sessionXml.contains("result=codex093-result.json"));
        QVERIFY(sessionXml.contains("request=codex093-request.json"));
        QVERIFY(sessionXml.contains("test_only=true"));
        QVERIFY(sessionXml.contains("dev_mock=true"));

        TestSVFileReaderPaneCallback callback;
        sv::Document reloadedDocument;
        sv::SVFileReader reader(&reloadedDocument, callback,
                                "backend-import-provenance-test");
        reader.parseXml(sessionXml);

        QVERIFY2(reader.isOK(), qPrintable(reader.getErrorString()));
        QCOMPARE(int(callback.panes.size()), 1);

        sv::Pane *reloadedPane = callback.panes.front().get();
        QVERIFY(reloadedPane);
        QCOMPARE(reloadedPane->getLayerCount(), 1);

        sv::Layer *reloadedLayer = reloadedPane->getLayer(0);
        QVERIFY(reloadedLayer);
        QCOMPARE(reloadedLayer->objectName(), identity);
        QCOMPARE(reloadedLayer->getLayerPresentationName(), identity);
        QVERIFY(dynamic_cast<sv::NoteLayer *>(reloadedLayer) != nullptr);
        QVERIFY(reloadedLayer->isLayerEditable());

        auto reloadedModel =
            sv::ModelById::getAs<sv::NoteModel>(reloadedLayer->getModel());
        QVERIFY(reloadedModel);
        QCOMPARE(reloadedModel->objectName(), identity);
        QCOMPARE(reloadedModel->getEventCount(), 2);

        const sv::EventVector events = reloadedModel->getAllEvents();
        QCOMPARE(int(events.size()), 2);
        QCOMPARE(events[0].getFrame(), sv::sv_frame_t(11025));
        QCOMPARE(events[0].getDuration(), sv::sv_frame_t(22050));
        QVERIFY(qAbs(events[0].getValue() - 60.0f) < 0.001f);
        QCOMPARE(events[0].getLabel(), QString("dev-mock-note-a"));
        QCOMPARE(events[1].getFrame(), sv::sv_frame_t(44100));
        QCOMPARE(events[1].getDuration(), sv::sv_frame_t(11025));
        QVERIFY(qAbs(events[1].getValue() - 64.0f) < 0.001f);
        QCOMPARE(events[1].getLabel(), QString("dev-mock-note-b"));

        BackendManifest manifest = devMockBackendManifest();
        QVERIFY(manifest.status == BackendStatus::NotConfigured);
        QVERIFY(manifest.status != BackendStatus::Ready);
        QVERIFY(manifest.status != BackendStatus::Completed);

        sv::CommandHistory::getInstance()->clear();
    }

    void tonyLayerImporterDoesNotClaimStructuredProvenancePersistence()
    {
        sv::CommandHistory::getInstance()->clear();

        UnifiedResult unifiedResult = validTonyLayerImportUnifiedResult();

        sv::Pane pane;
        sv::Document document;
        TonyLayerImportOptions options;
        options.sampleRate = 44100.0;
        options.resolution = 1;
        options.document = &document;
        options.createDocumentLayer = true;
        options.view = &pane;
        options.insertLayerIntoView = true;
        options.provenance.backendId = "dev_mock_backend";
        options.provenance.resultJsonPath =
            "C:/codex094/codex094-result.json";
        options.provenance.runId = "codex094_run_001";
        options.provenance.testOnly = true;
        options.provenance.devMock = true;

        TonyLayerImporter importer;
        const TonyLayerImportResult imported =
            importer.importResult(unifiedResult, options);

        QVERIFY(imported.isValid());
        QVERIFY(imported.importedIntoTonyLayers);
        QVERIFY(imported.insertedIntoView);
        QVERIFY(imported.provenanceAttached);
        QVERIFY(imported.durableIdentityPersisted);
        QVERIFY(!imported.structuredProvenancePersisted);
        QVERIFY(reportHasIssue(imported.report,
                               "structured_provenance_persistence_deferred"));
        QVERIFY(imported.debugSummaryString().contains(
            "durableIdentity=true"));
        QVERIFY(imported.debugSummaryString().contains(
            "structuredProvenance=false"));

        const QString sessionXml =
            serializeDocumentPaneSessionXml(document, pane);
        QVERIFY(sessionXml.contains("backend=dev_mock_backend"));
        QVERIFY(sessionXml.contains("run=codex094_run_001"));
        QVERIFY(sessionXml.contains("result=codex094-result.json"));
        QVERIFY(!sessionXml.contains("model_checkpoint"));
        QVERIFY(!sessionXml.contains("backend_settings_hash"));
        QVERIFY(!sessionXml.contains("user_edit_status"));

        BackendManifest manifest = devMockBackendManifest();
        QVERIFY(manifest.status == BackendStatus::NotConfigured);
        QVERIFY(manifest.status != BackendStatus::Ready);
        QVERIFY(manifest.status != BackendStatus::Completed);

        sv::CommandHistory::getInstance()->clear();
    }

    void tonyLayerImporterImportedNoteLayerExportsThroughRealCsvPath()
    {
        sv::CommandHistory::getInstance()->clear();

        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        UnifiedResult unifiedResult = validTonyLayerImportUnifiedResult();

        sv::Pane pane;
        sv::Document document;
        TonyLayerImportOptions options;
        options.sampleRate = 44100.0;
        options.resolution = 1;
        options.document = &document;
        options.createDocumentLayer = true;
        options.view = &pane;
        options.insertLayerIntoView = true;

        TonyLayerImporter importer;
        const TonyLayerImportResult imported =
            importer.importResult(unifiedResult, options);

        QVERIFY(imported.isValid());
        QVERIFY(imported.importedIntoTonyLayers);
        QVERIFY(imported.insertedIntoView);
        QVERIFY(imported.layer);
        QVERIFY(dynamic_cast<sv::NoteLayer *>(imported.layer) != nullptr);

        const sv::ModelId exportModelId =
            imported.layer->getExportModel(&pane);
        QCOMPARE(exportModelId, imported.modelId);

        auto exportModel = sv::ModelById::get(exportModelId);
        QVERIFY(exportModel);
        QVERIFY(sv::ModelById::getAs<sv::NoteModel>(exportModelId));

        const QString exportPath = directory.filePath("backend-notes.csv");
        sv::CSVFileWriter writer(
            exportPath,
            exportModel.get(),
            ",",
            sv::DataExportWriteTimeInFrames | sv::DataExportIncludeHeader);
        writer.write();

        QVERIFY2(writer.isOK(), qPrintable(writer.getError()));
        QVERIFY(QFile::exists(exportPath));
        QVERIFY(QFileInfo(exportPath).size() > 0);

        QString exported = readTextFile(exportPath);
        exported.replace("\r\n", "\n");
        exported.replace('\r', '\n');
        QVERIFY(!exported.trimmed().isEmpty());

        const QStringList lines = exported.trimmed().split('\n');
        QCOMPARE(lines.size(), 3);
        QCOMPARE(lines[0], QString("FRAME,VALUE,DURATION,LEVEL,LABEL"));

        const QStringList first = lines[1].split(',');
        QCOMPARE(first.size(), 5);
        QCOMPARE(first[0], QString("11025"));
        QVERIFY(qAbs(first[1].toFloat() - 60.0f) < 0.001f);
        QCOMPARE(first[2], QString("22050"));
        QVERIFY(qAbs(first[3].toFloat() - (100.0f / 127.0f)) < 0.001f);
        QCOMPARE(first[4], QString("dev-mock-note-a"));

        const QStringList second = lines[2].split(',');
        QCOMPARE(second.size(), 5);
        QCOMPARE(second[0], QString("44100"));
        QVERIFY(qAbs(second[1].toFloat() - 64.0f) < 0.001f);
        QCOMPARE(second[2], QString("11025"));
        QVERIFY(qAbs(second[3].toFloat() - 0.74f) < 0.001f);
        QCOMPARE(second[4], QString("dev-mock-note-b"));

        BackendManifest manifest = devMockBackendManifest();
        QVERIFY(manifest.status == BackendStatus::NotConfigured);
        QVERIFY(manifest.status != BackendStatus::Ready);
        QVERIFY(manifest.status != BackendStatus::Completed);

        sv::CommandHistory::getInstance()->clear();
    }

    void tonyLayerImporterModelOnlyDefersImportedFlag()
    {
        UnifiedResult unifiedResult = validTonyLayerImportUnifiedResult();

        TonyLayerImportOptions options;
        options.sampleRate = 44100.0;
        options.resolution = 1;
        options.createDocumentLayer = false;

        TonyLayerImporter importer;
        const TonyLayerImportResult imported =
            importer.importResult(unifiedResult, options);

        QVERIFY(imported.isValid());
        QVERIFY(imported.modelCreated);
        QVERIFY(imported.modelRegistered);
        QVERIFY(!imported.documentLayerCreated);
        QVERIFY(!imported.importedIntoTonyLayers);
        QVERIFY(!imported.layer);
        QCOMPARE(imported.noteCount, 2);

        auto model = sv::ModelById::getAs<sv::NoteModel>(imported.modelId);
        QVERIFY(model);
        QCOMPARE(model->getEventCount(), 2);

        sv::ModelById::release(imported.modelId);
    }

    void tonyLayerImporterEmptyUnifiedResultFailsSafely()
    {
        UnifiedResult unifiedResult;

        TonyLayerImportOptions options;
        options.sampleRate = 44100.0;

        TonyLayerImporter importer;
        const TonyLayerImportResult imported =
            importer.importResult(unifiedResult, options);

        QVERIFY(!imported.isValid());
        QVERIFY(!imported.succeeded);
        QVERIFY(!imported.modelCreated);
        QVERIFY(!imported.importedIntoTonyLayers);
        QVERIFY(reportHasIssue(imported.report, "no_notes_to_import"));
    }

    void tonyLayerImporterInvalidNoteTimingFailsCleanly()
    {
        UnifiedResult unifiedResult = validTonyLayerImportUnifiedResult();
        unifiedResult.notes[0].endSec = unifiedResult.notes[0].startSec - 0.1;

        TonyLayerImportOptions options;
        options.sampleRate = 44100.0;

        TonyLayerImporter importer;
        const TonyLayerImportResult imported =
            importer.importResult(unifiedResult, options);

        QVERIFY(!imported.isValid());
        QVERIFY(!imported.succeeded);
        QVERIFY(!imported.modelCreated);
        QVERIFY(!imported.importedIntoTonyLayers);
        QVERIFY(reportHasIssue(imported.report, "invalid_note_time_range"));
    }

    void tonyLayerImporterMissingPitchFailsCleanly()
    {
        UnifiedResult unifiedResult = validTonyLayerImportUnifiedResult();
        unifiedResult.notes.clear();

        NoteEvent note;
        note.id = "missing_pitch";
        note.startSec = 0.0;
        note.endSec = 0.5;
        unifiedResult.notes.push_back(note);

        TonyLayerImportOptions options;
        options.sampleRate = 44100.0;

        TonyLayerImporter importer;
        const TonyLayerImportResult imported =
            importer.importResult(unifiedResult, options);

        QVERIFY(!imported.isValid());
        QVERIFY(!imported.succeeded);
        QVERIFY(!imported.modelCreated);
        QVERIFY(!imported.importedIntoTonyLayers);
        QVERIFY(reportHasIssue(imported.report, "missing_note_pitch"));
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

    void basicPitchAdapterContractManifestIsConservative()
    {
        BasicPitchAdapterContract contract;
        const BackendManifest manifest = contract.manifest();

        QCOMPARE(manifest.id(), QString("basic_pitch"));
        QCOMPARE(manifest.displayName, QString("Basic Pitch"));
        QCOMPARE(manifest.executablePath, QString("basic-pitch"));
        QVERIFY(manifest.status == BackendStatus::NotConfigured);
        QVERIFY(manifest.backendType == BackendRuntimeType::PythonCli);
        QVERIFY(manifest.capabilities.supportsFullFile);
        QVERIFY(!manifest.capabilities.supportsSelectedRegion);
        QVERIFY(manifest.capabilities.outputsNotes);
        QVERIFY(manifest.capabilities.outputsPitchBends);
        QVERIFY(!manifest.capabilities.outputsTechniqueLabels);
        QVERIFY(manifest.capabilities.requiresPython);
        QVERIFY(!manifest.capabilities.requiresModelCheckpoint);
        QVERIFY(manifest.supportedInputFormats.contains("wav"));
        QVERIFY(manifest.supportedInputFormats.contains("flac"));
        QVERIFY(manifest.supportedOutputTypes.contains("midi"));
        QVERIFY(manifest.supportedOutputTypes.contains("note_events_csv"));
        QVERIFY(manifest.supportedOutputTypes.contains("pitch_bends"));
        QVERIFY(manifest.defaultSettings.value("possible_polyphony").toBool());
        QVERIFY(!manifest.defaultSettings.value("monophonic_guaranteed").toBool());
        QVERIFY(!manifest.defaultSettings.value(
            "pitch_bend_tony_mapping_proven").toBool());

        BackendRegistry registry;
        QVERIFY(!registry.hasBackend("basic_pitch"));
        QVERIFY(registry.allManifests().isEmpty());
    }

    void basicPitchAdapterContractBuildsCliRequestWithoutExecution()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString markerPath = directory.filePath("basic-pitch-ran.marker");
        const QString executablePath = directory.filePath("basic-pitch.bat");
        const QByteArray script =
            QByteArray("@echo off\r\n") +
            QByteArray("echo ran > \"") +
            QDir::toNativeSeparators(markerPath).toUtf8() +
            QByteArray("\"\r\n");
        QVERIFY(writeFile(executablePath, script));
        QVERIFY(makeExecutable(executablePath));

        BasicPitchAdapterContractParameters parameters;
        parameters.executablePath = executablePath;
        parameters.inputAudioPath = directory.filePath("input.wav");
        parameters.outputDirectoryPath = directory.filePath("basic-pitch-output");
        parameters.expectedUnifiedResultJsonPath =
            directory.filePath("result.json");
        parameters.timeoutMsec = 4567;
        parameters.environmentOverrides.insert("BASIC_PITCH_TEST", "1");

        BasicPitchAdapterContract contract;
        const BasicPitchAdapterContractResult built =
            contract.buildCliRequest(parameters);

        QVERIFY(built.isValid());
        QCOMPARE(built.request.executablePath, executablePath);
        QCOMPARE(built.request.timeoutMsec, 4567);
        QCOMPARE(built.request.environmentOverrides.value("BASIC_PITCH_TEST"),
                 QString("1"));
        QVERIFY(built.request.arguments.size() >= 2);
        QCOMPARE(built.request.arguments.at(0), parameters.outputDirectoryPath);
        QCOMPARE(built.request.arguments.at(1), parameters.inputAudioPath);
        QVERIFY(built.request.arguments.contains("--save-note-events"));
        QVERIFY(built.request.arguments.contains("--save-model-outputs"));
        QVERIFY(built.request.arguments.contains("--multiple-pitch-bends"));
        QVERIFY(!built.executesProcess);
        QVERIFY(!built.createsFakeResultJson);
        QVERIFY(!built.importsIntoTonyLayers);
        QVERIFY(!QFile::exists(markerPath));
        QVERIFY(!QFile::exists(parameters.expectedUnifiedResultJsonPath));
    }

    void basicPitchAdapterContractRejectsInvalidRequestShape()
    {
        BasicPitchAdapterContractParameters parameters;
        parameters.executablePath = " ";
        parameters.inputAudioPath = " ";
        parameters.outputDirectoryPath = " ";
        parameters.expectedUnifiedResultJsonPath = " ";
        parameters.timeoutMsec = -1;
        parameters.modelSerialization = "unsupported";

        BasicPitchAdapterContract contract;
        const BasicPitchAdapterContractResult built =
            contract.buildCliRequest(parameters);

        QVERIFY(!built.isValid());
        QVERIFY(reportHasIssue(built.report,
                               "empty_basic_pitch_executable_path"));
        QVERIFY(reportHasIssue(built.report,
                               "empty_basic_pitch_input_audio_path"));
        QVERIFY(reportHasIssue(built.report,
                               "empty_basic_pitch_output_directory"));
        QVERIFY(reportHasIssue(built.report,
                               "empty_basic_pitch_unified_result_path"));
        QVERIFY(reportHasIssue(built.report, "invalid_basic_pitch_timeout"));
        QVERIFY(reportHasIssue(
            built.report,
            "unsupported_basic_pitch_model_serialization"));
    }

    void basicPitchAdapterContractWarnsAboutUnprovenMappings()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BasicPitchAdapterContractParameters parameters;
        parameters.executablePath = "basic-pitch";
        parameters.inputAudioPath = directory.filePath("input.wav");
        parameters.outputDirectoryPath = directory.path();
        parameters.expectedUnifiedResultJsonPath =
            directory.filePath("result.json");
        parameters.modelSerialization = "onnx";
        parameters.modelPath = directory.filePath("nmp.onnx");

        BasicPitchAdapterContract contract;
        const BasicPitchAdapterContractResult built =
            contract.buildCliRequest(parameters);

        QVERIFY(built.isValid());
        QVERIFY(built.possiblePolyphony);
        QVERIFY(!built.monophonicGuaranteed);
        QVERIFY(!built.pitchBendTonyMappingProven);
        QVERIFY(!built.confidenceTonyMappingProven);
        QVERIFY(reportHasIssueWithSeverity(
            built.report,
            "basic_pitch_possible_polyphony",
            ValidationSeverity::Warning));
        QVERIFY(reportHasIssueWithSeverity(
            built.report,
            "basic_pitch_pitch_bend_mapping_deferred",
            ValidationSeverity::Warning));
        QVERIFY(reportHasIssueWithSeverity(
            built.report,
            "basic_pitch_unified_result_conversion_deferred",
            ValidationSeverity::Warning));
        const int modelPathIndex =
            built.request.arguments.indexOf("--model-path");
        QVERIFY(modelPathIndex >= 0);
        QCOMPARE(built.request.arguments.at(modelPathIndex + 1),
                 parameters.modelPath);
        const int serializationIndex =
            built.request.arguments.indexOf("--model-serialization");
        QVERIFY(serializationIndex >= 0);
        QCOMPARE(built.request.arguments.at(serializationIndex + 1),
                 QString("onnx"));
    }

    void basicPitchAdapterContractAvailabilityDoesNotCreateReadyState()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BasicPitchAdapterContract contract;
        BackendManifest manifest = contract.manifest();
        manifest.executablePath =
            directory.filePath("missing-basic-pitch.exe");

        BackendAvailabilityProbe availability;
        const BackendAvailabilityReport report = availability.probe(manifest);

        QVERIFY(report.status ==
                BackendAvailabilityProbeStatus::MissingExecutable);
        QVERIFY(manifest.status == BackendStatus::NotConfigured);
        QVERIFY(manifest.status != BackendStatus::Ready);

        BackendRegistry registry;
        QVERIFY(!registry.hasBackend("basic_pitch"));
        QVERIFY(registry.allManifests().isEmpty());
    }

    void basicPitchNoteEventsParserParsesVerifiedFixture()
    {
        BasicPitchNoteEventsParser parser;
        const BasicPitchNoteEventsParseResult parsed =
            parser.parseCsvText(basicPitchNoteEventsCsvFixture());

        QVERIFY(parsed.isValid());
        QCOMPARE(parsed.headerColumns.at(0), QString("start_time_s"));
        QCOMPARE(parsed.headerColumns.at(1), QString("end_time_s"));
        QCOMPARE(parsed.headerColumns.at(2), QString("pitch_midi"));
        QCOMPARE(parsed.headerColumns.at(3), QString("velocity"));
        QCOMPARE(parsed.headerColumns.at(4), QString("pitch_bend"));
        QCOMPARE(parsed.noteEvents.size(), 3);

        QCOMPARE(parsed.noteEvents.at(0).startSec, 0.10);
        QCOMPARE(parsed.noteEvents.at(0).endSec, 0.50);
        QCOMPARE(parsed.noteEvents.at(0).midiPitch, 60);
        QCOMPARE(parsed.noteEvents.at(0).velocity, 91);
        QCOMPARE(parsed.noteEvents.at(0).pitchBendValues.size(), 3);
        QCOMPARE(parsed.noteEvents.at(0).pitchBendValues.at(1), 12.0);
        QCOMPARE(parsed.noteEvents.at(2).velocity, 72);
        QVERIFY(!parsed.noteEvents.at(2).hasPitchBend());
    }

    void basicPitchOutputConverterCreatesUnifiedResultFromFixture()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BasicPitchOutputConversionParameters parameters;
        parameters.requestId = "req_basic_pitch_fixture";
        parameters.resultId = "res_basic_pitch_fixture";
        parameters.inputAudioPath = directory.filePath("input.wav");
        parameters.sourceArtifactPath =
            directory.filePath("basic_pitch_notes.csv");

        BasicPitchOutputConverter converter;
        const BasicPitchOutputConversionResult converted =
            converter.convertNoteEventsCsv(basicPitchNoteEventsCsvFixture(),
                                           parameters);

        QVERIFY(converted.isValid());
        QCOMPARE(converted.result.engine.engineId, QString("basic_pitch"));
        QVERIFY(converted.result.status == BackendStatus::Unknown);
        QCOMPARE(converted.result.notes.size(), 3);
        QCOMPARE(converted.result.summary.noteCount, 3);
        QCOMPARE(converted.result.notes.at(0).startSec, 0.10);
        QCOMPARE(converted.result.notes.at(0).endSec, 0.50);
        QVERIFY(converted.result.notes.at(0).midiPitch.has_value());
        QCOMPARE(*converted.result.notes.at(0).midiPitch, 60);
        QVERIFY(converted.result.notes.at(0).velocity.has_value());
        QCOMPARE(*converted.result.notes.at(0).velocity, 91);
        QVERIFY(converted.result.notes.at(0).pitchBendRef.has_value());
        QCOMPARE(converted.result.pitchBends.size(), 2);
        QCOMPARE(converted.result.summary.pitchBendCount, 2);
        QCOMPARE(converted.result.pitchBends.at(0).unit,
                 QString("midi_pitch_bend_units"));
        QCOMPARE(converted.result.pitchBends.at(0).points.size(), 3);
        QCOMPARE(converted.result.pitchBends.at(0).points.at(1).value, 12.0);
        QVERIFY(converted.pitchBendDataPreserved);
        QVERIFY(converted.pitchBendTonyMappingDeferred);
        QVERIFY(converted.fixtureOnly);
        QVERIFY(!converted.productionTranscription);
        QVERIFY(!converted.createdResultJson);
        QVERIFY(!converted.importsIntoTonyLayers);
    }

    void basicPitchOutputConverterWarnsAboutPolyphonyAndDeferredBends()
    {
        BasicPitchOutputConverter converter;
        const BasicPitchOutputConversionResult converted =
            converter.convertNoteEventsCsv(basicPitchNoteEventsCsvFixture(),
                                           BasicPitchOutputConversionParameters());

        QVERIFY(converted.isValid());
        QVERIFY(converted.possiblePolyphony);
        QVERIFY(converted.detectedPolyphony);
        QVERIFY(resultHasWarning(converted.result, "possible_polyphony"));
        QVERIFY(resultHasWarning(converted.result,
                                 "pitch_bend_mapping_deferred"));
        QVERIFY(resultHasWarning(converted.result,
                                 "fixture_only_conversion"));
        QVERIFY(resultHasWarning(converted.result,
                                 "production_transcription_false"));
        QVERIFY(reportHasIssueWithSeverity(converted.report,
                                           "possible_polyphony",
                                           ValidationSeverity::Warning));
        QVERIFY(reportHasIssueWithSeverity(
            converted.report,
            "pitch_bend_mapping_deferred",
            ValidationSeverity::Warning));
    }

    void basicPitchNoteEventsParserRejectsMissingColumns()
    {
        BasicPitchNoteEventsParser parser;
        const BasicPitchNoteEventsParseResult parsed =
            parser.parseCsvText(
                "start_time_s,end_time_s,pitch_midi,velocity\n"
                "0.10,0.50,60,91\n");

        QVERIFY(!parsed.isValid());
        QVERIFY(reportHasIssue(parsed.report,
                               "invalid_basic_pitch_note_events_header"));
    }

    void basicPitchNoteEventsParserRejectsInvalidRows()
    {
        BasicPitchNoteEventsParser parser;
        const BasicPitchNoteEventsParseResult parsed =
            parser.parseCsvText(
                "start_time_s,end_time_s,pitch_midi,velocity,pitch_bend\n"
                "0.50,0.10,60,91\n"
                "0.60,0.80,200,91\n"
                "0.90,1.10,64,200\n"
                "1.20,1.40,67,80,bend\n");

        QVERIFY(!parsed.isValid());
        QVERIFY(reportHasIssue(parsed.report,
                               "invalid_basic_pitch_note_time_range"));
        QVERIFY(reportHasIssue(parsed.report,
                               "invalid_basic_pitch_midi_pitch_range"));
        QVERIFY(reportHasIssue(parsed.report,
                               "invalid_basic_pitch_velocity_range"));
        QVERIFY(reportHasIssue(parsed.report,
                               "invalid_basic_pitch_pitch_bend_value"));
    }

    void basicPitchOutputConverterRejectsEmptyFixture()
    {
        BasicPitchOutputConverter converter;
        const BasicPitchOutputConversionResult converted =
            converter.convertNoteEventsCsv("  ",
                                           BasicPitchOutputConversionParameters());

        QVERIFY(!converted.isValid());
        QVERIFY(reportHasIssue(converted.report,
                               "empty_basic_pitch_note_events_csv"));
        QVERIFY(converted.result.notes.isEmpty());
        QVERIFY(converted.result.pitchBends.isEmpty());
        QVERIFY(!converted.importsIntoTonyLayers);
        QVERIFY(!converted.createdResultJson);
    }

    void basicPitchOutputConverterDoesNotCreateFilesOrBackendState()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString resultPath = directory.filePath("result.json");
        BasicPitchOutputConversionParameters parameters;
        parameters.inputAudioPath = directory.filePath("input.wav");
        parameters.sourceArtifactPath =
            directory.filePath("basic_pitch_notes.csv");
        parameters.resultId = "res_no_files";

        BasicPitchOutputConverter converter;
        const BasicPitchOutputConversionResult converted =
            converter.convertNoteEventsCsv(basicPitchNoteEventsCsvFixture(),
                                           parameters);

        QVERIFY(converted.isValid());
        QVERIFY(!QFile::exists(resultPath));
        QVERIFY(!converted.createdResultJson);
        QVERIFY(!converted.importsIntoTonyLayers);
        QVERIFY(!converted.marksBackendReadyInstalledOrCompleted);
        QVERIFY(converted.result.status == BackendStatus::Unknown);
        QCOMPARE(converted.result.provenance.value("created_result_json").toBool(),
                 false);
        QCOMPARE(converted.result.provenance.value(
                     "imported_into_tony_layers").toBool(),
                 false);

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.status = BackendStatus::NotConfigured;
        QVERIFY(manifest.status == BackendStatus::NotConfigured);
        QVERIFY(manifest.status != BackendStatus::Ready);

        BackendRegistry registry;
        QVERIFY(!registry.hasBackend("basic_pitch"));
        QVERIFY(registry.allManifests().isEmpty());
    }

    void basicPitchArtifactDiscoveryBuildsRequestWithoutExecution()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString markerPath = directory.filePath("basic-pitch-ran.marker");
        const QString executablePath = directory.filePath("basic-pitch.bat");
        const QByteArray script =
            QByteArray("@echo off\r\n") +
            QByteArray("echo ran > \"") +
            QDir::toNativeSeparators(markerPath).toUtf8() +
            QByteArray("\"\r\n");
        QVERIFY(writeFile(executablePath, script));
        QVERIFY(makeExecutable(executablePath));

        BasicPitchArtifactDiscoveryConfig config;
        config.executablePath = executablePath;
        config.inputAudioPath = directory.filePath("input.wav");
        config.outputDirectoryPath = directory.filePath("basic-pitch-output");
        config.timeoutMsec = 9876;
        config.environmentOverrides.insert("BASIC_PITCH_DISCOVERY_TEST", "1");

        BasicPitchArtifactDiscovery discovery;
        const BasicPitchArtifactDiscoveryResult built =
            discovery.buildRequest(config);

        QVERIFY(built.isValid());
        QCOMPARE(built.commandUsed, executablePath);
        QCOMPARE(built.request.executablePath, executablePath);
        QCOMPARE(built.request.timeoutMsec, 9876);
        QCOMPARE(built.request.environmentOverrides.value(
                     "BASIC_PITCH_DISCOVERY_TEST"),
                 QString("1"));
        QVERIFY(built.request.arguments.size() >= 2);
        QCOMPARE(built.request.arguments.at(0), config.outputDirectoryPath);
        QCOMPARE(built.request.arguments.at(1), config.inputAudioPath);
        QVERIFY(built.request.arguments.contains("--save-midi"));
        QVERIFY(built.request.arguments.contains("--save-note-events"));
        QVERIFY(built.request.arguments.contains("--save-model-outputs"));
        QVERIFY(built.request.arguments.contains("--multiple-pitch-bends"));
        QVERIFY(!built.ranBasicPitch);
        QVERIFY(!built.productionTranscription);
        QVERIFY(!built.importedIntoTonyLayers);
        QVERIFY(!built.readyInstalledCompletedMutation);
        QVERIFY(!QFile::exists(markerPath));
    }

    void basicPitchArtifactDiscoveryRunRequiresExplicitOptIn()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BasicPitchArtifactDiscoveryConfig config;
        config.executablePath = directory.filePath("missing-basic-pitch.exe");
        config.inputAudioPath = directory.filePath("input.wav");
        config.outputDirectoryPath = directory.path();

        BasicPitchArtifactDiscovery discovery;
        const BasicPitchArtifactDiscoveryResult result =
            discovery.runDiscovery(config);

        QVERIFY(result.isValid());
        QVERIFY(result.wasSkipped());
        QCOMPARE(result.skippedReason, QString("explicit_opt_in_required"));
        QVERIFY(!result.ranBasicPitch);
        QVERIFY(!result.productionTranscription);
        QVERIFY(!result.importedIntoTonyLayers);
        QVERIFY(!result.readyInstalledCompletedMutation);
        QVERIFY(reportHasIssueWithSeverity(
            result.report,
            "basic_pitch_discovery_explicit_opt_in_required",
            ValidationSeverity::Warning));
    }

    void basicPitchArtifactDiscoveryMissingConfigSkipsCleanly()
    {
        BasicPitchArtifactDiscoveryConfig config;
        config.explicitOptIn = true;
        config.executablePath = " ";
        config.inputAudioPath = " ";
        config.outputDirectoryPath = " ";

        BasicPitchArtifactDiscovery discovery;
        const BasicPitchArtifactDiscoveryResult result =
            discovery.runDiscovery(config);

        QVERIFY(!result.isValid());
        QVERIFY(result.wasSkipped());
        QCOMPARE(result.skippedReason, QString("not_configured"));
        QVERIFY(!result.ranBasicPitch);
        QVERIFY(reportHasIssue(result.report,
                               "empty_basic_pitch_discovery_command"));
        QVERIFY(reportHasIssue(result.report,
                               "empty_basic_pitch_discovery_audio_path"));
        QVERIFY(reportHasIssue(
            result.report,
            "empty_basic_pitch_discovery_output_directory"));
    }

    void basicPitchArtifactDiscoveryMissingAudioFailsBeforeExecution()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BasicPitchArtifactDiscoveryConfig config;
        config.explicitOptIn = true;
        config.executablePath = QCoreApplication::applicationFilePath();
        config.inputAudioPath = directory.filePath("missing-input.wav");
        config.outputDirectoryPath = directory.path();

        BasicPitchArtifactDiscovery discovery;
        const BasicPitchArtifactDiscoveryResult result =
            discovery.runDiscovery(config);

        QVERIFY(!result.isValid());
        QVERIFY(result.wasSkipped());
        QCOMPARE(result.skippedReason, QString("missing_audio_file"));
        QVERIFY(!result.ranBasicPitch);
        QVERIFY(reportHasIssue(result.report,
                               "missing_basic_pitch_input_audio_file"));
    }

    void basicPitchArtifactDiscoveryClassifiesSyntheticArtifacts()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        QVERIFY(writeFile(directory.filePath("input_basic_pitch.mid"),
                          QByteArray("MThd")));
        QVERIFY(writeFile(directory.filePath("input_basic_pitch.npz"),
                          QByteArray("npz")));
        QVERIFY(writeFile(directory.filePath("input_basic_pitch.csv"),
                          basicPitchNoteEventsCsvFixture().toUtf8()));
        QVERIFY(writeFile(directory.filePath("input_basic_pitch.wav"),
                          QByteArray("RIFF")));
        QVERIFY(writeFile(directory.filePath("basic_pitch_stdout.log"),
                          QByteArray("log")));

        BasicPitchArtifactDiscovery discovery;
        const BasicPitchArtifactDiscoveryResult result =
            discovery.inspectOutputDirectory(directory.path());

        QVERIFY(result.isValid());
        QCOMPARE(result.discoveredArtifacts.size(), 5);
        QVERIFY(hasDiscoveredArtifactType(result, "midi"));
        QVERIFY(hasDiscoveredArtifactType(result, "model_output_npz"));
        QVERIFY(hasDiscoveredArtifactType(result, "csv_note_events"));
        QVERIFY(hasDiscoveredArtifactType(result, "sonified_midi_wav"));
        QVERIFY(hasDiscoveredArtifactType(result, "log"));
        QVERIFY(!result.ranBasicPitch);
        QVERIFY(!result.productionTranscription);
        QVERIFY(!result.importedIntoTonyLayers);
    }

    void basicPitchArtifactDiscoveryConfigFromEnvironmentIsOptInOnly()
    {
        QProcessEnvironment emptyEnvironment;
        const BasicPitchArtifactDiscoveryConfig defaultConfig =
            BasicPitchArtifactDiscovery::configFromEnvironment(
                emptyEnvironment);

        QVERIFY(!defaultConfig.explicitOptIn);
        QCOMPARE(defaultConfig.executablePath, QString("basic-pitch"));

        QProcessEnvironment environment;
        environment.insert("TONY_BASIC_PITCH_DISCOVERY_ENABLE", "1");
        environment.insert("TONY_BASIC_PITCH_COMMAND", "python-basic-pitch");
        environment.insert("TONY_BASIC_PITCH_TEST_AUDIO",
                           "C:/audio/test.wav");
        environment.insert("TONY_BASIC_PITCH_OUTPUT_DIR",
                           "C:/audio/basic-pitch-output");

        const BasicPitchArtifactDiscoveryConfig config =
            BasicPitchArtifactDiscovery::configFromEnvironment(environment);

        QVERIFY(config.explicitOptIn);
        QCOMPARE(config.executablePath, QString("python-basic-pitch"));
        QCOMPARE(config.inputAudioPath, QString("C:/audio/test.wav"));
        QCOMPARE(config.outputDirectoryPath,
                 QString("C:/audio/basic-pitch-output"));
    }

    void basicPitchArtifactDiscoveryDoesNotCreateFakeResultOrState()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        QVERIFY(writeFile(directory.filePath("input_basic_pitch.csv"),
                          basicPitchNoteEventsCsvFixture().toUtf8()));

        BasicPitchArtifactDiscovery discovery;
        const BasicPitchArtifactDiscoveryResult result =
            discovery.inspectOutputDirectory(directory.path());

        QVERIFY(result.isValid());
        QVERIFY(!QFile::exists(directory.filePath("result.json")));
        QVERIFY(!result.ranBasicPitch);
        QVERIFY(!result.productionTranscription);
        QVERIFY(!result.importedIntoTonyLayers);
        QVERIFY(!result.readyInstalledCompletedMutation);

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.status = BackendStatus::NotConfigured;
        QVERIFY(manifest.status == BackendStatus::NotConfigured);
        QVERIFY(manifest.status != BackendStatus::Ready);

        BackendRegistry registry;
        QVERIFY(!registry.hasBackend("basic_pitch"));
        QVERIFY(registry.allManifests().isEmpty());
    }

    void basicPitchArtifactToUnifiedResultConvertsDiscoveredNoteEventsCsv()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString inputAudioPath = directory.filePath("input.wav");
        const QString csvPath =
            directory.filePath("input_basic_pitch.csv");
        QVERIFY(writeFile(inputAudioPath, QByteArray("audio")));
        QVERIFY(writeFile(csvPath, basicPitchNoteEventsCsvFixture().toUtf8()));

        BasicPitchArtifactDiscovery discovery;
        BasicPitchArtifactDiscoveryResult discovered =
            discovery.inspectOutputDirectory(directory.path());
        discovered.request.arguments << directory.path() << inputAudioPath;

        BasicPitchArtifactToUnifiedResultParameters parameters;
        parameters.requestId = "req_basic_pitch_artifact";
        parameters.resultId = "res_basic_pitch_artifact";

        BasicPitchArtifactToUnifiedResult converter;
        const BasicPitchArtifactToUnifiedResultResult converted =
            converter.convert(discovered, parameters);

        QVERIFY(converted.isValid());
        QVERIFY(converted.foundNoteEventsArtifact);
        QVERIFY(converted.conversionSucceeded);
        QVERIFY(converted.unifiedResultValidationPassed);
        QCOMPARE(converted.sourceArtifactPath, csvPath);
        QCOMPARE(converted.artifactType, QString("csv_note_events"));
        QCOMPARE(converted.fixtureOrRealArtifact,
                 QString("fixture_or_synthetic_artifact"));
        QCOMPARE(converted.noteCount, 3);
        QCOMPARE(converted.result.notes.size(), 3);
        QCOMPARE(converted.result.notes.at(0).startSec, 0.10);
        QCOMPARE(converted.result.notes.at(0).endSec, 0.50);
        QVERIFY(converted.result.notes.at(0).midiPitch.has_value());
        QCOMPARE(*converted.result.notes.at(0).midiPitch, 60);
        QVERIFY(converted.result.notes.at(0).velocity.has_value());
        QCOMPARE(*converted.result.notes.at(0).velocity, 91);
        QVERIFY(converted.result.notes.at(0).pitchBendRef.has_value());
        QCOMPARE(converted.result.pitchBends.size(), 2);
        QVERIFY(converted.possiblePolyphony);
        QVERIFY(converted.pitchBendMappingDeferred);
        QVERIFY(resultHasWarning(converted.result, "possible_polyphony"));
        QVERIFY(resultHasWarning(converted.result,
                                 "pitch_bend_mapping_deferred"));
        QVERIFY(resultHasWarning(converted.result,
                                 "fixture_only_conversion"));
        QCOMPARE(converted.result.provenance.value(
                     "source_artifact_type").toString(),
                 QString("csv_note_events"));
        QCOMPARE(converted.result.provenance.value(
                     "fixture_or_real_artifact").toString(),
                 QString("fixture_or_synthetic_artifact"));
        QCOMPARE(converted.result.provenance.value(
                     "production_transcription").toBool(),
                 false);
        QCOMPARE(converted.result.provenance.value(
                     "imported_into_tony_layers").toBool(),
                 false);
        QVERIFY(!QFile::exists(directory.filePath("result.json")));
        QVERIFY(!converted.createdResultJson);
        QVERIFY(!converted.importedIntoTonyLayers);
        QVERIFY(!converted.marksBackendReadyInstalledOrCompleted);
    }

    void basicPitchArtifactToUnifiedResultMissingNoteEventsFailsCleanly()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        QVERIFY(writeFile(directory.filePath("input_basic_pitch.mid"),
                          QByteArray("MThd")));

        BasicPitchArtifactDiscovery discovery;
        const BasicPitchArtifactDiscoveryResult discovered =
            discovery.inspectOutputDirectory(directory.path());

        BasicPitchArtifactToUnifiedResult converter;
        const BasicPitchArtifactToUnifiedResultResult converted =
            converter.convert(discovered,
                              BasicPitchArtifactToUnifiedResultParameters());

        QVERIFY(!converted.isValid());
        QVERIFY(!converted.foundNoteEventsArtifact);
        QVERIFY(!converted.conversionSucceeded);
        QCOMPARE(converted.noteCount, 0);
        QVERIFY(reportHasIssue(
            converted.report,
            "missing_basic_pitch_note_events_artifact"));
        QVERIFY(!converted.importedIntoTonyLayers);
        QVERIFY(!converted.createdResultJson);
    }

    void basicPitchArtifactToUnifiedResultIgnoresUnknownArtifacts()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        QVERIFY(writeFile(directory.filePath("unknown-artifact.bin"),
                          QByteArray("unknown")));

        BasicPitchArtifactDiscovery discovery;
        const BasicPitchArtifactDiscoveryResult discovered =
            discovery.inspectOutputDirectory(directory.path());

        QVERIFY(hasDiscoveredArtifactType(discovered, "unknown"));

        BasicPitchArtifactToUnifiedResult converter;
        const BasicPitchArtifactToUnifiedResultResult converted =
            converter.convert(discovered,
                              BasicPitchArtifactToUnifiedResultParameters());

        QVERIFY(!converted.isValid());
        QVERIFY(reportHasIssue(
            converted.report,
            "missing_basic_pitch_note_events_artifact"));
        QVERIFY(!converted.importedIntoTonyLayers);
        QVERIFY(!converted.marksBackendReadyInstalledOrCompleted);
    }

    void basicPitchArtifactToUnifiedResultEmptyCsvFailsCleanly()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString csvPath = directory.filePath("empty.csv");
        QVERIFY(writeFile(csvPath, QByteArray()));

        BasicPitchArtifactDiscoveryResult discovered =
            manualBasicPitchDiscoveryResultForArtifact(
                csvPath,
                "csv_note_events",
                false);

        BasicPitchArtifactToUnifiedResult converter;
        const BasicPitchArtifactToUnifiedResultResult converted =
            converter.convert(discovered,
                              BasicPitchArtifactToUnifiedResultParameters());

        QVERIFY(!converted.isValid());
        QVERIFY(converted.foundNoteEventsArtifact);
        QVERIFY(!converted.conversionSucceeded);
        QVERIFY(reportHasIssue(converted.report,
                               "empty_basic_pitch_note_events_csv"));
        QVERIFY(converted.result.notes.isEmpty());
        QVERIFY(!converted.importedIntoTonyLayers);
    }

    void basicPitchArtifactToUnifiedResultInvalidCsvSchemaFailsCleanly()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString csvPath = directory.filePath("invalid.csv");
        QVERIFY(writeFile(csvPath,
                          QByteArray("start,end,pitch\n0.10,0.20,60\n")));

        BasicPitchArtifactDiscoveryResult discovered =
            manualBasicPitchDiscoveryResultForArtifact(
                csvPath,
                "csv_note_events",
                false);

        BasicPitchArtifactToUnifiedResult converter;
        const BasicPitchArtifactToUnifiedResultResult converted =
            converter.convert(discovered,
                              BasicPitchArtifactToUnifiedResultParameters());

        QVERIFY(!converted.isValid());
        QVERIFY(converted.foundNoteEventsArtifact);
        QVERIFY(!converted.conversionSucceeded);
        QVERIFY(reportHasIssue(converted.report,
                               "invalid_basic_pitch_note_events_header"));
        QVERIFY(converted.result.notes.isEmpty());
        QVERIFY(!converted.createdResultJson);
    }

    void basicPitchArtifactToUnifiedResultRealArtifactStatusIsManualOnly()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString csvPath =
            directory.filePath("real_input_basic_pitch.csv");
        QVERIFY(writeFile(csvPath, basicPitchNoteEventsCsvFixture().toUtf8()));

        BasicPitchArtifactDiscoveryResult discovered =
            manualBasicPitchDiscoveryResultForArtifact(
                csvPath,
                "csv_note_events",
                true);
        discovered.request.arguments << directory.path()
                                     << directory.filePath("input.wav");

        BasicPitchArtifactToUnifiedResult converter;
        const BasicPitchArtifactToUnifiedResultResult converted =
            converter.convert(discovered,
                              BasicPitchArtifactToUnifiedResultParameters());

        QVERIFY(converted.isValid());
        QCOMPARE(converted.fixtureOrRealArtifact,
                 QString("real_discovered_artifact_manual_only"));
        QVERIFY(!converted.conversion.fixtureOnly);
        QVERIFY(!converted.productionTranscription);
        QVERIFY(!converted.importedIntoTonyLayers);
        QVERIFY(!converted.createdResultJson);
        QVERIFY(!converted.marksBackendReadyInstalledOrCompleted);
        QVERIFY(resultHasWarning(converted.result,
                                 "real_artifact_manual_only"));
        QVERIFY(resultHasWarning(converted.result,
                                 "production_transcription_false"));
        QVERIFY(!resultHasWarning(converted.result,
                                  "fixture_only_conversion"));
        QCOMPARE(converted.result.provenance.value(
                     "fixture_or_real_artifact").toString(),
                 QString("real_discovered_artifact_manual_only"));
        QCOMPARE(converted.result.provenance.value(
                     "artifact_discovery_ran_basic_pitch").toBool(),
                 true);

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.status = BackendStatus::NotConfigured;
        QVERIFY(manifest.status == BackendStatus::NotConfigured);
        QVERIFY(manifest.status != BackendStatus::Ready);

        BackendRegistry registry;
        QVERIFY(!registry.hasBackend("basic_pitch"));
        QVERIFY(registry.allManifests().isEmpty());
    }

    void basicPitchUnifiedResultHandoffWritesAndLoadsResultJson()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString inputAudioPath = directory.filePath("input.wav");
        const QString csvPath = directory.filePath("input_basic_pitch.csv");
        const QString resultPath = directory.filePath("result.json");
        QVERIFY(writeFile(inputAudioPath, QByteArray("audio")));
        QVERIFY(writeFile(csvPath, basicPitchNoteEventsCsvFixture().toUtf8()));

        BasicPitchArtifactDiscovery discovery;
        BasicPitchArtifactDiscoveryResult discovered =
            discovery.inspectOutputDirectory(directory.path());
        discovered.request.arguments << directory.path() << inputAudioPath;

        BasicPitchUnifiedResultHandoffParameters parameters;
        parameters.expectedUnifiedResultJsonPath = resultPath;
        parameters.conversionParameters.requestId =
            "req_basic_pitch_json_handoff";
        parameters.conversionParameters.resultId =
            "res_basic_pitch_json_handoff";

        BasicPitchUnifiedResultHandoff handoff;
        const BasicPitchUnifiedResultHandoffResult handedOff =
            handoff.handoff(discovered, parameters);

        QVERIFY(handedOff.isValid());
        QVERIFY(handedOff.wroteResultJson);
        QVERIFY(handedOff.outputHandoffAccepted);
        QVERIFY(handedOff.loadedUnifiedResult);
        QVERIFY(handedOff.reporterLoadedUnifiedResult);
        QVERIFY(QFile::exists(resultPath));
        QVERIFY(QFileInfo(resultPath).size() > 0);
        QCOMPARE(handedOff.noteCount, 3);
        QVERIFY(!handedOff.productionTranscription);
        QVERIFY(!handedOff.importedIntoTonyLayers);
        QVERIFY(!handedOff.marksBackendReadyInstalledOrCompleted);
        QVERIFY(handedOff.runResultReport.isValid());
        QVERIFY(handedOff.runResultReport.unifiedResultLoaded);
        QVERIFY(!handedOff.runResultReport.importedIntoTonyLayers);

        QVERIFY(handedOff.loadResult.loadedResult.has_value());
        const UnifiedResult loaded = *handedOff.loadResult.loadedResult;
        QVERIFY(loaded.status == BackendStatus::CompletedWithWarnings);
        QCOMPARE(loaded.notes.size(), 3);
        QCOMPARE(loaded.notes.at(0).startSec, 0.10);
        QCOMPARE(loaded.notes.at(0).endSec, 0.50);
        QVERIFY(loaded.notes.at(0).midiPitch.has_value());
        QCOMPARE(*loaded.notes.at(0).midiPitch, 60);
        QVERIFY(loaded.notes.at(0).velocity.has_value());
        QCOMPARE(*loaded.notes.at(0).velocity, 91);
        QCOMPARE(loaded.pitchBends.size(), 2);
        QCOMPARE(loaded.summary.noteCount, 3);
        QCOMPARE(loaded.provenance.value("created_result_json").toBool(),
                 true);
        QCOMPARE(loaded.provenance.value(
                     "unified_result_json_handoff").toBool(),
                 true);
        QCOMPARE(loaded.provenance.value(
                     "production_transcription").toBool(),
                 false);
        QCOMPARE(loaded.provenance.value(
                     "imported_into_tony_layers").toBool(),
                 false);
    }

    void basicPitchUnifiedResultHandoffPreservesWarnings()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString csvPath = directory.filePath("input_basic_pitch.csv");
        const QString resultPath = directory.filePath("result.json");
        QVERIFY(writeFile(csvPath, basicPitchNoteEventsCsvFixture().toUtf8()));

        BasicPitchArtifactDiscovery discovery;
        const BasicPitchArtifactDiscoveryResult discovered =
            discovery.inspectOutputDirectory(directory.path());

        BasicPitchUnifiedResultHandoffParameters parameters;
        parameters.expectedUnifiedResultJsonPath = resultPath;

        BasicPitchUnifiedResultHandoff handoff;
        const BasicPitchUnifiedResultHandoffResult handedOff =
            handoff.handoff(discovered, parameters);

        QVERIFY(handedOff.isValid());
        QVERIFY(handedOff.possiblePolyphony);
        QVERIFY(handedOff.pitchBendMappingDeferred);
        QVERIFY(handedOff.loadResult.loadedResult.has_value());
        const UnifiedResult loaded = *handedOff.loadResult.loadedResult;

        QVERIFY(resultHasWarning(loaded, "possible_polyphony"));
        QVERIFY(resultHasWarning(loaded, "pitch_bend_mapping_deferred"));
        QVERIFY(resultHasWarning(loaded, "fixture_only_conversion"));
        QVERIFY(resultHasWarning(loaded, "production_transcription_false"));
        QVERIFY(resultHasWarning(
            loaded,
            "basic_pitch_result_json_handoff_test_only"));
        QVERIFY(reportHasIssueWithSeverity(
            handedOff.report,
            "basic_pitch_result_json_handoff_test_only",
            ValidationSeverity::Warning));
    }

    void basicPitchUnifiedResultHandoffMissingArtifactFailsCleanly()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        QVERIFY(writeFile(directory.filePath("input_basic_pitch.mid"),
                          QByteArray("MThd")));

        BasicPitchArtifactDiscovery discovery;
        const BasicPitchArtifactDiscoveryResult discovered =
            discovery.inspectOutputDirectory(directory.path());

        BasicPitchUnifiedResultHandoffParameters parameters;
        parameters.expectedUnifiedResultJsonPath =
            directory.filePath("result.json");

        BasicPitchUnifiedResultHandoff handoff;
        const BasicPitchUnifiedResultHandoffResult handedOff =
            handoff.handoff(discovered, parameters);

        QVERIFY(!handedOff.isValid());
        QVERIFY(reportHasIssue(
            handedOff.report,
            "missing_basic_pitch_note_events_artifact"));
        QVERIFY(!handedOff.wroteResultJson);
        QVERIFY(!QFile::exists(parameters.expectedUnifiedResultJsonPath));
        QVERIFY(!handedOff.importedIntoTonyLayers);
        QVERIFY(!handedOff.marksBackendReadyInstalledOrCompleted);
    }

    void basicPitchUnifiedResultHandoffInvalidCsvFailsCleanly()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString csvPath = directory.filePath("invalid.csv");
        const QString resultPath = directory.filePath("result.json");
        QVERIFY(writeFile(csvPath,
                          QByteArray("start,end,pitch\n0.10,0.20,60\n")));

        BasicPitchArtifactDiscoveryResult discovered =
            manualBasicPitchDiscoveryResultForArtifact(
                csvPath,
                "csv_note_events",
                false);

        BasicPitchUnifiedResultHandoffParameters parameters;
        parameters.expectedUnifiedResultJsonPath = resultPath;

        BasicPitchUnifiedResultHandoff handoff;
        const BasicPitchUnifiedResultHandoffResult handedOff =
            handoff.handoff(discovered, parameters);

        QVERIFY(!handedOff.isValid());
        QVERIFY(reportHasIssue(handedOff.report,
                               "invalid_basic_pitch_note_events_header"));
        QVERIFY(!handedOff.wroteResultJson);
        QVERIFY(!QFile::exists(resultPath));
        QVERIFY(!handedOff.importedIntoTonyLayers);
    }

    void basicPitchUnifiedResultHandoffInvalidOutputPathFailsCleanly()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString csvPath =
            directory.filePath("input_basic_pitch.csv");
        QVERIFY(writeFile(csvPath, basicPitchNoteEventsCsvFixture().toUtf8()));

        BasicPitchArtifactDiscoveryResult discovered =
            manualBasicPitchDiscoveryResultForArtifact(
                csvPath,
                "csv_note_events",
                false);

        BasicPitchUnifiedResultHandoffParameters parameters;
        parameters.expectedUnifiedResultJsonPath =
            directory.filePath("missing-parent/result.json");

        BasicPitchUnifiedResultHandoff handoff;
        const BasicPitchUnifiedResultHandoffResult handedOff =
            handoff.handoff(discovered, parameters);

        QVERIFY(!handedOff.isValid());
        QVERIFY(handedOff.artifactConversion.isValid());
        QVERIFY(!handedOff.wroteResultJson);
        QVERIFY(reportHasIssue(handedOff.report,
                               "unified_result_output_parent_missing"));
        QVERIFY(!QFile::exists(parameters.expectedUnifiedResultJsonPath));
        QVERIFY(!handedOff.importedIntoTonyLayers);
        QVERIFY(!handedOff.marksBackendReadyInstalledOrCompleted);
    }

    void basicPitchUnifiedResultHandoffNeverMarksBackendReady()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString csvPath =
            directory.filePath("input_basic_pitch.csv");
        QVERIFY(writeFile(csvPath, basicPitchNoteEventsCsvFixture().toUtf8()));

        BasicPitchArtifactDiscoveryResult discovered =
            manualBasicPitchDiscoveryResultForArtifact(
                csvPath,
                "csv_note_events",
                false);

        BasicPitchUnifiedResultHandoffParameters parameters;
        parameters.expectedUnifiedResultJsonPath =
            directory.filePath("result.json");

        BasicPitchUnifiedResultHandoff handoff;
        const BasicPitchUnifiedResultHandoffResult handedOff =
            handoff.handoff(discovered, parameters);

        QVERIFY(handedOff.isValid());
        QVERIFY(!handedOff.productionTranscription);
        QVERIFY(!handedOff.importedIntoTonyLayers);
        QVERIFY(!handedOff.runResultReport.importedIntoTonyLayers);
        QVERIFY(!handedOff.marksBackendReadyInstalledOrCompleted);

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.status = BackendStatus::NotConfigured;
        QVERIFY(manifest.status == BackendStatus::NotConfigured);
        QVERIFY(manifest.status != BackendStatus::Ready);
        QVERIFY(manifest.status != BackendStatus::Completed);

        BackendRegistry registry;
        QVERIFY(!registry.hasBackend("basic_pitch"));
        QVERIFY(registry.allManifests().isEmpty());
    }

    void basicPitchRealRunHandoffProofSkipsWithoutOptIn()
    {
        QProcessEnvironment environment;
        BasicPitchRealRunHandoffProofConfig config =
            BasicPitchRealRunHandoffProof::configFromEnvironment(environment);

        BasicPitchRealRunHandoffProof proof;
        const BasicPitchRealRunHandoffProofResult result =
            proof.run(config);

        QVERIFY(result.isValid());
        QVERIFY(result.wasSkipped());
        QCOMPARE(result.skippedReason,
                 QString("explicit_opt_in_required"));
        QVERIFY(!result.ranBasicPitch);
        QVERIFY(!result.loadedResult);
        QVERIFY(!result.productionTranscription);
        QVERIFY(!result.importedIntoTonyLayers);
        QVERIFY(!result.readyInstalledCompletedMutation);
        QVERIFY(reportHasIssueWithSeverity(
            result.report,
            "basic_pitch_real_run_explicit_opt_in_required",
            ValidationSeverity::Warning));
    }

    void basicPitchRealRunHandoffProofConfigReadsEnvironment()
    {
        QProcessEnvironment environment;
        environment.insert("TONY_BASIC_PITCH_DISCOVERY_ENABLE", "1");
        environment.insert("TONY_BASIC_PITCH_COMMAND", "basic-pitch-test");
        environment.insert("TONY_BASIC_PITCH_TEST_AUDIO", "C:/audio/in.wav");
        environment.insert("TONY_BASIC_PITCH_OUTPUT_DIR",
                           "C:/audio/basic-pitch-out");
        environment.insert("TONY_BASIC_PITCH_RESULT_JSON",
                           "C:/audio/basic-pitch-out/result.json");

        const BasicPitchRealRunHandoffProofConfig config =
            BasicPitchRealRunHandoffProof::configFromEnvironment(environment);

        QVERIFY(config.discoveryConfig.explicitOptIn);
        QCOMPARE(config.discoveryConfig.executablePath,
                 QString("basic-pitch-test"));
        QCOMPARE(config.discoveryConfig.inputAudioPath,
                 QString("C:/audio/in.wav"));
        QCOMPARE(config.discoveryConfig.outputDirectoryPath,
                 QString("C:/audio/basic-pitch-out"));
        QCOMPARE(config.resultJsonPath,
                 QString("C:/audio/basic-pitch-out/result.json"));
    }

    void basicPitchRealRunHandoffProofMissingCommandFailsCleanly()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString inputAudioPath = directory.filePath("input.wav");
        QVERIFY(writeFile(inputAudioPath, QByteArray("audio")));

        BasicPitchRealRunHandoffProofConfig config;
        config.discoveryConfig.explicitOptIn = true;
        config.discoveryConfig.executablePath = " ";
        config.discoveryConfig.inputAudioPath = inputAudioPath;
        config.discoveryConfig.outputDirectoryPath = directory.path();
        config.resultJsonPath = directory.filePath("result.json");

        BasicPitchRealRunHandoffProof proof;
        const BasicPitchRealRunHandoffProofResult result =
            proof.run(config);

        QVERIFY(!result.isValid());
        QVERIFY(result.wasSkipped());
        QCOMPARE(result.skippedReason, QString("not_configured"));
        QVERIFY(!result.ranBasicPitch);
        QVERIFY(!result.loadedResult);
        QVERIFY(!QFile::exists(config.resultJsonPath));
        QVERIFY(reportHasIssue(result.report,
                               "empty_basic_pitch_discovery_command"));
        QVERIFY(!result.importedIntoTonyLayers);
        QVERIFY(!result.readyInstalledCompletedMutation);
    }

    void basicPitchRealRunHandoffProofMissingAudioFailsBeforeRunning()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString missingAudioPath =
            directory.filePath("missing-input.wav");

        BasicPitchRealRunHandoffProofConfig config;
        config.discoveryConfig.explicitOptIn = true;
        config.discoveryConfig.executablePath =
            QCoreApplication::applicationFilePath();
        config.discoveryConfig.inputAudioPath = missingAudioPath;
        config.discoveryConfig.outputDirectoryPath = directory.path();
        config.resultJsonPath = directory.filePath("result.json");

        BasicPitchRealRunHandoffProof proof;
        const BasicPitchRealRunHandoffProofResult result =
            proof.run(config);

        QVERIFY(!result.isValid());
        QVERIFY(result.wasSkipped());
        QCOMPARE(result.skippedReason, QString("missing_audio_file"));
        QVERIFY(!result.ranBasicPitch);
        QVERIFY(!result.loadedResult);
        QVERIFY(!QFile::exists(config.resultJsonPath));
        QVERIFY(reportHasIssue(result.report,
                               "missing_basic_pitch_input_audio_file"));
        QCOMPARE(result.discoveryResult.request.executablePath,
                 config.discoveryConfig.executablePath);
        QVERIFY(result.discoveryResult.request.arguments.size() >= 2);
        QCOMPARE(result.discoveryResult.request.arguments.at(0),
                 directory.path());
        QCOMPARE(result.discoveryResult.request.arguments.at(1),
                 missingAudioPath);
        QVERIFY(result.discoveryResult.request.arguments.contains(
                    "--save-note-events"));
        QVERIFY(!result.importedIntoTonyLayers);
        QVERIFY(!result.readyInstalledCompletedMutation);
    }

    void basicPitchRealRunHandoffProofMissingOutputDirectoryFailsSafely()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString inputAudioPath = directory.filePath("input.wav");
        const QString missingOutputDirectory =
            directory.filePath("missing-output");
        QVERIFY(writeFile(inputAudioPath, QByteArray("audio")));

        BasicPitchRealRunHandoffProofConfig config;
        config.discoveryConfig.explicitOptIn = true;
        config.discoveryConfig.executablePath =
            QCoreApplication::applicationFilePath();
        config.discoveryConfig.inputAudioPath = inputAudioPath;
        config.discoveryConfig.outputDirectoryPath = missingOutputDirectory;
        config.resultJsonPath =
            QDir(missingOutputDirectory).filePath("result.json");

        BasicPitchRealRunHandoffProof proof;
        const BasicPitchRealRunHandoffProofResult result =
            proof.run(config);

        QVERIFY(!result.isValid());
        QVERIFY(result.wasSkipped());
        QCOMPARE(result.skippedReason, QString("missing_output_directory"));
        QVERIFY(!result.ranBasicPitch);
        QVERIFY(!result.loadedResult);
        QVERIFY(!QFile::exists(config.resultJsonPath));
        QVERIFY(reportHasIssue(
            result.report,
            "missing_basic_pitch_discovery_output_directory"));
        QVERIFY(!result.importedIntoTonyLayers);
        QVERIFY(!result.readyInstalledCompletedMutation);
    }

    void basicPitchRealRunHandoffProofNeverMarksBackendReady()
    {
        BasicPitchRealRunHandoffProofConfig config;

        BasicPitchRealRunHandoffProof proof;
        const BasicPitchRealRunHandoffProofResult result =
            proof.run(config);

        QVERIFY(result.isValid());
        QVERIFY(result.wasSkipped());
        QVERIFY(!result.productionTranscription);
        QVERIFY(!result.importedIntoTonyLayers);
        QVERIFY(!result.readyInstalledCompletedMutation);

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.status = BackendStatus::NotConfigured;
        QVERIFY(manifest.status == BackendStatus::NotConfigured);
        QVERIFY(manifest.status != BackendStatus::Ready);
        QVERIFY(manifest.status != BackendStatus::Completed);

        BackendRegistry registry;
        QVERIFY(!registry.hasBackend("basic_pitch"));
        QVERIFY(registry.allManifests().isEmpty());
    }

    void basicPitchResultToTonyLayerProofCreatesDocumentNoteLayer()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString resultPath = directory.filePath("result.json");
        const BasicPitchUnifiedResultHandoffResult handedOff =
            basicPitchHandoffResultFromFixture(directory.path(), resultPath);
        QVERIFY(handedOff.isValid());

        sv::Document document;
        TonyLayerImportOptions options;
        options.sampleRate = 44100.0;
        options.resolution = 1;
        options.document = &document;
        options.createDocumentLayer = true;

        BasicPitchResultToTonyLayerProof proof;
        const BasicPitchResultToTonyLayerProofResult imported =
            proof.importHandoffResult(handedOff, options);

        QVERIFY(imported.isValid());
        QVERIFY(imported.loadedResult);
        QVERIFY(imported.basicPitchShaped);
        QVERIFY(imported.importedIntoTonyLayers);
        QVERIFY(!imported.insertedIntoView);
        QCOMPARE(imported.noteCount, 3);
        QVERIFY(imported.importResult.layer);
        QVERIFY(dynamic_cast<sv::NoteLayer *>(
                    imported.importResult.layer) != nullptr);
        QVERIFY(dynamic_cast<sv::FlexiNoteLayer *>(
                    imported.importResult.layer) == nullptr);
        QCOMPARE(imported.importResult.createdLayerType, QString("notes"));
        QVERIFY(imported.importResult.provenanceAttached);
        QVERIFY(!imported.importResult.structuredProvenancePersisted);

        const std::set<sv::Layer *> documentLayers = document.getLayers();
        QVERIFY(documentLayers.find(imported.importResult.layer) !=
                documentLayers.end());

        BackendManifest manifest = parsedBasicPitchManifest();
        QVERIFY(manifest.status == BackendStatus::NotConfigured);
        QVERIFY(manifest.status != BackendStatus::Ready);
        QVERIFY(manifest.status != BackendStatus::Completed);
    }

    void basicPitchResultToTonyLayerProofPreservesNoteData()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString resultPath = directory.filePath("result.json");
        const BasicPitchUnifiedResultHandoffResult handedOff =
            basicPitchHandoffResultFromFixture(directory.path(), resultPath);
        QVERIFY(handedOff.isValid());

        TonyLayerImportOptions options;
        options.sampleRate = 44100.0;
        options.resolution = 1;

        BasicPitchResultToTonyLayerProof proof;
        const BasicPitchResultToTonyLayerProofResult imported =
            proof.importHandoffResult(handedOff, options);

        QVERIFY(imported.isValid());
        QVERIFY(!imported.importedIntoTonyLayers);
        QVERIFY(imported.importResult.modelCreated);
        QVERIFY(imported.importResult.modelRegistered);
        QCOMPARE(imported.noteCount, 3);

        auto model =
            sv::ModelById::getAs<sv::NoteModel>(
                imported.importResult.modelId);
        QVERIFY(model);
        QCOMPARE(model->getScaleUnits(), QString("MIDI Pitch"));

        const sv::EventVector events = model->getAllEvents();
        QCOMPARE(int(events.size()), 3);
        QCOMPARE(events[0].getFrame(), sv::sv_frame_t(4410));
        QCOMPARE(events[0].getDuration(), sv::sv_frame_t(17640));
        QVERIFY(qAbs(events[0].getValue() - 60.0f) < 0.001f);
        QVERIFY(qAbs(events[0].getLevel() - (91.0f / 127.0f)) < 0.001f);

        QCOMPARE(events[1].getFrame(), sv::sv_frame_t(13230));
        QCOMPARE(events[1].getDuration(), sv::sv_frame_t(17640));
        QVERIFY(qAbs(events[1].getValue() - 64.0f) < 0.001f);
        QVERIFY(qAbs(events[1].getLevel() - (88.0f / 127.0f)) < 0.001f);

        QCOMPARE(events[2].getFrame(), sv::sv_frame_t(35280));
        QCOMPARE(events[2].getDuration(), sv::sv_frame_t(8820));
        QVERIFY(qAbs(events[2].getValue() - 67.0f) < 0.001f);
        QVERIFY(qAbs(events[2].getLevel() - (72.0f / 127.0f)) < 0.001f);
    }

    void basicPitchResultToTonyLayerProofPreservesWarnings()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString resultPath = directory.filePath("result.json");
        const BasicPitchUnifiedResultHandoffResult handedOff =
            basicPitchHandoffResultFromFixture(directory.path(), resultPath);
        QVERIFY(handedOff.isValid());

        TonyLayerImportOptions options;
        options.sampleRate = 44100.0;
        options.resolution = 1;

        BasicPitchResultToTonyLayerProof proof;
        const BasicPitchResultToTonyLayerProofResult imported =
            proof.importHandoffResult(handedOff, options);

        QVERIFY(imported.isValid());
        QVERIFY(imported.possiblePolyphony);
        QVERIFY(imported.pitchBendMappingDeferred);
        QVERIFY(imported.warningCodes.contains("possible_polyphony"));
        QVERIFY(imported.warningCodes.contains(
                    "pitch_bend_mapping_deferred"));
        QVERIFY(reportHasIssue(
            imported.report,
            "basic_pitch_possible_polyphony_not_resolved"));
        QVERIFY(reportHasIssue(
            imported.report,
            "basic_pitch_pitch_bend_tony_mapping_deferred"));
        QVERIFY(reportHasIssue(
            imported.report,
            "basic_pitch_result_to_tony_layer_test_only"));
    }

    void basicPitchResultToTonyLayerProofModelOnlyDoesNotClaimImported()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString resultPath = directory.filePath("result.json");
        const BasicPitchUnifiedResultHandoffResult handedOff =
            basicPitchHandoffResultFromFixture(directory.path(), resultPath);
        QVERIFY(handedOff.isValid());

        TonyLayerImportOptions options;
        options.sampleRate = 44100.0;
        options.resolution = 1;

        BasicPitchResultToTonyLayerProof proof;
        const BasicPitchResultToTonyLayerProofResult imported =
            proof.importResultJson(resultPath, options);

        QVERIFY(imported.isValid());
        QVERIFY(imported.importResult.modelCreated);
        QVERIFY(imported.importResult.modelRegistered);
        QVERIFY(!imported.importedIntoTonyLayers);
        QVERIFY(!imported.importResult.importedIntoTonyLayers);
        QVERIFY(!imported.insertedIntoView);
        QVERIFY(!imported.readyInstalledCompletedMutation);
    }

    void basicPitchResultToTonyLayerProofInsertsIntoPaneOnlyWhenProvided()
    {
        sv::CommandHistory::getInstance()->clear();

        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString resultPath = directory.filePath("result.json");
        const BasicPitchUnifiedResultHandoffResult handedOff =
            basicPitchHandoffResultFromFixture(directory.path(), resultPath);
        QVERIFY(handedOff.isValid());

        sv::Pane pane;
        sv::Document document;
        TonyLayerImportOptions options;
        options.sampleRate = 44100.0;
        options.resolution = 1;
        options.document = &document;
        options.createDocumentLayer = true;
        options.view = &pane;
        options.insertLayerIntoView = true;

        BasicPitchResultToTonyLayerProof proof;
        const BasicPitchResultToTonyLayerProofResult imported =
            proof.importHandoffResult(handedOff, options);

        QVERIFY(imported.isValid());
        QVERIFY(imported.importedIntoTonyLayers);
        QVERIFY(imported.insertedIntoView);
        QVERIFY(imported.importResult.layer);
        QCOMPARE(pane.getLayerCount(), 1);
        QCOMPARE(pane.getLayer(0), imported.importResult.layer);
        QVERIFY(dynamic_cast<sv::NoteLayer *>(
                    imported.importResult.layer) != nullptr);

        sv::CommandHistory::getInstance()->clear();
    }

    void basicPitchResultToTonyLayerProofMissingResultFailsCleanly()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        TonyLayerImportOptions options;
        options.sampleRate = 44100.0;
        options.resolution = 1;

        BasicPitchResultToTonyLayerProof proof;
        const BasicPitchResultToTonyLayerProofResult imported =
            proof.importResultJson(directory.filePath("missing-result.json"),
                                   options);

        QVERIFY(!imported.isValid());
        QVERIFY(!imported.loadedResult);
        QVERIFY(!imported.importedIntoTonyLayers);
        QVERIFY(!imported.insertedIntoView);
        QVERIFY(reportHasIssue(imported.report, "output_file_missing"));
    }

    void basicPitchResultToTonyLayerProofInvalidResultFailsCleanly()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString resultPath = directory.filePath("invalid-result.json");
        QVERIFY(writeFile(resultPath, QByteArray("{ invalid json\n")));

        TonyLayerImportOptions options;
        options.sampleRate = 44100.0;
        options.resolution = 1;

        BasicPitchResultToTonyLayerProof proof;
        const BasicPitchResultToTonyLayerProofResult imported =
            proof.importResultJson(resultPath, options);

        QVERIFY(!imported.isValid());
        QVERIFY(!imported.loadedResult);
        QVERIFY(!imported.importedIntoTonyLayers);
        QVERIFY(reportHasIssue(imported.report, "invalid_json"));
    }

    void basicPitchResultToTonyLayerProofEmptyResultFailsCleanly()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString resultPath = directory.filePath("empty-result.json");
        QVERIFY(writeFile(resultPath, QByteArray()));

        TonyLayerImportOptions options;
        options.sampleRate = 44100.0;
        options.resolution = 1;

        BasicPitchResultToTonyLayerProof proof;
        const BasicPitchResultToTonyLayerProofResult imported =
            proof.importResultJson(resultPath, options);

        QVERIFY(!imported.isValid());
        QVERIFY(!imported.loadedResult);
        QVERIFY(!imported.importedIntoTonyLayers);
        QVERIFY(reportHasIssue(imported.report, "empty_output_file"));
    }

    void basicPitchResultToTonyLayerProofNeverMarksBackendReady()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString resultPath = directory.filePath("result.json");
        const BasicPitchUnifiedResultHandoffResult handedOff =
            basicPitchHandoffResultFromFixture(directory.path(), resultPath);
        QVERIFY(handedOff.isValid());

        sv::Document document;
        TonyLayerImportOptions options;
        options.sampleRate = 44100.0;
        options.resolution = 1;
        options.document = &document;
        options.createDocumentLayer = true;

        BasicPitchResultToTonyLayerProof proof;
        const BasicPitchResultToTonyLayerProofResult imported =
            proof.importHandoffResult(handedOff, options);

        QVERIFY(imported.isValid());
        QVERIFY(!imported.productionTranscription);
        QVERIFY(!imported.readyInstalledCompletedMutation);

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.status = BackendStatus::NotConfigured;
        QVERIFY(manifest.status == BackendStatus::NotConfigured);
        QVERIFY(manifest.status != BackendStatus::Ready);
        QVERIFY(manifest.status != BackendStatus::Completed);

        BackendRegistry registry;
        QVERIFY(!registry.hasBackend("basic_pitch"));
        QVERIFY(registry.allManifests().isEmpty());
    }

    void basicPitchResultToTonyLayerProofRealRunPathSkipsByDefault()
    {
        QProcessEnvironment environment;
        const BasicPitchRealRunHandoffProofConfig config =
            BasicPitchRealRunHandoffProof::configFromEnvironment(environment);

        BasicPitchRealRunHandoffProof realRunProof;
        const BasicPitchRealRunHandoffProofResult realRunResult =
            realRunProof.run(config);
        QVERIFY(realRunResult.isValid());
        QVERIFY(realRunResult.wasSkipped());

        TonyLayerImportOptions options;
        options.sampleRate = 44100.0;
        options.resolution = 1;

        BasicPitchResultToTonyLayerProof proof;
        const BasicPitchResultToTonyLayerProofResult imported =
            proof.importRealRunResult(realRunResult, options);

        QVERIFY(!imported.isValid());
        QVERIFY(!imported.loadedResult);
        QVERIFY(!imported.importedIntoTonyLayers);
        QVERIFY(!imported.insertedIntoView);
        QVERIFY(!imported.readyInstalledCompletedMutation);
        QVERIFY(reportHasIssueWithSeverity(
            imported.report,
            "basic_pitch_real_run_explicit_opt_in_required",
            ValidationSeverity::Warning));
    }

    void basicPitchLayerPersistenceExportProofSavesLoadsAndExports()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString resultPath = directory.filePath("result.json");
        const QString exportPath = directory.filePath("basic-pitch-notes.csv");
        const BasicPitchUnifiedResultHandoffResult handedOff =
            basicPitchHandoffResultFromFixture(directory.path(), resultPath);
        QVERIFY(handedOff.isValid());

        BasicPitchLayerPersistenceExportProofOptions options;
        options.resultJsonPath = resultPath;
        options.exportCsvPath = exportPath;

        BasicPitchLayerPersistenceExportProof proof;
        const BasicPitchLayerPersistenceExportProofResult proven =
            proof.proveHandoffResult(handedOff, options);

        QVERIFY2(proven.isValid(), qPrintable(proven.debugSummaryString()));
        QVERIFY(proven.loadedResult);
        QVERIFY(proven.importedIntoTonyLayers);
        QVERIFY(proven.insertedIntoView);
        QVERIFY(proven.reloadedLayerIsNoteLayer);
        QVERIFY(proven.reloadedModelIsNoteModel);
        QVERIFY(proven.reloadedLayerEditable);
        QVERIFY(proven.saveLoadProven);
        QVERIFY(proven.exportProven);
        QVERIFY(proven.durableIdentityPersisted);
        QCOMPARE(proven.importedNoteCount, 3);
        QCOMPARE(proven.reloadedNoteCount, 3);
        QCOMPARE(proven.exportedNoteCount, 3);
        QVERIFY(proven.sessionXml.contains("type=\"notes\""));
        QVERIFY(proven.sessionXml.contains("backend=basic_pitch"));
        QVERIFY(proven.sessionXml.contains("test_only=true"));
        QVERIFY(QFile::exists(exportPath));
        QVERIFY(QFileInfo(exportPath).size() > 0);

        QString exported = proven.exportCsvText;
        exported.replace("\r\n", "\n");
        exported.replace('\r', '\n');
        const QStringList lines = exported.trimmed().split('\n');
        QCOMPARE(lines.size(), 4);
        QCOMPARE(lines[0], QString("FRAME,VALUE,DURATION,LEVEL,LABEL"));

        const QStringList first = lines[1].split(',');
        QCOMPARE(first.size(), 5);
        QCOMPARE(first[0], QString("4410"));
        QVERIFY(qAbs(first[1].toFloat() - 60.0f) < 0.001f);
        QCOMPARE(first[2], QString("17640"));
        QVERIFY(qAbs(first[3].toFloat() - (91.0f / 127.0f)) < 0.001f);

        const QStringList second = lines[2].split(',');
        QCOMPARE(second[0], QString("13230"));
        QVERIFY(qAbs(second[1].toFloat() - 64.0f) < 0.001f);
        QCOMPARE(second[2], QString("17640"));
        QVERIFY(qAbs(second[3].toFloat() - (88.0f / 127.0f)) < 0.001f);

        const QStringList third = lines[3].split(',');
        QCOMPARE(third[0], QString("35280"));
        QVERIFY(qAbs(third[1].toFloat() - 67.0f) < 0.001f);
        QCOMPARE(third[2], QString("8820"));
        QVERIFY(qAbs(third[3].toFloat() - (72.0f / 127.0f)) < 0.001f);
    }

    void basicPitchLayerPersistenceExportProofPreservesWarnings()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString resultPath = directory.filePath("result.json");
        const BasicPitchUnifiedResultHandoffResult handedOff =
            basicPitchHandoffResultFromFixture(directory.path(), resultPath);
        QVERIFY(handedOff.isValid());

        BasicPitchLayerPersistenceExportProofOptions options;
        options.exportCsvPath = directory.filePath("basic-pitch-notes.csv");

        BasicPitchLayerPersistenceExportProof proof;
        const BasicPitchLayerPersistenceExportProofResult proven =
            proof.proveHandoffResult(handedOff, options);

        QVERIFY(proven.isValid());
        QVERIFY(proven.possiblePolyphony);
        QVERIFY(proven.pitchBendMappingDeferred);
        QVERIFY(reportHasIssue(
            proven.report,
            "basic_pitch_possible_polyphony_not_resolved"));
        QVERIFY(reportHasIssue(
            proven.report,
            "basic_pitch_pitch_bend_tony_mapping_deferred"));
        QVERIFY(reportHasIssue(
            proven.report,
            "basic_pitch_layer_persistence_export_test_only"));
        QVERIFY(!proven.productionTranscription);
    }

    void basicPitchLayerPersistenceExportProofMissingResultFailsCleanly()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        BasicPitchLayerPersistenceExportProofOptions options;
        options.resultJsonPath = directory.filePath("missing-result.json");
        options.exportCsvPath = directory.filePath("basic-pitch-notes.csv");

        BasicPitchLayerPersistenceExportProof proof;
        const BasicPitchLayerPersistenceExportProofResult proven =
            proof.prove(options);

        QVERIFY(!proven.isValid());
        QVERIFY(!proven.loadedResult);
        QVERIFY(!proven.importedIntoTonyLayers);
        QVERIFY(!proven.saveLoadProven);
        QVERIFY(!proven.exportProven);
        QVERIFY(reportHasIssue(proven.report, "output_file_missing"));
    }

    void basicPitchLayerPersistenceExportProofRequiresExportPath()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString resultPath = directory.filePath("result.json");
        const BasicPitchUnifiedResultHandoffResult handedOff =
            basicPitchHandoffResultFromFixture(directory.path(), resultPath);
        QVERIFY(handedOff.isValid());

        BasicPitchLayerPersistenceExportProofOptions options;
        options.resultJsonPath = resultPath;

        BasicPitchLayerPersistenceExportProof proof;
        const BasicPitchLayerPersistenceExportProofResult proven =
            proof.proveHandoffResult(handedOff, options);

        QVERIFY(!proven.isValid());
        QVERIFY(proven.loadedResult);
        QVERIFY(proven.importedIntoTonyLayers);
        QVERIFY(proven.insertedIntoView);
        QVERIFY(proven.saveLoadProven);
        QVERIFY(!proven.exportProven);
        QVERIFY(reportHasIssue(proven.report,
                               "empty_basic_pitch_export_csv_path"));
    }

    void basicPitchLayerPersistenceExportProofNeverMarksBackendReady()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString resultPath = directory.filePath("result.json");
        const BasicPitchUnifiedResultHandoffResult handedOff =
            basicPitchHandoffResultFromFixture(directory.path(), resultPath);
        QVERIFY(handedOff.isValid());

        BasicPitchLayerPersistenceExportProofOptions options;
        options.exportCsvPath = directory.filePath("basic-pitch-notes.csv");

        BasicPitchLayerPersistenceExportProof proof;
        const BasicPitchLayerPersistenceExportProofResult proven =
            proof.proveHandoffResult(handedOff, options);

        QVERIFY(proven.isValid());
        QVERIFY(!proven.productionTranscription);
        QVERIFY(!proven.readyInstalledCompletedMutation);

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.status = BackendStatus::NotConfigured;
        QVERIFY(manifest.status == BackendStatus::NotConfigured);
        QVERIFY(manifest.status != BackendStatus::Ready);
        QVERIFY(manifest.status != BackendStatus::Completed);

        BackendRegistry registry;
        QVERIFY(!registry.hasBackend("basic_pitch"));
        QVERIFY(registry.allManifests().isEmpty());
    }

    void basicPitchDebugWorkflowMissingConfigSkipsSafely()
    {
        BasicPitchDebugWorkflow workflow;
        BasicPitchDebugWorkflowRequest request;
        request.mode =
            BasicPitchDebugWorkflowMode::RealBasicPitchManualOptIn;

        const BasicPitchDebugWorkflowReport report = workflow.run(request);

        QVERIFY2(report.isValid(), qPrintable(report.debugSummaryString()));
        QVERIFY(report.wasSkipped());
        QVERIFY(report.hasTruthState(
            BasicPitchDebugTruthState::BackendNotConfigured));
        QVERIFY(report.hasTruthState(BasicPitchDebugTruthState::Skipped));
        QVERIFY(!report.ranBasicPitch);
        QVERIFY(!report.resultJsonWritten);
        QVERIFY(!report.importedIntoTonyLayers);
        QVERIFY(!report.readyInstalledCompletedMutation);
        QVERIFY(!report.productionTranscription);
        QVERIFY(report.testOnlyDebugOnly);
    }

    void basicPitchDebugWorkflowSyntheticArtifactReachesResultAndLoaded()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString inputAudioPath = directory.filePath("input.wav");
        const QString csvPath = directory.filePath("input_basic_pitch.csv");
        const QString resultPath = directory.filePath("result.json");
        const QString exportPath = directory.filePath("basic-pitch-notes.csv");
        QVERIFY(writeFile(inputAudioPath, QByteArray("audio")));
        QVERIFY(writeFile(csvPath, basicPitchNoteEventsCsvFixture().toUtf8()));

        BasicPitchDebugWorkflowRequest request;
        request.mode = BasicPitchDebugWorkflowMode::SyntheticArtifactOnly;
        request.outputDirectoryPath = directory.path();
        request.inputAudioPath = inputAudioPath;
        request.resultJsonPath = resultPath;
        request.exportCsvPath = exportPath;

        BasicPitchDebugWorkflow workflow;
        const BasicPitchDebugWorkflowReport report = workflow.run(request);

        QVERIFY2(report.isValid(), qPrintable(report.debugSummaryString()));
        QVERIFY(report.hasTruthState(
            BasicPitchDebugTruthState::ArtifactsDiscovered));
        QVERIFY(report.hasTruthState(
            BasicPitchDebugTruthState::ResultJsonWritten));
        QVERIFY(report.hasTruthState(
            BasicPitchDebugTruthState::UnifiedResultLoaded));
        QVERIFY(QFile::exists(resultPath));
        QVERIFY(QFileInfo(resultPath).size() > 0);
        QVERIFY(report.resultJsonWritten);
        QVERIFY(report.unifiedResultLoaded);
        QCOMPARE(report.proofBundle.noteCount, 3);
        QVERIFY(!report.ranBasicPitch);
        QVERIFY(report.proofBundle.testOnlyDebugOnly);
    }

    void basicPitchDebugWorkflowImportsAndInsertsRealLayer()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString inputAudioPath = directory.filePath("input.wav");
        const QString csvPath = directory.filePath("input_basic_pitch.csv");
        QVERIFY(writeFile(inputAudioPath, QByteArray("audio")));
        QVERIFY(writeFile(csvPath, basicPitchNoteEventsCsvFixture().toUtf8()));

        BasicPitchDebugWorkflowRequest request;
        request.mode = BasicPitchDebugWorkflowMode::SyntheticArtifactOnly;
        request.outputDirectoryPath = directory.path();
        request.inputAudioPath = inputAudioPath;
        request.resultJsonPath = directory.filePath("result.json");
        request.exportCsvPath = directory.filePath("basic-pitch-notes.csv");

        BasicPitchDebugWorkflow workflow;
        const BasicPitchDebugWorkflowReport report = workflow.run(request);

        QVERIFY(report.isValid());
        QVERIFY(report.importedIntoTonyLayers);
        QVERIFY(report.insertedIntoView);
        QVERIFY(report.editProofPassed);
        QVERIFY(report.hasTruthState(
            BasicPitchDebugTruthState::ImportedIntoRealLayer));
        QVERIFY(report.hasTruthState(BasicPitchDebugTruthState::InsertedIntoView));
        QVERIFY(report.hasTruthState(BasicPitchDebugTruthState::EditProofPassed));
        QVERIFY(!report.proofBundle.documentPaneLayerModelSnapshotSummary
                     .trimmed()
                     .isEmpty());
    }

    void basicPitchDebugWorkflowReportsSaveLoadAndExportProof()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString inputAudioPath = directory.filePath("input.wav");
        const QString csvPath = directory.filePath("input_basic_pitch.csv");
        const QString exportPath = directory.filePath("basic-pitch-notes.csv");
        QVERIFY(writeFile(inputAudioPath, QByteArray("audio")));
        QVERIFY(writeFile(csvPath, basicPitchNoteEventsCsvFixture().toUtf8()));

        BasicPitchDebugWorkflowRequest request;
        request.outputDirectoryPath = directory.path();
        request.inputAudioPath = inputAudioPath;
        request.resultJsonPath = directory.filePath("result.json");
        request.exportCsvPath = exportPath;

        BasicPitchDebugWorkflow workflow;
        const BasicPitchDebugWorkflowReport report = workflow.run(request);

        QVERIFY(report.isValid());
        QVERIFY(report.saveLoadProofPassed);
        QVERIFY(report.exportProofPassed);
        QVERIFY(report.hasTruthState(
            BasicPitchDebugTruthState::SaveLoadProofPassed));
        QVERIFY(report.hasTruthState(
            BasicPitchDebugTruthState::ExportProofPassed));
        QVERIFY(QFile::exists(exportPath));
        QVERIFY(QFileInfo(exportPath).size() > 0);
        QVERIFY(report.proofBundle.saveLoadProof);
        QVERIFY(report.proofBundle.exportProof);
    }

    void basicPitchDebugWorkflowPreservesWarnings()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString inputAudioPath = directory.filePath("input.wav");
        const QString csvPath = directory.filePath("input_basic_pitch.csv");
        QVERIFY(writeFile(inputAudioPath, QByteArray("audio")));
        QVERIFY(writeFile(csvPath, basicPitchNoteEventsCsvFixture().toUtf8()));

        BasicPitchDebugWorkflowRequest request;
        request.outputDirectoryPath = directory.path();
        request.inputAudioPath = inputAudioPath;
        request.resultJsonPath = directory.filePath("result.json");
        request.exportCsvPath = directory.filePath("basic-pitch-notes.csv");

        BasicPitchDebugWorkflow workflow;
        const BasicPitchDebugWorkflowReport report = workflow.run(request);

        QVERIFY(report.isValid());
        QVERIFY(report.possiblePolyphony);
        QVERIFY(report.pitchBendMappingDeferred);
        QVERIFY(report.hasTruthState(
            BasicPitchDebugTruthState::CompletedWithWarnings));
        QVERIFY(report.proofBundle.warningCodes.contains("possible_polyphony"));
        QVERIFY(report.proofBundle.warningCodes.contains(
            "pitch_bend_mapping_deferred"));
        QVERIFY(report.proofBundle.warningCodes.contains(
            "basic_pitch_debug_workflow_test_only"));
    }

    void basicPitchDebugWorkflowNeverClaimsProductionOrBackendState()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString inputAudioPath = directory.filePath("input.wav");
        const QString csvPath = directory.filePath("input_basic_pitch.csv");
        QVERIFY(writeFile(inputAudioPath, QByteArray("audio")));
        QVERIFY(writeFile(csvPath, basicPitchNoteEventsCsvFixture().toUtf8()));

        BasicPitchDebugWorkflowRequest request;
        request.outputDirectoryPath = directory.path();
        request.inputAudioPath = inputAudioPath;
        request.resultJsonPath = directory.filePath("result.json");
        request.exportCsvPath = directory.filePath("basic-pitch-notes.csv");

        BasicPitchDebugWorkflow workflow;
        const BasicPitchDebugWorkflowReport report = workflow.run(request);

        QVERIFY(report.isValid());
        QVERIFY(!report.productionTranscription);
        QVERIFY(report.testOnlyDebugOnly);
        QVERIFY(!report.readyInstalledCompletedMutation);
        QVERIFY(!report.proofBundle.productionTranscription);
        QVERIFY(report.proofBundle.testOnlyDebugOnly);

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.status = BackendStatus::NotConfigured;
        BackendRegistry registry;
        QVERIFY(registry.addManifest(manifest));
        const std::optional<BackendManifest> storedManifest =
            registry.manifestById(manifest.id());
        QVERIFY(storedManifest.has_value());
        QVERIFY(storedManifest->status == BackendStatus::NotConfigured);
    }

    void basicPitchDebugWorkflowUiModelMissingConfigDisablesRun()
    {
        BasicPitchDebugWorkflow workflow;
        BasicPitchDebugWorkflowRequest request;
        request.mode =
            BasicPitchDebugWorkflowMode::RealBasicPitchManualOptIn;
        const BasicPitchDebugWorkflowReport workflowReport =
            workflow.run(request);

        BasicPitchDebugWorkflowUiModel model;
        const BasicPitchDebugWorkflowUiModelResult ui =
            model.fromReport(workflowReport);

        QVERIFY2(ui.isValid(), qPrintable(ui.debugSummaryString()));
        QCOMPARE(ui.primaryState, QString("backend_not_configured"));
        QCOMPARE(ui.secondaryState, QString("skipped"));
        QVERIFY(!ui.canRun);
        QVERIFY(!ui.canCancel);
        QVERIFY(!ui.canImport);
        QVERIFY(!ui.canEdit);
        QVERIFY(!ui.canSave);
        QVERIFY(!ui.canExport);
        QVERIFY(ui.shouldShowWarnings);
        QVERIFY(ui.shouldShowProofBundle);
        QVERIFY(!ui.productionTranscription);
        QVERIFY(ui.testOnlyDebugOnly);
    }

    void basicPitchDebugWorkflowUiModelSkippedDoesNotClaimRun()
    {
        BasicPitchDebugWorkflowReport workflowReport;
        workflowReport.skippedReason = "manual_test_skip";
        workflowReport.truthStates.push_back(
            BasicPitchDebugTruthState::Skipped);
        workflowReport.testOnlyDebugOnly = true;
        workflowReport.proofBundle.testOnlyDebugOnly = true;

        BasicPitchDebugWorkflowUiModel model;
        const BasicPitchDebugWorkflowUiModelResult ui =
            model.fromReport(workflowReport);

        QVERIFY(ui.isValid());
        QCOMPARE(ui.primaryState, QString("skipped"));
        QVERIFY(ui.secondaryState.isEmpty());
        QVERIFY(!ui.canRun);
        QVERIFY(!ui.canCancel);
        QVERIFY(!ui.canImport);
        QVERIFY(!ui.proofBundle.loadedResult);
        QVERIFY(!ui.proofBundle.importedIntoTonyLayers);
    }

    void basicPitchDebugWorkflowUiModelLoadedButNotVisible()
    {
        BasicPitchDebugWorkflowReport workflowReport;
        workflowReport.truthStates.push_back(
            BasicPitchDebugTruthState::UnifiedResultLoaded);
        workflowReport.unifiedResultLoaded = true;
        workflowReport.proofBundle.loadedResult = true;
        workflowReport.proofBundle.noteCount = 2;
        workflowReport.testOnlyDebugOnly = true;
        workflowReport.proofBundle.testOnlyDebugOnly = true;

        BasicPitchDebugWorkflowUiModel model;
        const BasicPitchDebugWorkflowUiModelResult ui =
            model.fromReport(workflowReport);

        QVERIFY(ui.isValid());
        QCOMPARE(ui.primaryState, QString("unified_result_loaded"));
        QCOMPARE(ui.secondaryState, QString("loaded_not_visible"));
        QVERIFY(ui.canImport);
        QVERIFY(!ui.canEdit);
        QVERIFY(!ui.canSave);
        QVERIFY(!ui.canExport);
        QVERIFY(!ui.proofBundle.importedIntoTonyLayers);
        QVERIFY(!ui.proofBundle.insertedIntoView);
    }

    void basicPitchDebugWorkflowUiModelDoesNotMapUninsertedLayerToVisible()
    {
        BasicPitchDebugWorkflowReport workflowReport;
        workflowReport.truthStates.push_back(
            BasicPitchDebugTruthState::ImportedIntoRealLayer);
        workflowReport.unifiedResultLoaded = true;
        workflowReport.importedIntoTonyLayers = true;
        workflowReport.proofBundle.loadedResult = true;
        workflowReport.proofBundle.importedIntoTonyLayers = true;
        workflowReport.testOnlyDebugOnly = true;
        workflowReport.proofBundle.testOnlyDebugOnly = true;

        BasicPitchDebugWorkflowUiModel model;
        const BasicPitchDebugWorkflowUiModelResult ui =
            model.fromReport(workflowReport);

        QVERIFY(ui.isValid());
        QCOMPARE(ui.primaryState, QString("imported_into_real_layer"));
        QCOMPARE(ui.secondaryState, QString("imported_not_visible"));
        QVERIFY(!ui.canImport);
        QVERIFY(!ui.canEdit);
        QVERIFY(!ui.canSave);
        QVERIFY(!ui.canExport);
        QVERIFY(ui.proofBundle.importedIntoTonyLayers);
        QVERIFY(!ui.proofBundle.insertedIntoView);
    }

    void basicPitchDebugWorkflowUiModelRequiresEditSaveExportProof()
    {
        BasicPitchDebugWorkflowReport workflowReport;
        workflowReport.truthStates.push_back(
            BasicPitchDebugTruthState::InsertedIntoView);
        workflowReport.unifiedResultLoaded = true;
        workflowReport.importedIntoTonyLayers = true;
        workflowReport.insertedIntoView = true;
        workflowReport.proofBundle.loadedResult = true;
        workflowReport.proofBundle.importedIntoTonyLayers = true;
        workflowReport.proofBundle.insertedIntoView = true;
        workflowReport.testOnlyDebugOnly = true;
        workflowReport.proofBundle.testOnlyDebugOnly = true;

        BasicPitchDebugWorkflowUiModel model;
        const BasicPitchDebugWorkflowUiModelResult ui =
            model.fromReport(workflowReport);

        QVERIFY(ui.isValid());
        QCOMPARE(ui.primaryState, QString("inserted_into_view"));
        QVERIFY(!ui.canEdit);
        QVERIFY(!ui.canSave);
        QVERIFY(!ui.canExport);
        QVERIFY(!ui.proofBundle.editProof);
        QVERIFY(!ui.proofBundle.saveLoadProof);
        QVERIFY(!ui.proofBundle.exportProof);
    }

    void basicPitchDebugWorkflowUiModelCompletedWarningsAreVisible()
    {
        const BasicPitchDebugWorkflowReport workflowReport =
            completedBasicPitchDebugWorkflowReportForUiModel();
        QVERIFY(workflowReport.isValid());

        BasicPitchDebugWorkflowUiModel model;
        const BasicPitchDebugWorkflowUiModelResult ui =
            model.fromReport(workflowReport);

        QVERIFY(ui.isValid());
        QCOMPARE(ui.primaryState, QString("completed_with_warnings"));
        QCOMPARE(ui.secondaryState, QString("export_proof_passed"));
        QVERIFY(ui.shouldShowWarnings);
        QVERIFY(ui.warnings.contains("possible_polyphony"));
        QVERIFY(ui.warnings.contains("pitch_bend_mapping_deferred"));
        QVERIFY(ui.warnings.contains("fixture_only_conversion"));
        QVERIFY(ui.warnings.contains("production_transcription_false"));
        QVERIFY(ui.canEdit);
        QVERIFY(ui.canSave);
        QVERIFY(ui.canExport);
        QVERIFY(ui.shouldShowProofBundle);
        QCOMPARE(ui.proofBundle.noteCount, 3);
        QVERIFY(ui.proofBundle.importedIntoTonyLayers);
        QVERIFY(ui.proofBundle.insertedIntoView);
        QVERIFY(ui.proofBundle.saveLoadProof);
        QVERIFY(ui.proofBundle.exportProof);
    }

    void basicPitchDebugWorkflowUiModelNeverCreatesFakeReadyCompletedState()
    {
        const BasicPitchDebugWorkflowReport workflowReport =
            completedBasicPitchDebugWorkflowReportForUiModel();

        BasicPitchDebugWorkflowUiModel model;
        const BasicPitchDebugWorkflowUiModelResult ui =
            model.fromReport(workflowReport);

        QVERIFY(ui.isValid());
        QVERIFY(ui.primaryState != "ready");
        QVERIFY(ui.primaryState != "installed");
        QVERIFY(ui.primaryState != "completed");
        QVERIFY(!ui.productionTranscription);
        QVERIFY(ui.testOnlyDebugOnly);
        QVERIFY(!ui.readyInstalledCompletedMutation);

        BackendManifest manifest = parsedBasicPitchManifest();
        manifest.status = BackendStatus::NotConfigured;
        BackendRegistry registry;
        QVERIFY(registry.addManifest(manifest));
        const std::optional<BackendManifest> storedManifest =
            registry.manifestById(manifest.id());
        QVERIFY(storedManifest.has_value());
        QVERIFY(storedManifest->status == BackendStatus::NotConfigured);
    }

private:
    static BasicPitchDebugWorkflowReport
    completedBasicPitchDebugWorkflowReportForUiModel()
    {
        BasicPitchDebugWorkflowReport report;
        report.truthStates.push_back(
            BasicPitchDebugTruthState::ArtifactsDiscovered);
        report.truthStates.push_back(
            BasicPitchDebugTruthState::ResultJsonWritten);
        report.truthStates.push_back(
            BasicPitchDebugTruthState::UnifiedResultLoaded);
        report.truthStates.push_back(
            BasicPitchDebugTruthState::ImportedIntoRealLayer);
        report.truthStates.push_back(
            BasicPitchDebugTruthState::InsertedIntoView);
        report.truthStates.push_back(
            BasicPitchDebugTruthState::EditProofPassed);
        report.truthStates.push_back(
            BasicPitchDebugTruthState::SaveLoadProofPassed);
        report.truthStates.push_back(
            BasicPitchDebugTruthState::ExportProofPassed);
        report.truthStates.push_back(
            BasicPitchDebugTruthState::CompletedWithWarnings);

        report.resultJsonWritten = true;
        report.unifiedResultLoaded = true;
        report.importedIntoTonyLayers = true;
        report.insertedIntoView = true;
        report.editProofPassed = true;
        report.saveLoadProofPassed = true;
        report.exportProofPassed = true;
        report.possiblePolyphony = true;
        report.pitchBendMappingDeferred = true;
        report.productionTranscription = false;
        report.testOnlyDebugOnly = true;
        report.readyInstalledCompletedMutation = false;

        report.proofBundle.resultJsonPath = "basic_pitch_debug_result.json";
        report.proofBundle.loadedResult = true;
        report.proofBundle.noteCount = 3;
        report.proofBundle.importedIntoTonyLayers = true;
        report.proofBundle.insertedIntoView = true;
        report.proofBundle.editProof = true;
        report.proofBundle.saveLoadProof = true;
        report.proofBundle.exportProof = true;
        report.proofBundle.productionTranscription = false;
        report.proofBundle.testOnlyDebugOnly = true;
        report.proofBundle.warningCodes << "possible_polyphony"
                                        << "pitch_bend_mapping_deferred"
                                        << "fixture_only_conversion";
        BasicPitchDebugWorkflowEvent event;
        event.state = BasicPitchDebugTruthState::CompletedWithWarnings;
        event.message = "Synthetic UI model fixture.";
        report.proofBundle.events.push_back(event);

        report.report.addIssue(
            ValidationSeverity::Warning,
            "possible_polyphony",
            "Basic Pitch fixture includes possible polyphony.");
        report.report.addIssue(
            ValidationSeverity::Warning,
            "pitch_bend_mapping_deferred",
            "Pitch bend mapping remains deferred for this UI model proof.");
        report.report.addIssue(
            ValidationSeverity::Warning,
            "fixture_only_conversion",
            "This UI model proof uses fixture-backed conversion only.");
        return report;
    }

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

    static bool resultHasWarning(const UnifiedResult &result,
                                 const QString &code)
    {
        for (const auto &warning: result.warnings) {
            if (warning.code == code) {
                return true;
            }
        }
        return false;
    }

    static bool hasDiscoveredArtifactType(
        const BasicPitchArtifactDiscoveryResult &result,
        const QString &artifactType)
    {
        for (const auto &artifact: result.discoveredArtifacts) {
            if (artifact.artifactType == artifactType) {
                return true;
            }
        }
        return false;
    }

    static BasicPitchArtifactDiscoveryResult
    manualBasicPitchDiscoveryResultForArtifact(const QString &path,
                                               const QString &artifactType,
                                               bool ranBasicPitch)
    {
        BasicPitchArtifactDiscoveryResult result;
        result.ranBasicPitch = ranBasicPitch;
        result.outputDirectoryPath = QFileInfo(path).absolutePath();

        BasicPitchDiscoveredArtifact artifact;
        artifact.path = path;
        artifact.fileName = QFileInfo(path).fileName();
        artifact.artifactType = artifactType;
        artifact.sizeBytes = QFileInfo(path).size();
        result.discoveredArtifacts.push_back(artifact);

        return result;
    }

    static BasicPitchUnifiedResultHandoffResult
    basicPitchHandoffResultFromFixture(const QString &directoryPath,
                                       const QString &resultPath)
    {
        const QString inputAudioPath =
            QDir(directoryPath).filePath("input.wav");
        const QString csvPath =
            QDir(directoryPath).filePath("input_basic_pitch.csv");

        writeFile(inputAudioPath, QByteArray("audio"));
        writeFile(csvPath, basicPitchNoteEventsCsvFixture().toUtf8());

        BasicPitchArtifactDiscovery discovery;
        BasicPitchArtifactDiscoveryResult discovered =
            discovery.inspectOutputDirectory(directoryPath);
        discovered.request.arguments << directoryPath << inputAudioPath;

        BasicPitchUnifiedResultHandoffParameters parameters;
        parameters.expectedUnifiedResultJsonPath = resultPath;
        parameters.conversionParameters.requestId =
            "req_basic_pitch_result_to_layer";
        parameters.conversionParameters.resultId =
            "res_basic_pitch_result_to_layer";

        BasicPitchUnifiedResultHandoff handoff;
        return handoff.handoff(discovered, parameters);
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

    static ExternalProcessResult successfulProcessResult()
    {
        ExternalProcessResult result;
        result.state = AnalysisRunState::Completed;
        result.started = true;
        result.exitCode = 0;
        result.exitStatus = QProcess::NormalExit;
        result.error = {
            BackendErrorCode::None,
            QString(),
            false
        };
        return result;
    }

    static ExternalProcessResult failedProcessResult()
    {
        ExternalProcessResult result;
        result.state = AnalysisRunState::Failed;
        result.started = true;
        result.exitCode = 7;
        result.exitStatus = QProcess::NormalExit;
        result.error = {
            BackendErrorCode::ExecutionFailed,
            "External backend process exited with code 7.",
            false
        };
        return result;
    }

    static ExternalProcessResult timedOutProcessResult()
    {
        ExternalProcessResult result;
        result.state = AnalysisRunState::Failed;
        result.started = true;
        result.timedOut = true;
        result.exitCode = -1;
        result.error = {
            BackendErrorCode::TimedOut,
            "External backend process timed out.",
            true
        };
        return result;
    }

    static ExternalProcessResult cancelledProcessResult()
    {
        ExternalProcessResult result;
        result.state = AnalysisRunState::Cancelled;
        result.started = true;
        result.cancelled = true;
        result.exitCode = -1;
        result.error = {
            BackendErrorCode::Cancelled,
            "External backend process was cancelled.",
            true
        };
        return result;
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

    static BackendManifest devMockBackendManifest()
    {
        BackendManifest manifest;
        manifest.contractVersion = "0.1";
        manifest.backendId = "dev_mock_backend";
        manifest.engineId = manifest.backendId;
        manifest.displayName = "Dev Mock Backend (test only)";
        manifest.description =
            "Test-only backend helper for backend pipeline proof.";
        manifest.backendType = BackendRuntimeType::DevelopmentTest;
        manifest.runtimeType = BackendRuntimeType::DevelopmentTest;
        manifest.executablePath = QCoreApplication::applicationFilePath();
        manifest.version = "0.1.0-test";
        manifest.engineVersion = "0.1.0-test";
        manifest.adapterVersion = "0.1.0-test";
        manifest.status = BackendStatus::NotConfigured;
        manifest.capabilities.supportsFullFile = true;
        manifest.capabilities.supportsSelectedRegion = true;
        manifest.capabilities.outputsNotes = true;
        manifest.capabilities.supportsCpu = true;
        manifest.supportedInputFormats << "wav";
        manifest.supportedOutputTypes << "notes";
        manifest.primaryOutputs << "notes";
        return manifest;
    }

    static UnifiedResult validTonyLayerImportUnifiedResult()
    {
        UnifiedResult result;
        result.contractVersion = "0.1";
        result.resultId = "dev_mock_layer_import_result";
        result.requestId = "dev_mock_layer_import_request";
        result.createdAt = QDateTime::fromString("2026-05-17T12:00:00Z",
                                                 Qt::ISODate);
        result.engine.engineId = "dev_mock_backend";
        result.engine.displayName = "Dev Mock Backend (test only)";
        result.engine.adapterVersion = "0.1.0-test";
        result.engine.runtimeType = BackendRuntimeType::DevelopmentTest;
        result.status = BackendStatus::CompletedWithWarnings;
        result.provenance.insert("dev_mock", true);
        result.provenance.insert("test_only", true);
        result.provenance.insert("production_transcription", false);

        NoteEvent first;
        first.id = "dev_mock_note_a";
        first.startSec = 0.25;
        first.endSec = 0.75;
        first.midiPitch = 60;
        first.velocity = 100;
        first.confidence = 0.81;
        first.label = QString("dev-mock-note-a");
        first.flags << "dev_mock" << "test_only";
        first.source.insert("dev_mock", true);
        first.source.insert("test_only", true);

        NoteEvent second;
        second.id = "dev_mock_note_b";
        second.startSec = 1.0;
        second.endSec = 1.25;
        second.midiPitch = 64;
        second.confidence = 0.74;
        second.label = QString("dev-mock-note-b");
        second.flags << "dev_mock" << "test_only";
        second.source.insert("dev_mock", true);
        second.source.insert("test_only", true);

        result.notes.push_back(first);
        result.notes.push_back(second);
        result.summary.noteCount = int(result.notes.size());

        return result;
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

    static QByteArray validBackendRunResultJson()
    {
        return R"json(
{
  "contract_version": "0.1",
  "result_id": "res_backend_run_loader_001",
  "request_id": "req_backend_run_loader_001",
  "created_at": "2026-05-17T12:01:00Z",
  "engine": {
    "engine_id": "basic_pitch",
    "display_name": "Basic Pitch",
    "engine_version": null,
    "adapter_version": "0.1.0",
    "runtime_type": "python_cli",
    "device_used": "cpu"
  },
  "status": "completed",
  "audio": {
    "path": "C:/audio/input.wav",
    "duration_sec": 12.345,
    "sample_rate_hz": 44100,
    "channels": 1
  },
  "region": null,
  "summary": {
    "note_count": 1,
    "pitch_point_count": 0,
    "pitch_bend_count": 0,
    "technique_label_count": 0,
    "mean_confidence": 0.91,
    "low_confidence_count": 0,
    "duration_analyzed_sec": 12.345
  },
  "notes": [
    {
      "id": "note_0001",
      "start_sec": 1.24,
      "end_sec": 1.68,
      "midi_pitch": 64,
      "frequency_hz": 329.63,
      "velocity": 82,
      "confidence": 0.91,
      "source": { "engine_id": "basic_pitch" },
      "flags": []
    }
  ],
  "pitch_curve": [],
  "pitch_bends": [],
  "technique_labels": [],
  "files": [],
  "warnings": [],
  "errors": [],
  "provenance": {
    "created_by": "backend_run_result_loader_test"
  }
}
)json";
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

    static QString basicPitchNoteEventsCsvFixture()
    {
        return QString(
            "start_time_s,end_time_s,pitch_midi,velocity,pitch_bend\n"
            "0.10,0.50,60,91,0,12,-8\n"
            "0.30,0.70,64,88,1,0,-1\n"
            "0.80,1.00,67,72\n");
    }
};

#endif
