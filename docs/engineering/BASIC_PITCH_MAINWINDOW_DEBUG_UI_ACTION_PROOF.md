# Basic Pitch MainWindow Debug UI Action Proof

Status: debug-only runtime UI boundary  
Scope: first MainWindow action that displays Basic Pitch workflow truth-state and proof-bundle data  
Production transcription support: not claimed
CODEX-107 update: report details now include explicit configuration/status sections.
CODEX-108 update: report details now include explicit manual-run env/config requirements, missing-key status, and run-allowed/skipped status.
CODEX-109 update: a second explicit debug/manual handoff action is available and stops before Tony layer import.
CODEX-110 update: a third explicit debug action imports an existing Basic Pitch-shaped result.json into a real Tony/SV layer when a current Document/Pane exists.
CODEX-111 update: a fourth explicit debug action composes manual handoff and post-run import, still without production claims.
CODEX-112 update: the combined action report includes debug post-import edit/save/load/export proof status.

## Files Inspected

| File | Purpose |
|---|---|
| `docs/engineering/BASIC_PITCH_MAINWINDOW_DEBUG_UI_INTEGRATION_PLAN.md` | Source-audited insertion plan for the first debug UI slice |
| `main/MainWindow.cpp` / `main/MainWindow.h` | Existing Analysis menu, pYIN action, slot, status, and dialog patterns |
| `svapp/framework/MainWindowBase.cpp` / `.h` | Existing status-bar and shared export/status patterns |
| `main/Analyser.cpp` / `.h` | Existing pYIN workflow that must remain separate |
| `main/backend/BasicPitchDebugWorkflow.*` | Debug workflow report and proof-bundle source |
| `main/backend/BasicPitchDebugWorkflowUiModel.*` | Mandatory UI-consumable truth-state adapter |
| `main/backend/BasicPitchDebugWorkflowUiReportFormatter.*` | New testable text formatter consumed by MainWindow |
| `main/backend/BasicPitchDebugManualRunStatus.*` | Debug-only manual-run configuration/status boundary |
| `main/backend/BasicPitchDebugManualRunAction.*` | Debug-only manual Basic Pitch real-run handoff boundary |
| `main/backend/BasicPitchDebugManualRunActionReportFormatter.*` | Testable report formatter for the manual-run action |
| `main/backend/BasicPitchDebugPostRunImportAction.*` | Debug-only existing result.json to real Tony layer import boundary |
| `main/backend/BasicPitchDebugPostRunImportActionReportFormatter.*` | Testable report formatter for the post-run import action |
| `main/backend/BasicPitchDebugCombinedRunImportAction.*` | Debug-only combined manual handoff plus import boundary |
| `main/backend/BasicPitchDebugCombinedRunImportActionReportFormatter.*` | Testable report formatter for the combined action |
| `main/backend/BasicPitchDebugPostImportProofAction.*` | Debug-only post-import edit/save/load/export proof boundary |
| `main/backend/BasicPitchDebugPostImportProofActionReportFormatter.*` | Testable report formatter for the post-import proof |

## MainWindow Insertion Point

The debug action is added in `MainWindow::setupAnalysisMenu()` after the existing pYIN analysis options and `Reset Options to Defaults`.

Action label:

```text
Debug: Basic Pitch Workflow Proof...
```

CODEX-109 adds a second debug-only action in the same debug section:

```text
Debug: Run Basic Pitch Manual Handoff Proof...
```

CODEX-110 adds a third debug-only action in the same debug section:

```text
Debug: Import Basic Pitch Manual Result...
```

CODEX-111 adds a fourth debug-only action in the same debug section:

```text
Debug: Run Basic Pitch Manual Handoff and Import...
```

The actions are separated by a menu separator from the normal pYIN controls. They do not reuse `Analyse Now!`, do not call `Analyser`, and do not change the existing pYIN option actions.

## Runtime Behavior

When the user explicitly invokes the action:

1. `MainWindow` builds a `BasicPitchDebugWorkflowRequest` in `RealBasicPitchManualOptIn` mode.
2. Configuration is read through `BasicPitchRealRunHandoffProof::configFromEnvironment()`.
3. If explicit opt-in is absent, the workflow returns `backend_not_configured` / `skipped`.
4. If explicit opt-in is present, the existing manual/debug proof path may run.
5. The report is converted through `BasicPitchDebugWorkflowUiModel`.
6. `BasicPitchDebugWorkflowUiReportFormatter` renders the UI-model output as plain text.
7. MainWindow shows a debug/test-only dialog with state, messages, warnings, action gates, and proof-bundle summary.
8. A short stage-based status-bar message is shown without any percentage progress.

MainWindow does not derive its own backend state. It displays values already produced by `BasicPitchDebugWorkflowUiModel`.

## Manual Handoff Action Behavior

When the user explicitly invokes `Debug: Run Basic Pitch Manual Handoff Proof...`:

1. MainWindow reads env/config through `BasicPitchRealRunHandoffProof::configFromEnvironment()`.
2. `BasicPitchDebugManualRunAction` preflights the config with `BasicPitchDebugManualRunStatus`.
3. If opt-in, command, input audio, or output directory is missing, the action returns a skipped report and does not run Basic Pitch.
4. If preflight passes, the action calls `BasicPitchRealRunHandoffProof`.
5. The report is formatted by `BasicPitchDebugManualRunActionReportFormatter`.
6. MainWindow shows a debug/test-only dialog with command, paths, artifacts, selected CSV, result.json path, loaded status, note count, warnings, and errors.

This action does not call `BasicPitchDebugWorkflow`, does not run TonyLayerImporter, and does not import into Tony layers.

## Post-Run Import Action Behavior

When the user explicitly invokes `Debug: Import Basic Pitch Manual Result...`:

1. MainWindow reads env/config through `BasicPitchRealRunHandoffProof::configFromEnvironment()`.
2. `BasicPitchDebugPostRunImportAction` resolves `result.json` from an explicit path, `TONY_BASIC_PITCH_RESULT_JSON`, or the derived `<output-dir>/basic_pitch_result.json` path.
3. The result file is loaded through `BackendRunResultLoader`.
4. Missing, empty, invalid, or non-importable results are reported without importing.
5. If the current Document or current Pane/View is unavailable, the action reports that import is unavailable and does not create hidden fake layers.
6. If a valid Basic Pitch-shaped result and a real current Document/Pane are available, the action delegates to `BasicPitchResultToTonyLayerProof` / `TonyLayerImporter`.
7. `importedIntoTonyLayers=true` is reported only after a real Document-owned Tony/SV layer is created.
8. `insertedIntoView=true` is reported only after `Document::addLayerToView` inserts that layer into the current real Pane/View.

This action does not run Basic Pitch, does not create result.json, and reports edit/save/load/export proof fields as not tested by this action.

## Combined Handoff and Import Action Behavior

When the user explicitly invokes `Debug: Run Basic Pitch Manual Handoff and Import...`:

1. MainWindow reads env/config through `BasicPitchRealRunHandoffProof::configFromEnvironment()`.
2. `BasicPitchDebugCombinedRunImportAction` calls `BasicPitchDebugManualRunAction`.
3. If manual-run config is missing or opt-in is false, the action reports skipped and does not import.
4. If the manual handoff fails, no import is attempted.
5. If the manual handoff writes and loads a real `result.json`, the action calls `BasicPitchDebugPostRunImportAction`.
6. `importedIntoTonyLayers=true` is reported only after a real Document-owned Tony/SV layer is created.
7. `insertedIntoView=true` is reported only after the layer is inserted into the current real Pane/View.
8. CODEX-112 then calls `BasicPitchDebugPostImportProofAction` to report edit, undo/redo, save/load, and CSV export proof status.

This combined action does not duplicate manual-run or import workflow logic in MainWindow.

## User-Visible Fields

The dialog displays:

- workflow mode
- manual real-run opt-in state
- configured/missing command path
- configured/missing input audio path
- configured/missing output directory path
- required manual-run env/config keys
- optional manual-run env/config keys
- missing manual-run env/config keys
- manual run allowed / would be skipped
- manual-run skipped reason
- derived result JSON path if applicable
- primary state
- secondary state
- user message
- technical message
- warnings
- errors
- action gates: run, cancel, import, edit, save, export
- proof-bundle event states and event count
- artifact count
- artifact summaries
- selected `csv_note_events` artifact status and path
- result JSON path
- post-run import loaded-result status
- post-run import Basic Pitch-shaped status
- post-run import current Document/Pane availability
- combined run/import stage sequence
- export CSV path
- note count
- loaded-result status
- imported-into-Tony-layer flag
- inserted-into-View/Pane flag
- edit proof flag
- undo/redo proof flag
- save/load proof flag
- export proof flag
- exported note row count from actual CSV rows
- `productionTranscription=false`
- `testOnlyDebugOnly=true`
- `readyInstalledCompletedMutation=false`

The dialog explicitly states:

```text
Debug/test-only Basic Pitch workflow proof.
This is not production transcription support.
```

When Basic Pitch is not configured for manual real-run proof, the report includes:

```text
Basic Pitch debug workflow is not configured.
No backend was run.
Manual Basic Pitch run would be skipped from this configuration.
This is not production transcription.
```

When a synthetic/test-only proof report is displayed, the report includes:

```text
Synthetic/test-only: true
Synthetic/test-only workflow data is not real audio transcription.
```

## Truth-State Rules Preserved

The UI action preserves the existing truth-state policy:

- no fake percentage progress
- no fake Ready / Installed / Completed state
- no fake `result.json`
- no fake notes
- no custom overlay pretending to be a Tony layer
- no Imported claim unless `importedIntoTonyLayers=true`
- no Visible claim unless `insertedIntoView=true`
- no Editable claim unless edit proof passed
- no undo/redo claim unless the CommandHistory proof passed
- no Save/Export verified claim unless those proof flags passed

Warnings such as `possible_polyphony`, `pitch_bend_mapping_deferred`, `fixture_only_conversion`, and `production_transcription_false` remain visible in the proof report when present.

## pYIN / Analyser Isolation

The CODEX-106 and CODEX-109 actions do not modify:

- `MainWindow::analyseNow()`
- `MainWindow::analyseNewMainModel()`
- `Analyser`
- pYIN transform IDs or settings
- existing pYIN layer creation
- existing save/export actions

The existing `Analysis -> Analyse Now!` workflow remains the pYIN path.

## What This Does Not Prove

This is not production Basic Pitch UI. It does not provide:

- persistent Basic Pitch settings UI
- a production configuration dialog
- production run/cancel workflow
- automatic Basic Pitch execution
- selected-region replacement
- user-facing backend readiness state
- production polyphony policy
- production pitch-bend layer mapping
- production result import workflow from the manual handoff action
- production combined run-and-import workflow
  beyond the explicit debug/test-only action added in CODEX-111
- production edit/save/export workflow beyond the debug proof summary added in CODEX-112

## Verification Expectations

Verification must include:

- build passes
- `backend-types` passes
- full suite passes where local environment permits
- no generated build files committed
- no MainWindow/pYIN/Analyser behavior changes beyond the debug-only action

## Recommended Next Task

`CODEX-113` should define the first production-readiness checklist for transitioning the debug Basic Pitch workflow toward a guarded user-facing workflow, still without production Basic Pitch claims.
