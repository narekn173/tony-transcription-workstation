# Basic Pitch Real-Run Handoff Proof

Status: CODEX-100 manual opt-in proof harness, consumed by CODEX-101 result-to-Tony-layer proof.

This document defines the manual-only Basic Pitch real-run-to-`result.json` proof path:

```text
real local audio
-> real Basic Pitch process
-> real Basic Pitch output artifacts
-> discovered csv_note_events artifact
-> UnifiedResult
-> real result.json
-> BackendRunOutputHandoff
-> BackendRunResultLoader / UnifiedResultFileLoader
-> BackendRunResultReporter
```

This remains a test/manual proof boundary. It is not runtime UI integration, not Tony layer import, and not production Basic Pitch support.

## Manual Opt-In Environment

The proof must not run unless explicitly enabled:

```text
TONY_BASIC_PITCH_DISCOVERY_ENABLE=1
TONY_BASIC_PITCH_COMMAND=<path or command>
TONY_BASIC_PITCH_TEST_AUDIO=<audio path>
TONY_BASIC_PITCH_OUTPUT_DIR=<existing output directory>
TONY_BASIC_PITCH_RESULT_JSON=<optional result json path>
```

If `TONY_BASIC_PITCH_RESULT_JSON` is omitted, the proof uses:

```text
<TONY_BASIC_PITCH_OUTPUT_DIR>/basic_pitch_result.json
```

The output directory must already exist. The harness does not create directories automatically.

## How To Run Manually

From a configured shell where Basic Pitch is installed and a real local audio file is available:

```text
set TONY_BASIC_PITCH_DISCOVERY_ENABLE=1
set TONY_BASIC_PITCH_COMMAND=basic-pitch
set TONY_BASIC_PITCH_TEST_AUDIO=C:\path\to\audio.wav
set TONY_BASIC_PITCH_OUTPUT_DIR=C:\path\to\basic-pitch-output
set TONY_BASIC_PITCH_RESULT_JSON=C:\path\to\basic-pitch-output\result.json
```

Then run the backend test target normally. Without those variables, the proof reports a skipped state and normal tests do not require Basic Pitch, Python, model files, or audio fixtures.

## Harness Behavior

`BasicPitchRealRunHandoffProof`:

1. reads explicit configuration from `BasicPitchArtifactDiscovery::configFromEnvironment`;
2. refuses to run unless explicit opt-in is enabled;
3. uses `BasicPitchArtifactDiscovery::runDiscovery` to build and execute an argument-list `ExternalProcessRequest`;
4. inspects the configured output directory for produced artifacts;
5. selects a recognized `csv_note_events` artifact;
6. delegates conversion and JSON handoff to `BasicPitchUnifiedResultHandoff`;
7. reports the loaded `BackendRunResultReport`.

The harness never invokes `TonyLayerImporter`, never writes UI state, and never mutates backend manifest/registry availability.

CODEX-101 consumes the loaded handoff result through `BasicPitchResultToTonyLayerProof`. That boundary still does not run Basic Pitch itself and still remains manual/test-only.

## Expected Artifacts

The Basic Pitch discovery boundary recognizes:

| Artifact | Use in CODEX-100 |
|---|---|
| `csv_note_events` | Selected for conversion and JSON handoff |
| MIDI | Discovered only; conversion deferred |
| NPZ/model output | Discovered only; conversion deferred |
| Sonified WAV | Discovered only |
| Logs/text | Discovered only |
| Unknown files | Reported but ignored |

If no `csv_note_events` artifact is found, the handoff fails cleanly. It must not fabricate a replacement CSV or `result.json`.

## What CODEX-100 Proves

When the manual environment is configured and Basic Pitch succeeds locally, the harness can prove:

- Basic Pitch ran as a real child process through `ExternalProcessRunner`;
- real artifacts were produced on disk;
- a recognized note-events CSV artifact was selected;
- the artifact converted to `UnifiedResult`;
- a real non-empty `result.json` was written;
- the result file loaded through `BackendRunResultLoader`;
- `BackendRunResultReporter` reported the loaded result;
- note count and warnings were preserved through the existing CODEX-099 handoff path.

## Normal Test Proof

Normal tests prove:

- missing opt-in skips without running Basic Pitch;
- missing command reports `not_configured`;
- missing audio fails before any process run;
- missing output directory fails before any process run;
- the discovery request uses argument-list execution;
- no Tony layer import occurs;
- no Basic Pitch Ready, Installed, or Completed state is created.

## What Remains Unproven

- Basic Pitch availability on a user's machine unless the manual opt-in proof is actually run.
- Production-quality transcription.
- MIDI/NPZ conversion.
- Tony layer import from Basic Pitch.
- Polyphony layer policy beyond warnings.
- Pitch-bend import into real Tony layers.
- Basic Pitch result edit/save/load/export inside Tony.

## Recommended Next Task

Recommended next task: CODEX-102 - Basic Pitch result layer save/load/export proof, still gated behind explicit manual proof data and the no-fake-state rules.
