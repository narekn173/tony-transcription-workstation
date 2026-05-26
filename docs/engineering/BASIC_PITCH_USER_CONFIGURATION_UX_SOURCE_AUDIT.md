# Basic Pitch User Configuration UX Source Audit

Status: CODEX-116 source audit and design documentation only
Runtime behavior changed: no
Production UI added: no
Production transcription support: not claimed

## Purpose

This audit identifies the Tony/Sonic Visualiser UI, settings, path-selection, status, and analysis patterns that a future guarded Basic Pitch setup/preflight dialog should follow.

The current Basic Pitch path remains debug/test-only. CODEX-116 does not add a production action, does not modify MainWindow runtime behavior, does not run Basic Pitch, and does not change pYIN or `Analyser`.

## Source Files Inspected

| File | Why it was inspected |
|---|---|
| `AGENTS.md` | Repository rules: no fake states, preserve pYIN, keep documentation-only tasks free of runtime changes |
| `docs/00_PROJECT_INDEX.md` | Current Basic Pitch reading order and next task pointer |
| `docs/engineering/BASIC_PITCH_PRODUCTION_READINESS_CHECKLIST.md` | Production gates that a setup UI must respect |
| `docs/engineering/BASIC_PITCH_DEBUG_TO_PRODUCTION_TRANSITION_PLAN.md` | Safe CODEX sequence from debug proof to guarded user workflow |
| `docs/engineering/BASIC_PITCH_PRODUCTION_READINESS_PREFLIGHT_MODEL.md` | Code-level gate model that future setup UI should consume |
| `docs/engineering/BASIC_PITCH_DEBUG_CONFIGURATION_ENTRY_PROOF.md` | Existing debug env/config status behavior |
| `docs/engineering/BASIC_PITCH_MAINWINDOW_DEBUG_UI_ACTION_PROOF.md` | Existing debug MainWindow action behavior and formatter pattern |
| `main/MainWindow.cpp` | Analysis menu, pYIN trigger, debug Basic Pitch actions, dialogs, status messages, save/import/export path behavior |
| `main/MainWindow.h` | MainWindow debug action members and slots |
| `svapp/framework/MainWindowBase.cpp` | Shared file picker, menu-state, message, status-bar, and save/export support patterns |
| `svapp/framework/MainWindowBase.h` | Shared MainWindowBase file picker and state APIs |
| `main/Analyser.cpp` | Existing pYIN analysis path, `QSettings` usage, layer insertion, destructive analysis behavior, and state persistence |
| `main/Analyser.h` | Analysis setting keys and pYIN workflow boundary |
| `main/backend/BasicPitchDebugManualRunStatus.*` | Current debug-only config/status model and required environment keys |
| `main/backend/BasicPitchProductionReadinessPreflight.*` | Gate record structure and production-readiness output |
| `main/backend/BasicPitchDebugWorkflowUiReportFormatter.*` | Debug report section structure and no-fake wording |
| `main/backend/BasicPitchDebugCombinedRunImportActionReportFormatter.*` | Combined handoff/import/post-import proof report structure |
| `svcore/base/Preferences.*` | Existing persistent preferences model and `QSettings` groups |
| `svgui/widgets/CSVFormatDialog.cpp` | Existing modal dialog, editable fields, checkboxes, and persisted import defaults |
| `svgui/widgets/PluginPathConfigurator.cpp` | Existing directory picker and path-list editing pattern |
| `main/NetworkPermissionTester.cpp` | Small modal status dialog pattern using `QDialogButtonBox` |
| `svgui/layer/NoteLayer.cpp` and related layer files | Existing user confirmation pattern around destructive/re-align paste operations |

## Existing Tony UI And Configuration Patterns

### Menu Actions

`MainWindow::setupAnalysisMenu()` builds the Analysis menu using `QAction`, `setStatusTip`, separators, and Qt signal/slot connections.

The existing pYIN path is:

```text
Analysis -> Analyse Now!
-> MainWindow::analyseNow()
-> Analyser::analyseExistingFile()
-> Analyser::doAllAnalyses(true)
-> Analyser::addAnalyses()
-> pYIN Vamp transforms
-> Document-created pitch and note layers
```

The existing Basic Pitch debug actions live after the pYIN settings and reset action, separated from normal analysis controls:

```text
Debug: Basic Pitch Workflow Proof...
Debug: Run Basic Pitch Manual Handoff Proof...
Debug: Import Basic Pitch Manual Result...
Debug: Run Basic Pitch Manual Handoff and Import...
```

These actions are currently safe because MainWindow delegates to backend action/report objects and formatters. It does not derive its own Basic Pitch state.

### Settings And Preferences

Tony uses `QSettings` in several scoped groups:

- `Analyser` for auto-analysis and pYIN analysis options;
- `MainWindow` for window/session/UI remembered values;
- `Preferences` and `TempDirectory` inside `sv::Preferences`;
- feature-specific groups such as `CSVImport`.

`Analyser::getAnalysisSettings()`, `MainWindow::updateAnalyseStates()`, and the pYIN option toggle slots show the local pattern for persistent settings. Importantly, the pYIN option toggles explicitly avoid running `analyseNow()` automatically because the operation is destructive.

Future Basic Pitch settings must not be added to the `Analyser` group because that would blur Basic Pitch with the existing pYIN workflow. If persistent Basic Pitch settings are added later, use a dedicated `BasicPitch` group or an existing backend settings store after a stale-path and privacy policy is implemented.

### File And Path Pickers

File selection for sessions/audio/layers is routed through `MainWindowBase::getOpenFileName()` and `getSaveFileName()`, which delegate to `FileFinder`. That keeps last-path behavior and file-type handling consistent.

Directory selection exists in `PluginPathConfigurator::addClicked()` through `QFileDialog::getExistingDirectory()`. That is a relevant pattern for a future Basic Pitch output directory picker, but it should still be wrapped in a Basic Pitch setup model so the UI does not treat a selected directory as proof of readiness.

### Message Boxes And Dialogs

Existing patterns include:

- `QMessageBox::critical` for file open/save/write failures;
- `QMessageBox::warning` for analysis failure or user-cancellable save waits;
- `QMessageBox::question` for overwrite or potentially destructive operations;
- modal `QDialog` with `QGridLayout`, read-only `QTextEdit`, and `QDialogButtonBox::Ok` for the Basic Pitch debug reports;
- status-bar messages through `statusBar()->showMessage(...)`.

The future setup/preflight dialog should reuse these patterns. It should be modal or explicitly user-triggered at first, read preflight/model output, and avoid background process state until a later run task implements it.

### Action Enablement

`MainWindowBase::updateMenuStates()` emits capability signals based on the real current document, pane, model, layer, editability, selection, and playback state. This is the correct style for future action gating: enable or claim a state only when the underlying object exists.

Basic Pitch setup fields can be editable in a dialog, but run/import/edit/export labels must still be gated by `BasicPitchProductionReadinessPreflight`, `BasicPitchDebugManualRunStatus`, and post-import proof reports.

### pYIN And Analyser Protection

`Analyser` owns the existing pYIN workflow. It stores analysis settings in `QSettings` group `Analyser`, creates pYIN pitch and note layers through Sonic Visualiser transform APIs, and removes/replaces existing pYIN layers when `analyseExistingFile()` runs.

Future Basic Pitch UI must not:

- call `MainWindow::analyseNow()`;
- call `Analyser::analyseExistingFile()`;
- reuse pYIN option settings;
- alter pYIN transform IDs or plugin parameters;
- use pYIN layer visibility/audibility settings as Basic Pitch state.

Basic Pitch should remain a separate backend workflow until a future task explicitly designs a shared analysis-entry strategy.

## Existing Basic Pitch Debug Configuration Boundary

`BasicPitchDebugManualRunStatus` currently represents the manual debug configuration. It reports:

- required keys:
  - `TONY_BASIC_PITCH_DISCOVERY_ENABLE=1`
  - `TONY_BASIC_PITCH_COMMAND`
  - `TONY_BASIC_PITCH_TEST_AUDIO`
  - `TONY_BASIC_PITCH_OUTPUT_DIR`
- optional key:
  - `TONY_BASIC_PITCH_RESULT_JSON`
- explicit opt-in;
- command path;
- input audio path;
- output directory;
- provided or derived result JSON path;
- missing configuration keys;
- whether a manual run is allowed;
- whether the run would be skipped;
- `productionTranscription=false`;
- `testOnlyDebugOnly=true`;
- `readyInstalledCompletedMutation=false`.

This is a good model shape for future setup UI, but its env-only configuration is not production UX.

## Existing Production Readiness Preflight Boundary

`BasicPitchProductionReadinessPreflight` represents production gates as code-level records:

- gate id;
- display name;
- status;
- user-facing message;
- technical message;
- evidence reference;
- blocking flag;
- debug-only flag.

Future UI should consume these gate records directly or through a thin view model. MainWindow must not recreate the gate logic.

## UX Findings

1. The safest future setup UI is a dedicated Basic Pitch setup/preflight dialog, not another pYIN option or an overload of `Analyse Now`.
2. `QSettings` is available, but persistent Basic Pitch paths need a dedicated stale-path and clear/reset policy before writing settings.
3. `FileFinder` should be preferred for audio/result file paths where file type semantics matter; `QFileDialog::getExistingDirectory()` is an existing directory-picker pattern for output directories.
4. Debug proof dialogs already use a robust pattern: backend action -> UI/report model -> formatter -> read-only dialog.
5. MainWindow can remain thin if future setup UI consumes `BasicPitchProductionReadinessPreflight` and `BasicPitchDebugManualRunStatus` through a model/formatter.
6. pYIN is protected by keeping Basic Pitch outside `Analyser` and outside `Analyse Now`.
7. Any persistent Basic Pitch UI that says "configured" must still validate paths every time it is shown and must not promote stale settings to Ready/Installed/Completed.

## Safe Future Insertion Points

These are safe candidates for later implementation, not changes made by CODEX-116:

- `MainWindow::setupAnalysisMenu()` for a guarded user-facing Basic Pitch setup/preflight action, separated from pYIN controls.
- A new backend/UI adapter near `main/backend/BasicPitchProductionReadinessPreflight.*` for setup dialog state.
- A new MainWindow slot that opens the setup dialog but does not run Basic Pitch in CODEX-117.
- `FileFinder`/`MainWindowBase` path helpers for audio/result file selection if the existing file type model fits.
- A dedicated Basic Pitch settings group or backend settings object after stale-path handling is designed.

## Unsafe Areas

- `MainWindow::analyseNow()` and `Analyser::*` pYIN paths.
- `QSettings` group `Analyser`.
- pYIN transform IDs and analysis parameters.
- save/export actions that currently operate on pYIN/Tony layers.
- any UI label that implies Ready, Installed, Completed, Imported, Visible, Editable, Saved, or Exported without proof.
- persistent result paths that can point to stale `result.json`.
- shell-string command construction for Windows paths with spaces.
- background process/cancel UI before real cancellation exists.

## Conclusion

The codebase has enough existing patterns to support a future guarded setup/preflight dialog, but CODEX-116 only documents that path. The next implementation should create a setup/preflight dialog skeleton with no run button behavior and no Basic Pitch execution.
