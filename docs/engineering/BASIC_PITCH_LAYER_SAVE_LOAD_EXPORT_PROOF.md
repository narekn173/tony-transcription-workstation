# Basic Pitch Layer Save Load Export Proof

Status: CODEX-102 manual/test-only Basic Pitch-shaped layer persistence and export proof.

This document defines the Basic Pitch-shaped result layer proof chain:

```text
Basic Pitch-shaped result.json
-> BackendRunResultLoader
-> loaded UnifiedResult
-> BasicPitchResultToTonyLayerProof
-> TonyLayerImporter
-> real Document-owned NoteLayer inserted into a real Pane
-> Document::toXml / Pane::toXml
-> SVFileReader::parseXml
-> getExportModel(...) / CSVFileWriter
```

This is not user-facing Basic Pitch UI integration and not production transcription support.

## Boundary

`BasicPitchLayerPersistenceExportProof` accepts an explicit `result.json` path, or a `BasicPitchUnifiedResultHandoffResult`, plus an explicit CSV export path. It does not run Basic Pitch, create Basic Pitch artifacts, fabricate `result.json`, or import anything into Tony startup/UI paths.

The boundary creates a local test-only `Document` and `Pane`, imports the loaded Basic Pitch-shaped notes through `BasicPitchResultToTonyLayerProof`, and then proves persistence/export through the same real Tony/Sonic Visualiser APIs already proven for generic imported layers.

## Real APIs Used

- `BackendRunResultLoader`
- `TonyLayerImporter`
- `Document::createImportedLayer`
- `Document::addLayerToView`
- `NoteModel`
- `NoteLayer`
- `Document::toXml`
- `Pane::toXml`
- `SVFileReader::parseXml`
- `Layer::getExportModel`
- `CSVFileWriter`

No custom overlay, manual drawing layer, fake result file, or fake backend availability state is introduced.

## What CODEX-102 Proves

- Basic Pitch-shaped `result.json` can be loaded and imported into a real Document-owned `NoteLayer`.
- The imported layer can be inserted into a real `Pane`.
- The saved session XML is non-empty and contains the Basic Pitch-derived note layer.
- Reload reconstructs a real editable `NoteLayer` backed by a real `NoteModel`.
- Note count, timing, duration, MIDI pitch, velocity-derived level, and labels survive save/load.
- Durable provenance-derived identity survives save/load through existing Tony/SV model/layer naming fields.
- The imported layer can be exported through `getExportModel(...)` and `CSVFileWriter`.
- The exported CSV is non-empty and preserves note count, timing, duration, MIDI pitch, velocity-derived level, and labels supported by the CSV path.
- `possible_polyphony` and `pitch_bend_mapping_deferred` warnings remain reported.
- `productionTranscription=false` remains true for this proof.

## Warnings Preserved

The proof intentionally keeps these Basic Pitch limitations visible:

- `possible_polyphony`
- `pitch_bend_mapping_deferred`
- `basic_pitch_possible_polyphony_not_resolved`
- `basic_pitch_pitch_bend_tony_mapping_deferred`
- `basic_pitch_layer_persistence_export_test_only`

Pitch-bend values remain in `UnifiedResult.pitchBends`, but no Tony pitch-bend layer is created or exported in CODEX-102.

Possible polyphony remains a warning. CODEX-102 proves a Basic Pitch-shaped note layer can survive real Tony/SV save/load/export APIs in a test-only path. It does not solve production polyphony mapping.

## What Remains Unproven

- User-facing Basic Pitch UI.
- Automatic runtime Basic Pitch execution.
- Production transcription quality.
- Production polyphony policy.
- Pitch-bend import into a real Tony/SV layer.
- MIDI, SVL, RDF, or other export formats for Basic Pitch-shaped layers.
- Export of pitch bends, NPZ/model outputs, or Basic Pitch MIDI artifacts.
- Selected-region replacement workflow.

## Recommended Next Task

Recommended next task: CODEX-103 - first debug-only Basic Pitch workflow wiring plan or boundary, gated by UI truth states and still without production-ready claims.
