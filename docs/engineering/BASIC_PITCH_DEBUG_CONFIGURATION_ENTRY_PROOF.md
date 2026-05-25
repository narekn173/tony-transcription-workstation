# Basic Pitch Debug Configuration Entry Proof

Status: CODEX-108 debug/test-only configuration and manual-run status boundary  
CODEX-109 update: consumed by the explicit debug-only manual-run handoff action.
Production transcription support: not claimed  
Runtime execution: no automatic execution

## Purpose

CODEX-108 adds a small debug-only configuration/status boundary for the existing `Analysis -> Debug: Basic Pitch Workflow Proof...` action. It helps the debug report explain whether a manual Basic Pitch run is configured, what is missing, and whether the run would be skipped.

This is not a production Basic Pitch settings UI. It does not persist settings, does not use `QSettings`, does not add a production run button, and does not change pYIN or `Analyser` behavior.

## Files Inspected

| File | Reason |
|---|---|
| `main/MainWindow.cpp` / `main/MainWindow.h` | Confirmed the existing debug action and that MainWindow consumes formatter output only |
| `main/backend/BasicPitchDebugWorkflow.*` | Confirmed workflow modes, truth states, proof bundle, and real-run request handling |
| `main/backend/BasicPitchDebugWorkflowUiModel.*` | Added UI-consumable manual-run status fields without duplicating workflow logic |
| `main/backend/BasicPitchDebugWorkflowUiReportFormatter.*` | Added report sections for required env/config keys and skipped/allowed status |
| `main/backend/BasicPitchRealRunHandoffProof.*` | Confirmed manual opt-in env/config boundary |
| `main/backend/BasicPitchArtifactDiscovery.*` | Confirmed command/audio/output config and explicit opt-in behavior |
| `main/backend/test/TestBackendTypes.h` | Added compile/test-only coverage for configuration status and formatter output |

## Boundary Added

`BasicPitchDebugManualRunStatus` accepts a `BasicPitchRealRunHandoffProofConfig` and reports:

- required manual-run env/config keys;
- optional result JSON env/config key;
- explicit opt-in status;
- command/audio/output/result paths;
- missing configuration keys;
- whether `result.json` would be derived from the output directory;
- whether a manual run is allowed;
- whether a run would be skipped and why;
- `productionTranscription=false`;
- `testOnlyDebugOnly=true`;
- `readyInstalledCompletedMutation=false`.

The boundary does not run Basic Pitch, does not check or create result files, and does not mutate backend readiness.

## Required Manual-Run Configuration

The debug report lists these required env/config keys:

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

If `TONY_BASIC_PITCH_RESULT_JSON` is not provided and an output directory is configured, the debug status reports the derived path:

```text
<output-dir>/basic_pitch_result.json
```

## User-Visible Debug Report Behavior

The existing debug report now includes:

- manual-run required keys;
- optional keys;
- missing keys;
- manual run allowed: true/false;
- manual run would be skipped: true/false;
- skipped reason;
- derived result JSON path when applicable;
- proof bundle fields and warnings from previous CODEX tasks.

When configuration is incomplete, the report states:

```text
Basic Pitch debug workflow is not configured.
Manual Basic Pitch run would be skipped from this configuration.
No backend was run unless a separate proof report says otherwise.
This is not production transcription.
```

## Truth-State Guarantees

CODEX-108 preserves:

- no fake percentage progress;
- no fake Ready / Installed / Completed;
- no fake result.json;
- no fake notes;
- no fake overlay;
- no Imported claim unless `importedIntoTonyLayers=true`;
- no Visible claim unless `insertedIntoView=true`;
- no Editable claim unless edit proof passed;
- no Save/Export verified claim unless those proof flags passed.

## pYIN / Analyser Isolation

No pYIN or `Analyser` code is changed. `MainWindow` still uses the existing debug action, which consumes:

```text
BasicPitchDebugWorkflow
-> BasicPitchDebugWorkflowUiModel
-> BasicPitchDebugWorkflowUiReportFormatter
```

The existing `Analysis -> Analyse Now!` workflow remains unchanged.

## CODEX-109 Consumer

CODEX-109 adds:

```text
Analysis -> Debug: Run Basic Pitch Manual Handoff Proof...
```

That action consumes `BasicPitchDebugManualRunStatus` before any process execution. If manual-run status is not allowed, it shows a skipped report and does not run Basic Pitch. If manual-run status is allowed, it delegates to `BasicPitchRealRunHandoffProof` and reports result.json handoff/loading without importing into Tony layers.

CODEX-110 adds a separate post-run import action that can reuse the derived manual-run `result.json` path, while still requiring an explicit user action and a real current Document/Pane before it can claim imported/visible layer status.

## What Remains

The debug action is still not production Basic Pitch UI. A future production workflow must:

- remain user-triggered;
- keep manual opt-in required;
- show stage-based truth states only;
- preserve proof bundle access;
- avoid production support claims.

## Recommended Next Task

Recommended next task after CODEX-110: CODEX-111 - add a debug-only combined workflow action, still without production Basic Pitch UI claims.
