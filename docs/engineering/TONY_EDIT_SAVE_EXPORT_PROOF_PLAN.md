# Tony Edit, Save, and Export Proof Plan

Status: mandatory proof plan  
Applies before: TonyLayerImporter, result layer work, selected-region replacement, save/load/export claims

## 1. Purpose

This plan defines proof gates that must be passed before future tasks call a Tony backend feature complete.

## 2. Proof Gates

### Gate 1: Source Inspection Gate

Required evidence:

- exact Tony/Sonic Visualiser source files inspected;
- existing layer/model ownership path identified;
- existing edit, save/load, and export paths identified if relevant;
- risky files/classes listed.

Do not implement if the target layer/model path is unknown.

### Gate 2: Design Map Gate

Required evidence:

- UnifiedResult fields mapped to concrete Tony/Sonic Visualiser model fields;
- unsupported fields listed;
- monophonic/polyphonic handling documented;
- selected-region behavior documented if relevant;
- undo/redo implications documented.

### Gate 3: Compile-Only Gate

Required evidence:

- code compiles;
- no runtime behavior is changed;
- existing pYIN path is untouched;
- tests prove only model/boundary behavior, not user-facing success.

### Gate 4: Dev/Mock Proof Gate

Required evidence:

- dev/mock source is labeled debug/test-only;
- fixture is real JSON consumed through the same UnifiedResult loader/validator;
- mock result cannot be confused with real backend success;
- result reaches the intended layer path only if the task is layer proof.

### Gate 5: Real Backend Proof Gate

Required evidence:

- real backend executable/environment/configuration used;
- real audio input used;
- process run output captured;
- real `result.json` or adapter output produced;
- failures are visible and do not fabricate output.

### Gate 6: Real Tony/Sonic Visualiser Layer Proof Gate

Required evidence:

- imported data becomes a real Tony/Sonic Visualiser layer/model;
- layer is owned by existing Tony document/view infrastructure;
- layer is visible in Tony;
- no custom overlay is presented as a layer;
- unsupported output types remain separate or unavailable.

### Gate 7: Edit Proof Gate

Required evidence:

- user can edit imported notes using Tony mechanisms where editability is claimed;
- start/end/pitch edits persist in the model;
- undo/redo behavior is tested or explicitly documented as unsupported;
- non-note data is not called editable notes.

### Gate 8: Save/Load Proof Gate

Required evidence:

- session/project save includes the claimed layer/result state;
- reload restores the layer/result state;
- provenance is preserved where applicable;
- missing backend on reload is handled honestly.

### Gate 9: Export Proof Gate

Required evidence:

- export command/path is identified;
- exported MIDI/CSV/etc. contains the expected edited result;
- result source is correct: corrected layer vs result layer vs preview layer;
- unsupported exports are disabled or reported honestly.

## 3. Feature Complete Rule

A feature may be called complete only for the highest gate actually passed.

Examples:

- A parser that validates UnifiedResult is parser-complete, not Tony-import-complete.
- A process runner that creates output files is execution-complete, not transcription-complete.
- A layer importer that displays notes is display-complete, not edit/save/export-complete.
- A mock end-to-end proof is dev-proof-complete, not real-backend-complete.

## 4. Required Evidence in Final Reports

Future final reports must state:

- highest proof gate reached;
- commands/manual steps run;
- input/output files used;
- Tony layer/model type involved;
- whether edit/save/export were tested;
- limitations and unproven claims.

## 5. Stop Conditions

Stop before claiming completion if:

- the real Tony layer type is unknown;
- output is fake or fabricated;
- backend did not run when real backend proof is required;
- layer is not editable but task claims editable;
- save/load/export is untested but task claims persistence/export.
