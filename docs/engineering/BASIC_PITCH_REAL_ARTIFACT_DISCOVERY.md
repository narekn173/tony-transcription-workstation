# Basic Pitch Real Artifact Discovery

Status: CODEX-097 manual/test-only discovery harness, consumed by CODEX-098 artifact conversion proof and CODEX-099 JSON handoff proof.

This document defines an opt-in harness for discovering what a locally configured Basic Pitch command actually writes to disk. It does not make Basic Pitch a production backend, does not create `result.json`, does not import Tony layers, and does not mark Basic Pitch Ready, Installed, or Completed.

## Sources Inspected

- `docs/engineering/BASIC_PITCH_ADAPTER_AUDIT.md`
- `docs/engineering/BASIC_PITCH_OUTPUT_CONVERSION_PLAN.md`
- `main/backend/BasicPitchAdapterContract.*`
- `main/backend/BasicPitchOutputConverter.*`
- `main/backend/ExternalProcessRunner.*`
- `main/backend/BasicPitchArtifactToUnifiedResult.*`
- Spotify Basic Pitch CLI source: https://github.com/spotify/basic-pitch/blob/main/basic_pitch/predict.py
- Spotify Basic Pitch inference source: https://github.com/spotify/basic-pitch/blob/main/basic_pitch/inference.py

## Verified Command Shape

The verified Basic Pitch CLI command shape remains:

```text
basic-pitch <output-directory> <input-audio-path>
```

The discovery harness can request these verified CLI artifact flags through argument-list execution:

```text
--save-midi
--save-note-events
--save-model-outputs
--multiple-pitch-bends
```

The harness never builds a shell command string.

## Opt-In Environment Variables

Manual discovery is disabled unless explicitly opted in:

```text
TONY_BASIC_PITCH_DISCOVERY_ENABLE=1
TONY_BASIC_PITCH_COMMAND=<path or command>
TONY_BASIC_PITCH_TEST_AUDIO=<wav/mp3/audio path>
TONY_BASIC_PITCH_OUTPUT_DIR=<existing directory>
```

If opt-in is missing, discovery reports `explicit_opt_in_required` and does not run a process.

If a command, audio path, or output directory is missing, discovery reports a skipped/not-configured state or a clean validation error. Normal tests do not require Basic Pitch, Python, a model runtime, or test audio.

## Artifact Classification

The harness inspects an output directory and classifies files conservatively:

| Artifact | Recognition |
|---|---|
| MIDI | `.mid` or `.midi` |
| Note-events CSV | `.csv` with verified header `start_time_s,end_time_s,pitch_midi,velocity,pitch_bend` |
| Model output | `.npz` |
| Sonified MIDI/audio artifact | `.wav` |
| Logs/text | `.log` or `.txt` |
| Unknown | Any other file |

Classification proves file presence and shape only. It does not convert artifacts to `UnifiedResult` and does not import Tony layers.

CODEX-098 consumes only artifacts classified as `csv_note_events`. MIDI, NPZ/model-output, WAV, logs, and unknown files remain discovery-only until separate converter tasks prove them.

## Structured Result

The discovery result reports:

- whether Basic Pitch actually ran;
- skipped reason when it did not run;
- command and argument-list request;
- output directory;
- discovered artifacts and file sizes;
- stdout/stderr summaries;
- external process status when run;
- `productionTranscription=false`;
- `importedIntoTonyLayers=false`;
- `readyInstalledCompletedMutation=false`.

These flags exist to prevent fake success claims during discovery.

## Normal Test Behavior

Normal tests prove:

- request construction does not execute Basic Pitch;
- discovery skips without explicit opt-in;
- missing config and missing audio fail or skip cleanly;
- synthetic output-directory files can be classified without running Basic Pitch;
- no fake `result.json` is created;
- no Tony layer import occurs;
- no Ready, Installed, or Completed state is created.

Real Basic Pitch execution is allowed only in an explicit manual/test-only path using the environment variables above.

## What CODEX-097 Proves

- Tony can construct a Basic Pitch discovery process request using safe argument-list execution.
- The process is not run unless explicitly opted in.
- The output directory can be inspected after a run.
- Recognized Basic Pitch artifact types can be reported with file sizes.
- Discovery remains separate from conversion, `UnifiedResult` loading, Tony import, and backend status mutation.
- CODEX-098 adds a separate bridge that can convert discovered `csv_note_events` artifacts into in-memory `UnifiedResult` while preserving this no-runtime/no-UI boundary.

## What Remains Unproven

- Basic Pitch is installed locally.
- Basic Pitch can run successfully on the user's machine.
- Real Basic Pitch artifact filenames and edge cases for all supported input formats.
- Real Basic Pitch MIDI/NPZ conversion into `UnifiedResult`.
- Production `result.json` generation.
- Tony layer import from real Basic Pitch output.
- Pitch-bend, polyphony, velocity, and confidence mapping in real Tony layers.

## Recommended Next Task

Recommended next task: CODEX-100 - Basic Pitch manual real-run artifact JSON handoff verification.

That task should use the explicit opt-in environment variables with a locally configured Basic Pitch command and real test audio, then feed discovered artifacts through the CODEX-099 JSON handoff proof without importing Tony layers or creating fake backend state.
