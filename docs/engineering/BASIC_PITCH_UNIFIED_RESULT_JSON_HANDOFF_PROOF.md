# Basic Pitch UnifiedResult JSON Handoff Proof

Status: CODEX-099 manual/test-only JSON handoff proof, consumed by CODEX-100 manual real-run proof and CODEX-101 result-to-Tony-layer proof.

This document defines the next Basic Pitch proof boundary:

```text
discovered note-events CSV artifact
-> in-memory UnifiedResult
-> real result.json written to disk
-> BackendRunOutputHandoff
-> BackendRunResultLoader / UnifiedResultFileLoader
-> BackendRunResultReporter
```

This is still not production Basic Pitch transcription support. It does not run Basic Pitch by default, does not import Tony layers, and does not mark Basic Pitch Ready, Installed, or Completed.

## Sources Inspected

- `docs/engineering/BASIC_PITCH_ADAPTER_AUDIT.md`
- `docs/engineering/BASIC_PITCH_OUTPUT_CONVERSION_PLAN.md`
- `docs/engineering/BASIC_PITCH_REAL_ARTIFACT_DISCOVERY.md`
- `docs/engineering/BASIC_PITCH_REAL_ARTIFACT_TO_UNIFIED_RESULT_PROOF.md`
- `main/backend/BasicPitchArtifactDiscovery.*`
- `main/backend/BasicPitchOutputConverter.*`
- `main/backend/BasicPitchArtifactToUnifiedResult.*`
- `main/backend/UnifiedResult.*`
- `main/backend/UnifiedResultParser.*`
- `main/backend/UnifiedResultFileLoader.*`
- `main/backend/BackendRunOutputHandoff.*`
- `main/backend/BackendRunResultLoader.*`
- `main/backend/BackendRunResultReporter.*`

## New Boundary

`BasicPitchUnifiedResultHandoff` accepts a `BasicPitchArtifactDiscoveryResult` plus an explicit expected `result.json` path. It:

1. selects the recognized `csv_note_events` artifact through `BasicPitchArtifactToUnifiedResult`;
2. converts parsed artifact data into `UnifiedResult`;
3. writes the result through `UnifiedResultFileWriter`;
4. checks file presence/readability through `BackendRunOutputHandoff`;
5. loads the file through `BackendRunResultLoader` and `UnifiedResultFileLoader`;
6. builds a `BackendRunResultReport`.

The written `result.json` is produced from parsed artifact rows. It is not a hardcoded fake result.

## Schema Fields Used

The writer serializes the current `UnifiedResult` model into the existing loader-compatible JSON shape:

- `contract_version`
- `result_id`
- `request_id`
- `created_at`
- `engine`
- `status`
- `audio`
- `region`
- `summary`
- `notes`
- `pitch_curve`
- `pitch_bends`
- `technique_labels`
- `files`
- `warnings`
- `errors`
- `provenance`

The in-memory CODEX-096/098 converter keeps `status=Unknown` because conversion alone is not a backend run. The CODEX-099 handoff file uses `completed_with_warnings` only for the written JSON handoff artifact so the existing loader can accept it. This is paired with explicit warnings and provenance flags:

- `basic_pitch_result_json_handoff_test_only`
- `production_transcription=false`
- `imported_into_tony_layers=false`
- `basic_pitch_handoff_test_only=true`
- `created_result_json=true`

This does not mutate manifest, registry, availability, or UI state.

## Warnings Preserved

The handoff preserves converter warnings through the JSON round trip:

- `possible_polyphony`
- `pitch_bend_mapping_deferred`
- `fixture_only_conversion` or `real_artifact_manual_only`
- `production_transcription_false`
- `basic_pitch_result_json_handoff_test_only`

The pitch-bend values remain in `UnifiedResult.pitchBends`, but Tony pitch-bend layer mapping is still deferred.

## What CODEX-099 Proves

- A recognized Basic Pitch note-events CSV artifact can be converted to `UnifiedResult`.
- The converted result can be written to a real non-empty `result.json`.
- `BackendRunOutputHandoff` accepts the written file.
- `BackendRunResultLoader` loads the file successfully.
- `BackendRunResultReporter` reports that a `UnifiedResult` was loaded.
- Note count, timing, MIDI pitch, velocity, pitch bends, and warnings survive the artifact-to-file-to-loader round trip.
- `importedIntoTonyLayers=false` remains true for the whole boundary.
- No Basic Pitch Ready, Installed, or Completed backend state is created.

## What Remains Unproven

- Basic Pitch is installed or runnable locally.
- Basic Pitch runs on real user audio inside Tony.
- The output is production transcription.
- MIDI or NPZ artifacts are converted.
- Production Basic Pitch output is imported into real Tony layers.
- Basic Pitch polyphony and pitch bends are displayed or editable in Tony.
- Basic Pitch results survive Tony layer save/load/export.

## Recommended Next Task

Recommended next task: CODEX-102 - Basic Pitch result layer save/load/export proof with the same strict no-fake-state rules.

CODEX-100 adds a manual opt-in wrapper that can run a locally configured Basic Pitch command, discover its artifacts, and feed the recognized note-events CSV through this JSON handoff path. CODEX-101 consumes a Basic Pitch-shaped `result.json` from this path and imports notes through the proven real Tony/Sonic Visualiser `NoteModel`/`NoteLayer` boundary. It still does not add UI, solve pitch-bend layer mapping, or claim production transcription.
