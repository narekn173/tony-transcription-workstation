# Basic Pitch Debug Manual Run Action Proof

Status: CODEX-109 debug-only manual-run UI boundary  
Production transcription support: not claimed  
Runtime behavior: explicit user action only

## Purpose

CODEX-109 adds a second debug-only Analysis menu action:

```text
Analysis -> Debug: Run Basic Pitch Manual Handoff Proof...
```

The action preflights manual-run configuration, runs Basic Pitch only when the explicit debug/manual requirements are satisfied, and reports the real-run handoff result. It stops at artifact/result.json handoff and loading; it does not import into Tony layers.

CODEX-110 adds a separate post-run import action for an existing `result.json`. That action is intentionally separate so manual process execution and Tony layer import remain two explicit debug/user-triggered steps.

CODEX-111 adds a separate combined debug action that composes the manual handoff and post-run import boundaries. The manual handoff action itself still stops before Tony layer import.

## Files Inspected

| File | Reason |
|---|---|
| `main/MainWindow.cpp` / `main/MainWindow.h` | Existing Analysis menu, debug proof action, status/dialog patterns |
| `main/backend/BasicPitchDebugManualRunStatus.*` | Manual-run env/config preflight boundary |
| `main/backend/BasicPitchRealRunHandoffProof.*` | Existing explicit real Basic Pitch process/artifact/result handoff proof |
| `main/backend/BasicPitchArtifactDiscovery.*` | Argument-list process request and artifact classification |
| `main/backend/BasicPitchUnifiedResultHandoff.*` | Artifact-to-result.json loader/reporter handoff |
| `main/backend/BasicPitchDebugWorkflowUiReportFormatter.*` | Existing report style and proof-bundle display pattern |
| `main/backend/test/TestBackendTypes.h` | Backend test coverage for manual-run status/action/formatter |

## Boundary Added

`BasicPitchDebugManualRunAction` accepts `BasicPitchRealRunHandoffProofConfig` and:

1. calls `BasicPitchDebugManualRunStatus::fromConfig(...)`;
2. returns a skipped report without running when opt-in, command, audio, or output directory is missing;
3. calls `BasicPitchRealRunHandoffProof::run(...)` only when preflight allows it;
4. reports command, input audio, output directory, discovered artifacts, selected `csv_note_events`, result.json path, loaded status, note count, warnings, and errors;
5. preserves `productionTranscription=false`, `testOnlyDebugOnly=true`, `importedIntoTonyLayers=false`, and `readyInstalledCompletedMutation=false`.

`BasicPitchDebugManualRunActionReportFormatter` formats this result for the MainWindow dialog. MainWindow does not duplicate workflow/preflight logic.

## Required Manual-Run Configuration

Required:

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

If `TONY_BASIC_PITCH_RESULT_JSON` is absent, the result path may be derived from the output directory by the existing real-run handoff proof.

## UI Behavior

When configuration is incomplete:

- no backend process is run;
- the dialog reports missing keys;
- the report states that the manual run would be skipped;
- this is treated as a successful debug preflight/skipped report, not a production failure.

When configuration is complete and opt-in is true:

- the action calls the existing real-run handoff proof;
- Basic Pitch is run through `ExternalProcessRunner`;
- produced artifacts are discovered;
- the selected note-events CSV is converted to UnifiedResult;
- a real result.json is written and loaded when the proof succeeds;
- Tony layer import remains false.

## Truth-State Guarantees

CODEX-109 preserves:

- no automatic Basic Pitch execution;
- no fake percentage progress;
- no fake Ready / Installed / Completed;
- no fake result.json;
- no fake notes;
- no fake overlays;
- no Tony layer import from this action;
- no pYIN / `Analyser` behavior changes.

## What This Does Not Prove

This is not production Basic Pitch UI. It does not provide:

- persistent Basic Pitch configuration;
- production run/cancel workflow;
- selected-region replacement;
- production polyphony policy;
- production pitch-bend mapping;
- Tony layer import from this manual-run action.

## Recommended Next Task

Recommended next task after CODEX-111: CODEX-112 - add a debug-only post-import proof summary/action for edit, save/load, and export verification from the visible debug path.
