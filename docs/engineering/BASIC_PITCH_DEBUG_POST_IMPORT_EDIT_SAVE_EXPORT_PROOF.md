# Basic Pitch Debug Post-Import Edit Save Export Proof

Status: CODEX-112 debug-only post-import proof summary
CODEX-113 update: combined debug report presents a dedicated post-import proof section
Production transcription support: not claimed
Runtime behavior: proof summary inside the explicit debug combined action

## Purpose

CODEX-112 adds a debug-only post-import proof boundary used by:

```text
Analysis -> Debug: Run Basic Pitch Manual Handoff and Import...
```

After the combined action successfully produces a real `result.json` and imports it into a real Tony/Sonic Visualiser layer, the post-import proof verifies the same Basic Pitch-shaped result through the real edit, save/load, and export proof chain.

This is a debug proof summary. It is not production Basic Pitch UI and does not change backend readiness state.

## Files Inspected

| File | Reason |
|---|---|
| `main/backend/BasicPitchDebugCombinedRunImportAction.*` | Existing combined manual handoff plus import boundary |
| `main/backend/BasicPitchDebugPostRunImportAction.*` | Existing result.json to real Tony layer import boundary |
| `main/backend/BasicPitchLayerPersistenceExportProof.*` | Real save/load/export proof path extended with edit/undo/redo proof |
| `main/backend/TonyLayerImporter.*` | Real NoteModel/NoteLayer import and CommandHistory edit proof |
| `svapp/framework/Document.*` / `SVFileReader.*` | Real XML session save/load path used by the proof |
| `svgui/layer/NoteLayer.*` / `CSVFileWriter` | Real editable note layer and CSV export path |
| `main/backend/test/TestBackendTypes.h` | Backend tests for unavailable and valid post-import proof reports |

## Boundary Added

`BasicPitchDebugPostImportProofAction` accepts:

- `resultJsonPath`
- optional export CSV path
- sample rate
- model resolution

It uses `BasicPitchLayerPersistenceExportProof`, which now verifies:

- real `NoteModel` exists;
- real `NoteLayer` exists;
- the layer is Document-owned;
- the layer is inserted into a real Pane/View in the proof harness;
- the layer is editable;
- edit/undo/redo works through `CommandHistory` / `ChangeEventsCommand`;
- XML save/load survives through `Document::toXml`, `Pane::toXml`, and `SVFileReader::parseXml`;
- CSV export survives through `getExportModel(...)` and `CSVFileWriter`;
- exported note row count comes from the actual CSV output.

`BasicPitchDebugPostImportProofActionReportFormatter` renders these proof fields for future/debug UI consumption. MainWindow does not duplicate proof logic.

CODEX-113 also makes `BasicPitchDebugCombinedRunImportActionReportFormatter` render the same proof outcome in a dedicated `Post-Import Proof` section after a successful combined debug import.

## UI/Report Behavior

The CODEX-112 proof is displayed as part of the combined debug action report after a successful import.

When no result is available:

- `loadedResult=false`;
- edit/undo/save/export proof fields remain false or not tested;
- no fake layer is created;
- no fake export row count is reported.

When the combined debug action does not reach a successful import, the combined report explicitly says that post-import proof was not tested by this action. It does not claim visibility, editability, save/load survival, or export survival.

When a valid Basic Pitch-shaped result is available:

- the proof imports through the real Tony layer path;
- the combined report shows the proof `resultJsonPath` and export CSV path;
- real `NoteModel`, real `NoteLayer`, Document-owned layer, and proof Pane/View insertion flags are shown;
- edit proof is marked passed only after the real command proof succeeds;
- undo/redo proof is marked passed only after the command proof restores/redo/undo behavior;
- save/load proof is marked passed only after XML reload preserves note data;
- export proof is marked passed only after CSV rows are produced and parsed;
- `exportedNoteCount` is based on actual exported CSV rows.

## Truth-State Guarantees

CODEX-112 preserves:

- no fake percentage progress;
- no fake Ready / Installed / Completed state;
- no fake result.json;
- no fake notes;
- no fake overlays;
- no fake edit/save/export proof;
- no production transcription claim;
- no silent hiding of `possible_polyphony`;
- no silent hiding of `pitch_bend_mapping_deferred`;
- no pYIN / `Analyser` behavior changes.

## What This Does Not Prove

This is still not production Basic Pitch UI. It does not provide:

- persistent Basic Pitch settings;
- production run/cancel/progress UI;
- selected-region replacement;
- production polyphony resolution;
- production pitch-bend layer mapping;
- production MIDI/SVL/RDF export coverage;
- user-facing backend readiness/install/completion state.

## Recommended Next Task

Recommended next task after CODEX-116: CODEX-117 - guarded user-facing Basic Pitch setup/preflight dialog skeleton, no run.
