# Basic Pitch Debug Combined Run Import Action Proof

Status: CODEX-111 debug-only combined manual handoff plus import boundary
CODEX-112 update: includes debug post-import edit/save/load/export proof summary
CODEX-113 update: formatter shows a dedicated, truth-gated post-import proof section
Production transcription support: not claimed
Runtime behavior: explicit user action only

## Purpose

CODEX-111 adds a fourth debug-only Analysis menu action:

```text
Analysis -> Debug: Run Basic Pitch Manual Handoff and Import...
```

The action composes the two proven debug boundaries from CODEX-109 and CODEX-110:

```text
manual Basic Pitch preflight/run/handoff
-> real result.json
-> BackendRunResultLoader / UnifiedResultFileLoader
-> TonyLayerImporter / BasicPitchResultToTonyLayerProof
-> real Document-owned Tony/Sonic Visualiser NoteLayer
-> optional real Pane/View insertion
-> debug post-import edit/save/load/export proof summary
```

It remains debug/test-only. It does not add production Basic Pitch UI, does not alter pYIN, and does not claim backend readiness.

## Files Inspected

| File | Reason |
|---|---|
| `main/MainWindow.cpp` / `main/MainWindow.h` | Existing debug actions, current Document/Pane access, status/dialog patterns |
| `main/backend/BasicPitchDebugManualRunAction.*` | Proven manual handoff/preflight/action boundary |
| `main/backend/BasicPitchDebugPostRunImportAction.*` | Proven existing result.json to real Tony layer import boundary |
| `main/backend/BasicPitchRealRunHandoffProof.*` | Explicit opt-in real Basic Pitch process/artifact/result handoff proof |
| `main/backend/BasicPitchResultToTonyLayerProof.*` | Basic Pitch-shaped result.json to real Tony/SV NoteLayer proof |
| `main/backend/TonyLayerImporter.*` | Real Tony/SV model, Document layer, and View insertion APIs |
| `main/backend/test/TestBackendTypes.h` | Backend tests for skipped, failed, and successful combined paths |

## Boundary Added

`BasicPitchDebugCombinedRunImportAction` accepts `BasicPitchDebugCombinedRunImportActionOptions` containing:

- `BasicPitchRealRunHandoffProofConfig`
- sample rate and resolution
- optional real `Document`
- optional real `View/Pane`
- View insertion policy

It then:

1. runs `BasicPitchDebugManualRunAction`;
2. stops without import if preflight is skipped or manual execution fails;
3. requires the manual handoff to produce and load a real `result.json`;
4. passes that `result.json` to `BasicPitchDebugPostRunImportAction`;
5. runs `BasicPitchDebugPostImportProofAction` after successful import;
6. reports real import/edit/save/export flags only from proven debug proof boundaries.

MainWindow only constructs the action options, calls the action, formats the report, and shows the dialog. It does not duplicate workflow state logic.

## UI Behavior

When configuration is missing:

- no backend process is run;
- no result import is attempted;
- `importedIntoTonyLayers=false`;
- `insertedIntoView=false`;
- the report states that the combined debug action was skipped.

When opt-in or paths are incomplete:

- the manual handoff action reports the missing key/path;
- the combined action stops before import;
- no fake result, layer, or visibility state is produced.

When manual execution fails:

- the backend process failure or artifact/handoff error remains visible;
- no import is attempted;
- no hidden Tony layer is created.

When manual execution succeeds and a real `result.json` is written:

- `BackendRunResultLoader` loads the file;
- `BasicPitchDebugPostRunImportAction` imports only through the proven real-layer path;
- `importedIntoTonyLayers=true` only after a real Document-owned layer exists;
- `insertedIntoView=true` only after real Pane/View insertion occurs.
- post-import edit, undo/redo, save/load, and export proof fields are shown only after the real proof action passes.

If import fails or the proof is disabled, the formatted report shows:

- `Post-import proof: not tested by this action`;
- an explanatory line that the proof did not run because import did not complete or the proof was disabled;
- no edit, undo/redo, save/load, or export success claim from the post-import proof.

When import succeeds and CODEX-112 proof runs, the formatted report includes a dedicated `Post-Import Proof` section, separate from the manual handoff and import sections.

## Report Fields

The formatted report includes:

- workflow stage sequence
- manual run status
- command
- input audio path
- output directory
- result.json path
- discovered artifact count and summaries
- selected `csv_note_events` artifact
- loaded-result status
- note count
- warnings and errors
- `possible_polyphony`
- `pitch_bend_mapping_deferred`
- `importedIntoTonyLayers`
- `insertedIntoView`
- post-import proof status
- proof `resultJsonPath`
- proof export CSV path
- proof loaded-result status
- real `NoteModel` existence
- real `NoteLayer` existence
- Document-owned layer status
- proof Pane/View insertion status
- layer editable status
- edit proof status
- undo/redo proof status
- save/load proof status
- export proof status
- exported note row count from the real CSV proof
- exported CSV non-empty status
- timing/duration/pitch/velocity export preservation status
- `productionTranscription=false`
- `testOnlyDebugOnly=true`
- `readyInstalledCompletedMutation=false`

## Truth-State Guarantees

CODEX-111 preserves:

- no automatic Basic Pitch execution;
- no startup execution;
- no execution from normal `Analyse Now`;
- no fake percentage progress;
- no fake Ready / Installed / Completed state;
- no fake result.json;
- no fake notes;
- no fake overlays;
- no production transcription claim;
- no silent hiding of polyphony or pitch-bend warnings;
- no pYIN / `Analyser` behavior changes.

## What This Does Not Prove

This is not production Basic Pitch UI. It does not provide:

- persistent Basic Pitch configuration;
- production run/cancel/progress UI;
- selected-region replacement;
- production polyphony resolution;
- production pitch-bend layer mapping;
- production edit/save/load/export workflow beyond the debug proof summary;
- production backend readiness or install-state management.

## Verification Expectations

Verification must include:

- build passes;
- `backend-types` passes;
- full suite passes where local environment permits;
- no generated build files committed;
- no MainWindow/pYIN/Analyser behavior changes beyond the debug-only action.

## Recommended Next Task

Recommended next task after CODEX-114: CODEX-115 - add a Basic Pitch production-readiness preflight model, still without production UI or runtime behavior changes.
