# Basic Pitch Debug UI Consumption Model

Status: CODEX-104 compile-only/test-only UI-consumption boundary.
CODEX-108 update: the UI model now carries explicit manual-run configuration status from `BasicPitchDebugManualRunStatus`.

This document defines the first user-visible status adapter for the Basic Pitch debug workflow. It does not add UI, menus, buttons, dialogs, or runtime startup wiring.

## Boundary

`BasicPitchDebugWorkflowUiModel` accepts a `BasicPitchDebugWorkflowReport` and returns a `BasicPitchDebugWorkflowUiModelResult`.

The model is intended for future UI consumption only. It must be treated as the single conservative mapping boundary between workflow proof evidence and visible UI state.

## Output Fields

The UI model exposes:

- `workflowMode`
- skipped reason
- command/input/output/result path configuration summary
- required manual-run env/config keys
- optional manual-run env/config keys
- missing manual-run env/config keys
- manual-run allowed/skipped status
- derived result JSON path when applicable
- `primaryState`
- `secondaryState`
- `userMessage`
- `technicalMessage`
- `canRun`
- `canCancel`
- `canImport`
- `canEdit`
- `canSave`
- `canExport`
- `shouldShowWarnings`
- `shouldShowProofBundle`
- `productionTranscription`
- `testOnlyDebugOnly`
- proof-bundle summary fields
- warning and error code lists

These fields are state concepts, not real UI widgets. Future `MainWindow` work may consume them, but CODEX-104 does not touch `MainWindow`.

## Honest State Mapping

| Workflow evidence | UI model mapping |
|---|---|
| `backend_not_configured` | Tell the user Basic Pitch must be configured; `canRun=false` |
| `skipped` | Nothing ran; no backend success is implied |
| `running_backend` | Running state; `canCancel=true`; no fake percentage |
| `artifacts_discovered` | Artifacts found; no result/load/import claim |
| `result_json_written` | Result file exists; not visible unless loaded/imported |
| `unified_result_loaded` | Loaded but not visible; `canImport=true` if no real layer exists |
| `imported_into_real_layer` | Real Tony/SV layer exists; not visible unless inserted |
| `inserted_into_view` | Real layer is in a real View/Pane |
| `edit_proof_passed` | Editable proof exists; `canEdit=true` |
| `save_load_proof_passed` | Persistence proof exists; `canSave=true` |
| `export_proof_passed` | Export proof exists; `canExport=true` |
| `completed_with_warnings` | Proof chain completed but warnings must remain visible |
| `failed` / `backend_failed` | Actionable technical details required |

The adapter never maps partial evidence to Ready, Installed, or Completed.

## Warnings Preserved

The model preserves and exposes warnings such as:

- `possible_polyphony`
- `pitch_bend_mapping_deferred`
- `fixture_only_conversion`
- `production_transcription_false`
- `structured_provenance_persistence_deferred` when present

Warnings must remain visible in future UI. A successful proof path with warnings is not a clean production success state.

## Proof Bundle Summary

The UI model summarizes:

- workflow mode;
- skipped reason;
- real-run opt-in state;
- command configured state and command path;
- input audio configured state and path;
- output directory configured state and path;
- required manual-run env/config keys;
- optional manual-run env/config keys;
- missing manual-run env/config keys;
- whether a manual run would be allowed;
- whether a manual run would be skipped and why;
- derived result JSON path when no explicit result path is provided;
- event state names;
- event count;
- artifact count;
- artifact summaries;
- selected `csv_note_events` artifact status and path;
- result JSON path;
- export CSV path;
- note count;
- loaded result status;
- imported real layer status;
- inserted View/Pane status;
- edit proof;
- save/load proof;
- export proof;
- debug/test-only status.

Future UI may expose this in a debug panel or details dialog. It must not hide proof gaps behind a success badge.

## Progress Rule

The adapter does not produce percentage progress. Future UI must show stage-based states unless a real backend provides measured progress events tied to actual work.

## What CODEX-104 Proves

- Missing config maps to `backend_not_configured` and disables run.
- Skipped workflows do not claim anything ran.
- Loaded results remain loaded-not-visible until real layer import exists.
- Imported layers remain not visible until real View/Pane insertion exists.
- Edit/save/export enablement requires the matching proof flags.
- Warnings remain visible.
- `productionTranscription=false` and `testOnlyDebugOnly=true` remain explicit.
- No fake Ready, Installed, or Completed state is created.

## What CODEX-108 Adds

CODEX-108 adds a configuration/manual-run status boundary that the UI model consumes. The model can now tell a future UI exactly which debug/manual-run keys are required and which values are missing:

```text
TONY_BASIC_PITCH_DISCOVERY_ENABLE=1
TONY_BASIC_PITCH_COMMAND
TONY_BASIC_PITCH_TEST_AUDIO
TONY_BASIC_PITCH_OUTPUT_DIR
```

Optional:

```text
TONY_BASIC_PITCH_RESULT_JSON
```

The model exposes `manualRunAllowed=false` unless explicit opt-in, command, input audio, and output directory are present. It also exposes `productionTranscription=false`, `testOnlyDebugOnly=true`, and no Ready/Installed/Completed mutation.

## What Remains Before Real UI

- MainWindow menu/action design.
- UI screenshot/smoke proof.
- Real user configuration controls.
- Manual opt-in real-run trigger.
- Cancellation UI.
- Selected-region replacement UI.
- Production polyphony and pitch-bend policies.

## Recommended Next Task

Recommended next task: CODEX-109 - add an explicit debug/manual Basic Pitch run action or preparation dialog, still using this model and without production Basic Pitch claims.
