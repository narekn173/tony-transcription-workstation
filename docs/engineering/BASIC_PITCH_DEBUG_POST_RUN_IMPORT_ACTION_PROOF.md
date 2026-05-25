# Basic Pitch Debug Post-Run Import Action Proof

Status: CODEX-110 debug-only post-run result import boundary  
Production transcription support: not claimed  
Runtime behavior: explicit user action only

## Purpose

CODEX-110 adds a third debug-only Analysis menu action:

```text
Analysis -> Debug: Import Basic Pitch Manual Result...
```

The action imports an existing Basic Pitch-shaped `result.json` through the proven loader and Tony layer importer path. It does not run Basic Pitch, does not create `result.json`, and does not claim production transcription support.

CODEX-111 adds a separate combined debug action that first performs the manual handoff proof and then calls this post-run import boundary only when a real loaded `result.json` exists.

## Files Inspected

| File | Reason |
|---|---|
| `main/MainWindow.cpp` / `main/MainWindow.h` | Existing debug actions, current Document/Pane access, status/dialog patterns |
| `main/backend/BasicPitchDebugManualRunStatus.*` | Manual-run config/status path used to derive the result path |
| `main/backend/BasicPitchResultToTonyLayerProof.*` | Proven Basic Pitch-shaped result.json to real Tony NoteLayer boundary |
| `main/backend/TonyLayerImporter.*` | Real Tony/SV `NoteModel`, Document-owned `NoteLayer`, and View insertion API use |
| `main/backend/BackendRunResultLoader.*` / `UnifiedResultFileLoader.*` | Real result.json loading and validation |
| `main/backend/test/TestBackendTypes.h` | Compile/test-only coverage for missing/invalid/valid import cases |

## Boundary Added

`BasicPitchDebugPostRunImportAction` accepts:

- explicit `resultJsonPath`, or
- a `BasicPitchRealRunHandoffProofConfig` from which the result path can be derived through `BasicPitchDebugManualRunStatus`;
- optional real `Document` and `View/Pane` pointers;
- sample rate and model resolution.

It then:

1. resolves a `result.json` path;
2. loads it through `BackendRunResultLoader`;
3. reports missing/empty/invalid files without importing;
4. requires a real `Document` before creating a Tony layer;
5. optionally requires a real current `Pane/View` for UI-visible insertion;
6. delegates actual import to `BasicPitchResultToTonyLayerProof` / `TonyLayerImporter`;
7. reports `importedIntoTonyLayers=true` only after a real Document-owned layer exists;
8. reports `insertedIntoView=true` only after real View/Pane insertion.

`BasicPitchDebugPostRunImportActionReportFormatter` renders the report for MainWindow. MainWindow displays formatter output and does not duplicate import state logic.

## Result Path Selection

The action uses this order:

1. explicit debug import `resultJsonPath`;
2. `TONY_BASIC_PITCH_RESULT_JSON` from manual-run config;
3. derived `<output-dir>/basic_pitch_result.json` when `TONY_BASIC_PITCH_OUTPUT_DIR` is available.

If no path can be resolved, the report uses `missing_basic_pitch_debug_import_result_json_path` and imports nothing.

## UI Behavior

When `result.json` is missing:

- no import is attempted;
- `loadedResult=false`;
- `importedIntoTonyLayers=false`;
- `insertedIntoView=false`;
- the report shows the loader error such as `output_file_missing`.

When `result.json` is empty or invalid:

- no import is attempted;
- the report shows `empty_output_file` or `invalid_json`;
- no Tony layer or hidden model is created.

When `result.json` is valid but no current Document/Pane is available:

- no fake layer or hidden UI object is created;
- the report explains the missing active `Document` or `Pane/View`;
- `importedIntoTonyLayers=false` unless a real Document-owned layer was actually created.

When `result.json` is valid and a real Document/Pane is available:

- UnifiedResult loads through the existing loader;
- Basic Pitch warnings such as `possible_polyphony` and `pitch_bend_mapping_deferred` remain visible;
- `TonyLayerImporter` creates a real `NoteModel` and Document-owned `NoteLayer`;
- `Document::addLayerToView` inserts the layer into the current real Pane/View;
- the report sets `importedIntoTonyLayers=true` and `insertedIntoView=true`.

## Truth-State Guarantees

CODEX-110 preserves:

- no automatic Basic Pitch execution;
- no fake percentage progress;
- no fake Ready / Installed / Completed;
- no fake result.json;
- no fake notes;
- no fake overlays;
- no production transcription claim;
- no pYIN / `Analyser` behavior changes.

The action reports edit, save/load, and export proof fields as not tested by this action. Those gates remain covered by earlier proof tasks and are not re-claimed here.

## What This Does Not Prove

This is not production Basic Pitch UI. It does not provide:

- persistent Basic Pitch settings;
- production run/cancel workflow;
- selected-region replacement;
- production polyphony resolution;
- production pitch-bend layer mapping;
- automatic post-run import;
- save/load/export execution from this UI action.

## Recommended Next Task

Recommended next task after CODEX-111: CODEX-112 - add a debug-only post-import proof summary/action for edit, save/load, and export verification from the visible debug path.
