# CODEX-012 Codebase Map

Status: documentation-only map  
Repository: `narekn173/tony-transcription-workstation`  
Branch: `default`  
Date: 2026-05-15

This document records the current Tony code locations that matter for future backend integration. It does not change application behavior, add backend integrations, redesign UI, or refactor C++ code. Existing pYIN/Vamp behavior remains the baseline.

## Source Scope

Required documents reviewed before mapping:

- `AGENTS.md`
- `docs/00_PROJECT_INDEX.md`
- `docs/02_PRD.md`
- `docs/03_SRS.md`
- `docs/04_BACKEND_CONTRACT.md`
- `docs/05_TDD_ARCHITECTURE.md`
- `docs/06_EXECUTION_PLAN.md`
- `docs/07_CODEX_TASK_LIST.md`

Key source areas inspected:

- `main/main.cpp`
- `main/MainWindow.h`
- `main/MainWindow.cpp`
- `main/Analyser.h`
- `main/Analyser.cpp`
- `meson.build`
- `svapp/framework/Document.h`
- `svapp/framework/Document.cpp`
- `svapp/framework/MainWindowBase.h`
- `svapp/framework/MainWindowBase.cpp`
- `svgui/layer/LayerFactory.h`

## 1. pYIN Analysis Trigger Location

### `main/MainWindow.cpp`

Relevant functions:

- `MainWindow::analyseNow()`
- `MainWindow::analyseNewMainModel()`
- `MainWindow::selectionChangedByUser()`
- `MainWindow::regionOutlined(QRect)`

`MainWindow::analyseNow()` is the user-triggered path from the Analysis menu. It wraps analysis in `CommandHistory::startCompoundOperation("Analyse Audio")` and calls `m_analyser->analyseExistingFile()`.

`MainWindow::analyseNewMainModel()` is the load-time path used after a session or audio file is loaded. The constructor connects document/audio load signals to this function, which prepares the pane and delegates to `m_analyser->newFileLoaded(...)`.

`MainWindow::selectionChangedByUser()` and `MainWindow::regionOutlined(QRect)` form the selected-region re-analysis path. A selected time/frequency region is converted to a constraint and passed to `m_analyser->reAnalyseSelection(...)`.

### `main/Analyser.cpp`

Relevant functions:

- `Analyser::newFileLoaded(...)`
- `Analyser::analyseExistingFile()`
- `Analyser::doAllAnalyses(bool withPitchTrack)`
- `Analyser::addAnalyses()`
- `Analyser::reAnalyseSelection(...)`

`Analyser::newFileLoaded(...)` reads the `Analyser/auto-analysis` setting and calls `doAllAnalyses(...)`.

`Analyser::analyseExistingFile()` removes existing analysis layers, then calls `doAllAnalyses(true)`.

`Analyser::addAnalyses()` is the central pYIN/Vamp path. It checks for the transforms:

- `vamp:pyin:pyin:smoothedpitchtrack`
- `vamp:pyin:pyin:notes`

It configures pYIN parameters from `QSettings`, creates transforms, calls `m_document->createDerivedLayers(...)`, then assigns the returned pitch and note layers.

## 2. Analysis Menu and Action Locations

### `main/MainWindow.cpp`

Relevant functions:

- `MainWindow::setupMenus()`
- `MainWindow::setupAnalysisMenu()`
- `MainWindow::analyseNow()`
- `MainWindow::autoAnalysisToggled()`
- `MainWindow::precisionAnalysisToggled()`
- `MainWindow::lowampAnalysisToggled()`
- `MainWindow::onsetAnalysisToggled()`
- `MainWindow::pruneAnalysisToggled()`
- `MainWindow::resetAnalyseOptions()`

`MainWindow::setupMenus()` calls `setupAnalysisMenu()`.

`setupAnalysisMenu()` defines the Analysis menu actions:

- Auto-Analyse New Audio
- Analyse Now!
- Unbiased Timing (slow)
- Penalise Soft Pitches
- High Onset Sensitivity
- Drop Short Notes
- Reset Options to Defaults

The toggle slots persist pYIN analysis options in the `QSettings` group `Analyser`.

## 3. Pitch Layer Creation Locations

### `main/Analyser.cpp`

Relevant functions:

- `Analyser::addAnalyses()`
- `Analyser::layersCreated(...)`
- `Analyser::takePitchTrackFrom(Layer *)`
- `Analyser::switchPitchCandidate(...)`

`Analyser::addAnalyses()` creates the main pitch track through `m_document->createDerivedLayers(...)`. When the returned layer is a `TimeValueLayer`, it is stored in `m_layers[PitchTrack]`, styled, connected for completion updates, and added to the pane.

`Analyser::layersCreated(...)` handles async candidate pitch layers produced during selected-region re-analysis. Candidate layers are also `TimeValueLayer` instances, but are styled and marked separately.

`Analyser::takePitchTrackFrom(Layer *)` imports pitch values from another layer into Tony's main pitch track. This is important for future backend import because it shows the current model-level handoff pattern.

### Framework APIs

Relevant files/classes:

- `svapp/framework/Document.h`
- `svapp/framework/Document.cpp`
- `svgui/layer/LayerFactory.h`

Relevant functions:

- `Document::createDerivedLayers(...)`
- `Document::createDerivedLayersAsync(...)`
- `Document::createImportedLayer(...)`
- `Document::createEmptyLayer(...)`
- `Document::addLayerToView(...)`
- `LayerFactory::LayerType`

Future pitch imports should preserve `Document` ownership and layer lifecycle instead of constructing layers outside the document framework.

## 4. Note Layer Creation Locations

### `main/Analyser.cpp`

Relevant functions:

- `Analyser::addAnalyses()`
- `Analyser::materialiseReAnalysis(...)`

`Analyser::addAnalyses()` creates the main note layer through the pYIN notes transform. When the returned layer is a `FlexiNoteLayer`, it is stored in `m_layers[Notes]`, styled Bright Blue, connected to completion updates, and added to the pane.

The note layer also connects re-analysis related signals, including region re-analysis and materialisation of candidate analysis into notes.

### `main/MainWindow.cpp`

Relevant functions:

- `MainWindow::splitNote()`
- `MainWindow::mergeNotes()`
- `MainWindow::deleteNotes()`
- `MainWindow::formNoteFromSelection()`
- `MainWindow::snapNotesToPitches()`
- `MainWindow::auxSnapNotes(...)`

These functions edit existing notes using `FlexiNoteLayer`, `NoteModel`, and `CommandHistory`. They are not backend insertion points yet, but they define the behavior that imported backend notes must remain compatible with.

## 5. Import and Export Locations

### `main/MainWindow.cpp`

Relevant functions:

- `MainWindow::importPitchLayer()`
- `MainWindow::importPitchLayer(FileSource source)`
- `MainWindow::exportPitchLayer()`
- `MainWindow::exportNoteLayer()`
- `MainWindow::exportToSVL(QString path, Layer *layer)`

`importPitchLayer(FileSource)` currently handles CSV pitch import. It loads a CSV model through `DataFileReaderFactory`, creates a temporary imported layer using `m_document->createImportedLayer(...)`, copies valid pitch values into Tony's pitch track through `m_analyser->takePitchTrackFrom(...)`, then deletes the temporary layer.

RDF and XML import branches are present but currently fail with TODO comments.

`exportPitchLayer()` exports the current pitch track as SVL/XML, RDF, CSV, or TSV.

`exportNoteLayer()` exports the current note layer as SVL/XML, MIDI, RDF, CSV, or TSV.

### Supporting file I/O classes

Relevant files/classes:

- `svcore/data/fileio/DataFileReaderFactory.*`
- `svcore/data/fileio/CSVFileWriter.*`
- `svcore/data/fileio/MIDIFileWriter.*`
- `svcore/rdf/RDFExporter.*`
- `svcore/rdf/RDFImporter.*`

These are existing file-format paths. Future backend result import should not overload them unless the backend result is intentionally being treated as a file import.

## 6. Project and Session Save/Load Locations

### `main/main.cpp`

Relevant functions/classes:

- `TonyApplication`
- `TonyApplication::handleFilepathArgument(...)`
- `main(...)`

`TonyApplication::handleFilepathArgument(...)` routes `.ton` files to `MainWindow::openSessionPath(...)`; other paths go through `MainWindow::openPath(...)`.

`main(...)` configures application identity, sets up Vamp path handling, creates `MainWindow`, and handles command-line or open-file arguments.

### `main/MainWindow.cpp`

Relevant functions:

- `MainWindow::openFile()`
- `MainWindow::openLocation()`
- `MainWindow::openRecentFile()`
- `MainWindow::drop(...)`
- `MainWindow::saveSession()`
- `MainWindow::saveSessionAs()`
- `MainWindow::saveSessionInAudioPath()`
- `MainWindow::commitData(...)`

Open paths mostly delegate to inherited `MainWindowBase` behavior. Save paths call `saveSessionFile(...)` and clear pending re-analysis or selection state before saving.

### `svapp/framework/MainWindowBase.h` and `svapp/framework/MainWindowBase.cpp`

Relevant functions include:

- `MainWindowBase::openPath(...)`
- `MainWindowBase::openSessionPath(...)`
- `MainWindowBase::openSession(...)`
- `MainWindowBase::openAudio(...)`
- `MainWindowBase::openLayer(...)`
- `MainWindowBase::saveSessionFile(...)`
- `MainWindowBase::exportLayerToSVL(...)`
- `MainWindowBase::exportLayerToMIDI(...)`
- `MainWindowBase::exportLayerToRDF(...)`
- `MainWindowBase::exportLayerToCSV(...)`

This framework layer owns general Sonic Visualiser session and file behavior. Prefer extension points in Tony code over direct changes here.

## 7. Settings and Preferences Locations

### `main/main.cpp`

Relevant functions:

- `main(...)`
- `setupTonyVampPath()`

`main(...)` sets organization/application identity and supports `--first-run`, which clears `QSettings`.

`setupTonyVampPath()` configures the Vamp plugin search path. On Windows it includes the application directory and `ProgramFiles\Tony`. This is critical for pYIN plugin discovery.

### `main/MainWindow.cpp`

Relevant areas:

- Constructor defaults
- `MainWindow::setupAnalysisMenu()`
- Analysis option toggle slots

The constructor configures several defaults, including resampling, fixed sample rate, spectrogram defaults, playback/status settings, window geometry, and `Transformer/use-flexi-note-model`.

The analysis menu reads and writes `QSettings` group `Analyser` for current pYIN options.

### `main/Analyser.cpp`

Relevant functions:

- `Analyser::Analyser()`
- `Analyser::getAnalysisSettings()`
- `Analyser::saveState(...)`
- `Analyser::loadState(...)`

`Analyser::getAnalysisSettings()` defines default pYIN settings:

- `precision-analysis`
- `lowamp-analysis`
- `onset-analysis`
- `prune-analysis`

`saveState(...)` and `loadState(...)` persist per-layer visibility and audibility under the `Analyser` settings group.

Future backend settings should use a separate settings group and should not silently change existing pYIN defaults.

## 8. Safe Insertion Points for Future `AnalysisEngineManager`

The safest first step is to add architecture skeleton code that is compiled but not connected to the current runtime path.

Recommended future insertion pattern:

1. Add isolated backend architecture classes without changing Tony behavior.
2. Add a pYIN-compatible adapter boundary before routing existing pYIN analysis through the manager.
3. Add result validation and normalized import before allowing any backend result into Tony layers.
4. Integrate UI only after the manager can preserve current pYIN behavior.

Potential locations:

- New backend architecture module, for example under a dedicated source folder such as `main/analysis/` or another agreed location.
- `MainWindow::setupAnalysisMenu()` later, for engine selection or backend manager actions.
- `MainWindow::analyseNow()` later, once the manager can route to pYIN without behavior change.
- `Analyser::addAnalyses()` later, only through a careful adapter or wrapper.
- `Document` APIs for future `TonyLayerImporter`, specifically `createImportedLayer(...)`, `createEmptyLayer(...)`, `addNonDerivedModel(...)`, and `addLayerToView(...)`.

Do not replace direct pYIN behavior until a pYIN adapter is tested against current output and workflow expectations.

## 9. Files and Classes Not to Touch Yet

Avoid changing these until the architecture boundary and pYIN compatibility strategy are explicit:

- `main/Analyser.h`
- `main/Analyser.cpp`
- `main/MainWindow.h`
- `main/MainWindow.cpp` existing analysis, save/load, import/export behavior
- `main/main.cpp` Vamp path and application startup behavior
- `svapp/framework/Document.h`
- `svapp/framework/Document.cpp`
- `svapp/framework/MainWindowBase.h`
- `svapp/framework/MainWindowBase.cpp`
- `svgui/layer/FlexiNoteLayer.*`
- `svgui/layer/NoteLayer.*`
- `svgui/layer/TimeValueLayer.*`
- `svgui/layer/LayerFactory.*`
- `svcore/transform/*`
- `svcore/plugin/*`
- `pyin/*`
- `chp/*`

These files define the existing Tony/pYIN behavior, framework ownership model, plugin discovery, or rendering/editing semantics. They should remain stable while backend architecture is introduced.

## 10. Risks and Unknowns

- `Analyser::addAnalyses()` currently combines transform setup, settings, layer creation, styling, pane insertion, and completion wiring. It is a high-coupling area.
- Selected-region re-analysis uses special candidate layers and should remain pYIN-only until a backend model for candidates is designed.
- Pitch import currently goes through a temporary imported layer before copying values into the main pitch track. Backend import should decide whether to reuse that path or use a dedicated importer.
- Note import from future backend results needs to preserve compatibility with `FlexiNoteLayer`, `NoteModel`, note editing commands, and `CommandHistory`.
- RDF/XML pitch import paths are present but not implemented.
- Backend progress and completion must not be faked. Completion should only be reported after valid output is parsed, validated, and imported.
- `Document` owns model and layer lifecycle. Backend integration must not create orphaned models or layers outside that ownership model.
- Existing pYIN settings live under `Analyser`; backend settings should use separate keys to avoid changing pYIN behavior.
- Windows Vamp path handling in `setupTonyVampPath()` is critical and should not be disturbed by backend executable discovery.

## 11. Recommended Next Task

Proceed with `CODEX-030`: propose concrete source locations for the backend architecture skeleton.

The next task should define, without runtime integration:

- Where `AnalysisEngineManager` should live
- Where backend registry and manifest loading should live
- Where result validation should live
- Where normalized backend results should be represented
- Where Tony layer import should be isolated
- Which Meson source list changes will be needed for a compile-only skeleton

After that, `CODEX-031` can add the compile-only architecture skeleton while preserving all current Tony/pYIN behavior.
