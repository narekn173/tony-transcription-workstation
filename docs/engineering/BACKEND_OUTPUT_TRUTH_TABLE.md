# Backend Output Truth Table

Status: mandatory engineering reference  
Applies before: backend adapters, TonyLayerImporter, UI state claims, save/load/export claims

## 1. Purpose

This table states what each planned backend is expected to produce, how that output may map into real Tony/Sonic Visualiser models and layers, and what evidence proves the integration works.

Planning expectations are not implementation facts. Each backend must be re-verified against the exact local backend version, command, manifest, and sample output before adapter implementation.

## 2. Verification Basis

This document is based on:

- Tony source audits in `docs/engineering/TONY_SOURCE_ARCHITECTURE_AUDIT.md` and `docs/engineering/TONY_LAYER_EDIT_SAVE_EXPORT_AUDIT.md`.
- Current backend skeleton types in `main/backend/BackendTypes.h`, `main/backend/UnifiedResult.h`, `main/backend/BackendRunResultReporter.h`, and `main/backend/TonyLayerImporter.*`.
- Existing pYIN/Tony behavior from local source inspection and Tony public documentation.
- External primary references where available:
  - Spotify Basic Pitch repository: https://github.com/spotify/basic-pitch
  - Basic Pitch adapter audit: `docs/engineering/BASIC_PITCH_ADAPTER_AUDIT.md`
  - CREPE repository: https://github.com/marl/crepe
  - CREPE Notes paper: https://arxiv.org/abs/2311.08884
  - PESTO repository: https://github.com/SonyCSLParis/pesto
  - PENN repository: https://github.com/interactiveaudiolab/penn
  - FCPE repository: https://github.com/CNChTu/FCPE
  - FCPE paper: https://arxiv.org/abs/2509.15140
  - MUSC violin transcription repository: https://github.com/MTG/violin-transcription
  - MUSC paper: https://archives.ismir.net/ismir2023/paper/000025.pdf
  - VioPTT paper: https://arxiv.org/abs/2509.23759

If a backend has only paper-level verification or no local adapter/sample output yet, this document marks its Tony mapping as provisional.

## 3. Current Project Boundary

Current code can represent backend results and has proof-only Tony note-layer import boundaries, but production backend UI integration remains deferred.

- `UnifiedResult` can represent notes, pitch curve points, pitch bends, technique labels, warnings, errors, files, and provenance.
- `BackendManifest` capabilities can express `outputsNotes`, `outputsPitchCurve`, `outputsPitchBends`, and `outputsTechniqueLabels`.
- `BackendRunResultReport` explicitly keeps `importedIntoTonyLayers = false` for loader/reporter-only boundaries.
- `TonyLayerImporter` has proof-only `NoteModel`/`NoteLayer` import paths from CODEX-089 through CODEX-094.
- CODEX-101 adds a Basic Pitch-shaped `result.json` to real Tony note-layer proof boundary.

Therefore no backend result can be described as user-facing "imported", "editable", "saved", "exported", "Ready", or "Completed" unless the specific task proves that state through real Tony/Sonic Visualiser models, layers, commands, and files.

## 4. Truth Table

| Backend | Verification status | Expected input | Expected output categories | Mono/poly policy | Proper Tony/Sonic Visualiser target | Editable / CommandHistory / save-load-export policy | Risks | What proves it works |
|---|---|---|---|---|---|---|---|---|
| pYIN Vamp, existing Tony baseline | Source-verified locally; Tony public docs describe monophonic pitch and note extraction. | Audio loaded into Tony through the existing main model path. | Monophonic pitch track and notes. | Monophonic baseline. Do not reinterpret as polyphonic. | Existing pitch: `SparseTimeValueModel` + `TimeValueLayer`. Existing notes: `NoteModel::FLEXI_NOTE` + `FlexiNoteLayer`. | Existing Tony behavior remains the baseline. Any new backend work must leave pYIN edit/save/export behavior unchanged. | Regressing `MainWindow`, `Analyser`, pYIN menu actions, selection re-analysis, or existing exports. | Existing Tony pYIN flow still creates real pitch and note layers; edit/save/load/export regression checks still pass. |
| Basic Pitch | Repository-verified and CODEX-095 audited. Basic Pitch is an audio-to-MIDI AMT system with multipitch/polyphonic support and pitch-bend-capable MIDI output. CLI shape is `basic-pitch <output-directory> <input-audio-path>`. | Compatible audio file accepted by the Basic Pitch Python package, including README-listed `mp3`, `ogg`, `wav`, `flac`, and `m4a`; internally down-mixed to mono and resampled. Selected-region input requires future adapter-prepared audio. | MIDI by default, optional note-events CSV, optional raw model-output NPZ, optional MIDI sonification WAV. Note-events CSV can include start/end, MIDI pitch, velocity, and pitch-bend values. Basic Pitch does not directly emit Tony `UnifiedResult` JSON. | Treat as potentially polyphonic. Never force overlapping notes into one monophonic Tony interpretation without an explicit policy. | Notes may map to `NoteModel` + `NoteLayer` only after polyphony policy. `NoteModel::FLEXI_NOTE` + `FlexiNoteLayer` only if flexi semantics are intentionally chosen. Pitch bends require a separate bend/deviation representation or explicit warning. | Editable only after real Tony note layer proof. Import must use `Document`/`LayerFactory`/`CommandHistory`. Save/load/export proof is required for any feature claim. | Flattened polyphony, lost pitch bends, lost confidence/velocity, ambiguous pitch-bend handling for overlapping notes, treating CSV/MIDI artifacts as a completed Tony import, false "completed" state from generated files alone. | Real Basic Pitch run on real audio; real output file; converter-created valid `UnifiedResult`; polyphony policy applied; notes imported into real Tony layer; bends/confidence preserved or explicitly warned; edit/save/load/export proof if claimed. |
| CREPE Notes | Paper-verified as post-processing CREPE f0 into monophonic note segmentation. CREPE itself is repository-verified as a monophonic pitch tracker that outputs time, frequency, and voicing confidence. | Monophonic audio or prepared audio accepted by CREPE/CREPE Notes flow. | Raw CREPE f0/time/confidence; notes only if a real segmentation stage produces discrete start/end/pitch events. | Monophonic. Repeated same-pitch notes depend on segmentation quality. | Raw f0 maps to `SparseTimeValueModel` + `TimeValueLayer`. Segmented notes may map to `NoteModel` + `NoteLayer` or `NoteModel::FLEXI_NOTE` + `FlexiNoteLayer` after explicit subtype choice. | f0 layer edit proof is separate from note edit proof. Note creation requires segmentation evidence. Save/load/export proof is required for both layer types if claimed. | Calling f0 output notes, hiding segmentation uncertainty, losing confidence, ambiguous repeated notes. | Real CREPE/CREPE Notes run; sample output proving segmentation; valid `UnifiedResult.pitchCurve` and/or `UnifiedResult.notes`; f0 and notes displayed as distinct real layers. |
| PESTO | Repository-verified and paper-verified as pitch estimation. Repository output format is time, frequency, confidence, with optional activations. | Mono audio for single-pitch estimation; repo notes stereo channels are treated as batch dimensions unless mixed. | f0/pitch curve, confidence, activations; no verified note transcription output. | Single-pitch/f0-first. Do not claim notes without a separate segmentation stage. | `SparseTimeValueModel` + `TimeValueLayer` for f0 in Hz. Confidence may need a future time-value/diagnostic representation. Activations are not currently mapped. | No note edit claim. Pitch curve edit/save/load/export proof required if imported. Confidence save/export proof required if exposed. | Mistaking fast pitch estimation for transcription, dropping confidence, accepting multi-channel batch output as polyphony. | Real PESTO run; real CSV/NPZ output; valid `UnifiedResult.pitchCurve`; no notes unless a tested segmentation component exists. |
| PENN | Repository-verified as pitch and periodicity estimation from audio files. | Audio file(s), optional checkpoint/config, hop/frequency bounds. | Pitch/f0 plus periodicity/voicing-like values. | Single-pitch/f0-first unless a future backend version proves otherwise. | Pitch maps to `SparseTimeValueModel` + `TimeValueLayer`. Periodicity/voicing needs a diagnostic time-value or metadata strategy. | No note edit claim without segmentation. Pitch/periodicity preservation and save/export proof are separate. | Treating periodicity as confidence without documenting semantics, losing unvoiced regions, assuming note segmentation. | Real PENN run; real pitch and periodicity files; validated `UnifiedResult.pitchCurve`; documented periodicity mapping. |
| FCPE | Repository-verified and paper-verified as fast context-based pitch estimation for monophonic audio. Repository says MIDI extraction is quantized from f0 using non-neural methods. | Monophonic audio or prepared audio accepted by TorchFCPE. | f0/pitch extraction; possible MIDI-like output from quantized f0, not primary neural note transcription. | Monophonic f0-first. MIDI-like output is not accepted as final notes until the quantization/segmentation rules are tested. | f0 maps to `SparseTimeValueModel` + `TimeValueLayer`. Quantized notes may map to `NoteModel` only after segmentation acceptance proof. | No note edit/save/export claim from f0 alone. Any MIDI-like output must be validated as real segmented notes. | Mistaking quantized f0 for robust transcription, losing voicing/confidence if available, noisy f0 causing false note boundaries. | Real FCPE run; real f0 output; if MIDI-like output is used, compare event timing/pitch and document quantization limits before note layer import. |
| MUSC Violin Transcription | Repository-verified and paper-verified. MUSC is a high-resolution solo violin transcriber producing MIDI and modeling fine pitch deviations/pitch bends. | Solo violin audio, typically 44.1 kHz raw audio per project description; model/checkpoint dependencies as required. | Notes/MIDI plus high-resolution pitch deviation or pitch-bend representation. | Solo violin focused. Treat as monophonic or near-monophonic only after local output inspection; do not assume generic polyphonic support. | Notes may map to `NoteModel`/`NoteLayer` or `FlexiNoteLayer` after subtype choice. Pitch deviations/bends require a separate `pitchBends` strategy or warning. | Editable note proof required. Pitch bend/deviation edit/export proof remains separate and likely deferred. | Dropping expressive intonation, silently losing pitch bends, violin-specific model fragility, checkpoint availability, uncertain export semantics. | Real MUSC run; real MIDI/output; valid notes and deviations in `UnifiedResult`; real note layer proof; explicit bend preservation or warning. |
| VioPTT | Paper-level verification only in this audit. The paper describes violin transcription with playing technique prediction in addition to pitch onset and offset. No local code/sample output was verified. | Violin audio and model/checkpoint once a real implementation is selected. | Pitch onset/offset and playing technique labels; possibly note-level technique classes. | Violin-focused. Mono/poly behavior must be verified from implementation and sample output. | Notes may map to a real note layer only after output format proof. Technique labels must map to a real annotation/label strategy, not notes. | Technique labels are not note-editable. Annotation edit/save/export proof is separate from note proof. | Inventing labels, encoding technique labels as fake notes, unknown ontology, no local implementation verified. | Real VioPTT implementation selected; real output schema inspected; technique labels mapped to annotations/metadata; notes and labels proven separately. |

## 5. Backend-Specific Rules

### pYIN

- Preserve existing pYIN behavior unless a task explicitly scopes a guarded change.
- pYIN remains the only source-backed Tony baseline for real editable monophonic pitch and note layers today.

### Basic Pitch

- Treat output as potentially polyphonic until local sample output proves a stricter case.
- Basic Pitch CLI artifacts are not `UnifiedResult` until a real converter creates and validates `result.json`.
- Do not flatten overlapping notes into one monophonic melody without explicit user-facing policy and warnings.
- Do not discard pitch bends, channel/voice hints, confidence, or velocity silently.
- If pitch bends cannot yet be represented, the import/report must say so.

### CREPE Notes

- CREPE f0 output is pitch curve data, not notes.
- Notes require the CREPE Notes segmentation stage or another real segmentation stage.
- Confidence/voicing should be preserved in `UnifiedResult.pitchCurve` where possible.

### PESTO / PENN / FCPE

- Treat these as f0/voicing/periodicity engines first.
- Notes require a real segmentation or quantization component with its own acceptance tests.
- f0-to-note conversion must never be hidden inside `TonyLayerImporter` without a documented segmentation stage.

### MUSC Violin Transcription

- Preserve violin-specific pitch deviation or pitch-bend semantics where available.
- If the first importer only handles notes, it must warn that bend/deviation output is not yet imported.
- Do not claim full violin transcription quality from notes alone if the backend output includes expressive pitch detail.

### VioPTT

- Technique labels must be separate annotations/labels or explicit note metadata after design proof.
- Do not invent technique labels from note names, velocity, confidence, or backend status.
- Do not claim VioPTT integration until a real implementation and output schema are verified.

## 6. Mandatory Policy Conclusions

- f0/pitch output is not automatically final notes.
- Note segmentation must be real before creating note layers from f0 backends.
- Technique labels must not be stored as fake notes.
- Basic Pitch polyphonic output must not be blindly forced into one monophonic Tony note layer.
- Pitch bends and pitch deviations must not be silently discarded without warning.
- Confidence, voicing, and periodicity should be preserved or exposed where possible.
- Dev/mock output remains debug/test-only and never production success.
- A valid `UnifiedResult` is not an imported Tony layer.
- A visible custom overlay is not a Tony/Sonic Visualiser layer.

## 7. Verification Rule

Before implementing a backend adapter, create or update backend-specific integration notes with:

- backend version and source;
- command used;
- input audio used;
- files produced;
- sample output schema;
- fields mapped to `UnifiedResult`;
- fields intentionally not mapped;
- Tony target model/layer;
- risks and unsupported claims;
- proof status for real audio, real backend, real result, `UnifiedResult`, real layer import, edit, save/load, and export.
