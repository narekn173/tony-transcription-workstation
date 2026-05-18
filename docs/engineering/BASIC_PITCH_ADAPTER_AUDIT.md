# Basic Pitch Adapter Audit

Status: CODEX-095 audit, contract, and compile-only boundary; updated through CODEX-098 artifact-to-UnifiedResult proof.

This document defines the first conservative Basic Pitch adapter contract for Tony. It does not claim Basic Pitch runs inside Tony, does not import Basic Pitch results into Tony layers, and does not mark Basic Pitch Ready, Installed, or Completed.

## Sources Inspected

Local project files:

- `AGENTS.md`
- `docs/00_PROJECT_INDEX.md`
- `docs/engineering/REAL_RESULT_ACCEPTANCE_CHECKLIST.md`
- `docs/engineering/REAL_RESULT_AND_TONY_LAYER_INTEGRATION_RULES.md`
- `docs/engineering/LAYER_TYPE_POLICY.md`
- `docs/engineering/BACKEND_OUTPUT_TRUTH_TABLE.md`
- `docs/engineering/BACKEND_TO_TONY_LAYER_MAPPING_MATRIX.md`
- `docs/engineering/TONY_SOURCE_ARCHITECTURE_AUDIT.md`
- `docs/engineering/TONY_LAYER_EDIT_SAVE_EXPORT_AUDIT.md`
- `docs/engineering/PROVENANCE_METADATA_POLICY.md`
- `docs/engineering/STRUCTURED_PROVENANCE_PERSISTENCE_STRATEGY.md`
- `docs/examples/backend_manifest_basic_pitch.example.json`
- `main/backend/BackendTypes.*`
- `main/backend/BackendManifestParser.*`
- `main/backend/BackendSettingsStore.*`
- `main/backend/BackendRunRequestBuilder.*`
- `main/backend/BackendRunRequestPreparer.*`
- `main/backend/BackendRunOrchestrator.*`
- `main/backend/ExternalProcessRunner.*`
- `main/backend/UnifiedResult.*`
- `main/backend/UnifiedResultFileLoader.*`
- `main/backend/ResultValidator.*`
- `main/backend/BackendRunResultReporter.*`
- `main/backend/TonyLayerImporter.*`
- `main/backend/test/TestBackendTypes.h`

External primary sources:

- Spotify Basic Pitch repository: https://github.com/spotify/basic-pitch
- Basic Pitch README: https://github.com/spotify/basic-pitch/blob/main/README.md
- Basic Pitch CLI source: https://github.com/spotify/basic-pitch/blob/main/basic_pitch/predict.py
- Basic Pitch inference source: https://github.com/spotify/basic-pitch/blob/main/basic_pitch/inference.py
- Basic Pitch note creation source: https://github.com/spotify/basic-pitch/blob/main/basic_pitch/note_creation.py
- Basic Pitch package model path source: https://github.com/spotify/basic-pitch/blob/main/basic_pitch/__init__.py

## Verified Basic Pitch Facts

- Basic Pitch is a Python library and command-line tool for automatic music transcription.
- It generates MIDI from compatible audio and supports pitch bends.
- It is instrument-agnostic and supports polyphonic instruments, while the README says it works best on one instrument at a time.
- The Python package is installed with `pip install basic-pitch`.
- The command-line entry point is `basic-pitch`.
- The basic CLI shape is:

```text
basic-pitch <output-directory> <input-audio-path>
```

- The CLI also accepts multiple input audio paths after the output directory.
- The CLI supports optional output flags:
  - `--save-midi`, default true in the current CLI source.
  - `--save-note-events`, for note events CSV.
  - `--save-model-outputs`, for raw model output NPZ.
  - `--sonify-midi`, for a rendered WAV of the MIDI output.
- The CLI supports model/runtime options:
  - `--model-path`
  - `--model-serialization` with `tf`, `coreml`, `tflite`, or `onnx`.
- The CLI supports transcription threshold controls and `--multiple-pitch-bends`.
- The CLI writes into a caller-provided output directory; CODEX-097 treats exact filenames as discovered artifacts rather than hard-coding production assumptions.
- The Python package includes the ICASSP 2022 model in TensorFlow, CoreML, TensorFlowLite, and ONNX serialized forms.
- The default model path is chosen from the first available runtime in the package priority.
- README-documented audio inputs include `.mp3`, `.ogg`, `.wav`, `.flac`, and `.m4a`, subject to the installed `librosa` support.
- Prediction down-mixes to mono and resamples to 22050 Hz internally.
- Programmatic `predict()` returns model output, MIDI data, and note events.
- Saved note events CSV uses the verified header `start_time_s,end_time_s,pitch_midi,velocity,pitch_bend`.
- Note-events rows contain start time, end time, MIDI pitch, velocity, and zero or more appended pitch-bend values when bends are present.
- Note creation source shows note events may include pitch-bend lists and that overlapping notes with pitch bends are special: without `multiple_pitch_bends`, overlapping pitch bends may be dropped; with it, each pitch may be mapped to its own MIDI instrument.

## Unverified Or Deferred Assumptions

- Tony has not run Basic Pitch in this task.
- No local Basic Pitch executable, Python environment, package import, model runtime, or model file was probed in this task.
- Exact artifact filenames are not relied on by the C++ boundary yet. Basic Pitch output is treated as an output-directory contract plus artifact categories.
- Basic Pitch does not directly emit Tony `UnifiedResult` JSON. A future converter must parse Basic Pitch MIDI, note-events CSV, model-output NPZ, or a dedicated adapter output into `UnifiedResult`.
- Selected-region support is not direct CLI support. A future adapter may create a clipped audio file for the selected region, but that is not implemented here.
- Structured provenance persistence remains deferred per `STRUCTURED_PROVENANCE_PERSISTENCE_STRATEGY.md`.

## Backend ID Policy

Spotify's package and CLI use the command name `basic-pitch`.

Tony backend IDs currently require lowercase snake_case through `BackendManifest::isValidBackendId()`. The compile-only contract therefore uses:

```text
backendId: basic_pitch
cli command: basic-pitch
```

This avoids weakening the shared backend ID validation rule just for one product name.

## Manifest Contract Boundary

`BasicPitchAdapterContract::manifest()` returns a conservative manifest:

- `backendId`: `basic_pitch`
- `displayName`: `Basic Pitch`
- `runtimeType`: `PythonCli`
- `executablePath`: `basic-pitch` as a placeholder command name only
- `status`: `NotConfigured`
- `supportsFullFile`: true
- `supportsSelectedRegion`: false until an adapter proves clipping or region handling
- `outputsNotes`: true
- `outputsPitchBends`: true
- `outputsPitchCurve`: false for this contract because raw model output is not yet mapped as a Tony f0 curve
- `outputsTechniqueLabels`: false
- `requiresPython`: true
- `requiresModelCheckpoint`: false for default package models
- supported inputs: `mp3`, `ogg`, `wav`, `flac`, `m4a`
- primary outputs: `midi`, `note_events_csv`
- optional outputs: `model_output_npz`, `pitch_bends`, `warnings`

The existing shared `BackendCapability` model has no `possiblePolyphony` field. CODEX-095 therefore records Basic Pitch-specific polyphony policy through contract defaults and validation warnings rather than pretending the shared model can express it directly.

## Request Contract Boundary

`BasicPitchAdapterContract::buildCliRequest()` builds an `ExternalProcessRequest` but never runs it.

The built argument list is intentionally a direct Basic Pitch CLI concept:

```text
<output-directory> <input-audio-path> --save-note-events --save-model-outputs --multiple-pitch-bends
```

Optional model fields add:

```text
--model-path <model-path>
--model-serialization <tf|coreml|tflite|onnx>
```

The boundary validates only request shape:

- executable path is non-empty
- input audio path is non-empty
- output directory path is non-empty
- expected future `UnifiedResult` JSON path is non-empty
- timeout is non-negative
- model serialization is one of the verified values if supplied

It does not:

- execute Basic Pitch
- create output directories
- create request files
- create `result.json`
- parse MIDI/CSV/NPZ
- import Tony layers
- mark backend availability

## Mapping To UnifiedResult

Future conversion should map Basic Pitch artifacts conservatively:

| Basic Pitch output | UnifiedResult target | Status |
|---|---|---|
| MIDI note events | `UnifiedResult.notes` | Deferred until parser/converter proof |
| Note-events CSV `start_time_s`, `end_time_s`, `pitch_midi`, `velocity` | `UnifiedResult.notes` | CODEX-096 fixture-backed converter boundary |
| Note-events CSV appended pitch-bend values | `UnifiedResult.pitchBends` plus note-linked bend references | CODEX-096 preserves data but Tony layer mapping remains deferred |
| Raw model NPZ | Diagnostic/model artifact reference, not direct Tony notes | Deferred |
| Warnings/process logs | `UnifiedResult.warnings` or backend run report warnings | Deferred |

No conversion may silently drop pitch bends, velocity/confidence-like fields, warnings, or polyphony while claiming complete transcription support.

## Mapping To Tony Layers

Basic Pitch notes may eventually map to real Tony/Sonic Visualiser note layers only after applying the layer mapping matrix:

- Potential note target: `NoteModel` + `NoteLayer`.
- Potential flexi target: `NoteModel::FLEXI_NOTE` + `FlexiNoteLayer` only if flexi-note semantics are explicitly chosen.
- Potential polyphony handling: preserve overlaps in a proven note layer strategy, split voices/layers, or reject with warning.
- Pitch bends: no proven Tony target yet. They require a future bend/deviation strategy or explicit unsupported warning.
- Velocity/confidence: preserve in `UnifiedResult`, layer identity/provenance, labels, or future diagnostics only after proof.

Basic Pitch polyphonic output must not be blindly forced into one monophonic Tony note layer.

## What Must Be Proven Before Claiming Basic Pitch Works

Before any user-facing Basic Pitch support claim:

1. Basic Pitch executable/Python/package/model availability is probed and reported without fake Ready/Installed states.
2. A real audio file is processed by a real Basic Pitch process.
3. Real Basic Pitch artifacts are produced on disk.
4. A converter creates a real `UnifiedResult` JSON from those artifacts.
5. `UnifiedResultFileLoader` loads and validates it.
6. Polyphony, pitch bends, velocity/confidence, and warnings are preserved or reported as unsupported.
7. Notes are imported into real Tony/Sonic Visualiser model/layer structures.
8. Real Pane/View insertion is proven if the feature is visible.
9. Edit/undo/redo, save/load, export, and provenance proof gates pass for any claims made.

## What Must Not Be Claimed Yet

- Basic Pitch is installed.
- Basic Pitch is Ready.
- Basic Pitch analysis Completed.
- Basic Pitch produces Tony `UnifiedResult` JSON directly.
- Basic Pitch result import is production-ready.
- Basic Pitch polyphony has a solved Tony mapping.
- Basic Pitch pitch bends are preserved in Tony layers.
- Basic Pitch output is editable, saved, loaded, or exported in Tony.

## CODEX-095 Proof

CODEX-095 proves only:

- Basic Pitch official CLI and artifact facts were audited.
- A conservative compile-only manifest/request contract can be represented.
- The contract builds an argument-list `ExternalProcessRequest` without shell strings.
- The contract reports polyphony and pitch-bend mapping risks.
- The contract never executes a process, creates fake `result.json`, imports Tony layers, or marks Ready/Installed/Completed.

CODEX-096 adds only a fixture-backed note-events CSV parser/converter boundary. It still does not run Basic Pitch, create production `result.json`, import Tony layers, or mark Ready/Installed/Completed.

CODEX-097 adds only a manual/test-only artifact discovery harness. It can build a safe Basic Pitch process request, skip unless explicitly opted in, and classify output-directory artifacts. It still does not convert real artifacts into production `UnifiedResult`, create `result.json`, import Tony layers, or mark Ready/Installed/Completed.

CODEX-098 adds only a manual/test-only bridge from discovered note-events CSV artifacts to in-memory `UnifiedResult`. It still does not create `result.json`, import Tony layers, or mark Ready/Installed/Completed.

## Recommended Next Task

Recommended next task: CODEX-099 - Basic Pitch UnifiedResult JSON handoff proof.

That task should write a `UnifiedResult` produced by CODEX-098 to an explicitly marked test/manual result file, then load it through `UnifiedResultFileLoader` without importing into Tony layers yet.
