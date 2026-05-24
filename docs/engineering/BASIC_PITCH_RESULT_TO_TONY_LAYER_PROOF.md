# Basic Pitch Result To Tony Layer Proof

Status: CODEX-101 manual/test-only result-to-layer proof boundary, extended by CODEX-102 save/load/export proof.

This document defines the Basic Pitch-shaped `result.json` to real Tony/Sonic Visualiser note layer proof path:

```text
Basic Pitch-shaped result.json
-> BackendRunResultLoader / UnifiedResultFileLoader
-> loaded UnifiedResult
-> TonyLayerImporter
-> real NoteModel / NoteLayer
-> optional Document-owned layer
-> optional Pane/View insertion
```

This is not user-facing Basic Pitch UI integration and not production transcription support.

## Boundary

`BasicPitchResultToTonyLayerProof` accepts either:

- an explicit `result.json` path;
- an already loaded `BackendRunResultLoadResult`;
- a `BasicPitchUnifiedResultHandoffResult`;
- a `BasicPitchRealRunHandoffProofResult`.

It does not run Basic Pitch. It does not create Basic Pitch artifacts. It does not write `result.json`. It consumes only result files or handoff results produced by earlier proof boundaries.

## Proof Behavior

The boundary:

1. loads `result.json` through `BackendRunResultLoader`;
2. requires the loaded `UnifiedResult` to identify as `basic_pitch`;
3. preserves Basic Pitch warning codes;
4. reports `possible_polyphony` when present;
5. reports `pitch_bend_mapping_deferred` when present;
6. passes the loaded notes to `TonyLayerImporter`;
7. creates a real `NoteModel`;
8. creates a real Document-owned `NoteLayer` only when a real `Document` is supplied;
9. inserts into a real `Pane`/`View` only when one is supplied;
10. keeps `productionTranscription=false`.

The proof uses the real Tony/Sonic Visualiser APIs already proven by CODEX-089 through CODEX-094:

- `NoteModel`;
- `Document::createImportedLayer`;
- `LayerFactory`;
- `Document::addLayerToView`;
- `NoteLayer`;
- durable provenance identity through model/layer naming fields.

## Warnings Preserved

CODEX-101 must not silently ignore Basic Pitch risk fields. It reports:

- `possible_polyphony`;
- `pitch_bend_mapping_deferred`;
- `basic_pitch_possible_polyphony_not_resolved`;
- `basic_pitch_pitch_bend_tony_mapping_deferred`;
- `basic_pitch_result_to_tony_layer_test_only`.

Pitch bends remain in `UnifiedResult.pitchBends`, but no Tony pitch-bend layer is created in this task.

Possible polyphony remains a warning. CODEX-101 proves that Basic Pitch-shaped notes can enter a real Tony note layer; it does not solve a production polyphony policy.

## What CODEX-101 Proves

- A Basic Pitch-shaped `result.json` can be loaded through the existing result loader.
- The loaded `UnifiedResult` notes can be imported through `TonyLayerImporter`.
- A real `NoteModel` is created.
- A real Document-owned `NoteLayer` is created when a `Document` is supplied.
- A real `Pane`/`View` insertion is reported only when a real insertion happens.
- Note count, timing, duration, MIDI pitch, and velocity-derived level survive the path.
- Durable provenance identity is attached through existing Tony/SV fields.
- No fake overlay is introduced.
- No Basic Pitch Ready, Installed, or Completed state is created.

## What Remains Unproven

- Production Basic Pitch integration.
- User-facing Basic Pitch UI.
- Automatic runtime startup wiring.
- Production polyphony handling.
- Pitch-bend import into Tony layers.
- MIDI/NPZ conversion.
- Basic Pitch-specific save/load/export proof from a real manual run.
- Selected-region Basic Pitch replacement workflow.

CODEX-102 separately proves that the Basic Pitch-shaped imported `NoteLayer`
survives real Tony/Sonic Visualiser session XML save/load and lower-level CSV
export in a manual/test-only path. It still does not prove user-facing Basic
Pitch UI integration, production transcription, pitch-bend layer import, or a
production polyphony policy.

## Manual Real-Run Use

CODEX-100 can produce a `BasicPitchRealRunHandoffProofResult` when the manual environment is explicitly configured:

```text
TONY_BASIC_PITCH_DISCOVERY_ENABLE=1
TONY_BASIC_PITCH_COMMAND=<path or command>
TONY_BASIC_PITCH_TEST_AUDIO=<audio path>
TONY_BASIC_PITCH_OUTPUT_DIR=<existing output directory>
TONY_BASIC_PITCH_RESULT_JSON=<optional result json path>
```

CODEX-101 may consume that result in a test harness, but normal tests do not require Basic Pitch to be installed and do not run it by default.

## Recommended Next Task

Recommended next task after CODEX-102: CODEX-103 - first debug-only Basic Pitch workflow wiring plan or boundary, still gated by UI truth states and no production-ready claims.
