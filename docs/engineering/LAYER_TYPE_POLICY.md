# Layer Type Policy

Status: mandatory engineering policy  
Applies before: TonyLayerImporter, display, selected-region replacement, compare mode, save/load/export

## 1. Purpose

This policy defines how backend output types should map to Tony/Sonic Visualiser concepts. It prevents treating every backend output as editable notes.

## 2. Output Type Mapping

| Output type | Meaning | Tony/Sonic Visualiser target | Editable? | Policy |
|---|---|---|---|---|
| Notes | Discrete note events with start/end/pitch | Real Tony note-capable layer/model after importer proof | Yes only if model/layer supports editing | May become editable note layer only after real layer proof. |
| Pitch/f0 curves | Continuous or framewise frequency estimates | Pitch/time-value style layer | Usually not note-editable | f0 is not automatically notes. |
| Pitch bends | Continuous pitch deviation linked to notes or time | Dedicated bend/deviation representation or annotation until mapped | Not assumed | Do not collapse into fixed-pitch notes. |
| Confidence/voicing | Reliability, voiced/unvoiced state | Overlay, color/metadata, or annotation after design proof | Not directly | Do not invent confidence; do not make confidence look like notes. |
| Technique labels | Playing technique classifications | Annotation/label layer or note metadata after design proof | Not note-editable by default | Technique labels are not notes. |
| Warnings/errors | Backend or validation issues | Status/log/annotation, not notes | No | Must remain visible and not be hidden by successful UI. |
| Annotations | Time-based non-note information | Tony annotation/text/region concept after source audit | Maybe | Must not masquerade as notes. |
| Selected-region replacement | Replacement of notes inside a user-selected time range | Real editable note layer plus command/undo path | Yes if replacing editable notes | Must require explicit user action and preserve outside-region notes. |
| Monophonic output | One melody/f0/note stream | Monophonic note or pitch layer | Maybe | Compatible with Tony baseline only after proof. |
| Polyphonic output | Overlapping notes/chords/multiple voices | Separate representation or constrained import strategy | Not assumed | Must not be blindly forced into one monophonic Tony note layer. |

## 3. Non-Negotiable Rules

- f0 is not automatically notes.
- Technique labels are not notes.
- Confidence and voicing are not notes.
- Pitch bends are not fixed note events.
- Basic Pitch polyphonic output must not be blindly forced into one monophonic Tony note layer.
- CREPE/PESTO/PENN/FCPE f0 output requires real segmentation before note claims.
- A custom drawing overlay is not a Tony layer unless it uses real Tony/Sonic Visualiser layer infrastructure.

## 4. Notes Policy

Note import requires:

- valid UnifiedResult note events;
- real target Tony model/layer identified by source audit;
- proof that start/end/pitch map correctly;
- proof of edit behavior before claiming editable;
- proof of save/export behavior before claiming persistence/export.

## 5. Pitch/F0 Policy

Pitch or f0 import requires:

- frequency/time/voicing semantics preserved;
- pitch layer or time-value layer target identified;
- no automatic note generation unless a segmentation component is explicitly implemented and tested;
- clear UI distinction between pitch curve and notes.

## 6. Technique Label Policy

Technique labels require:

- labels from real backend output;
- a separate annotation/label display or explicit note metadata design;
- no invented labels;
- export/save behavior documented separately from notes.

## 7. Selected-Region Replacement Policy

Selected-region replacement requires:

- source audit of existing Tony selection and note edit command paths;
- preview/result layer first;
- explicit user confirmation;
- only notes inside the selected region are replaced;
- outside-region notes remain unchanged;
- undo/redo behavior proven or documented as unavailable.

## 8. Polyphony Policy

Backends that may output overlapping notes require a design decision before import:

- reject polyphony with clear warning;
- split into multiple result layers/voices;
- preserve polyphony in a compatible model if Tony supports it;
- convert only after user-approved, documented constraints.

Do not silently drop or flatten overlapping notes.
