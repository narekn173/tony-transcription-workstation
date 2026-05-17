# Tony Layer Edit, Save, and Export Audit

Status: CODEX-087B research/documentation only.

This audit documents the source-backed requirements for future backend result import into Tony/Sonic Visualiser layers, with special focus on editability, undo-redo, session persistence, load behavior, and export behavior.

No runtime behavior, C++ source, UI, pYIN behavior, backend execution, or layer import code was changed for this audit.

Highest proof gate reached: Source Inspection Gate only. No layer import, edit proof, save/load proof, or export proof was executed in this task.

## Source Files Inspected

- `AGENTS.md`
- `docs/00_PROJECT_INDEX.md`
- `docs/engineering/REAL_RESULT_AND_TONY_LAYER_INTEGRATION_RULES.md`
- `docs/engineering/LAYER_TYPE_POLICY.md`
- `docs/engineering/BACKEND_OUTPUT_TRUTH_TABLE.md`
- `docs/engineering/TONY_EDIT_SAVE_EXPORT_PROOF_PLAN.md`
- `docs/engineering/UI_VISUAL_TRUTH_STATES.md`
- `docs/engineering/PROVENANCE_METADATA_POLICY.md`
- `docs/engineering/TONY_SOURCE_ARCHITECTURE_AUDIT.md`
- `main/MainWindow.cpp`
- `main/MainWindow.h`
- `main/Analyser.cpp`
- `main/Analyser.h`
- `svapp/framework/Document.cpp`
- `svapp/framework/Document.h`
- `svapp/framework/MainWindowBase.cpp`
- `svapp/framework/MainWindowBase.h`
- `svapp/framework/SVFileReader.cpp`
- `svapp/framework/SVFileReader.h`
- `svgui/layer/Layer.cpp`
- `svgui/layer/Layer.h`
- `svgui/layer/LayerFactory.cpp`
- `svgui/layer/LayerFactory.h`
- `svgui/layer/NoteLayer.cpp`
- `svgui/layer/NoteLayer.h`
- `svgui/layer/FlexiNoteLayer.cpp`
- `svgui/layer/FlexiNoteLayer.h`
- `svgui/layer/TimeValueLayer.cpp`
- `svgui/layer/TimeValueLayer.h`
- `svgui/layer/TimeInstantLayer.cpp`
- `svgui/layer/RegionLayer.cpp`
- `svgui/layer/TextLayer.cpp`
- `svgui/view/View.cpp`
- `svgui/view/View.h`
- `svgui/view/Pane.cpp`
- `svgui/view/Pane.h`
- `svgui/view/ViewManager.cpp`
- `svgui/widgets/CommandHistory.cpp`
- `svgui/widgets/CommandHistory.h`
- `svgui/widgets/ModelDataTableDialog.cpp`
- `svcore/base/Command.cpp`
- `svcore/base/Command.h`
- `svcore/base/Event.h`
- `svcore/base/NoteData.h`
- `svcore/base/Selection.h`
- `svcore/data/model/EventCommands.h`
- `svcore/data/model/Model.cpp`
- `svcore/data/model/Model.h`
- `svcore/data/model/ModelDataTableModel.cpp`
- `svcore/data/model/NoteModel.h`
- `svcore/data/model/SparseTimeValueModel.h`
- `svcore/data/model/TabularModel.h`
- `svcore/data/fileio/CSVFileWriter.cpp`
- `svcore/data/fileio/CSVFileWriter.h`
- `svcore/data/fileio/MIDIFileWriter.cpp`
- `svcore/data/fileio/MIDIFileWriter.h`
- `svcore/rdf/RDFExporter.cpp`
- `svcore/rdf/RDFImporter.cpp`

No separate `FlexiNoteModel` class was found. Flexible note behavior is represented by `NoteModel` with subtype `NoteModel::FLEXI_NOTE`, displayed through `FlexiNoteLayer`.

## Concrete Findings

### 1. Genuinely Editable Note And Pitch Layer Types

`Layer::isLayerEditable()` defaults to false. The inspected layer classes that explicitly return true include:

- `NoteLayer`
- `FlexiNoteLayer`
- `TimeValueLayer`
- `TimeInstantLayer`
- `RegionLayer`
- `TextLayer`
- `BoxLayer`
- `ImageLayer`

For backend note and pitch work, the relevant editable candidates are:

| Output | Model candidate | Layer candidate | Source-backed editability |
|---|---|---|---|
| Normal notes | `NoteModel` with `NoteModel::NORMAL_NOTE` | `NoteLayer` | Editable through draw, erase, drag, dialog edit, selection move/resize/delete, copy/paste, and table commands |
| Tony-style flexible notes | `NoteModel` with `NoteModel::FLEXI_NOTE` | `FlexiNoteLayer` | Editable through note split, add, snap to pitch, merge, draw, erase, drag, selection move/resize/delete, copy/paste, and table commands |
| Pitch/f0 curve | `SparseTimeValueModel` | `TimeValueLayer` | Editable as time-value points through draw, erase, drag, dialog edit, selection move/resize/delete, copy/paste, and table commands |

For pYIN, Tony currently uses:

- pitch: `SparseTimeValueModel` plus `TimeValueLayer`;
- notes: `NoteModel::FLEXI_NOTE` plus `FlexiNoteLayer`.

Future backend import must not call a result editable unless the imported result lands in one of these real editable model/layer paths and edit behavior is proven.

### 2. How Note Creation, Deletion, Movement, Resizing, And Pitch/Time Edits Are Represented

Sparse editable data is represented with immutable-style `Event` objects.

Relevant `Event` fields:

- frame: start frame;
- value: pitch/frequency/value depending on model units;
- duration: note or region duration in frames;
- level: note level/velocity-like value where present;
- label: optional text;
- URI: optional URI field;
- reference frame: clipboard alignment helper.

Note model event shape:

- `NoteModel` stores `Event` objects with frame, value, duration, optional level, and label.
- `NoteModel::toXml()` writes a sparse model with `dimensions="3"`, dataset id, `subtype="note"` or `subtype="flexinote"`, scale units, min/max, and the event dataset.
- `NoteModel` implements `NoteExportable`, so it can be exported to MIDI.

Pitch/time-value model event shape:

- `SparseTimeValueModel` stores `Event` objects with frame, value, and label. It strips duration during `add(Event)`.
- `SparseTimeValueModel::toXml()` writes a sparse model with `dimensions="2"`, dataset id, scale units, min/max, and the event dataset.

Layer editing operations:

- `NoteLayer::moveSelection`, `resizeSelection`, and `deleteSelection` remove and re-add `Event` objects through `ChangeEventsCommand`.
- `NoteLayer::paste` adds clipboard events, filling missing value/duration from model defaults where needed.
- `NoteLayer::addNoteOn` and `addNoteOff` create a note event from live MIDI-style note-on/note-off pairs.
- `FlexiNoteLayer::splitNotesAt` removes a covering note and adds two shorter notes, optionally updating note pitch from the associated pitch curve.
- `FlexiNoteLayer::addNote` creates a new note event in the target model.
- `FlexiNoteLayer::snapSelectedNotesToPitchTrack` removes selected notes and re-adds notes with values calculated from the associated `SparseTimeValueModel` pitch curve.
- `FlexiNoteLayer::mergeNotes` removes a group of notes and adds one merged note.
- `FlexiNoteLayer::deleteSelectionInclusive` removes notes spanning the selected range, not only notes starting within it.
- `TimeValueLayer::drawStart`, `drawDrag`, and `drawEnd` create or move time-value points.
- `TimeValueLayer::eraseEnd`, `editDrag`, `editOpen`, `moveSelection`, `resizeSelection`, `deleteSelection`, and `paste` modify `SparseTimeValueModel` events through commands.

Selected-region replacement already exists only for specific Tony/pYIN pitch behaviors. `Analyser::switchPitchCandidate`, `shiftOctave`, `deletePitches`, `abandonReAnalysis`, and `takePitchTrackFrom` use `Layer::copy`, `deleteSelection`, and `paste` on the current pitch track. This is a useful source pattern, but it is coupled to `Analyser` and should not be reused for backend note import without a dedicated design.

### 3. Model APIs A Future TonyLayerImporter Should Use

Required low-level data APIs:

- `ModelById::add(std::shared_ptr<Model>)` to register a newly constructed model in the ID store.
- `NoteModel(sampleRate, resolution, notifyOnAdd, subtype)` for note outputs.
- `NoteModel::setScaleUnits(QString)` to define whether event values are MIDI-like or Hz-like.
- `NoteModel::add(Event)` for initially populating an imported note model before layer insertion.
- `SparseTimeValueModel(sampleRate, resolution, notifyOnAdd)` for pitch/f0 curves.
- `SparseTimeValueModel::setScaleUnits("Hz")` for f0/pitch curve outputs.
- `SparseTimeValueModel::add(Event)` for initially populating imported f0 points.
- `Event(frame, value, duration, level, label)` for note events.
- `Event(frame, value, label)` for time-value events.

Required document/layer APIs:

- `Document::addNonDerivedModel(ModelId)` when a model is created by backend import rather than by a Tony transform.
- `Document::createImportedLayer(ModelId)` when the model-to-layer mapping is correct.
- `Document::createLayer(LayerFactory::LayerType)` plus `Document::setModel(Layer *, ModelId)` only when the importer must force an explicit layer type and has registered the model first.
- `Document::addLayerToView(View *, Layer *)` to insert the layer through the command-backed path.
- `LayerFactory::getValidLayerTypes(ModelId)` to verify that a model maps to the expected layer type before claiming import.
- `LayerFactory::setModel(Layer *, ModelId)` indirectly through `Document::setModel`.

Important source detail:

- `LayerFactory::getValidLayerTypes()` maps `NoteModel::FLEXI_NOTE` to `FlexiNotes` and normal `NoteModel` to `Notes`.
- `LayerFactory::createEmptyModel(LayerFactory::FlexiNotes)` was observed to create a default `NoteModel` without explicitly passing `NoteModel::FLEXI_NOTE`. A future importer should not rely on this empty-layer helper to create flexi-note backend models without verifying behavior.

### 4. How Edits Become Undoable/Redoable

Undo-redo is command based.

- `CommandHistory::addCommand` pushes commands to undo history and emits modification/activity signals.
- `CommandHistory::startCompoundOperation` and `endCompoundOperation` group multiple commands into one undoable macro.
- `CommandHistory::documentSaved` records the undo stack position for restore/modified tracking.
- `MainWindowBase` connects `CommandHistory::commandExecuted()` to `documentModified()` and `CommandHistory::documentRestored()` to `documentRestored()`.
- `EventCommands.h` defines `AddEventCommand`, `RemoveEventCommand`, and `ChangeEventsCommand`.
- `ChangeEventsCommand::add(Event)` and `remove(Event)` execute each sub-command immediately by default, then keep the command for undo.
- Layer helper methods call `finish(command)` and add the resulting command to `CommandHistory` with `execute=false` because the command has already executed its sub-commands.
- `ModelDataTableModel` obtains `Command *` objects from tabular models and `ModelDataTableDialog` adds them to `CommandHistory`.

Future importer implications:

- Initial model population before the model/layer is visible may be done directly with `model->add(Event)` if the import itself is represented by the layer insertion command.
- Any mutation of an already visible/imported layer must use `ChangeEventsCommand`, model-provided tabular commands, or existing layer editing APIs so undo-redo remains honest.
- Selected-region replacement should use a `CommandHistory` compound operation that removes old events and adds new events through `ChangeEventsCommand`.
- Directly mutating a visible model without commands would bypass undo-redo and should not be used for user-visible import, replacement, or edit operations.

### 5. How Imported Layers Should Be Inserted So CommandHistory Remains Safe

The safe insertion path is:

1. Construct and populate a real model.
2. Register the model with `ModelById::add`.
3. Register the resulting `ModelId` with `Document::addNonDerivedModel` or use `Document::createImportedLayer`, which calls `addNonDerivedModel`.
4. Create a real layer through `Document::createImportedLayer` or an explicit `Document::createLayer` plus `Document::setModel` path.
5. Insert the layer into a real `View`/`Pane` with `Document::addLayerToView`.

`Document::addLayerToView` creates a `Document::AddLayerCommand`. Its `execute()` calls `View::addLayer`, marks the layer non-dormant, updates the document layer/view map, and emits `layerInAView`. Its `unexecute()` removes the layer from the view and updates the map. This path also allows `MainWindowBase::layerInAView` and playback/model ownership side effects to run through existing infrastructure.

Existing generic import flows in `MainWindowBase` use this model:

- imported audio/model path: `addNonDerivedModel`, `createImportedLayer`, `addLayerToView`;
- generic layer import path: load a model, `ModelById::add`, `createImportedLayer`, `addLayerToView`;
- RDF import path: create imported layers and distribute them to panes.

A future `TonyLayerImporter` should follow this pattern from a new component rather than modifying `MainWindow` or `Analyser` first.

### 6. How Layers Are Saved In Sessions

Session save is XML based.

- `MainWindow::saveSession`, `saveSessionAs`, and `saveSessionInAudioPath` call `saveSessionFile` after clearing re-analysis candidates and selections.
- `MainWindowBase::saveSessionFile` writes compressed SV XML using `toXml(out, false)`.
- `MainWindowBase::toXml` writes the document, display panes, and selections.
- `Document::toXml` writes:
  - the main model;
  - used non-derived models;
  - used derived models and derivation metadata;
  - play parameters;
  - alignment models;
  - all layers.
- `Document::toXml` first computes used models from `m_layerViewMap`. Models not used by a layer in a view are skipped.
- `Layer::toXml` writes layer type, object name, model export id, and `presentationName` when present.
- `NoteModel::toXml` writes the event dataset and `subtype="note"` or `subtype="flexinote"`.
- `SparseTimeValueModel::toXml` writes the event dataset for pitch/f0 curves.
- `View::toXml` writes view-level layer references through `toBriefXml`, including visibility state.
- `Pane::toXml` wraps view XML with pane attributes such as centre-line visibility and height.

Save requirement for backend layers:

- A backend-created model must be attached to a layer.
- That layer must be in a real view/pane so the model appears in `m_layerViewMap`.
- The model and layer must have valid XML serialization through existing model/layer `toXml` implementations.
- If a backend result is only held in a backend report or in memory, it will not be preserved as a Tony layer.

### 7. How Layers Are Loaded Back

Session load is handled by `SVFileReader`.

- `SVFileReader::readModel` reconstructs model classes from XML attributes.
- Sparse model `dimensions="2"` without text/box/path subtype becomes `SparseTimeValueModel`.
- Sparse model `dimensions="3"` with `subtype="flexinote"` becomes `NoteModel` with `NoteModel::FLEXI_NOTE`.
- Sparse model `dimensions="3"` without flexi subtype becomes a normal `NoteModel`.
- `SVFileReader::readDatasetStart` validates dataset dimensions against the target model type.
- `SVFileReader::addPointToDataset` adds point events back into `SparseTimeValueModel`, `NoteModel`, `RegionModel`, `TextModel`, `BoxModel`, and other sparse models.
- `SVFileReader::readLayer` creates layer objects through `LayerFactory::getLayerTypeForName`, sets object name and `presentationName`, attaches models through `Document::setModel`, restores properties, and adds view-section layers back with `Document::addLayerToView`.

Load requirement for backend layers:

- The saved model subtype must be sufficient for `SVFileReader` to reconstruct the intended model type.
- The saved layer type must be one that `LayerFactory::getLayerTypeForName` can recreate.
- Layer properties must round-trip through each layer's `setProperties`.
- The loaded layer must appear in a pane/view, not only in the data section.

### 8. How Note And Pitch Layers Are Exported

Tony-specific exports:

- `MainWindow::exportPitchLayer` exports `Analyser::PitchTrack`, requiring a `SparseTimeValueModel`.
- Pitch export writes:
  - SVL/XML through `exportToSVL`;
  - RDF through `RDFExporter`;
  - CSV/TSV through `CSVFileWriter` with `DataExportFillGaps`.
- `MainWindow::exportNoteLayer` exports `Analyser::Notes`, requiring a `NoteModel`.
- Note export writes:
  - SVL/XML through `exportToSVL`;
  - MIDI through `MIDIFileWriter`;
  - RDF through `RDFExporter`;
  - CSV/TSV through `CSVFileWriter` with `DataExportOmitLevel`.

Generic exports:

- `MainWindowBase::exportLayerToSVL` writes a layer's export model and layer XML.
- `MainWindowBase::exportLayerToMIDI` accepts only `NoteModel` layers and writes through `MIDIFileWriter`.
- `MainWindowBase::exportLayerToRDF` delegates to `RDFExporter`, which supports region, note, text, time instants, and time values.
- `MainWindowBase::exportLayerToCSV` uses `CSVFileWriter` and a layer's `getExportModel`.
- `Layer::getExportModel` defaults to `getModel`, which is sufficient for note and time-value layers.

Exporter model behavior:

- `MIDIFileWriter` requires a `NoteExportable`; `NoteModel` implements `NoteExportable`.
- `CSVFileWriter` writes model rows through the model's string export APIs.
- `NoteModel::toStringExportRows` and `SparseTimeValueModel::toStringExportRows` delegate to their event series.
- `RDFExporter::canExportModel` returns true for `RegionModel`, `NoteModel`, `SparseTimeValueModel`, `SparseOneDimensionalModel`, and `TextModel`.

Export requirement for backend layers:

- Note export proof must use a real `NoteModel` layer and inspect the produced MIDI/CSV/SVL/RDF as relevant.
- Pitch export proof must use a real `SparseTimeValueModel` layer and inspect the produced CSV/SVL/RDF as relevant.
- Tony-specific `MainWindow::exportNoteLayer` and `exportPitchLayer` currently target `Analyser` layers, not arbitrary backend layers. A backend-created layer may need generic export proof or a future UI/export selection design before Tony-specific export claims are allowed.

### 9. Metadata And Provenance Implications

Source-backed persistence fields currently visible in inspected code:

- `Model::toXml` writes model id, object name, sample rate, start frame, end frame, and extra attributes supplied by subclasses.
- `Layer::toXml` writes layer id, layer type, object name, model export id, and `presentationName` if set.
- `Event::toXml` writes frame, value, duration, level, reference frame, label, URI, and extra attributes supplied by caller.
- `NoteModel::toXml` and `SparseTimeValueModel::toXml` do not currently define backend provenance fields.
- `SVFileReader` restores model object name and layer `presentationName`.

Implications:

- Basic naming provenance can likely survive through model object names, layer object names, and layer presentation names.
- Event labels and URI fields can survive, but using them for backend provenance would mix metadata into user-visible analysis data and needs a design decision.
- Full provenance required by `PROVENANCE_METADATA_POLICY.md` does not have a verified dedicated storage location yet.
- A future provenance implementation may need XML attributes, sidecar metadata, model/layer naming conventions, or a dedicated metadata model. That choice must be source-backed and tested for save/load.
- Export provenance is format dependent. MIDI export will not preserve full backend metadata through the inspected `MIDIFileWriter` path. CSV/SVL/RDF may preserve more, but this must be proven.

## Required Model APIs

| Need | Source-backed API | Notes |
|---|---|---|
| Create backend note model | `std::make_shared<NoteModel>(sampleRate, resolution, notifyOnAdd, subtype)` | Use `NoteModel::FLEXI_NOTE` only when flexi semantics are intended |
| Create backend pitch curve model | `std::make_shared<SparseTimeValueModel>(sampleRate, resolution, notifyOnAdd)` | Set units to `Hz` for f0/pitch |
| Add note event | `NoteModel::add(Event(frame, value, duration, level, label))` | Direct add is acceptable before visible import; visible mutations need commands |
| Add pitch point | `SparseTimeValueModel::add(Event(frame, value, label))` | Duration is stripped in `SparseTimeValueModel::add` |
| Register model ID | `ModelById::add(std::shared_ptr<Model>)` | Required before document ownership |
| Register imported model | `Document::addNonDerivedModel(ModelId)` | `createImportedLayer` does this internally |
| Create display layer | `Document::createImportedLayer(ModelId)` | Verify expected `LayerFactory` mapping |
| Attach explicit layer/model | `Document::createLayer` plus `Document::setModel` | Only after model registration |
| Insert layer into pane | `Document::addLayerToView(View *, Layer *)` | Command-backed and persistence-relevant |
| Mutate visible events | `ChangeEventsCommand` | Required for undo-redo |
| Edit table rows | `TabularModel` commands via `getSetDataCommand`, `getInsertRowCommand`, `getRemoveRowCommand` | `NoteModel` and `SparseTimeValueModel` implement these |

## CommandHistory / Undo-Redo Requirements

For future backend import and replacement work:

- Use `Document::addLayerToView` so layer insertion is an undoable `AddLayerCommand`.
- Use `CommandHistory::startCompoundOperation` and `endCompoundOperation` when importing multiple layers or replacing selected regions.
- Use `ChangeEventsCommand` for any event changes after a model is visible.
- Add already-executed `ChangeEventsCommand::finish()` results to `CommandHistory` with `execute=false`, matching existing layer code.
- Never mutate visible `NoteModel` or `SparseTimeValueModel` directly when claiming undoable editing.
- Verify undo removes/restores imported layer insertion if layer import is claimed undoable.
- Verify undo/redo restores event content after selected-region replacement before claiming replacement proof.

## Save/Load Requirements

A future backend-created layer survives save/load only if:

- its model is registered with `Document`;
- its layer is owned by `Document`;
- the layer is added to a real view/pane;
- the model type serializes through `toXml`;
- the layer type serializes through `toXml`;
- `SVFileReader` can reconstruct the model type, dataset, layer type, layer properties, and view placement;
- any required provenance fields are stored in a source-backed place and verified after reload.

A backend report, `UnifiedResult`, or detached data structure does not survive as an editable Tony layer unless it becomes a real model/layer and is saved by `Document::toXml`.

## Export Requirements

Before claiming export support for imported backend results:

- identify whether the export path is Tony-specific `MainWindow::exportNoteLayer`/`exportPitchLayer` or generic `MainWindowBase::exportLayerTo`;
- ensure the backend-created layer is the actual layer exported;
- for note MIDI export, prove the layer model is a `NoteModel`;
- for pitch CSV/RDF/SVL export, prove the layer model is a `SparseTimeValueModel`;
- inspect exported file contents, not only that a file exists;
- document whether provenance survives export or is lost.

## Safe Insertion Points

Safe to use from a future new importer component:

- `ModelById::add`
- `NoteModel`
- `SparseTimeValueModel`
- `Event`
- `Document::addNonDerivedModel`
- `Document::createImportedLayer`
- `Document::createLayer`
- `Document::setModel`
- `Document::addLayerToView`
- `LayerFactory::getValidLayerTypes`
- `LayerFactory::getLayerTypeName`
- `ChangeEventsCommand`
- `CommandHistory` compound operations
- `Selection` and `MultiSelection` for selected-region replacement design
- `CSVFileWriter`, `MIDIFileWriter`, and `RDFExporter` for proof tasks, not for importer internals

These should be called from a new backend/Tony import boundary rather than by modifying existing pYIN flow.

## Unsafe Areas Not To Modify Yet

- `main/MainWindow.cpp` and `main/MainWindow.h`: menu/action/session/export glue and Tony-specific note/pitch actions.
- `main/Analyser.cpp` and `main/Analyser.h`: pYIN baseline, current note/pitch layer ownership, candidate layers, selected pitch replacement.
- `svapp/framework/Document.cpp` and `svapp/framework/Document.h`: central lifetime, model registration, layer/view mapping, XML persistence, command-backed layer insertion.
- `svapp/framework/MainWindowBase.cpp` and `svapp/framework/MainWindowBase.h`: generic open/import/export/session framework.
- `svapp/framework/SVFileReader.cpp` and `SVFileReader.h`: XML/session loading.
- `svgui/layer/LayerFactory.cpp` and `LayerFactory.h`: global model-to-layer mapping.
- `svgui/layer/NoteLayer.*`, `FlexiNoteLayer.*`, and `TimeValueLayer.*`: established editing behavior.
- `svgui/widgets/CommandHistory.*`: global undo-redo behavior.
- `svcore/data/model/EventCommands.h`: core event command semantics.
- `svcore/data/fileio/*` exporters/importers: use for proof before modifying.

## Risks

- `NoteLayer` and `FlexiNoteLayer` both use `NoteModel`, but they have different semantics. A future importer must choose intentionally.
- `FlexiNoteLayer` depends on an associated pitch curve for snap/merge behavior in some operations. Backend note layers without a companion pitch curve may not behave like current pYIN notes.
- `FlexiNoteLayer::getAssociatedPitchModel` searches view layers for a `SparseTimeValueModel` in `Hz` and ignores candidate layers. Multiple backend pitch layers could make this association ambiguous.
- `LayerFactory::createImportedLayer` chooses the first valid layer type from the set. The importer must verify that the chosen type is expected.
- Existing Tony-specific note/pitch exports target `Analyser` layers, not arbitrary backend result layers.
- Full provenance persistence is not solved by existing model/layer names alone.
- MIDI export will likely lose provenance and non-note details such as confidence, pitch bends, warnings, and technique labels.
- Polyphonic backend note output may be representable in a `NoteModel` as overlapping events, but Tony edit semantics and export correctness for polyphonic backend output are not proven here.
- Pitch bends, confidence/voicing, and technique labels have no verified import target in this audit.

## Unanswered Questions

- Should backend note import default to `NoteLayer` or `FlexiNoteLayer`?
- Should Basic Pitch polyphonic output be imported into one note model, multiple note layers, or rejected until a polyphony policy is implemented?
- How should backend pitch bends be represented in Tony without losing semantics?
- How should confidence/voicing be displayed and exported?
- Should technique labels use `TextModel`, `RegionModel`, note labels, or a dedicated metadata strategy?
- Where should backend provenance metadata be stored so it survives session save/load and remains distinguishable from user note labels?
- Should backend-created layers be connected to `Analyser` at all, or remain separate from existing pYIN `Analyser::Notes` and `Analyser::PitchTrack` until UI/export work explicitly chooses otherwise?
- What exact UI/export path should export backend-created layers rather than pYIN `Analyser` layers?

## Acceptance Criteria

### A. Real Editable Note Layer Import

Before claiming real editable note layer import:

1. Use a real or explicitly dev/test-only `UnifiedResult` containing note events.
2. Convert note event seconds to frames using the correct audio/model sample rate.
3. Create a real `NoteModel` with a deliberate subtype.
4. Populate it with real `Event` objects, with frame, value, duration, level where applicable, and label only when source data supports it.
5. Register it with `ModelById` and `Document`.
6. Create a real `NoteLayer` or `FlexiNoteLayer` through `Document`/`LayerFactory`.
7. Insert it into a real pane/view with `Document::addLayerToView`.
8. Prove the layer is visible through normal Tony view behavior.
9. Edit a note through Tony mechanisms: change time, duration, pitch/value, delete, move, or resize.
10. Prove the underlying `NoteModel` changed.
11. Prove undo and redo restore the edit state.
12. Do not claim Basic Pitch, real backend, or production success unless the result came from a real backend run.

### B. Real Pitch Curve Import

Before claiming real pitch curve import:

1. Use real or explicitly dev/test-only pitch/f0 data from `UnifiedResult`.
2. Convert point times to frames using the correct sample rate.
3. Create a real `SparseTimeValueModel`.
4. Set scale units to `Hz` for f0/pitch data.
5. Populate it with real `Event(frame, frequency, label)` values.
6. Register it with `ModelById` and `Document`.
7. Create a real `TimeValueLayer`.
8. Insert it into a real pane/view with `Document::addLayerToView`.
9. Prove it is visible through normal Tony view behavior.
10. Prove editability only by editing real time-value points and verifying undo/redo.
11. Do not convert f0 to notes unless a real segmentation stage exists and is tested.

### C. Save/Load Survival

Before claiming save/load survival:

1. Import a backend-created note or pitch layer into a real Tony session.
2. Save the session through Tony's session save path.
3. Inspect or verify the saved XML contains the expected model and layer data.
4. Reopen the saved session.
5. Verify `SVFileReader` reconstructs the correct model class and layer class.
6. Verify the layer appears in a real pane/view after reload.
7. Verify event counts and key event values match the imported/edited state.
8. Verify editability still works after reload if editability is claimed.
9. Verify provenance survives reload where provenance is claimed.

### D. Export Survival

Before claiming export survival:

1. Export the backend-created layer, not an unrelated pYIN `Analyser` layer.
2. For notes, export through a real `NoteModel` path to MIDI, CSV/TSV, SVL/XML, or RDF as relevant.
3. For pitch curves, export through a real `SparseTimeValueModel` path to CSV/TSV, SVL/XML, or RDF as relevant.
4. Inspect the exported file contents for expected timing, pitch/value, duration, level, and labels where applicable.
5. Verify edited values, not just raw imported values, are exported when corrected-layer export is claimed.
6. Document which provenance is preserved, omitted, or needs a sidecar format.
7. Do not claim export support for unsupported output types such as pitch bends, confidence, or technique labels until their export representation is designed and proven.

## Recommended Next CODEX Task

Recommended next task: CODEX-087C - backend output truth table verification.

That task should verify planned backend output assumptions against actual backend documentation or sample outputs before `TonyLayerImporter` maps fields into the model/layer paths audited here.
