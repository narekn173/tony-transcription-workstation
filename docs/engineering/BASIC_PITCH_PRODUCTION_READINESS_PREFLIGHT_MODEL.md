# Basic Pitch Production Readiness Preflight Model

Status: CODEX-115 compile/test-level model
Runtime behavior changed: no
Production UI added: no
Production transcription support: not claimed

## Purpose

`BasicPitchProductionReadinessPreflight` is a backend-only model that represents the CODEX-114 production-readiness gates in code. It lets tests and future UI planning ask, "What blocks production Basic Pitch?" without running Basic Pitch, adding production UI, mutating backend readiness, or weakening the existing debug-only proof labels.

The model is deliberately conservative. Debug proof can become evidence for a gate, but a debug-only gate still blocks production readiness.

## Files Inspected

| File | Reason |
|---|---|
| `docs/engineering/BASIC_PITCH_PRODUCTION_READINESS_CHECKLIST.md` | Source of the production-readiness gates |
| `docs/engineering/BASIC_PITCH_DEBUG_TO_PRODUCTION_TRANSITION_PLAN.md` | CODEX-115 placement in the transition sequence |
| `docs/engineering/BASIC_PITCH_DEBUG_COMBINED_RUN_IMPORT_ACTION_PROOF.md` | Current debug run/import evidence boundary |
| `docs/engineering/BASIC_PITCH_DEBUG_POST_IMPORT_EDIT_SAVE_EXPORT_PROOF.md` | Current edit/save/load/export proof boundary |
| `docs/engineering/BASIC_PITCH_MAINWINDOW_DEBUG_UI_ACTION_PROOF.md` | Current debug MainWindow action limits |
| `main/backend/BasicPitchDebugManualRunStatus.*` | Existing manual-run env/config preflight |
| `main/backend/BasicPitchDebugManualRunAction.*` | Existing manual handoff action report |
| `main/backend/BasicPitchDebugPostRunImportAction.*` | Existing result.json import action report |
| `main/backend/BasicPitchDebugCombinedRunImportAction.*` | Existing combined handoff/import proof report |
| `main/backend/BasicPitchDebugPostImportProofAction.*` | Existing post-import edit/save/load/export proof report |
| `main/backend/BasicPitchRealRunHandoffProof.*` | Existing manual opt-in real-run handoff |
| `main/backend/BasicPitchArtifactDiscovery.*` | Existing artifact discovery boundary |
| `main/backend/BasicPitchOutputConverter.*` | Existing Basic Pitch note-events conversion boundary |
| `main/backend/TonyLayerImporter.*` | Real Tony/SV layer import and edit proof boundary |
| `main/backend/test/TestBackendTypes.h` | Backend regression test harness |

## Code Model

New backend files:

- `main/backend/BasicPitchProductionReadinessPreflight.h`
- `main/backend/BasicPitchProductionReadinessPreflight.cpp`

Main types:

- `BasicPitchProductionReadinessGateStatus`
- `BasicPitchProductionReadinessGate`
- `BasicPitchProductionReadinessPreflightInput`
- `BasicPitchProductionReadinessReport`
- `BasicPitchProductionReadinessPreflight`

Each gate carries:

- gate id;
- display name;
- status: `passed`, `failed`, `warning`, `not_tested`, `blocked`, or `debug_only`;
- user-facing message;
- technical message;
- evidence/proof reference;
- whether it blocks production readiness;
- whether the evidence is debug-only.

## Code-Represented Gates

CODEX-115 represents these gates directly:

- `configuration`
- `command_runtime`
- `audio_input`
- `output_directory`
- `manual_run_permission`
- `artifact_discovery`
- `result_validation`
- `layer_mapping`
- `layer_import`
- `visibility`
- `edit`
- `undo_redo`
- `save_load`
- `export`
- `warning_visibility`
- `progress_cancel`
- `error_recovery`
- `documentation`
- `regression_protection`
- `pitch_bend_policy`
- `polyphony_policy`

The model intentionally separates production gates from debug evidence. For example, a debug post-import proof can make `edit`, `undo_redo`, `save_load`, and `export` report `debug_only`, but it does not make them `passed`.

## Documentation-Only Or Deferred Gates

These concerns remain documentation-only or deferred beyond the CODEX-115 model:

- detailed UX wording for a future production setup dialog;
- persistent settings and stale path behavior;
- GUI screenshot/smoke proof;
- real production progress events;
- real cancellation behavior;
- selected-region preview/accept/reject behavior;
- production pitch-bend layer/export policy;
- production polyphony/voice policy;
- end-user setup documentation.

They are still represented as blockers so future tasks cannot forget them.

## Current Expected Behavior

Current preflight output must remain:

```text
productionReady=false
productionTranscription=false
testOnlyDebugOnly=true
readyInstalledCompletedMutation=false
```

Reasons:

- production Basic Pitch configuration UI does not exist;
- production audio/output selection does not exist;
- progress/cancel is not production-proven;
- pitch-bend and polyphony production policies remain unresolved;
- debug proof is not production approval.

## Debug Evidence Inputs

The model can consume:

- `BasicPitchRealRunHandoffProofConfig`, via `BasicPitchDebugManualRunStatus`;
- `BasicPitchDebugCombinedRunImportActionReport`;
- `BasicPitchDebugPostImportProofActionReport`.

These inputs may populate evidence references and `debug_only` status. They never remove production blockers by themselves.

## Test Coverage

Backend tests prove:

- default preflight is valid but `productionReady=false`;
- debug manual-run availability remains `debug_only`;
- post-import proof evidence remains `debug_only`;
- missing production configuration and audio selection block readiness;
- missing progress/cancel support blocks readiness;
- pitch-bend and polyphony policies block readiness;
- even all boolean production gates cannot create production readiness while the CODEX-115 preflight boundary itself is debug/test-only;
- no Ready, Installed, or Completed backend state is created.

## Why Production Ready Is Still False

The preflight model is a planning and regression boundary, not a production feature. It does not add:

- production MainWindow action;
- production setup dialog;
- production run button;
- production audio/output chooser;
- production progress/cancel;
- production pitch-bend/polyphony policy;
- production end-user documentation.

Therefore it must not claim Basic Pitch production transcription support.

## Next Task

Recommended next task: CODEX-117 - guarded user-facing Basic Pitch setup/preflight dialog skeleton, no run.
