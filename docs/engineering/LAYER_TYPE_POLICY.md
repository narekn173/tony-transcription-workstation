# Layer Type Policy

Status: mandatory engineering policy  
Applies before: TonyLayerImporter, display, selected-region replacement, compare mode, save/load/export

## 1. Purpose

This policy defines how backend output types may map to Tony/Sonic Visualiser concepts. It prevents treating every backend output as editable notes and prevents UI-only representations from masquerading as real Tony layers.

## 2. Source-Backed Tony Targets

The CODEX-087A and CODEX-087B audits establish these real layer/model targets:

- Notes: `NoteModel` with `NoteModel::NORMAL_NOTE` displayed by `NoteLayer`.
- Tony flexible notes: `NoteModel` with `NoteModel::FLEXI_NOTE` displayed by `FlexiNoteLayer`.
- Pitch/f0 curves: `SparseTimeValueModel` displayed by `TimeValueLayer`.
- Time/text/region annotation candidates: existing editable Sonic Visualiser text, region, or time-instant layers may be candidates, but technique-label mapping is not yet source-proven for backend import.

Future import code must use real model registration and layer insertion paths such as `ModelById`, `Document`, `LayerFactory`, and `Document::addLayerToView`. A custom drawing overlay or backend report is not a Tony layer.

## 3. Output Type Mapping

| Output type | Meaning | `UnifiedResult` field or concept | Tony/Sonic Visualiser target | Editable? | Policy |
|---|---|---|---|---|---|
| Notes | Discrete note events with real start, end, pitch, and optional confidence/velocity/label | `notes` | `NoteModel` + `NoteLayer`, or `NoteModel::FLEXI_NOTE` + `FlexiNoteLayer` after subtype decision | Yes only after real model/layer and edit proof | May become editable note layer only after real layer proof. Polyphonic notes require policy before import. |
| Pitch/f0 curves | Continuous or framewise frequency estimates | `pitchCurve` | `SparseTimeValueModel` + `TimeValueLayer` with `Hz` units | Editable as time-value points after proof, not note-editable | f0 is not automatically notes. |
| Pitch bends / deviations | Continuous pitch deviation linked to notes or time | `pitchBends`, note `pitchBendRef`, `centsDeviation` | Separate bend/deviation representation, auxiliary time-value layer, or explicit unsupported warning until designed | Not assumed | Do not collapse into fixed-pitch notes or silently discard. |
| Confidence | Reliability score for notes, f0, bends, or labels | `confidence` fields | Metadata, label, color/overlay, auxiliary time-value layer, or future diagnostic layer after design proof | Not directly | Preserve where possible; do not make confidence look like notes. |
| Voicing / periodicity | Voiced/unvoiced or periodicity estimate | `PitchPoint.voiced` or source metadata | Future diagnostic time-value/region/metadata strategy | Not directly | Preserve or expose where possible; do not invent note boundaries from voicing alone. |
| Technique labels | Playing technique classifications | `techniqueLabels` | Annotation/label strategy or explicit note metadata after design proof | Not note-editable by default | Technique labels are not notes. |
| Warnings/errors | Backend, validation, or mapping issues | `warnings`, `errors`, report issues | Status/log/diagnostic annotation, not notes | No | Must remain visible and not be hidden by successful UI. |
| Annotations | Time-based non-note information | Future label/annotation model | Existing text/region/time annotation concepts after source proof | Maybe | Must not masquerade as notes. |
| Selected-region replacement | Replacement of existing events inside a user-selected time range | Import operation, not raw backend output | Real editable model/layer plus command-backed mutation path | Yes only after undo/redo proof | Must require explicit user action and preserve outside-region data. |
| Monophonic output | One melody/f0/note stream | Backend-specific | Monophonic note or pitch layer after proof | Maybe | Compatible with Tony baseline only after proof. |
| Polyphonic output | Overlapping notes/chords/multiple voices | Backend-specific notes/channels | Multiple layers, a polyphony-preserving note model strategy, or rejection with warning | Not assumed | Must not be blindly forced into one monophonic Tony note layer. |

## 4. Backend-Specific Mapping Policy

| Backend | Primary mapping | Secondary mapping | Prohibited mapping |
|---|---|---|---|
| pYIN | Existing `SparseTimeValueModel` pitch and `NoteModel::FLEXI_NOTE` notes through current Tony flow | None for new backend importer until explicitly scoped | Any change that regresses existing pYIN behavior |
| Basic Pitch | Converted `UnifiedResult.notes` from real MIDI or note-events CSV may map to note layers only after polyphony policy | Pitch bends, velocity, confidence, and artifact warnings need preservation or warnings | Flattening polyphony, dropping pitch bends silently, or calling generated MIDI/CSV "imported" before `UnifiedResult` conversion and Tony layer proof |
| CREPE Notes | Raw CREPE f0 maps to `TimeValueLayer`; segmented notes may map to note layers only after segmentation proof | Confidence/voicing diagnostic strategy | Creating notes directly from f0 points without real segmentation |
| PESTO | f0 and confidence map to pitch/diagnostic layers | Future segmentation may create notes only as a separate component | Calling PESTO output note transcription |
| PENN | pitch plus periodicity map to pitch/diagnostic layers | Future segmentation may create notes only as a separate component | Treating periodicity as note confidence without documenting semantics |
| FCPE | f0 maps to pitch layer first | Quantized/MIDI-like output may become notes only after quantization proof | Treating non-neural f0 quantization as proven transcription by default |
| MUSC Violin Transcription | Notes may map to note layers after output proof | Pitch bends/deviations need separate preservation or warnings | Discarding expressive pitch deviation while claiming full transcription |
| VioPTT | Notes may map to note layers after output proof | Technique labels need annotation/metadata strategy | Storing technique labels as fake notes |

## 5. Non-Negotiable Rules

- f0 is not automatically notes.
- Technique labels are not notes.
- Confidence, voicing, and periodicity are not notes.
- Pitch bends are not fixed note events.
- Basic Pitch polyphonic output must not be blindly forced into one monophonic Tony note layer.
- Basic Pitch CLI artifacts are not a Tony result until converted into a validated `UnifiedResult`.
- CREPE/PESTO/PENN/FCPE f0 output requires real segmentation before note claims.
- A custom drawing overlay is not a Tony layer unless it uses real Tony/Sonic Visualiser layer infrastructure.
- A valid `UnifiedResult` is not a successful Tony import.
- A dev/mock backend result is debug/test-only and never production success.

## 6. Notes Policy

Note import requires:

- valid `UnifiedResult.notes`;
- deliberate choice between `NoteLayer` and `FlexiNoteLayer`;
- conversion from seconds to frames using the correct audio/model sample rate;
- preservation of pitch as MIDI or frequency according to documented units;
- handling of velocity/confidence/labels without inventing values;
- polyphony policy before importing overlapping notes;
- proof that start/end/pitch map correctly;
- proof of edit behavior before claiming editable;
- proof of save/load/export behavior before claiming persistence/export.

Initial population of a not-yet-visible model may use direct model adds. Mutating a visible/imported layer must use existing command paths such as `ChangeEventsCommand` and `CommandHistory`.

## 7. Pitch/F0 Policy

Pitch or f0 import requires:

- frequency/time/voicing/confidence semantics preserved where available;
- `SparseTimeValueModel` + `TimeValueLayer` target identified;
- `Hz` scale units for f0/frequency;
- no automatic note generation unless a segmentation component is explicitly implemented and tested;
- clear distinction between pitch curve, note layer, confidence/voicing diagnostics, and warnings.

## 8. Pitch Bend / Deviation Policy

Pitch bends and expressive deviations require:

- source-backed units such as cents, semitones, MIDI pitch bend values, or frequency deviation;
- note-linked or time-linked association strategy;
- preservation in `UnifiedResult.pitchBends` or explicit warning if not imported;
- no silent loss when the backend advertises pitch-bend or deviation output;
- separate save/load/export proof before claiming persistence.

## 9. Confidence, Voicing, And Periodicity Policy

Confidence, voicing, and periodicity are diagnostic data.

- Preserve them in `UnifiedResult` when source output provides them.
- Do not invent them when absent.
- Do not use them as note events.
- Do not hide low-confidence regions behind a generic success state.
- A future display may use an auxiliary time-value or annotation strategy only after proof.

## 10. Technique Label Policy

Technique labels require:

- labels from real backend output;
- source-backed ontology or label vocabulary;
- a separate annotation/label display or explicit note metadata design;
- no invented labels;
- no fake notes;
- save/load/export behavior documented separately from notes.

## 11. Selected-Region Replacement Policy

Selected-region replacement requires:

- source audit of existing Tony selection and note edit command paths;
- preview/result layer first unless a task explicitly scopes destructive replacement;
- explicit user confirmation;
- only notes/events inside the selected region are replaced;
- outside-region notes remain unchanged;
- frame conversion from backend seconds using the correct sample rate;
- undo/redo behavior proven or documented as unavailable.

## 12. Polyphony Policy

Backends that may output overlapping notes require a design decision before import:

- reject polyphony with a clear warning;
- split into multiple result layers or voices;
- preserve overlaps in a compatible note model if Tony edit/export behavior is proven;
- import only a constrained subset after user-approved, documented constraints.

Do not silently drop, merge, quantize, or flatten overlapping notes.

## 13. Save/Load/Export Policy

Before claiming persistence or export:

- the backend-created model must be owned by `Document`;
- the backend-created layer must be in a real pane/view;
- save must write the model/layer into session XML;
- load must reconstruct the correct model/layer type through existing readers;
- export must target the backend-created layer, not an unrelated pYIN layer;
- exported content must be inspected for timing, pitch/value, duration, labels, and any supported provenance.
