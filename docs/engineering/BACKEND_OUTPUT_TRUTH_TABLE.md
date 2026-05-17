# Backend Output Truth Table

Status: mandatory engineering reference  
Applies before: backend adapters, TonyLayerImporter, UI state claims, save/load/export claims

## 1. Purpose

This table states what each planned backend is expected to produce, how it should appear in Tony, and what evidence proves the integration works.

Planning expectations are not implementation facts. Each backend must be re-verified against its actual local output before adapter implementation.

## 2. Truth Table

| Backend | Expected input | Expected output | Proper Tony display target | Editable? | Risks | What proves it works |
|---|---|---|---|---|---|---|
| Basic Pitch / NeuralNote path | Audio file, usually WAV or adapter-prepared audio | Notes, MIDI-like data, pitch bends/velocity/confidence where available, possible polyphony | Real result note layer only after polyphony policy; pitch-bend metadata/layer if mapped | Editable only after real Tony note layer proof | Polyphonic output may not fit monophonic Tony assumptions; pitch bends may be lost; confidence semantics may differ | Run real backend on real audio; collect real output; convert to UnifiedResult; validate; import into real Tony layer; verify overlap/polyphony handling; edit/save/export proof if claimed |
| CREPE Notes | Monophonic audio or prepared audio | f0 plus segmentation; notes only if segmentation is real | f0 curve layer for raw f0; note layer only after verified segmentation | f0 no; notes maybe after segmentation proof | Monophonic limitation; segmentation may be external/implicit; repeated same-pitch notes may be ambiguous | Verify local output; prove segmentation creates real note events; validate UnifiedResult; display f0 separately from notes |
| PESTO | Audio file or prepared audio | f0/voicing/confidence first | Pitch/f0 layer, voicing/confidence annotation or metadata | No note edit unless segmentation added | Not a note backend; f0 density/performance; voicing confidence interpretation | Real PESTO run; real f0 output; UnifiedResult pitch_curve validates; no note claim without segmentation proof |
| PENN | Audio file or prepared audio | f0/periodicity/voicing first | Pitch/f0 layer, periodicity/voicing metadata | No note edit unless segmentation added | Not a note backend; output semantics and units must be verified | Real PENN run; real f0 output; pitch_curve validation; documented periodicity mapping |
| FCPE | Audio file or prepared audio | f0/voicing/confidence first; any MIDI-like output must be treated cautiously | Pitch/f0 layer first; notes only after real segmentation or verified note output | No note edit unless note output is proven | May be mistaken for note transcription; f0-to-note conversion quality | Real FCPE run; real f0 output; pitch_curve display; no note completion claim without segmentation |
| MUSC Violin Transcription | Violin-focused audio, model/checkpoint where required | Notes plus pitch deviation/bend representation may be required | Violin result note layer plus pitch-deviation/bend representation if mapped | Notes maybe after Tony layer proof; bends maybe not | Research-code fragility; model/checkpoint availability; violin-specific assumptions | Real MUSC run; real output; UnifiedResult notes and deviations validate; display/edit proof for notes; separate proof for bends |
| VioPTT | Violin audio, model/checkpoint where required | Notes and technique labels where available | Note layer for notes; separate annotation/label target for technique labels | Notes maybe; labels not note-editable by default | Technique labels can be misrepresented as notes; label ontology may differ | Real VioPTT run; real technique label output; labels imported as annotations/metadata, not fake notes |
| pYIN Vamp | Audio loaded into Tony; existing Vamp plugin path | Existing Tony monophonic pitch and note analysis | Existing Tony pitch and note layers | Existing behavior only | Must not be broken; may not map cleanly to UnifiedResult initially | Existing Tony pYIN workflow still works; layer behavior unchanged; regression proof |

## 3. Backend-Specific Rules

### Basic Pitch

- Treat output as potentially polyphonic until verified otherwise.
- Do not flatten overlapping notes into one monophonic layer without an explicit design.
- Do not claim pitch bend support unless actual output is parsed and represented.

### CREPE Notes

- Treat as monophonic/f0-derived unless real note segmentation is verified.
- Do not call raw f0 output note transcription.

### PESTO / PENN / FCPE

- Treat as f0/voicing engines first.
- Notes require a real segmentation stage with its own tests and acceptance proof.

### MUSC Violin Transcription

- Preserve violin-specific pitch deviation/bend semantics when available.
- Do not discard deviations silently if they are central to output quality.

### VioPTT

- Technique labels must be separate annotations/labels or explicit note metadata.
- Do not invent technique labels from note names, velocity, or confidence.

### pYIN

- pYIN remains the Tony baseline.
- New backend work must not regress existing pYIN analysis or existing note/pitch layer behavior.

## 4. Verification Rule

Before implementing a backend adapter, create or update backend-specific integration notes with:

- command used;
- input audio used;
- files produced;
- sample output schema;
- fields mapped to UnifiedResult;
- fields intentionally not mapped;
- Tony target layer/model;
- risks and unsupported claims.
