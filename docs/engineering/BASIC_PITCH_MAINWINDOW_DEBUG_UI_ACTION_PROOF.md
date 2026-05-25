# Basic Pitch MainWindow Debug UI Action Proof

Status: debug-only runtime UI boundary  
Scope: first MainWindow action that displays Basic Pitch workflow truth-state and proof-bundle data  
Production transcription support: not claimed

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

- primary state
- secondary state
- user message
- technical message
- warnings
- errors
- action gates: run, cancel, import, edit, save, export
- proof-bundle event states and event count
- artifact count
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
- production run/cancel workflow
- selected-region replacement
- user-facing backend readiness state
- automatic Basic Pitch execution
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

`CODEX-107` should add the next debug UI hardening slice: a non-modal proof-bundle/details surface or explicit debug settings/configuration entry, still without production Basic Pitch claims.
