# Basic Pitch Debug Workflow Proof

Status: CODEX-103 debug-only/manual-test workflow boundary.

This document defines the first Basic Pitch workflow coordinator boundary:

```text
synthetic or manual Basic Pitch artifact
-> artifact discovery/classification
-> UnifiedResult conversion
-> real result.json
-> BackendRunOutputHandoff
-> BackendRunResultLoader / BackendRunResultReporter
-> TonyLayerImporter real NoteLayer proof
-> CommandHistory edit proof
-> Document/Pane save/load proof
-> getExportModel(...) / CSVFileWriter export proof
```

This is not user-facing UI, not automatic Basic Pitch runtime wiring, and not production transcription support.

## Boundary

`BasicPitchDebugWorkflow` produces `BasicPitchDebugWorkflowReport`.

The workflow supports explicit modes:

- `synthetic_artifact_only`
- `discovered_artifact_manual_only`
- `real_basic_pitch_manual_opt_in`

The first two modes inspect an explicit artifact directory and never run Basic Pitch. The real-run mode only runs through `BasicPitchRealRunHandoffProof` when the caller supplies explicit opt-in configuration. Normal tests must skip real Basic Pitch execution.

## Truth States

The debug workflow reports stage-based truth states only:

- `no_audio`
- `backend_not_configured`
- `missing_executable`
- `missing_model_or_runtime`
- `path_checks_passed`
- `running_backend`
- `backend_failed`
- `artifacts_discovered`
- `result_json_written`
- `unified_result_loaded`
- `imported_into_real_layer`
- `inserted_into_view`
- `edit_proof_passed`
- `save_load_proof_passed`
- `export_proof_passed`
- `completed_with_warnings`
- `failed`
- `skipped`

The boundary does not produce fake percentages. Future UI must map these states conservatively and must preserve warnings.

## Proof Bundle Contents

The workflow proof bundle includes:

- structured event log;
- command used if any;
- input audio path if provided;
- output directory if provided;
- discovered artifacts;
- result JSON path;
- loaded result status;
- note count;
- Tony Document/Pane/Layer/Model snapshot summary where available;
- `importedIntoTonyLayers`;
- `insertedIntoView`;
- edit proof status;
- save/load proof status;
- export proof status;
- warnings/errors;
- `productionTranscription=false`;
- `testOnlyDebugOnly=true`.

## Real APIs Composed

CODEX-103 composes existing proof boundaries instead of duplicating conversion or import logic:

- `BasicPitchArtifactDiscovery`
- `BasicPitchUnifiedResultHandoff`
- `BasicPitchResultToTonyLayerProof`
- `TonyLayerImporter::proveCommandHistoryEdit`
- `BasicPitchLayerPersistenceExportProof`
- `BackendRunOutputHandoff`
- `BackendRunResultLoader`
- `BackendRunResultReporter`

For layer proof it uses real Tony/Sonic Visualiser APIs already proven in CODEX-089 through CODEX-102: `Document`, `Pane`, `NoteModel`, `NoteLayer`, `Document::addLayerToView`, `ChangeEventsCommand`, `Document::toXml`, `Pane::toXml`, `SVFileReader::parseXml`, `getExportModel(...)`, and `CSVFileWriter`.

## What CODEX-103 Proves

- Missing real-run configuration skips safely and reports `backend_not_configured` / `skipped`.
- Synthetic Basic Pitch-shaped artifacts can reach `result_json_written` and `unified_result_loaded`.
- Basic Pitch-shaped results can reach `imported_into_real_layer`.
- A supplied real `Pane` path can reach `inserted_into_view`.
- CommandHistory-safe edit proof can reach `edit_proof_passed`.
- Save/load and export proof status can be reported by the same coordinator.
- `possible_polyphony` and `pitch_bend_mapping_deferred` warnings remain visible.
- `productionTranscription=false` and `testOnlyDebugOnly=true` remain explicit.
- No Basic Pitch Ready, Installed, or Completed global state is created.

## What Remains Unproven

- User-facing Basic Pitch UI.
- Automatic runtime Basic Pitch execution.
- Production Basic Pitch transcription quality.
- Production polyphony policy.
- Pitch-bend import into a dedicated Tony/SV representation.
- Screenshot/GUI smoke proof.
- Selected-region replacement workflow.

Future UI work must add GUI smoke proof and screenshots, and it must consume this workflow/proof-bundle state without inventing weaker state logic.

CODEX-104 adds `BasicPitchDebugWorkflowUiModel`, a compile-only/test-only
adapter that maps this report into user-visible truth-state strings, action
enablement concepts, warning visibility, and proof-bundle summaries. It still
does not add UI or runtime wiring.

## Recommended Next Task

Recommended next task after CODEX-104: CODEX-105 - design a debug-only
MainWindow integration plan, with source inspection and screenshots required
before any runtime UI change.
