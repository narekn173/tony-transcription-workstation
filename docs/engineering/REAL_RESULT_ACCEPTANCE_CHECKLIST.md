# Real Result Acceptance Checklist

Status: mandatory final acceptance checklist  
Applies before: backend integration, UI integration, TonyLayerImporter, layer import, selected-region replacement, save/load, export, and feature-complete claims

## Purpose

Use this checklist before any future task claims a backend, UI, layer, edit, save/load, or export feature is complete. A task may stop at an earlier proof gate, but its final report must say which checklist items passed and which remain unproven.

## Completion Checklist

### Real Backend And Process Proof

- [ ] The backend execution path is real, explicit, and reproducible.
- [ ] The executable/script path, arguments, working directory, timeout, environment, stdout, stderr, exit code, and cancellation behavior are captured where applicable.
- [ ] Missing executable, missing model, failed start, non-zero exit, timeout, and cancellation are reported distinctly.
- [ ] The backend is not marked Ready, Installed, Running, Completed, or CompletedWithWarnings unless the corresponding real proof exists.

### Real `result.json` Proof

- [ ] The result file is created by the backend or by an explicitly marked dev/test-only fixture path.
- [ ] The result file path is non-empty, exists, is not a directory, is readable, and is non-empty where required.
- [ ] No fake `result.json` is fabricated to force a success path.
- [ ] Missing, empty, invalid, or unreadable result files remain failure states.

### `UnifiedResult` Validation Proof

- [ ] The result file is parsed through the project `UnifiedResult` loader/parser path.
- [ ] Schema-aware and semantic validation pass where applicable.
- [ ] Note times, MIDI pitch, frequency, confidence, pitch bends, technique labels, warnings, and errors are validated or explicitly reported unsupported.
- [ ] A loaded `UnifiedResult` is not treated as imported into Tony layers.

### Layer-Type Mapping Proof

- [ ] The backend output type is checked against `LAYER_TYPE_POLICY.md`, `BACKEND_OUTPUT_TRUTH_TABLE.md`, and `BACKEND_TO_TONY_LAYER_MAPPING_MATRIX.md`.
- [ ] f0/pitch output is not treated as final notes without real segmentation.
- [ ] Technique labels are not stored as fake notes.
- [ ] Pitch bends/deviations, confidence, voicing, and periodicity are preserved or reported as not yet imported.
- [ ] Polyphonic output, especially Basic Pitch output, is not blindly forced into one monophonic Tony note layer.

### Real Tony/Sonic Visualiser Layer Creation Proof

- [ ] Imported data becomes a real Tony/Sonic Visualiser model and layer, such as `NoteModel` + `NoteLayer`, `NoteModel::FLEXI_NOTE` + `FlexiNoteLayer`, or `SparseTimeValueModel` + `TimeValueLayer`.
- [ ] The model is registered through source-proven Tony/Sonic Visualiser ownership paths.
- [ ] The layer is created through source-proven `Document`/`LayerFactory` paths.
- [ ] The layer is inserted into a real pane/view through a source-proven path such as `Document::addLayerToView`.
- [ ] No custom overlay, drawing, log view, or report is described as a Tony layer.

### Editability Proof

- [ ] Editability is claimed only for real editable Tony/Sonic Visualiser layer types.
- [ ] At least one representative edit is performed and verified against the underlying model.
- [ ] Note time, duration, pitch/value, deletion, movement, or resize are verified where applicable.
- [ ] Pitch/time-value edits are verified separately from note edits.
- [ ] Technique label or diagnostic data is not called note-editable unless a real editable representation is proven.

### CommandHistory / Undo-Redo Proof

- [ ] User-visible imports or edits use existing Tony/Sonic Visualiser command paths where undo/redo is claimed.
- [ ] Undo removes/restores imported layer insertion where claimed.
- [ ] Undo/redo restores edited event content where claimed.
- [ ] Selected-region replacement uses a compound command path where claimed.
- [ ] Direct visible-model mutation is not used for undoable user-facing changes.

### Save/Load Proof

- [ ] The imported model/layer is saved through the real Tony session save path.
- [ ] The saved session contains the expected model/layer data.
- [ ] Reload reconstructs the correct model and layer class.
- [ ] The layer reappears in a real pane/view after reload.
- [ ] Event counts and representative values match after reload.
- [ ] Editability after reload is verified where claimed.

### Export Proof

- [ ] The exported layer is the backend-created or corrected backend layer, not an unrelated pYIN layer.
- [ ] The export file is created by a real export path.
- [ ] The export file contents are inspected for expected timing, pitch/value, duration, labels, and supported metadata.
- [ ] MIDI, CSV/TSV, SVL/XML, RDF, or other export claims are proven separately.
- [ ] Unsupported output types, such as pitch bends or technique labels, are not claimed exported unless their export representation is proven.

### Provenance Metadata Proof

- [ ] Backend name and version, when available, are recorded or explicitly reported unavailable.
- [ ] Input audio path, selected region, backend settings, model/checkpoint path, result file path, confidence/warnings, run timestamp, and user-edit status are handled according to `PROVENANCE_METADATA_POLICY.md`.
- [ ] Provenance survives save/load where claimed.
- [ ] Export provenance is documented as preserved, partially preserved, omitted, or sidecar-only.

### UI Visual Truth State Proof

- [ ] Visible UI states follow `UI_VISUAL_TRUTH_STATES.md`.
- [ ] Path checks passed is not shown as Ready or Installed.
- [ ] Completed is not shown if the backend did not run.
- [ ] UnifiedResult loaded is not shown as Imported.
- [ ] Imported is not shown if no real Tony layer exists.
- [ ] Editable is not shown if the layer is not a real editable Tony layer.
- [ ] Saved, Loaded, or Exported are shown only after those proofs pass.

### Dev/Mock Boundary Proof

- [ ] Dev/mock output is clearly marked debug/test-only.
- [ ] Dev/mock output uses the same loader/validator/import proof paths being tested.
- [ ] Dev/mock success is never production backend success.
- [ ] Dev/mock fixtures are not used to claim real Basic Pitch, CREPE, MUSC, VioPTT, PESTO, PENN, FCPE, or pYIN integration.

### Forbidden Claims Check

- [ ] No fake Ready, Installed, Completed, Imported, Editable, Saved, Loaded, or Exported state is introduced.
- [ ] No fake `result.json` is introduced.
- [ ] No fake notes are introduced.
- [ ] No custom overlay pretends to be a Tony layer.
- [ ] No feature is called complete without evidence from this checklist.

## Reporting Requirement

Every future backend/UI/layer/save/export task must report:

- highest proof gate reached;
- checklist items passed;
- checklist items not attempted;
- checklist items failed or blocked;
- evidence files, commands, screenshots, or saved/exported artifacts where applicable;
- next task needed to advance the proof chain.

## Next Phase

After CODEX-087D, the project moves from documentation hardening to proof work:

1. `CODEX-088` - dev/mock backend end-to-end proof.
2. `CODEX-089` - `UnifiedResult` to real editable Tony `NoteLayer` proof.
3. `CODEX-090` - edit/save/load/export proof.
