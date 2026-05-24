# Basic Pitch Real Artifact To UnifiedResult Proof

Status: CODEX-098 manual/test-only artifact conversion proof, extended by CODEX-099 JSON handoff proof, CODEX-100 manual real-run proof, CODEX-101 result-to-Tony-layer proof, and CODEX-102 layer save/load/export proof.

This document defines the explicit proof boundary that connects Basic Pitch artifact discovery to the fixture-backed note-events CSV converter. It proves that a discovered Basic Pitch note-events CSV artifact can become an in-memory `UnifiedResult`. It does not create `result.json`, does not run Basic Pitch by default, does not import Tony layers, and does not mark Basic Pitch Ready, Installed, or Completed.

## Sources Inspected

- `docs/engineering/BASIC_PITCH_ADAPTER_AUDIT.md`
- `docs/engineering/BASIC_PITCH_OUTPUT_CONVERSION_PLAN.md`
- `docs/engineering/BASIC_PITCH_REAL_ARTIFACT_DISCOVERY.md`
- `main/backend/BasicPitchArtifactDiscovery.*`
- `main/backend/BasicPitchOutputConverter.*`
- `main/backend/UnifiedResult.*`
- `main/backend/ResultValidator.*`
- Spotify Basic Pitch inference source: https://github.com/spotify/basic-pitch/blob/main/basic_pitch/inference.py

## Verified Schema

The Basic Pitch source writes note-events CSV with the header:

```text
start_time_s,end_time_s,pitch_midi,velocity,pitch_bend
```

Rows contain start time, end time, MIDI pitch, velocity, and zero or more appended pitch-bend values. CODEX-098 only converts artifacts classified as `csv_note_events` by the discovery harness.

## Artifact Selection Rules

`BasicPitchArtifactToUnifiedResult` accepts a `BasicPitchArtifactDiscoveryResult` and selects the first discovered artifact whose `artifactType` is:

```text
csv_note_events
```

Other artifacts are ignored for this boundary:

- MIDI remains deferred.
- NPZ/model output remains deferred.
- WAV sonification remains diagnostic only.
- Logs remain diagnostic only.
- Unknown artifacts are reported safely as not convertible.

If no `csv_note_events` artifact exists, conversion fails with `missing_basic_pitch_note_events_artifact`.

## Conversion Mapping

The selected CSV is read from disk and passed to `BasicPitchOutputConverter`.

Mapping remains:

| Basic Pitch CSV field | UnifiedResult target |
|---|---|
| `start_time_s` | `NoteEvent.startSec` |
| `end_time_s` | `NoteEvent.endSec` |
| `pitch_midi` | `NoteEvent.midiPitch` |
| `velocity` | `NoteEvent.velocity` |
| appended pitch-bend values | `UnifiedResult.pitchBends` and note `pitchBendRef` |

The resulting `UnifiedResult` is validated with `ResultValidator`.

## Truth Flags

Every conversion result reports:

- source artifact path;
- artifact type;
- note count;
- conversion success/failure;
- `possible_polyphony`;
- `pitch_bend_mapping_deferred` when pitch bends are present;
- `fixture_or_real_artifact` status;
- `productionTranscription=false`;
- `importedIntoTonyLayers=false`;
- `createdResultJson=false`;
- `marksBackendReadyInstalledOrCompleted=false`.

If `BasicPitchArtifactDiscoveryResult::ranBasicPitch` is false, the conversion is marked `fixture_or_synthetic_artifact`.

If `ranBasicPitch` is true, the conversion is marked `real_discovered_artifact_manual_only`. This still is not production transcription support and still must not be shown as a completed backend result.

## What CODEX-098 Proves

- A recognized Basic Pitch note-events CSV artifact can be selected from discovery output.
- The selected artifact is read from disk.
- The artifact is converted into an in-memory `UnifiedResult`.
- Note count, timing, MIDI pitch, velocity, and pitch bends are preserved by the converter.
- Possible polyphony and pitch-bend mapping warnings are preserved.
- Semantic validation runs through `ResultValidator`.
- Empty, missing, invalid, or unknown artifacts fail cleanly.
- No `result.json` is created.
- No Tony layer import occurs.
- No Basic Pitch Ready, Installed, or Completed state is created.

## What Remains Unproven After CODEX-102

- Basic Pitch is installed locally.
- Basic Pitch can run successfully on real user audio.
- A production Basic Pitch run writes a Tony-owned `result.json`.
- A production Basic Pitch result file is loaded without manual/test-only flags.
- MIDI and NPZ Basic Pitch artifacts are converted.
- Basic Pitch polyphony mapping into Tony layers is solved.
- Basic Pitch pitch bends are imported into real Tony layers.
- Basic Pitch results are visible through user-facing UI.
- Basic Pitch results are editable, saved, loaded, or exported in a production workflow rather than a manual/test-only proof harness.

## Follow-On Proofs

CODEX-099 adds the JSON handoff proof in `docs/engineering/BASIC_PITCH_UNIFIED_RESULT_JSON_HANDOFF_PROOF.md`. CODEX-100 adds the manual opt-in real-run wrapper in `docs/engineering/BASIC_PITCH_REAL_RUN_HANDOFF_PROOF.md`. CODEX-101 adds the result-to-real-Tony-note-layer proof in `docs/engineering/BASIC_PITCH_RESULT_TO_TONY_LAYER_PROOF.md`. CODEX-102 adds the Basic Pitch-shaped imported layer save/load/export proof in `docs/engineering/BASIC_PITCH_LAYER_SAVE_LOAD_EXPORT_PROOF.md`. None of these creates backend Ready/Installed/Completed state.
