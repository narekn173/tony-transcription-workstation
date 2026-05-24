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

#include "BasicPitchLayerPersistenceExportProof.h"

#include "data/fileio/CSVFileWriter.h"
#include "data/model/NoteModel.h"
#include "base/Selection.h"
#include "framework/Document.h"
#include "framework/SVFileReader.h"
#include "layer/FlexiNoteLayer.h"
#include "layer/NoteLayer.h"
#include "view/Pane.h"
#include "widgets/CommandHistory.h"

#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QSize>
#include <QTextStream>
#include <QtGlobal>

#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

namespace Tony {
namespace Backend {

namespace {

class ProofPaneCallback : public sv::SVFileReaderPaneCallback
{
public:
    std::vector<std::unique_ptr<sv::Pane>> panes;

    sv::Pane *addPane() override
    {
        panes.push_back(std::make_unique<sv::Pane>());
        return panes.back().get();
    }

    void setWindowSize(int width, int height) override
    {
        windowSize = QSize(width, height);
    }

    void addSelection(sv::sv_frame_t, sv::sv_frame_t) override
    {
    }

    QSize windowSize;
};

QString
serializeDocumentPaneSessionXml(const sv::Document &document,
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

QString
readTextFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }
    return QString::fromUtf8(file.readAll());
}

bool
eventsMatch(const sv::EventVector &expected,
            const sv::EventVector &actual)
{
    if (expected.size() != actual.size()) {
        return false;
    }

    for (int i = 0; i < int(expected.size()); ++i) {
        const sv::Event &lhs = expected[i];
        const sv::Event &rhs = actual[i];
        if (lhs.getFrame() != rhs.getFrame()) return false;
        if (lhs.getDuration() != rhs.getDuration()) return false;
        if (std::fabs(lhs.getValue() - rhs.getValue()) >= 0.001f) return false;
        if (std::fabs(lhs.getLevel() - rhs.getLevel()) >= 0.001f) return false;
        if (lhs.getLabel() != rhs.getLabel()) return false;
    }

    return true;
}

bool
csvRowsMatchEvents(const QString &csvText,
                   const sv::EventVector &events,
                   int &exportedNoteCount)
{
    QString normalized = csvText;
    normalized.replace("\r\n", "\n");
    normalized.replace('\r', '\n');
    const QString trimmed = normalized.trimmed();
    if (trimmed.isEmpty()) {
        exportedNoteCount = 0;
        return false;
    }

    const QStringList lines = trimmed.split('\n');
    if (lines.size() != int(events.size()) + 1) {
        exportedNoteCount = qMax(0, lines.size() - 1);
        return false;
    }
    if (lines[0] != "FRAME,VALUE,DURATION,LEVEL,LABEL") {
        exportedNoteCount = qMax(0, lines.size() - 1);
        return false;
    }

    exportedNoteCount = 0;
    for (int i = 0; i < int(events.size()); ++i) {
        const QStringList fields = lines[i + 1].split(',');
        if (fields.size() != 5) {
            return false;
        }

        const sv::Event &event = events[i];
        if (fields[0] != QString::number(event.getFrame())) return false;
        if (fields[2] != QString::number(event.getDuration())) return false;
        if (std::fabs(fields[1].toFloat() - event.getValue()) >= 0.001f) {
            return false;
        }
        if (std::fabs(fields[3].toFloat() - event.getLevel()) >= 0.001f) {
            return false;
        }
        if (fields[4] != event.getLabel()) return false;
        ++exportedNoteCount;
    }

    return true;
}

sv::MultiSelection
selectionSpanningEvents(const sv::EventVector &events)
{
    sv::MultiSelection selection;
    if (events.empty()) {
        return selection;
    }

    sv::sv_frame_t startFrame = events.front().getFrame();
    sv::sv_frame_t endFrame =
        events.front().getFrame() +
        std::max<sv::sv_frame_t>(events.front().getDuration(), 1);

    for (const sv::Event &event: events) {
        startFrame = std::min(startFrame, event.getFrame());
        endFrame = std::max(
            endFrame,
            event.getFrame() +
                std::max<sv::sv_frame_t>(event.getDuration(), 1));
    }

    selection.addSelection(sv::Selection(startFrame, endFrame));
    return selection;
}

bool
completeProof(BasicPitchLayerPersistenceExportProofResult &result,
              sv::Document &document,
              sv::Pane &pane,
              const BasicPitchLayerPersistenceExportProofOptions &options)
{
    if (!result.layerImportResult.isValid() ||
        !result.layerImportResult.importResult.layer) {
        return false;
    }

    result.loadedResult = result.layerImportResult.loadedResult;
    result.importedIntoTonyLayers =
        result.layerImportResult.importedIntoTonyLayers;
    result.insertedIntoView = result.layerImportResult.insertedIntoView;
    result.importedNoteCount = result.layerImportResult.noteCount;
    result.possiblePolyphony = result.layerImportResult.possiblePolyphony;
    result.pitchBendMappingDeferred =
        result.layerImportResult.pitchBendMappingDeferred;
    result.productionTranscription =
        result.layerImportResult.productionTranscription;
    result.readyInstalledCompletedMutation =
        result.layerImportResult.readyInstalledCompletedMutation;
    result.provenanceIdentity =
        result.layerImportResult.importResult.provenanceIdentity;

    auto importedModel = sv::ModelById::getAs<sv::NoteModel>(
        result.layerImportResult.importResult.modelId);
    if (!importedModel) {
        result.report.addError(
            "basic_pitch_imported_model_missing",
            "Basic Pitch imported layer did not expose a real NoteModel.");
        return false;
    }
    const sv::EventVector importedEvents = importedModel->getAllEvents();

    result.sessionXml = serializeDocumentPaneSessionXml(document, pane);
    if (result.sessionXml.trimmed().isEmpty()) {
        result.report.addError(
            "basic_pitch_session_xml_empty",
            "Basic Pitch imported layer produced empty session XML.");
        return false;
    }

    ProofPaneCallback callback;
    sv::Document reloadedDocument;
    sv::SVFileReader reader(&reloadedDocument,
                            callback,
                            "basic-pitch-layer-persistence-proof");
    reader.parseXml(result.sessionXml);
    if (!reader.isOK()) {
        result.report.addError(
            "basic_pitch_session_reload_failed",
            reader.getErrorString());
        return false;
    }
    if (callback.panes.size() != 1 || !callback.panes.front()) {
        result.report.addError(
            "basic_pitch_reloaded_pane_missing",
            "Basic Pitch session reload did not restore one real Pane.");
        return false;
    }

    sv::Pane *reloadedPane = callback.panes.front().get();
    if (reloadedPane->getLayerCount() != 1) {
        result.report.addError(
            "basic_pitch_reloaded_layer_count_mismatch",
            "Basic Pitch session reload did not restore exactly one layer.");
        return false;
    }

    sv::Layer *reloadedLayer = reloadedPane->getLayer(0);
    result.reloadedLayerIsNoteLayer =
        dynamic_cast<sv::NoteLayer *>(reloadedLayer) != nullptr &&
        dynamic_cast<sv::FlexiNoteLayer *>(reloadedLayer) == nullptr;
    result.reloadedLayerEditable =
        reloadedLayer && reloadedLayer->isLayerEditable();
    if (!result.reloadedLayerIsNoteLayer || !result.reloadedLayerEditable) {
        result.report.addError(
            "basic_pitch_reloaded_layer_not_editable_note_layer",
            "Basic Pitch session reload did not restore an editable NoteLayer.");
        return false;
    }

    result.durableIdentityPersisted =
        !result.provenanceIdentity.trimmed().isEmpty() &&
        reloadedLayer->objectName() == result.provenanceIdentity &&
        reloadedLayer->getLayerPresentationName() == result.provenanceIdentity;

    auto reloadedModel =
        sv::ModelById::getAs<sv::NoteModel>(reloadedLayer->getModel());
    result.reloadedModelIsNoteModel = bool(reloadedModel);
    if (!reloadedModel) {
        result.report.addError(
            "basic_pitch_reloaded_model_not_note_model",
            "Basic Pitch session reload did not restore a real NoteModel.");
        return false;
    }

    const sv::EventVector reloadedEvents = reloadedModel->getAllEvents();
    result.reloadedNoteCount = int(reloadedEvents.size());
    result.saveLoadProven = eventsMatch(importedEvents, reloadedEvents);
    if (!result.saveLoadProven) {
        result.report.addError(
            "basic_pitch_save_load_note_data_mismatch",
            "Basic Pitch imported note timing, duration, pitch, velocity, "
            "or labels did not survive session save/load.");
        return false;
    }

    if (options.exportCsvPath.trimmed().isEmpty()) {
        result.report.addError(
            "empty_basic_pitch_export_csv_path",
            "Basic Pitch layer export proof requires a non-empty CSV path.");
        return false;
    }

    const sv::ModelId exportModelId =
        reloadedLayer->getExportModel(reloadedPane);
    auto exportModel = sv::ModelById::get(exportModelId);
    if (!exportModel) {
        result.report.addError(
            "basic_pitch_export_model_missing",
            "Basic Pitch imported layer did not expose a real export model.");
        return false;
    }

    sv::CSVFileWriter writer(
        result.exportCsvPath,
        exportModel.get(),
        ",",
        sv::DataExportWriteTimeInFrames | sv::DataExportIncludeHeader);
    writer.writeSelection(selectionSpanningEvents(reloadedEvents));
    if (!writer.isOK()) {
        result.report.addError(
            "basic_pitch_csv_export_failed",
            writer.getError());
        return false;
    }
    if (!QFile::exists(result.exportCsvPath) ||
        QFileInfo(result.exportCsvPath).size() <= 0) {
        result.report.addError(
            "basic_pitch_csv_export_missing",
            "Basic Pitch CSV export file was not created or was empty.");
        return false;
    }

    result.exportCsvText = readTextFile(result.exportCsvPath);
    result.exportedCsvNonEmpty = !result.exportCsvText.trimmed().isEmpty();
    result.exportedTimingDurationPitchVelocity =
        csvRowsMatchEvents(result.exportCsvText,
                           reloadedEvents,
                           result.exportedNoteCount);
    result.exportProven =
        result.exportedCsvNonEmpty &&
        result.exportedTimingDurationPitchVelocity;
    if (!result.exportProven) {
        result.report.addError(
            "basic_pitch_csv_export_note_data_mismatch",
            "Basic Pitch exported CSV did not preserve expected note count, "
            "timing, duration, MIDI pitch, velocity level, or labels.");
        return false;
    }

    result.report.addIssue(
        ValidationSeverity::Warning,
        "basic_pitch_layer_persistence_export_test_only",
        "Basic Pitch layer save/load/export proof is manual/test-only and "
        "is not production Basic Pitch UI integration.");

    return true;
}

TonyLayerImportOptions
documentPaneImportOptions(sv::Document &document,
                          sv::Pane &pane,
                          double sampleRate,
                          int resolution)
{
    TonyLayerImportOptions options;
    options.sampleRate = sampleRate;
    options.resolution = resolution;
    options.document = &document;
    options.createDocumentLayer = true;
    options.view = &pane;
    options.insertLayerIntoView = true;
    return options;
}

}

bool
BasicPitchLayerPersistenceExportProofResult::isValid() const
{
    return report.isValid() &&
        layerImportResult.isValid() &&
        loadedResult &&
        importedIntoTonyLayers &&
        insertedIntoView &&
        reloadedLayerIsNoteLayer &&
        reloadedModelIsNoteModel &&
        reloadedLayerEditable &&
        saveLoadProven &&
        exportProven &&
        durableIdentityPersisted &&
        !productionTranscription &&
        !readyInstalledCompletedMutation;
}

QString
BasicPitchLayerPersistenceExportProofResult::debugSummaryString() const
{
    return QString("basic_pitch_layer_persistence_export result=%1 notes=%2 "
                   "reloaded=%3 exported=%4 save_load=%5 export=%6 "
                   "polyphony=%7 bends_deferred=%8 production=%9 "
                   "ready_mutation=%10 valid=%11")
        .arg(resultJsonPath)
        .arg(importedNoteCount)
        .arg(reloadedNoteCount)
        .arg(exportedNoteCount)
        .arg(saveLoadProven ? QString("true") : QString("false"))
        .arg(exportProven ? QString("true") : QString("false"))
        .arg(possiblePolyphony ? QString("true") : QString("false"))
        .arg(pitchBendMappingDeferred ? QString("true") : QString("false"))
        .arg(productionTranscription ? QString("true") : QString("false"))
        .arg(readyInstalledCompletedMutation ? QString("true") :
                                               QString("false"))
        .arg(isValid() ? QString("true") : QString("false"));
}

BasicPitchLayerPersistenceExportProofResult
BasicPitchLayerPersistenceExportProof::prove(
    const BasicPitchLayerPersistenceExportProofOptions &options) const
{
    sv::CommandHistory::getInstance()->clear();

    BasicPitchLayerPersistenceExportProofResult result;
    result.resultJsonPath = options.resultJsonPath.trimmed();
    result.exportCsvPath = options.exportCsvPath.trimmed();
    result.productionTranscription = false;
    result.readyInstalledCompletedMutation = false;

    sv::Pane pane;
    sv::Document document;
    const TonyLayerImportOptions importOptions =
        documentPaneImportOptions(document,
                                  pane,
                                  options.sampleRate,
                                  options.resolution);

    BasicPitchResultToTonyLayerProof proof;
    result.layerImportResult =
        proof.importResultJson(result.resultJsonPath, importOptions);
    appendIssues(result.report, result.layerImportResult.report);

    completeProof(result, document, pane, options);
    sv::CommandHistory::getInstance()->clear();
    return result;
}

BasicPitchLayerPersistenceExportProofResult
BasicPitchLayerPersistenceExportProof::proveHandoffResult(
    const BasicPitchUnifiedResultHandoffResult &handoffResult,
    const BasicPitchLayerPersistenceExportProofOptions &options) const
{
    sv::CommandHistory::getInstance()->clear();

    BasicPitchLayerPersistenceExportProofOptions resolvedOptions = options;
    if (resolvedOptions.resultJsonPath.trimmed().isEmpty()) {
        resolvedOptions.resultJsonPath =
            handoffResult.expectedUnifiedResultJsonPath;
    }

    BasicPitchLayerPersistenceExportProofResult result;
    result.resultJsonPath = resolvedOptions.resultJsonPath.trimmed();
    result.exportCsvPath = resolvedOptions.exportCsvPath.trimmed();
    result.productionTranscription = false;
    result.readyInstalledCompletedMutation = false;

    sv::Pane pane;
    sv::Document document;
    const TonyLayerImportOptions importOptions =
        documentPaneImportOptions(document,
                                  pane,
                                  resolvedOptions.sampleRate,
                                  resolvedOptions.resolution);

    BasicPitchResultToTonyLayerProof proof;
    result.layerImportResult =
        proof.importHandoffResult(handoffResult, importOptions);
    appendIssues(result.report, result.layerImportResult.report);

    completeProof(result, document, pane, resolvedOptions);
    sv::CommandHistory::getInstance()->clear();
    return result;
}

void
BasicPitchLayerPersistenceExportProof::appendIssues(
    ValidationReport &target,
    const ValidationReport &source)
{
    for (const ValidationIssue &issue: source.issues) {
        target.addIssue(issue.severity, issue.code, issue.message);
    }
}

}
}
