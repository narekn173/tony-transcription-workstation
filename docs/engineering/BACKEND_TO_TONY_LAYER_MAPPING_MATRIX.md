# Backend To Tony Layer Mapping Matrix

Status: mandatory engineering reference  
Applies before: TonyLayerImporter, backend adapter integration, UI state claims, selected-region replacement, save/load/export claims

## 1. Purpose

This matrix is the conservative bridge between backend outputs and real Tony/Sonic Visualiser layer targets. It is not an implementation plan. It states which mapping is allowed, which mapping is blocked, and what proof is required before a future task can claim a backend output is visible, editable, saved, loaded, or exported.

## 2. Mapping Legend

- Allowed now means source audits identify an existing Tony/Sonic Visualiser model/layer target, but code still must be implemented and proven in a later task.
- Deferred means the `UnifiedResult` model can represent the data, but no safe Tony layer mapping has been proven.
- Forbidden means the mapping would create fake notes, fake overlays, fake success states, or silent data loss.

## 3. Backend Matrix

| Backend | Output category | Allowed Tony target | Deferred target | Forbidden target | Proof required |
|---|---|---|---|---|---|
| pYIN | Monophonic pitch | Existing `SparseTimeValueModel` + `TimeValueLayer` path | None | New backend importer altering pYIN flow | Regression proof that pYIN pitch still loads, edits, saves, and exports as before |
| pYIN | Monophonic notes | Existing `NoteModel::FLEXI_NOTE` + `FlexiNoteLayer` path | None | Replacing pYIN notes without explicit task scope | Regression proof that pYIN notes still edit, save, load, and export as before |
| Basic Pitch | MIDI or note-events CSV notes, possibly polyphonic | `NoteModel` + `NoteLayer` or `FlexiNoteLayer` only after polyphony policy and converter proof | Multiple-layer/voice strategy if needed | Blind monophonic flattening or treating raw CLI artifacts as Tony import | Real Basic Pitch output, converter-created `UnifiedResult`, overlap policy, real layer import, edit/save/load/export proof. CODEX-096 proves fixture-only CSV conversion, not real backend output. |
| Basic Pitch | Pitch bends from MIDI/CSV/model-derived output | None proven yet | Auxiliary bend/deviation representation, note-linked metadata, or time-value strategy | Silent discard while claiming full support | CODEX-096 preserves fixture CSV pitch-bend values in `UnifiedResult.pitchBends`; real bend samples, unit mapping, and Tony layer/import proof remain required |
| Basic Pitch | Velocity/confidence/artifact warnings | Note metadata or provenance only after design | Diagnostic annotation/time-value strategy | Invented or discarded fields without warning | Field preservation proof or explicit warning |
| CREPE Notes | Raw f0/confidence | `SparseTimeValueModel` + `TimeValueLayer` for f0 | Confidence/voicing diagnostic strategy | Direct note layer from raw f0 | Real f0 output, f0 layer import proof |
| CREPE Notes | Segmented notes | `NoteModel` + `NoteLayer` or `FlexiNoteLayer` after segmentation proof | None | Notes without real segmentation | Segmentation sample, note event validation, real note layer proof |
| PESTO | f0/confidence | `SparseTimeValueModel` + `TimeValueLayer` for f0 | Confidence diagnostic strategy | Note layer without segmentation | Real PESTO output, f0 layer import proof |
| PENN | pitch/periodicity | `SparseTimeValueModel` + `TimeValueLayer` for pitch | Periodicity/voicing diagnostic strategy | Treating periodicity as notes | Real PENN pitch/periodicity files, documented semantics |
| FCPE | f0 | `SparseTimeValueModel` + `TimeValueLayer` for f0 | Confidence/voicing if output provides it | Note layer from f0 alone | Real FCPE output, f0 layer import proof |
| FCPE | Quantized/MIDI-like output | Possible `NoteModel` only after quantization policy | Segmentation/quantization report | Calling quantized f0 full transcription by default | Real output comparison, quantization limits, note layer proof |
| MUSC | Violin notes | `NoteModel` + `NoteLayer` or `FlexiNoteLayer` after output proof | None | Generic transcription claims without violin scope | Real MUSC output, note layer proof, violin scope documented |
| MUSC | Pitch deviations/bends | None proven yet | Bend/deviation representation or warning | Silent discard while claiming full MUSC support | Real deviation/bend samples, preservation or warning proof |
| VioPTT | Notes | `NoteModel` + `NoteLayer` or `FlexiNoteLayer` after implementation/output proof | None | Claiming integration from paper only | Real implementation selected, output schema inspected, note proof |
| VioPTT | Technique labels | None proven yet | Text/region/annotation strategy or explicit note metadata after proof | Fake notes for techniques | Real label ontology/output, annotation proof, save/load/export proof |

## 4. Cross-Backend Import Blocks

A future `TonyLayerImporter` task is blocked from claiming success until it proves:

- the backend output is real or explicitly dev/test-only;
- `UnifiedResult` contains the expected fields;
- the Tony target is a real `Document`-owned model/layer;
- the layer is added to a real pane/view through a source-backed path;
- editability is proven through existing model/layer edit commands;
- save/load/export are proven before those claims are visible to users;
- unsupported fields produce warnings rather than silent loss.

## 5. High-Risk Cases

- Basic Pitch overlapping notes: do not force one monophonic melody.
- Any f0 backend: do not create notes without segmentation.
- MUSC pitch deviations: do not discard expressive pitch detail silently.
- VioPTT technique labels: do not encode techniques as note events.
- Confidence/voicing/periodicity: preserve or warn; do not hide behind success.
- Dev/mock output: use only for debug/test proof and never production success.

## 6. Recommended Next Use

Use this matrix during CODEX-087D real-result acceptance rules verification and before CODEX-088/CODEX-089 implementation. Any future importer task should cite the exact row it is implementing and the proof gates it intends to satisfy.
