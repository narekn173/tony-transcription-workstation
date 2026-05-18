# Basic Pitch Output Conversion Plan

Status: CODEX-096 compile-only/test-only converter boundary, extended by CODEX-097 discovery and CODEX-098 artifact-to-UnifiedResult proof.

This document defines the first fixture-backed Basic Pitch note-events CSV to `UnifiedResult` conversion boundary. It does not run Basic Pitch, does not create production `result.json`, does not import Tony layers, and does not mark Basic Pitch Ready, Installed, or Completed.

## Sources Inspected

- `docs/engineering/BASIC_PITCH_ADAPTER_AUDIT.md`
- `docs/engineering/LAYER_TYPE_POLICY.md`
- `docs/engineering/BACKEND_OUTPUT_TRUTH_TABLE.md`
- `docs/engineering/BACKEND_TO_TONY_LAYER_MAPPING_MATRIX.md`
- `main/backend/BasicPitchAdapterContract.*`
- `main/backend/UnifiedResult.*`
- `main/backend/ResultValidator.*`
- `main/backend/UnifiedResultFileLoader.*`
- `main/backend/TonyLayerImporter.*`
- `main/backend/BasicPitchArtifactDiscovery.*`
- `main/backend/BasicPitchArtifactToUnifiedResult.*`
- Spotify Basic Pitch inference source: https://github.com/spotify/basic-pitch/blob/main/basic_pitch/inference.py
- Spotify Basic Pitch note creation source: https://github.com/spotify/basic-pitch/blob/main/basic_pitch/note_creation.py

## Verified Output Facts

The Basic Pitch source writes note-events CSV with this header:

```text
start_time_s,end_time_s,pitch_midi,velocity,pitch_bend
```

For each note event, Basic Pitch writes:

- start time in seconds;
- end time in seconds;
- MIDI pitch number;
- MIDI velocity derived from the note amplitude;
- zero or more pitch-bend values appended after velocity when pitch bends exist.

The source header has one `pitch_bend` column, but rows may contain multiple pitch-bend values because Basic Pitch appends the pitch-bend list to the row. Rows without pitch bends may contain only the first four values.

The note-creation source describes pitch bends as evenly spaced MIDI pitch-bend control units across each note's time span.

## Fixture Schema Used In CODEX-096

The test fixture uses the verified Basic Pitch note-events CSV shape:

```csv
start_time_s,end_time_s,pitch_midi,velocity,pitch_bend
0.10,0.50,60,91,0,12,-8
0.30,0.70,64,88,1,0,-1
0.80,1.00,67,72
```

This fixture intentionally includes overlapping notes so the converter must report possible polyphony. It also includes pitch-bend values so the converter must preserve them in `UnifiedResult.pitchBends` and warn that Tony pitch-bend layer mapping remains deferred.

## Conversion Mapping

| Basic Pitch CSV field | UnifiedResult target | Notes |
|---|---|---|
| `start_time_s` | `NoteEvent.startSec` | Preserved as seconds |
| `end_time_s` | `NoteEvent.endSec` | Preserved as seconds |
| `pitch_midi` | `NoteEvent.midiPitch` | Must remain in 0-127 range |
| `velocity` | `NoteEvent.velocity` | Must remain in 0-127 range |
| appended pitch-bend values | `UnifiedResult.pitchBends` plus `NoteEvent.pitchBendRef` | Preserved as `midi_pitch_bend_units`; Tony layer mapping still unproven |

The converter sets:

- `result.engine.engineId = basic_pitch`
- `result.engine.displayName = Basic Pitch`
- `result.engine.adapterVersion = fixture-converter-0.1`
- `result.status = Unknown`
- `result.provenance.fixture_only_conversion = true`
- `result.provenance.production_transcription = false`
- `result.provenance.created_result_json = false`
- `result.provenance.imported_into_tony_layers = false`

`Unknown` status is intentional. A fixture conversion is not a backend run and must not create fake Completed state.

## Required Warnings

Every successful CODEX-096 conversion includes:

- `possible_polyphony`
- `fixture_only_conversion`
- `production_transcription_false`

When pitch-bend values are present, conversion also includes:

- `pitch_bend_mapping_deferred`

The converter may preserve pitch-bend data in `UnifiedResult`, but no Tony pitch-bend layer behavior is claimed.

## Validation Rules

The parser fails cleanly when:

- CSV text is empty;
- required header columns are missing or reordered;
- note rows have too few columns;
- note timing is invalid;
- MIDI pitch is outside 0-127;
- velocity is outside 0-127;
- pitch-bend values are not numeric.

Warnings do not make the conversion invalid. They exist to prevent fake feature-complete claims.

## What CODEX-096 Proves

- A controlled Basic Pitch note-events CSV fixture can be parsed.
- Note count is preserved.
- Start/end timing is preserved.
- MIDI pitch is preserved.
- Velocity is preserved.
- Pitch-bend values can be preserved in `UnifiedResult.pitchBends`.
- Possible polyphony is detected and reported.
- The converter creates a `UnifiedResult` in memory only.
- No production `result.json` is created.
- No Tony layer import occurs.
- No backend Ready, Installed, or Completed state is created.

## What Remains Required

Before production Basic Pitch integration:

1. Run real Basic Pitch on real audio in a controlled backend task.
2. Capture real MIDI/CSV/NPZ artifacts from the selected Basic Pitch version.
3. Confirm artifact filenames and CSV edge cases from actual output.
4. Decide whether the converter should parse CSV, MIDI, NPZ, or adapter-produced JSON first.
5. Preserve or warn for pitch bends, polyphony, velocity, and confidence-like data.
6. Write a real `result.json` only from a real backend run or explicitly marked dev/test path.
7. Load that `result.json` through `UnifiedResultFileLoader`.
8. Import notes into real Tony layers only after layer mapping policy is applied.

CODEX-097 adds the manual/test-only discovery harness for steps 1 and 2. CODEX-098 adds the manual/test-only bridge that selects a discovered note-events CSV artifact and converts it into an in-memory `UnifiedResult`. It still does not create `result.json`, run Basic Pitch in normal tests, import Tony layers, or mark Basic Pitch Ready/Installed/Completed.

## Recommended Next Task

Recommended next task: CODEX-099 - Basic Pitch UnifiedResult JSON handoff proof.

That task should write a `UnifiedResult` produced by CODEX-098 to an explicitly marked test/manual result file, then load it through `UnifiedResultFileLoader` without importing Tony layers.
