# Basic Pitch MainWindow Debug UI Action Proof

Status: debug-only runtime UI boundary  
Scope: first MainWindow action that displays Basic Pitch workflow truth-state and proof-bundle data  
Production transcription support: not claimed
CODEX-107 update: report details now include explicit configuration/status sections.
CODEX-108 update: report details now include explicit manual-run env/config requirements, missing-key status, and run-allowed/skipped status.

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

## MainWindow Insertion Point

The debug action is added in `MainWindow::setupAnalysisMenu()` after the existing pYIN analysis options and `Reset Options to Defaults`.

Action label:

```text
Debug: Basic Pitch Workflow Proof...
```

The action is separated by a menu separator from the normal pYIN controls. It does not reuse `Analyse Now!`, does not call `Analyser`, and does not change the existing pYIN option actions.

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
- export CSV path
- note count
- loaded-result status
- imported-into-Tony-layer flag
- inserted-into-View/Pane flag
- edit proof flag
- save/load proof flag
- export proof flag
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
- no Save/Export verified claim unless those proof flags passed

Warnings such as `possible_polyphony`, `pitch_bend_mapping_deferred`, `fixture_only_conversion`, and `production_transcription_false` remain visible in the proof report when present.

## pYIN / Analyser Isolation

The CODEX-106 action does not modify:

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

## Verification Expectations

Verification must include:

- build passes
- `backend-types` passes
- full suite passes where local environment permits
- no generated build files committed
- no MainWindow/pYIN/Analyser behavior changes beyond the debug-only action

## Recommended Next Task

`CODEX-109` should add an explicit debug/manual Basic Pitch run action or preparation dialog, still without production Basic Pitch claims.
