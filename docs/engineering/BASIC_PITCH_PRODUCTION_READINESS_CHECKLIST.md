# Basic Pitch Production Readiness Checklist

Status: CODEX-114 guarded transition gate
Scope: documentation, audit, and design only
Production transcription support: not yet claimed
Runtime behavior changed: no

## Purpose

This checklist is the first gate for moving Basic Pitch beyond the current debug/test-only workflow. It does not approve production UI by itself. It defines the evidence required before any future task may remove the debug-only label, present Basic Pitch as a user-facing production workflow, or claim Basic Pitch analysis is ready, complete, imported, editable, saved, or exported.

Future implementation tasks must cite this checklist and state which gates passed, which gates remain unproven, and which warnings are still user-visible.

CODEX-115 adds `BasicPitchProductionReadinessPreflight`, a backend-only model that represents these gates in code. It does not mark Basic Pitch production-ready, add production UI, or run Basic Pitch.

CODEX-116 adds a source-backed UX/configuration audit and setup/preflight dialog plan. It does not add production UI, change MainWindow runtime behavior, run Basic Pitch, or remove any production blocker.

## Files Inspected

| File | Reason |
|---|---|
| `AGENTS.md` | Repository no-fake and pYIN preservation rules |
| `docs/00_PROJECT_INDEX.md` | Current reading order and implementation status through CODEX-113 |
| `docs/engineering/REAL_RESULT_ACCEPTANCE_CHECKLIST.md` | Mandatory feature-complete evidence checklist |
| `docs/engineering/REAL_RESULT_AND_TONY_LAYER_INTEGRATION_RULES.md` | Real backend/result/layer proof chain |
| `docs/engineering/UI_VISUAL_TRUTH_STATES.md` | Honest UI state policy |
| `docs/engineering/PROOF_BUNDLE_POLICY.md` | Required evidence bundle and forbidden fake states |
| `docs/engineering/BACKEND_OUTPUT_TRUTH_TABLE.md` | Backend-specific output expectations and Basic Pitch risks |
| `docs/engineering/BACKEND_TO_TONY_LAYER_MAPPING_MATRIX.md` | Basic Pitch to Tony layer mapping boundary |
| `docs/engineering/LAYER_TYPE_POLICY.md` | Note, pitch-bend, confidence, and polyphony layer policy |
| `docs/engineering/BASIC_PITCH_ADAPTER_AUDIT.md` | Verified Basic Pitch CLI/artifact facts and deferred claims |
| `docs/engineering/BASIC_PITCH_OUTPUT_CONVERSION_PLAN.md` | CSV note-events to UnifiedResult conversion boundary |
| `docs/engineering/BASIC_PITCH_REAL_ARTIFACT_DISCOVERY.md` | Manual artifact discovery harness |
| `docs/engineering/BASIC_PITCH_REAL_ARTIFACT_TO_UNIFIED_RESULT_PROOF.md` | Artifact to in-memory UnifiedResult proof |
| `docs/engineering/BASIC_PITCH_UNIFIED_RESULT_JSON_HANDOFF_PROOF.md` | Artifact to real result.json handoff proof |
| `docs/engineering/BASIC_PITCH_REAL_RUN_HANDOFF_PROOF.md` | Manual opt-in real Basic Pitch run proof |
| `docs/engineering/BASIC_PITCH_RESULT_TO_TONY_LAYER_PROOF.md` | Basic Pitch-shaped result.json to real Tony layer proof |
| `docs/engineering/BASIC_PITCH_LAYER_SAVE_LOAD_EXPORT_PROOF.md` | Basic Pitch-shaped layer save/load/export proof |
| `docs/engineering/BASIC_PITCH_DEBUG_WORKFLOW_PROOF.md` | Debug workflow truth states and proof bundle |
| `docs/engineering/BASIC_PITCH_DEBUG_UI_CONSUMPTION_MODEL.md` | UI-consumable truth-state model |
| `docs/engineering/BASIC_PITCH_MAINWINDOW_DEBUG_UI_ACTION_PROOF.md` | Current debug MainWindow actions and limitations |
| `docs/engineering/BASIC_PITCH_DEBUG_CONFIGURATION_ENTRY_PROOF.md` | Manual-run configuration status boundary |
| `docs/engineering/BASIC_PITCH_DEBUG_MANUAL_RUN_ACTION_PROOF.md` | Debug manual handoff action |
| `docs/engineering/BASIC_PITCH_DEBUG_POST_RUN_IMPORT_ACTION_PROOF.md` | Debug result.json import action |
| `docs/engineering/BASIC_PITCH_DEBUG_COMBINED_RUN_IMPORT_ACTION_PROOF.md` | Debug handoff plus import action |
| `docs/engineering/BASIC_PITCH_DEBUG_POST_IMPORT_EDIT_SAVE_EXPORT_PROOF.md` | Debug post-import edit/save/load/export proof |
| `main/backend/BasicPitchDebugManualRunAction.*` | Manual handoff action fields and no-import boundary |
| `main/backend/BasicPitchDebugPostRunImportAction.*` | Existing result.json to real layer import action |
| `main/backend/BasicPitchDebugCombinedRunImportAction.*` | Combined manual handoff plus import action |
| `main/backend/BasicPitchDebugPostImportProofAction.*` | Edit/undo/save/load/export proof summary boundary |
| `main/backend/BasicPitchDebugWorkflow.*` | Debug workflow truth states and proof bundle |
| `main/backend/BasicPitchDebugWorkflowUiModel.*` | UI truth-state mapping |
| `main/backend/BasicPitchRealRunHandoffProof.*` | Manual opt-in real run handoff |
| `main/backend/BasicPitchArtifactDiscovery.*` | External process artifact discovery |
| `main/backend/BasicPitchArtifactToUnifiedResult.*` | Artifact to UnifiedResult bridge |
| `main/backend/BasicPitchOutputConverter.*` | Basic Pitch note-events CSV parser/converter |
| `main/backend/BasicPitchLayerPersistenceExportProof.*` | Real edit/save/load/export proof path |
| `main/backend/TonyLayerImporter.*` | Real Tony/SV NoteModel, NoteLayer, Document, View, and CommandHistory boundary |
| `main/MainWindow.*` | Current debug actions and pYIN/Analyser separation |
| `main/backend/test/TestBackendTypes.h` | Regression evidence for no fake backend state and Basic Pitch proof boundaries |

## Current Proven Capabilities

These are genuinely proven in the current repository, but only in debug, manual, or test-only boundaries unless noted otherwise.

| Capability | Current proof |
|---|---|
| Manual Basic Pitch handoff preflight | `BasicPitchDebugManualRunStatus` and `BasicPitchDebugManualRunAction` report missing opt-in, command, audio, and output directory without running when incomplete. |
| Manual opt-in real process boundary | `BasicPitchRealRunHandoffProof` can run only when explicit manual env/config is present. Normal tests skip real Basic Pitch. |
| Real artifact discovery | `BasicPitchArtifactDiscovery` uses `ExternalProcessRunner`, structured arguments, and output-directory classification. |
| Basic Pitch note-events CSV parsing | `BasicPitchOutputConverter` parses the verified note-events CSV shape and preserves timing, duration, MIDI pitch, velocity, and pitch-bend values where represented. |
| Artifact to UnifiedResult | `BasicPitchArtifactToUnifiedResult` converts recognized `csv_note_events` artifacts into in-memory `UnifiedResult` without creating fake Tony layers. |
| result.json handoff | `BasicPitchUnifiedResultHandoff` writes a real non-empty loader-compatible `result.json` from parsed artifact data. |
| UnifiedResult loading/reporting | `BackendRunOutputHandoff`, `BackendRunResultLoader`, `UnifiedResultFileLoader`, and `BackendRunResultReporter` load/report the written file. |
| Real Tony/SV note import | `BasicPitchResultToTonyLayerProof` and `TonyLayerImporter` import Basic Pitch-shaped notes into real `NoteModel`/`NoteLayer` data. |
| Document-owned layer creation | Import sets `importedIntoTonyLayers=true` only after a real Document-owned Tony/SV layer is created. |
| Pane/View insertion | `insertedIntoView=true` only after a real `Document::addLayerToView` insertion into a real `Pane`/`View`. |
| Edit proof | CODEX-112 extends the Basic Pitch proof through `TonyLayerImporter::proveCommandHistoryEdit`. |
| Undo/redo proof | CODEX-112 reports undo/redo proof through the same CommandHistory-safe edit proof path. |
| Save/load proof | `BasicPitchLayerPersistenceExportProof` proves real XML persistence through `Document::toXml`, `Pane::toXml`, and `SVFileReader::parseXml`. |
| CSV export proof | `BasicPitchLayerPersistenceExportProof` proves real lower-level CSV export through `getExportModel(...)` and `CSVFileWriter`. |
| Exported row count proof | `exportedNoteCount` is parsed from actual exported CSV rows, not copied from in-memory note count. |
| Warning preservation | `possible_polyphony` and `pitch_bend_mapping_deferred` remain visible through conversion, result.json, import, and debug reports. |
| No fake backend state | Tests assert no Ready, Installed, or Completed mutation across manifest, settings, run, handoff, import, and debug action boundaries. |
| pYIN/Analyser isolation | Current Basic Pitch debug actions are separate MainWindow actions and do not call `Analyser` or replace the pYIN workflow. |

## Current Debug-Only Limitations

The following remain unproven or intentionally deferred. They block production claims.

- No production Basic Pitch UI exists.
- No normal user-facing Basic Pitch configuration UI exists.
- No production Basic Pitch run button exists.
- No robust user-facing progress or cancellation workflow exists.
- No user-facing audio selection workflow exists for Basic Pitch.
- No selected-region Basic Pitch workflow exists.
- No selected-region replacement, preview, accept, reject, or merge workflow exists.
- No production pitch-bend Tony layer mapping exists.
- `pitch_bend_mapping_deferred` must remain visible until solved.
- No production polyphony policy exists.
- `possible_polyphony` must remain visible until solved.
- No production MIDI, SVL, RDF, or other export guarantee exists beyond the debug-proven lower-level CSV path.
- No polished production UX exists.
- No persistent settings or installer/package validation exists.
- No user-facing Basic Pitch readiness/install/completion semantics exist.
- No production provenance persistence beyond durable identity/name fields is proven.
- No GUI screenshot/smoke proof for the full Basic Pitch user workflow is required by the current debug tests.

## Production Readiness Gates

All gates below are mandatory before Basic Pitch may be described as production-ready or moved out of debug/test-only UI.

| Gate | Required proof |
|---|---|
| Configuration gate | User-facing configuration validates command/runtime, optional model/runtime requirements, output policy, and saved settings without marking Ready from paths alone. |
| Audio input gate | User-selected audio is real, readable, and the selected full-file or region scope is explicit. |
| Backend executable/runtime gate | The configured Basic Pitch command or runtime is probed safely, with missing executable/runtime/model errors visible and no fake Installed/Ready labels. |
| Run/preflight gate | A production preflight produces a structured go/no-go report before execution and preserves all failure details. |
| Artifact discovery gate | A real run discovers real output artifacts and classifies note-events CSV, MIDI, NPZ, logs, and unknown files without fabricating missing artifacts. |
| result.json gate | Production result JSON is written only from parsed real artifacts and validated against the loader contract. |
| Result validation gate | `UnifiedResult` schema/semantic checks pass before import; invalid or empty output blocks import. |
| Layer mapping gate | Notes map to a real Tony/SV note layer. Pitch bends, confidence, and polyphony must be preserved, represented, or visibly warned. |
| Import visibility gate | Imported and visible states are separate: real Document-owned layer first, real Pane/View insertion second. |
| Edit gate | Editable is shown only when the layer is a real editable Tony/SV layer and edit behavior is proven or directly supported by the production path. |
| Undo/redo gate | Undo/redo is shown only when CommandHistory-safe behavior is proven in the user path. |
| Save/load gate | Persistence is shown only after real project/session save and reload proof for the imported production path. |
| Export gate | Each export format claim is proven separately. CSV proof does not imply MIDI, SVL, RDF, or pitch-bend export support. |
| Warning visibility gate | `possible_polyphony`, `pitch_bend_mapping_deferred`, backend stderr, validation warnings, and unsupported-output warnings are visible to the user. |
| Progress/cancel gate | UI uses stage-based progress until real backend progress exists; cancel is shown only if real cancellation is wired and tested. |
| Error recovery gate | Missing config, backend failure, invalid output, no active pane, import failure, save/load failure, and export failure are recoverable without corrupting existing layers. |
| User documentation gate | User docs explain setup, local execution, limitations, warnings, and debug/proof semantics. |
| Regression protection gate | Tests and manual checks prove pYIN, Analyser, existing audio load, existing edit, save, and export workflows remain unchanged. |

## UI Truth-State Requirements

Future production UI may show these labels only under the listed conditions.

| UI state | Allowed only after |
|---|---|
| Configured | User configuration passes validation. This still is not Ready or Installed. |
| Running | A real Basic Pitch process is currently executing. |
| Result loaded | A real `result.json` was written, handed off, loaded, and validated. |
| Imported | A real Tony/SV model/layer was created and registered. |
| Visible | A real layer was inserted into a real Pane/View. |
| Editable | Real editable layer state and edit proof exist. |
| Undo/redo available | CommandHistory-safe edit/undo/redo proof exists in the relevant path. |
| Save/load verified | Real save and reload proof exists for the imported layer. |
| Export verified | A real export file was created and inspected for the claimed format. |

Future production UI must not:

- show Ready, Installed, or Completed globally without explicit proof semantics;
- show fake percent progress without real backend progress events;
- show Imported when only `UnifiedResult` loaded;
- show Visible when no Pane/View insertion occurred;
- show Editable from a custom overlay or non-layer representation;
- show Saved or Exported without real file evidence;
- hide `possible_polyphony` or `pitch_bend_mapping_deferred`.

## Basic Pitch Output Policy

- Basic Pitch note-events CSV can map to `NoteModel` + `NoteLayer` after converter, validation, import, edit, save/load, and export proof.
- Basic Pitch pitch bends must not be silently discarded.
- `pitch_bend_mapping_deferred` must remain visible until a Tony/SV pitch-bend or deviation representation is designed, implemented, saved/loaded, and exported or explicitly declared unsupported.
- `possible_polyphony` must remain visible until a production polyphony policy exists.
- A monophonic Tony `NoteLayer` import is acceptable only if overlaps are preserved where the layer supports them or if the UI clearly warns about unresolved polyphony semantics.
- Future pitch-bend support may require a separate time-value layer, note-linked metadata, a deviation layer, or sidecar/export policy. It cannot be claimed from CSV note export alone.
- Velocity/level and confidence-like fields must be preserved where supported or reported as unavailable.

## Real Manual Run To Production Workflow Gap

The current debug manual path proves mechanics, not a production workflow. The missing bridge must define:

- how the user chooses Basic Pitch configuration without environment variables;
- how the user selects audio or selected region scope;
- how the user chooses or accepts an output directory/workspace;
- how the Basic Pitch command/runtime is validated before the run;
- how logs, stdout, stderr, artifacts, and result paths are shown;
- how cancellation works, if exposed;
- how the result is imported after validation;
- how warnings are shown before and after import;
- how the user confirms, keeps, discards, renames, or replaces imported layers;
- how save/load and export proof is surfaced without implying unsupported formats;
- how existing pYIN analysis remains available and unchanged.

## Acceptance Criteria Before Removing Debug-Only Label

Do not remove the debug/test-only label until all of the following are true:

- [ ] Real Basic Pitch is installed or configured by the user through a production-safe path.
- [ ] A real audio input is used.
- [ ] A real Basic Pitch process runs through structured argument execution.
- [ ] Real artifacts are discovered.
- [ ] A real `result.json` is written from parsed artifact data.
- [ ] `UnifiedResult` loads and validates.
- [ ] A real Tony/SV layer is created.
- [ ] The layer is visible to the user in a real Pane/View.
- [ ] The user can see notes.
- [ ] The user can edit notes.
- [ ] Undo/redo works.
- [ ] Save/load works.
- [ ] Export works for each claimed format.
- [ ] Warnings are visible.
- [ ] Failure states are honest and recoverable.
- [ ] No fake states are introduced.
- [ ] Automated tests pass.
- [ ] Manual proof passes on Windows with real local Basic Pitch configuration.
- [ ] pYIN and existing Tony workflows are regression-checked.

## Regression Test Requirements

Before any production Basic Pitch task is accepted, tests or documented manual proof must cover:

- missing executable;
- missing audio input;
- missing output directory;
- explicit opt-in or user confirmation missing;
- backend process failure;
- timeout/cancellation if exposed;
- missing artifacts;
- missing note-events artifact;
- invalid result JSON;
- empty result JSON;
- malformed CSV;
- invalid note timing, pitch, or duration;
- polyphony warning preservation;
- pitch-bend warning preservation;
- import success;
- no active document/pane;
- view insertion success and failure;
- edit success;
- undo/redo success;
- save/load success;
- CSV export success;
- each additional claimed export format;
- pYIN unaffected;
- existing Tony save/export workflows unaffected.

## Risks And Unsafe Areas

- MainWindow action integration can accidentally blur debug and production labels.
- `Analyser` and pYIN workflow changes risk breaking Tony's existing baseline.
- Document/Pane lifetime ordering can crash if Pane/View objects do not outlive Document-owned layers in tests/proofs.
- CSV export selection/range handling can produce header-only output if the wrong layer/model/range is used.
- ModelById and session persistence require real reload proof, not in-memory checks.
- QSettings or persistent configuration can create stale, private, or misleading backend state if added without a migration/clear policy.
- External process execution must preserve structured arguments and path quoting, especially on Windows paths with spaces.
- Temporary directory and AppData permissions can make tests fail outside the code under test.
- Users may confuse debug proof dialogs with production support unless labels remain explicit.
- Pitch-bend and polyphony warnings can be lost if future UI only displays note import success.

## Stop Conditions

Stop implementation and update this checklist if:

- Basic Pitch output shape differs from the verified CSV/MIDI assumptions;
- a future production UI would need to claim Ready, Installed, Completed, Imported, Editable, Saved, or Exported without matching proof;
- pitch bends or polyphony would be silently discarded;
- selected-region behavior would overwrite notes without preview/confirmation;
- any task requires modifying pYIN/Analyser to make Basic Pitch work;
- backend runtime setup implies bundled Python/models or installer behavior not yet licensed and documented.

## Next Task

Recommended next task: CODEX-117 - guarded user-facing Basic Pitch setup/preflight dialog skeleton, no run.
