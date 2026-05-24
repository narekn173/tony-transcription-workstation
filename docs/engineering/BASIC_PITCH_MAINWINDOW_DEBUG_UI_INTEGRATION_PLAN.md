# Basic Pitch MainWindow Debug UI Integration Plan

Status: source audit and design plan only  
Scope: future debug-only Basic Pitch workflow UI consumption  
Runtime behavior: unchanged

## 1. Source Files Inspected

These files and classes were inspected to ground the future UI plan in existing Tony/Sonic Visualiser patterns:

| File | Classes / functions inspected | Reason |
|---|---|---|
| `main/MainWindow.cpp` | `setupMenus`, `setupFileMenu`, `setupAnalysisMenu`, `updateMenuStates`, `analyseNow`, `analyseNewMainModel`, `exportPitchLayer`, `exportNoteLayer`, status and error helpers | Existing menu/action, pYIN trigger, status, pane setup, and export patterns |
| `main/MainWindow.h` | Signals, slots, QAction members, `m_analyser`, setup methods | Future slot/member placement and risk boundaries |
| `svapp/framework/MainWindowBase.cpp` | `updateMenuStates`, `getStatusLabel`, `exportLayerToCSV`, `exportLayerTo`, session/XML helpers | Shared action enablement, status label, progress dialog, and export behavior |
| `svapp/framework/MainWindowBase.h` | `can*` signals, protected members, export helper declarations | Safe future access to `m_document`, `m_paneStack`, and existing UI state signals |
| `main/Analyser.cpp` | `newFileLoaded`, `analyseExistingFile`, `doAllAnalyses`, `addAnalyses`, pYIN derived-layer creation, reanalysis paths | Existing pYIN workflow and areas that must not be disturbed |
| `main/Analyser.h` | Public pYIN analysis entry points and layer accessors | Boundaries that must remain pYIN-specific |
| `main/backend/BasicPitchDebugWorkflow.*` | debug workflow request/report, truth states, proof bundle | Existing proof data a future UI must consume |
| `main/backend/BasicPitchDebugWorkflowUiModel.*` | UI-consumable state mapping and action enablement | Mandatory adapter for future UI display logic |
| `docs/engineering/UI_VISUAL_TRUTH_STATES.md` | allowed visible states and forbidden claims | Truth-state policy for future UI text |
| `docs/engineering/PROOF_BUNDLE_POLICY.md` | proof-bundle fields and future UI rule | Evidence policy for debug workflow details |

No runtime source file was changed for this task.

## 2. Existing Tony UI Patterns

Tony creates top-level menus in `MainWindow::setupMenus`, then delegates to specific setup methods such as `setupFileMenu`, `setupEditMenu`, `setupViewMenu`, and `setupAnalysisMenu`.

Actions are created as `QAction` instances, usually with:

- a translated text label via `tr(...)`;
- an optional shortcut;
- a status tip;
- a `triggered()` connection to a `MainWindow` slot;
- an enablement connection to an existing `can*` signal where the action depends on document/layer state;
- optional registration with `m_keyReference`;
- placement in a `QMenu` and sometimes a toolbar.

The File menu demonstrates action enablement through signals such as `canSave`, `canSaveAs`, `canImportLayer`, `canExportPitchTrack`, and `canExportNotes`. The existing pitch/note export actions call `exportPitchLayer` and `exportNoteLayer`; both use real analyser-owned layers, real model types, real writers, and `QMessageBox::critical` on write failure.

The Analysis menu currently owns pYIN-facing actions:

- `Auto-Analyse New Audio`;
- `Analyse Now!`;
- pYIN option toggles backed by `QSettings` in the `Analyser` group;
- `Reset Options to Defaults`.

`Analyse Now!` calls `MainWindow::analyseNow`, which starts a `CommandHistory` compound operation, calls `m_analyser->analyseExistingFile()`, ends the compound operation, and reports errors with `QMessageBox::warning`.

Status text uses the shared `MainWindowBase::getStatusLabel()` pattern. `MainWindowBase` creates a `QLabel` in the status bar and existing code updates it with `m_myStatusMessage`. This is a safe future display surface for short stage labels, but it must not show fake progress percentages or success states.

Progress dialogs exist for real file/session/import/export operations through `ProgressDialog`. `MainWindowBase::exportLayerToCSV` uses a `ProgressDialog` with `CSVFileWriter`. Future Basic Pitch UI must not reuse `ProgressDialog` with invented percentages; it may show stage-based progress only until a real backend provides measured progress.

Error reporting uses `QMessageBox::warning`, `QMessageBox::critical`, or `QMessageBox::information` depending on the existing pattern. Future Basic Pitch debug UI should follow this style for explicit user-triggered failures, while keeping proof-bundle details available in a debug details view.

Layer visibility and pane insertion feedback come from real `Document`, `PaneStack`, and `Pane` operations. `analyseNewMainModel` creates panes through `m_paneStack->addPane()` and inserts a real time ruler layer through `m_document->addLayerToView(...)`. pYIN analysis layers are added by `Analyser`, also through `Document::addLayerToView`.

Save and export actions are evidence-based:

- session save enablement is based on `MainWindowBase::canSave` / `canSaveAs`;
- pitch/note export actions are based on analyser-owned pitch/notes layers;
- generic export helpers resolve real model data through `Layer::getExportModel(...)` and writer classes.

Future Basic Pitch UI must not enable save/export verification text based only on a debug workflow report unless the matching proof flag has passed.

## 3. Existing pYIN / Analyser Workflow

The existing pYIN workflow is concentrated in `Analyser` and must remain separate from Basic Pitch debug UI work.

User-triggered analysis:

- The user chooses `Analysis -> Analyse Now!`.
- `MainWindow::analyseNow()` opens a `CommandHistory` compound operation named `Analyse Audio`.
- `m_analyser->analyseExistingFile()` removes existing analyser-owned pitch and note layers from the pane.
- `Analyser::doAllAnalyses(true)` recreates waveform/visualisation state and calls `addAnalyses()`.
- Errors are returned as strings and shown by `MainWindow` with `QMessageBox::warning`.

New-file analysis:

- `MainWindow` connects `sessionLoaded()` and `audioFileLoaded()` to `analyseNewMainModel()`.
- `analyseNewMainModel()` creates or reuses the main pane and selection strip, inserts a real time ruler layer, and calls `m_analyser->newFileLoaded(...)`.
- `Analyser::newFileLoaded()` reads `Analyser/auto-analysis` from `QSettings` and delegates to `doAllAnalyses(autoAnalyse)`.

pYIN-derived layers:

- `Analyser::addAnalyses()` uses Vamp transform IDs under `vamp:pyin:pyin:`.
- It checks for `smoothedpitchtrack` and `notes`.
- It creates derived layers with `m_document->createDerivedLayers(...)`.
- It identifies `TimeValueLayer` as the pitch track and `FlexiNoteLayer` as notes.
- It inserts those real layers into the pane through `m_document->addLayerToView(...)`.

Long-running and regeneration behavior:

- Existing pYIN work relies on transform/model completion paths and layer completion signals.
- Selection reanalysis uses `CommandHistory`, clipboard-based layer edits, and async derived-layer creation paths.
- These paths are tightly coupled to Tony's pYIN editing behavior and must not be reused as a shortcut for Basic Pitch debug UI.

What must not be disturbed:

- `Analyser::newFileLoaded`;
- `Analyser::analyseExistingFile`;
- `Analyser::doAllAnalyses`;
- `Analyser::addAnalyses`;
- pYIN transform IDs, settings, layer colors, playback parameters, and reanalysis paths;
- existing export actions that target `Analyser::PitchTrack` and `Analyser::Notes`.

## 4. Proposed Future Debug-Only Basic Pitch UI Slice

The first future UI slice should be explicitly debug-only and should consume `BasicPitchDebugWorkflowUiModel` rather than deriving state directly from backend classes.

Recommended placement:

- Add a separated debug action under the existing `Analysis` menu.
- Suggested label: `Debug: Basic Pitch Workflow Proof...`
- Suggested status tip: `Run or inspect the debug-only Basic Pitch proof workflow. This is not production transcription support.`

Recommended behavior:

- The action must be user-triggered only.
- It must not run when a file is opened.
- It must not reuse `Analyse Now!`.
- It must not call `Analyser`.
- It must not write `QSettings` production configuration unless a later task explicitly designs that settings flow.
- Manual real Basic Pitch execution must remain opt-in through the existing explicit configuration path.
- Synthetic and discovered-artifact modes must remain clearly labelled as debug/test-only.

Recommended UI flow:

1. User invokes the debug action.
2. A future controller gathers an explicit debug workflow request.
3. `BasicPitchDebugWorkflow` runs only the selected explicit mode.
4. `BasicPitchDebugWorkflowUiModel::fromReport(...)` converts the report into visible state data.
5. `MainWindow` displays only the adapter output:
   - primary/secondary state;
   - user message;
   - technical message;
   - warnings;
   - proof bundle summary;
   - action enablement flags.
6. No alternate UI state machine is created in `MainWindow`.

Suggested future UI surfaces:

- Status bar: current stage label only, for short non-final state.
- Modal or modeless debug details dialog: proof bundle, warnings, and technical details.
- Message boxes: blocking failures or missing configuration after explicit user action.
- Optional disabled proof-bundle action: visible only after a report exists.

The first UI slice should prefer a small `BasicPitchDebugWorkflowUiController` or equivalent bridge so that `MainWindow` only owns the QAction/slot and display calls. This reduces the risk of spreading backend workflow logic into UI code.

## 5. UI Truth-State Mapping

Future UI must map `BasicPitchDebugWorkflowUiModel` states conservatively.

| State | User-visible label | Technical detail | Enabled actions | Warnings / proof bundle |
|---|---|---|---|---|
| `backend_not_configured` | `Basic Pitch not configured` | Required command/configuration is absent. | `canRun=false`; configure/details action may be shown in a later settings task. | Proof bundle available only if a report explains skipped configuration. |
| `skipped` | `Debug workflow skipped` | Nothing ran and no success is claimed. | `canRun` depends on adapter output; import/edit/save/export disabled. | Show skipped reason. Proof bundle may be available. |
| `running_backend` | `Running Basic Pitch debug workflow` | A real process or proof stage is active. | `canCancel=true` only if a real cancellable run exists; import/edit/save/export disabled. | No fake percent. Logs may be shown only as logs. |
| `artifacts_discovered` | `Artifacts discovered` | Files were found/classified, but result JSON is not yet proven unless separately reported. | Import disabled unless `canImport=true` from the adapter. | Show artifact list and warnings. Proof bundle should be available. |
| `result_json_written` | `Result JSON written` | A real UnifiedResult-compatible file was written. | Import may be enabled only if loader proof is not yet complete and adapter permits it. | Show result path. Do not claim imported or visible. |
| `unified_result_loaded` | `UnifiedResult loaded` | Loader accepted the result file. | `canImport=true` only when `importedIntoTonyLayers=false`; edit/save/export disabled unless proof flags passed. | Show warnings such as possible polyphony and deferred pitch bends. |
| `imported_into_real_layer` | `Imported into Tony layer` | A real Tony/Sonic Visualiser layer/model exists. | Edit/save/export enabled only from proof flags. | Do not show visible unless `insertedIntoView=true`. |
| `inserted_into_view` | `Layer visible in pane` | A real layer was inserted into a real View/Pane. | Edit/save/export enabled only from proof flags. | Proof bundle should show document/pane/layer summary. |
| `edit_proof_passed` | `Editable proof passed` | Existing CommandHistory-safe edit proof passed. | `canEdit=true`; save/export only if their proof flags passed. | Show that this is proof evidence, not user edit history. |
| `save_load_proof_passed` | `Save/load proof passed` | Real XML persistence proof passed. | `canSave=true`; export only if export proof passed. | Proof bundle must include save/load evidence summary. |
| `export_proof_passed` | `Export proof passed` | Real export proof passed through the identified export path. | `canExport=true`; edit/save from their own proof flags. | Proof bundle must include export path/evidence. |
| `completed_with_warnings` | `Debug proof completed with warnings` | The proof reached a terminal debug state, but warnings remain relevant. | Enable only actions whose proof flags passed. | Warnings must be visible; never hide them behind success text. |
| `failed` | `Debug workflow failed` | Structured errors explain the failed stage. | Retry/configure/details may be shown; import/edit/save/export disabled unless independently proven by the report. | Error details and proof bundle should be available. |

Additional rules:

- `importedIntoTonyLayers=false` must never map to Imported.
- `insertedIntoView=false` must never map to Visible.
- `editProofPassed=false` must never map to Editable.
- `saveLoadProofPassed=false` must never map to Save verified.
- `exportProofPassed=false` must never map to Export verified.
- `productionTranscription` must remain false for this debug workflow.
- `testOnlyDebugOnly` must remain true for this debug workflow.

Mandatory visible warnings:

- `possible_polyphony`;
- `pitch_bend_mapping_deferred`;
- `fixture_only_conversion`;
- `production_transcription_false`;
- `structured_provenance_persistence_deferred`, when present;
- any backend/process/loader/import/export warning included in the proof bundle.

## 6. Progress Policy

Future UI must use stage-based progress only.

Allowed:

- `Running backend`;
- `Artifacts discovered`;
- `Result JSON written`;
- `UnifiedResult loaded`;
- `Imported into real layer`;
- `Save/load proof passed`;
- `Export proof passed`;
- stdout/stderr/log snippets clearly labelled as logs.

Forbidden:

- fake percentage progress;
- fake estimated time remaining;
- progress bars driven only by elapsed time;
- `Completed` if the backend did not run or the requested proof stage did not pass;
- `Ready` when only filesystem checks passed;
- cancellation UI when no real cancellable process handle exists.

Cancellation may be shown only if the active workflow is backed by the existing real `ExternalProcessRunner` cancellation/lifecycle boundary and the report can distinguish cancellation from timeout/failure.

## 7. Proof Bundle Policy For Future UI

The future UI must expose a debug proof bundle when a report exists. It may summarize by default, but full details must be reachable for debugging and acceptance review.

Required proof bundle fields for UI consumption:

- structured event log;
- command used, only if any command was configured or run;
- input audio path, only if provided;
- output directory, only if provided;
- discovered artifacts with type and file size;
- `result.json` path;
- selected `csv_note_events` artifact, where applicable;
- loaded-result status;
- note count;
- Tony Document/Pane/Layer/Model snapshot summary, where available;
- `importedIntoTonyLayers`;
- `insertedIntoView`;
- edit proof flag;
- save/load proof flag;
- export proof flag;
- warnings and errors;
- `productionTranscription=false`;
- `testOnly/debugOnly=true`.

Proof bundle display must not become a success badge. It is evidence, not marketing text.

## 8. Safe Future Insertion Points

Potential future runtime insertion points, with caution:

- `MainWindow::setupAnalysisMenu()`:
  - safest menu placement for a clearly separated debug-only action;
  - should add a separator and explicit debug wording;
  - must not alter existing `Analyse Now!` or pYIN option actions.
- `MainWindow::updateMenuStates()`:
  - possible place to update the debug action's enabled state from a cached `BasicPitchDebugWorkflowUiModelResult`;
  - must not reuse `canExportPitchTrack` or `canExportNotes`, because those are currently analyser/pYIN layer signals.
- `MainWindowBase::getStatusLabel()`:
  - suitable for short stage labels after explicit user action;
  - must not show fake Ready/Completed/progress states.
- Existing `QMessageBox` patterns in `MainWindow`:
  - suitable for explicit missing configuration, failed run, invalid result, or warning summaries;
  - must not hide detailed proof bundle warnings.
- Existing `m_document`, `m_paneStack`, and current `Pane` access from `MainWindowBase`:
  - suitable only after invoking proven importer boundaries;
  - must preserve the rule that `insertedIntoView=true` requires real `Document::addLayerToView` or equivalent insertion.
- Existing `BasicPitchDebugWorkflow`, `BasicPitchDebugWorkflowUiModel`, and proof classes:
  - mandatory source of state and proof;
  - future UI must call these rather than reimplementing state logic.

Recommended future structure:

- add a small UI controller/adapter class first;
- keep backend workflow execution outside `MainWindow` where practical;
- make `MainWindow` responsible only for user action, display surface, and passing real `Document`/`Pane` references when explicitly requested.

## 9. Unsafe Areas

Do not modify these areas for the first debug UI slice unless a future task explicitly proves the need:

- `Analyser::newFileLoaded`, `analyseExistingFile`, `doAllAnalyses`, `addAnalyses`, `reAnalyseSelection`:
  - these are pYIN-specific and control existing Tony analysis behavior.
- `MainWindow::analyseNow()`:
  - destructive pYIN analysis action; Basic Pitch must not piggyback on it.
- `MainWindow::analyseNewMainModel()`:
  - tied to file/session load and pYIN auto-analysis; Basic Pitch must not auto-run here.
- existing pYIN QSettings keys under the `Analyser` group:
  - do not mix Basic Pitch debug config into pYIN settings.
- `MainWindowBase::updateMenuStates()` internals:
  - broad global enablement surface; prefer subclass-local action state first.
- generic save/export actions:
  - existing note/pitch exports target analyser-owned layers; do not claim Basic Pitch export through them until a future UI task proves the selected layer and export path.
- `ProgressDialog`:
  - do not use it for fake percent progress.
- low-level `Document` ownership/deletion internals:
  - use `TonyLayerImporter` and documented `Document` APIs, not ad-hoc ownership changes.

## 10. Acceptance Criteria For CODEX-106

If CODEX-106 adds the first debug-only UI adapter or menu/action boundary, it must prove:

1. The action is explicitly labelled debug/test-only.
2. The action is user-triggered only and does not run on file/session load.
3. `MainWindow::analyseNow()` and `Analyser` behavior remain unchanged.
4. Basic Pitch does not run unless the existing manual opt-in configuration is present and the user explicitly starts the debug workflow.
5. UI display data comes from `BasicPitchDebugWorkflowUiModel`.
6. No duplicate or weaker UI state machine is introduced.
7. No fake Ready, Installed, Completed, Imported, Visible, Editable, Saved, or Exported labels are shown.
8. No fake percentage progress is shown.
9. Warnings such as possible polyphony and deferred pitch-bend mapping remain visible.
10. Proof bundle details are available after a report exists.
11. `importedIntoTonyLayers`, `insertedIntoView`, edit proof, save/load proof, and export proof are displayed only from report evidence.
12. Normal tests do not require Basic Pitch installed.
13. No user-facing production transcription support is claimed.

Recommended next task:

`CODEX-106 Add debug-only Basic Pitch MainWindow UI action boundary`

That task should add the smallest runtime-disabled or explicitly debug-only action/adapter wiring possible, with tests proving that existing Tony/pYIN workflows are untouched and that visible state is sourced from `BasicPitchDebugWorkflowUiModel`.
