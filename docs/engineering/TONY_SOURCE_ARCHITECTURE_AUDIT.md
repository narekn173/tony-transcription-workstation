# Tony Source Architecture Audit

Status: CODEX-087A research/documentation only.

This audit documents how the current Tony/Sonic Visualiser code performs pYIN analysis, layer creation, display, editing, save/load, and export. It is intended to constrain future backend result import work so that backend output becomes real Tony/Sonic Visualiser data and layers, not UI-only or fake success state.

No runtime behavior, C++ source, UI, pYIN behavior, backend execution, or layer import code was changed for this audit.

## Source Files Inspected

- `main/main.cpp`
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
- `svgui/layer/Layer.h`
- `svgui/layer/LayerFactory.cpp`
- `svgui/layer/LayerFactory.h`
- `svgui/layer/NoteLayer.cpp`
- `svgui/layer/NoteLayer.h`
- `svgui/layer/FlexiNoteLayer.cpp`
- `svgui/layer/FlexiNoteLayer.h`
- `svgui/layer/TimeValueLayer.cpp`
- `svgui/layer/TimeValueLayer.h`
- `svgui/view/View.cpp`
- `svgui/view/View.h`
- `svgui/view/Pane.cpp`
- `svgui/view/Pane.h`
- `svgui/view/ViewManager.cpp`
- `svgui/view/PaneStack.cpp`
- `svgui/view/PaneStack.h`
- `svgui/widgets/CommandHistory.cpp`
- `svgui/widgets/CommandHistory.h`
- `svcore/base/Command.cpp`
- `svcore/base/Command.h`
- `svcore/base/Selection.h`
- `svcore/data/model/EventCommands.h`
- `svcore/data/model/NoteModel.h`
- `svcore/data/model/SparseTimeValueModel.h`
- `svcore/data/fileio/CSVFileWriter.cpp`
- `svcore/data/fileio/CSVFileWriter.h`
- `svcore/data/fileio/MIDIFileWriter.cpp`
- `svcore/data/fileio/MIDIFileWriter.h`
- `svcore/rdf/RDFExporter.cpp`
- `svcore/rdf/RDFImporter.cpp`
- `svcore/transform/FeatureExtractionModelTransformer.cpp`
- `svcore/transform/FeatureExtractionModelTransformer.h`
- `svcore/transform/ModelTransformerFactory.cpp`
- `svcore/transform/ModelTransformerFactory.h`
- `svcore/transform/TransformFactory.cpp`
- `svcore/transform/TransformFactory.h`
- `svcore/plugin/NativeVampPluginFactory.cpp`
- `svcore/plugin/NativeVampPluginFactory.h`
- `svcore/plugin/PiperVampPluginFactory.cpp`
- `svcore/plugin/PiperVampPluginFactory.h`

## Concrete Findings

### 1. How Tony Triggers pYIN Analysis

Tony creates one `Analyser` instance from `MainWindow`.

- `MainWindow::MainWindow` constructs `m_analyser = new Analyser()`.
- `MainWindow` connects `sessionLoaded()` and `audioFileLoaded()` to `MainWindow::analyseNewMainModel()`.
- `MainWindow::analyseNewMainModel()` prepares the main pane, selection strip, time ruler, and `regionOutlined` signal, then calls `m_analyser->newFileLoaded(m_document, getMainModelId(), m_paneStack, pane)`.
- `Analyser::newFileLoaded()` validates the main model as a `WaveFileModel`, reads `QSettings` key `Analyser/auto-analysis`, and calls `doAllAnalyses(autoAnalyse)`.
- `MainWindow::setupAnalysisMenu()` defines the analysis UI actions:
  - Auto-Analyse New Audio toggles `QSettings` key `Analyser/auto-analysis`.
  - Analyse Now calls `MainWindow::analyseNow()`.
  - pYIN-related analysis settings write `QSettings` keys including `precision-analysis`, `lowamp-analysis`, `onset-analysis`, and `prune-analysis`.
- `MainWindow::analyseNow()` wraps `m_analyser->analyseExistingFile()` in a `CommandHistory` compound operation named `Analyse Audio`.
- `Analyser::analyseExistingFile()` removes existing pitch/note layers, then calls `doAllAnalyses(true)`.

This means the current pYIN path is controlled by `MainWindow` actions/signals and implemented by `Analyser`. Future backend work must not alter this flow unless a task explicitly scopes pYIN regression protection.

### 2. How Pitch Tracks Are Created

`Analyser::addAnalyses()` creates the pYIN pitch track.

- It uses transform id prefix `vamp:pyin:pyin:`.
- The pitch output is `smoothedpitchtrack`.
- It checks `TransformFactory::haveTransform("vamp:pyin:pyin:smoothedpitchtrack")`.
- It configures transform step size `256`, block size `2048`, and analysis parameters from `QSettings`.
- It passes the pitch transform into `Document::createDerivedLayers(transforms, m_fileModel)`.
- `FeatureExtractionModelTransformer::createOutputModels()` creates a `SparseTimeValueModel` for time-value Vamp outputs.
- `LayerFactory::getValidLayerTypes()` maps `SparseTimeValueModel` to `LayerFactory::TimeValues`.
- `Document::createLayersForDerivedModels()` creates a `TimeValueLayer` for the pitch model.
- `Analyser::addAnalyses()` casts the returned layer to `TimeValueLayer`, stores it as `PitchTrack`, sets display color, play parameters, and adds it to the pane with `Document::addLayerToView`.

Current pYIN pitch is therefore a real `SparseTimeValueModel` displayed by a real `TimeValueLayer`.

### 3. How Note Layers Are Created

`Analyser::addAnalyses()` creates the pYIN notes layer alongside pitch.

- The note output is `notes`.
- It checks `TransformFactory::haveTransform("vamp:pyin:pyin:notes")`.
- `MainWindow::MainWindow` sets `QSettings` group `Transformer`, key `use-flexi-note-model`, to `true`.
- `FeatureExtractionModelTransformer::createOutputModels()` detects note-style Vamp output and creates a `NoteModel`.
- When `Transformer/use-flexi-note-model` is true, it creates a `NoteModel` with subtype `NoteModel::FLEXI_NOTE`.
- `LayerFactory::getValidLayerTypes()` maps `NoteModel::FLEXI_NOTE` to `LayerFactory::FlexiNotes`.
- `Document::createLayersForDerivedModels()` creates a `FlexiNoteLayer`.
- `Analyser::addAnalyses()` casts the returned layer to `FlexiNoteLayer`, stores it as `Notes`, sets display/play parameters, connects re-analysis signals, and adds it to the pane with `Document::addLayerToView`.

Current pYIN notes are therefore a real `NoteModel` with `FLEXI_NOTE` subtype displayed by a real `FlexiNoteLayer`.

### 4. How Layers Are Inserted Into Document And Panes

Layer ownership and display go through `Document` and `View`.

- `Document` owns the flat list of `Layer` objects and tracks which views contain each layer.
- `Document::createLayer()` creates a layer through `LayerFactory`, stores it in `m_layers`, assigns a unique object name, and emits `layerAdded`.
- `Document::setModel()` attaches a registered model to a layer using `LayerFactory::setModel`.
- `Document::createDerivedLayers()` runs transforms, registers returned models as derived models, and creates suitable layers for those models.
- `Document::createImportedLayer(ModelId)` chooses a valid layer type for an already registered non-derived model, creates the layer, sets the model, and records it as an imported/non-derived model.
- `Document::addLayerToView(View *, Layer *)` validates the layer/model relationship and adds a `Document::AddLayerCommand` to `CommandHistory`.
- `Document::AddLayerCommand::execute()` calls `View::addLayer(layer)`, marks the layer non-dormant, updates `m_layerViewMap`, and emits `layerInAView`.
- `View::addLayer()` stores the layer in the view stack, creates progress/cancel UI helpers, connects model/layer signals, and emits `propertyContainerAdded`.
- `MainWindowBase::layerInAView()` ensures models used by layers in views are added to playback and removed when no longer viewed.

Future imported backend output must follow this ownership path or an equally source-proven equivalent. A visual overlay that bypasses `Document`, `ModelById`, `LayerFactory`, and `View::addLayer` is not a Tony layer.

### 5. Which Model/Layer Types Are Editable

`Layer::isLayerEditable()` defaults to false. Editable layer classes override it.

Inspected editable layer types include:

- `NoteLayer`
- `FlexiNoteLayer`
- `TimeValueLayer`
- `TimeInstantLayer`
- `RegionLayer`
- `TextLayer`
- `BoxLayer`
- `ImageLayer`

Inspected editable model families include:

- `NoteModel`
- `SparseTimeValueModel`
- `SparseOneDimensionalModel`
- `RegionModel`
- `TextModel`
- `BoxModel`
- `ImageModel`

For current pYIN:

- Notes use `NoteModel` plus `FlexiNoteLayer` and are editable.
- Pitch uses `SparseTimeValueModel` plus `TimeValueLayer` and is editable, with Tony-specific pitch edits mostly routed through `Analyser` candidate replacement and layer copy/paste behavior.

Editable must not be claimed for a future backend output until that output is imported into a real editable model/layer and edit commands are proven.

### 6. How Note Editing Works

Tony-specific note editing is centered on `MainWindow` actions and `FlexiNoteLayer`.

- `MainWindow::setupEditMenu()` creates actions for Split Note, Merge Notes, Delete Notes, Form Note from Selection, and Snap Notes to Pitch Track.
- `MainWindow::splitNote()` calls `FlexiNoteLayer::splitNotesAt`.
- `MainWindow::mergeNotes()` calls `FlexiNoteLayer::mergeNotes` for current selections.
- `MainWindow::deleteNotes()` calls `FlexiNoteLayer::deleteSelectionInclusive`.
- `MainWindow::formNoteFromSelection()` gets the `NoteModel`, splits notes at selection boundaries, deletes existing selected notes, creates note-on/note-off events, and merges notes.
- `MainWindow::auxSnapNotes()` calls `FlexiNoteLayer::snapSelectedNotesToPitchTrack`.
- `FlexiNoteLayer` editing operations use `ChangeEventsCommand` against the note model and finish by adding the command to `CommandHistory`.
- `FlexiNoteLayer` can locate the associated pitch model by scanning the view for `SparseTimeValueModel` layers with units `Hz` while excluding candidate layers.

The current edit path proves that imported backend notes should use existing note model/layer editing infrastructure instead of custom edit widgets.

### 7. How CommandHistory / Undo-Redo Is Used

`CommandHistory` is the core undo-redo mechanism.

- `MainWindow::setupEditMenu()` connects Undo/Redo actions to `CommandHistory`.
- High-level user operations use `startCompoundOperation` and `endCompoundOperation`, including Analyse Audio, Snap Notes, Merge Notes, Delete Notes, Form Note from Selection, and pitch candidate operations.
- `Document::addLayerToView` and `Document::removeLayerFromView` add command objects for layer insertion/removal.
- `ChangeEventsCommand` groups model event edits such as note additions, removals, and replacements.
- `ViewManager::setSelections()` creates `SetSelectionCommand` for selection state changes.
- `Pane::editSelectionEnd()` wraps drag/resize selection edits in a compound operation and calls layer move/resize operations.

Future backend import should use commands for user-visible mutations when those mutations are expected to participate in undo-redo. Initial import proof may rely on `Document::addLayerToView`, which already uses a command for layer insertion, but event-level edits must use model command paths.

### 8. How Selections/Regions Are Represented

Selections are frame ranges.

- `Selection` stores a start frame and end frame. The end frame is exclusive.
- `MultiSelection` stores multiple `Selection` ranges.
- `ViewManager` owns the active selections and exposes methods to set, add, remove, and clear them.
- `ViewManager::setSelections()` can emit user selection changes and records selection updates through `CommandHistory`.
- `Pane` handles mouse-based selection creation, dragging, resizing, and Tony's region outline gesture.
- `Pane::regionOutlined(QRect)` is emitted for Tony's pitch re-analysis region flow.
- `MainWindow::regionOutlined(QRect)` converts the outlined rectangle to a frame range and frequency range, stores a pending `FrequencyRange`, and calls `ViewManager::setSelection`.
- `MainWindow::selectionChangedByUser()` passes the active selection and pending frequency constraint to `Analyser::reAnalyseSelection`.

Selected-region backend replacement must respect this frame-based selection model. Time seconds in backend structures must be converted to frame ranges using the correct model sample rate before touching Tony models.

### 9. How Save/Load/Export Preserves Analysis Layers

Save/load preservation depends on real models and layers being owned by `Document` and present in views.

Save:

- `MainWindow::saveSession()`, `saveSessionAs()`, and `saveSessionInAudioPath()` clear temporary re-analysis candidates and selections before saving.
- `saveSessionAs()` and `saveSessionInAudioPath()` call `waitForInitialAnalysis()` before writing. The current-path `saveSession()` path should be inspected further before relying on equivalent waiting behavior.
- `MainWindowBase::saveSessionFile()` writes compressed SV XML.
- `MainWindowBase::toXml()` writes the document, display panes, and selections.
- `Document::toXml()` writes the main model, non-derived models, derived models, derivation metadata, play parameters, alignment models, and all layers.

Load:

- `MainWindowBase::openSession()` constructs a `Document`, parses the session with `SVFileReader`, clears command history, and emits `sessionLoaded()`.
- `SVFileReader` reconstructs models and layers from XML.
- `SVFileReader` maps sparse model dimensions and subtype values to model classes, including `SparseTimeValueModel`, `NoteModel`, and `NoteModel::FLEXI_NOTE`.
- `SVFileReader::readLayer()` creates the declared layer type through `LayerFactory`, sets its model through `Document`, applies properties, and inserts it into the current pane.
- After session load, `MainWindow::analyseNewMainModel()` calls `Analyser::newFileLoaded()`. `Analyser::addAnalyses()` detects existing `TimeValueLayer` and `FlexiNoteLayer` analysis layers and records them instead of rerunning pYIN when both are present.

Export:

- `MainWindow::exportPitchLayer()` exports the analyser's `PitchTrack` as SVL/XML, RDF, or CSV/TSV.
- `MainWindow::exportNoteLayer()` exports the analyser's `Notes` as SVL/XML, MIDI, RDF, or CSV/TSV.
- `MainWindowBase` also provides generic export helpers for layer SVL, MIDI, RDF, and delimited text.

Import:

- `MainWindow::importPitchLayer()` supports CSV pitch import into a two-dimensional model, creates an imported layer, passes it to `Analyser::takePitchTrackFrom`, and deletes the temporary layer.
- RDF/SVL pitch import paths are present but not completed in the inspected Tony-specific code.
- No Tony-specific note import menu path was verified in this audit.

A backend result cannot be considered preserved until it is saved through `Document::toXml`, reloaded through `SVFileReader`, and exported through the appropriate real export path.

## Safe Insertion Points For Future TonyLayerImporter

These are design candidates only. Do not implement them until a dedicated task scopes code changes and proof.

- Add a new importer component outside `MainWindow` and `Analyser`, likely under `main/backend`, that consumes `UnifiedResult` and receives explicit `Document *`, `Pane *` or `View *`, main model id, and sample-rate context.
- Convert backend note output to a real `NoteModel`, choosing `NoteModel` subtype deliberately. Tony pYIN uses `NoteModel::FLEXI_NOTE`, but generic imported notes might not automatically need flexi-note semantics.
- Convert f0/pitch output to a real `SparseTimeValueModel` with scale units such as `Hz`, then display it through `TimeValueLayer`.
- Use `ModelById` registration plus `Document` public APIs. The expected path is to register a model, let `Document::createImportedLayer(modelId)` create the layer when the model maps correctly, and call `Document::addLayerToView(view, layer)`.
- Use existing layer/model command mechanisms for later edits. For imported note event changes, use `ChangeEventsCommand` or model-provided commands so undo-redo works.
- Use `Document::addLayerToView` for layer insertion so `CommandHistory`, view ownership, playback registration, and XML persistence are exercised.
- Treat selected-region replacement as a separate mutation path requiring selection-to-frame conversion, event deletion, event insertion, and undo grouping.
- Keep provenance metadata design separate until a source-backed metadata storage location is verified.

## Unsafe Insertion Points

These files/classes are dangerous and should not be modified yet:

- `main/MainWindow.cpp` and `main/MainWindow.h`: central menu/action/session/selection glue. Changes here can easily alter pYIN, save/load, and editing behavior.
- `main/Analyser.cpp` and `main/Analyser.h`: current pYIN baseline, pitch candidate re-analysis, note/pitch layer pointers, and Tony-specific editing integration.
- `svapp/framework/Document.cpp` and `svapp/framework/Document.h`: central model/layer ownership, command insertion, XML persistence, and lifetime management.
- `svapp/framework/MainWindowBase.cpp` and `svapp/framework/MainWindowBase.h`: generic session, import, export, and window behavior.
- `svgui/layer/LayerFactory.cpp` and `svgui/layer/LayerFactory.h`: global model-to-layer mapping. Avoid changes unless adding a genuinely new layer type.
- `svgui/layer/NoteLayer.*`, `svgui/layer/FlexiNoteLayer.*`, and `svgui/layer/TimeValueLayer.*`: established editable layer behavior. Prefer using these as-is before modifying them.
- `svapp/framework/SVFileReader.*`: session load behavior. Avoid changes until save/load proof requires a specific extension.
- `svcore/transform/*` and `svcore/plugin/*`: Vamp transform and plugin infrastructure. External backend import should not modify Vamp paths.
- `svgui/widgets/CommandHistory.*` and `svcore/data/model/EventCommands.h`: undo-redo primitives. Use these mechanisms rather than changing them.

## Risks

- Backend note import target is not fully decided. Tony pYIN uses `FlexiNoteLayer`, but forcing all backend notes into `FlexiNoteLayer` may be wrong for some backends.
- Basic Pitch may produce polyphonic note events. The current Tony pYIN baseline is monophonic, and Basic Pitch output must not be blindly collapsed into one monophonic layer.
- f0, pitch bends, confidence, voicing, technique labels, and warnings/errors require separate mapping decisions. They are not automatically notes.
- `Document::createImportedLayer()` chooses a valid layer type from `LayerFactory`; importer code must prove the resulting layer type is the intended one.
- Imported backend layers must be saved, reloaded, edited, and exported before any feature-complete claim.
- Existing Tony save paths do not appear identical for save-existing versus save-as behavior around `waitForInitialAnalysis()`. This audit does not change that behavior.
- Provenance metadata storage is not yet source-proven. Object names, presentation names, layer properties, model metadata, and XML extensions need separate evaluation.
- A layer visible on screen is not enough. It must be a real `Document`-owned layer backed by a real model to count.

## Unanswered Questions

- Should backend note results use `NoteLayer`, `FlexiNoteLayer`, or both depending on output semantics?
- What is the correct Tony-native representation for pitch bends: separate time-value layer, note-level metadata, or a combination?
- What is the correct Tony-native representation for confidence/voicing: time-value layer, region layer, or non-editable diagnostic layer?
- Should technique labels use `TextLayer`, `RegionLayer`, or another annotation model?
- Where should backend provenance metadata live so that save/load/export semantics remain honest?
- Which existing generic import/export paths can be reused for backend-created layers without Tony-specific `Analyser` coupling?
- How should selected-region replacement handle overlapping existing notes, partial notes, and undo grouping?

## Acceptance Criteria For Proving A Future Backend Result Becomes A Real Editable Tony Layer

A future backend result import task must prove all applicable criteria before claiming feature completion:

1. A real audio file is processed by an explicitly run backend or by a debug/mock backend clearly marked test-only.
2. The backend produces a real result file; no fake `result.json` is fabricated.
3. The result file is loaded through the `UnifiedResult` file loader and passes validation.
4. The importer creates real Tony/Sonic Visualiser models, such as `NoteModel` or `SparseTimeValueModel`, from the loaded data.
5. The models are registered with `Document`/`ModelById` through source-proven APIs.
6. Real layers are created through `LayerFactory`/`Document` APIs, such as `FlexiNoteLayer`, `NoteLayer`, or `TimeValueLayer`.
7. The layers are inserted into a real pane/view through `Document::addLayerToView` or an equivalent source-proven command path.
8. The visible layer is editable when editability is claimed, and edits must use existing model/layer command paths.
9. Undo/redo works for user-visible import or edit mutations that should participate in history.
10. Save writes the imported backend model/layer into the Tony session.
11. Load restores the imported backend model/layer as a real editable layer.
12. Export produces a correct external artifact for the imported layer type where export is claimed.
13. Provenance is preserved or explicitly documented as not yet available.
14. The UI truth state, if added later, must distinguish loaded result, imported real layer, edited, saved, and exported.

## Recommended Next CODEX Task

Recommended next task: CODEX-087B - Tony layer/edit/save/export audit.

That task should focus on proving the exact layer/model/event command and persistence requirements for imported backend notes, pitch curves, annotations, save/load, and export before implementing `TonyLayerImporter`.
