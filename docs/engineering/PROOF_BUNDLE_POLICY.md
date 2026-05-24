# Proof Bundle Policy

Status: mandatory engineering rule for debug workflows and future UI workflows.

## Purpose

Any backend workflow that claims progress beyond path checks must carry a proof bundle: a structured record of the evidence that was actually produced. This prevents UI, logs, and future automation from turning partial plumbing into fake Ready, Installed, Completed, Imported, Saved, or Exported states.

## Required Contents

A proof bundle must include, where applicable:

- structured event log with stage-based truth states;
- command used, only if a process was actually configured or run;
- input audio path, only if provided;
- output directory, only if provided;
- discovered artifacts with file paths, types, and sizes;
- `result.json` path;
- UnifiedResult loader status;
- note count or output count;
- Tony Document/Pane/Layer/Model snapshot summary where available;
- `importedIntoTonyLayers`;
- `insertedIntoView`;
- edit proof status;
- save/load proof status;
- export proof status;
- warnings and errors;
- `productionTranscription=false` for debug/test/manual proof paths;
- `testOnly/debugOnly=true` for debug/test/manual proof paths.

## Truth-State Rule

Proof bundles use named stage states only. They must not include invented percentages or progress bars unless a real backend provides measured progress events.

Allowed debug workflow states include:

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

## Forbidden Claims

- Do not show Ready when only path checks passed.
- Do not show Installed when only files exist.
- Do not show Completed if no backend process ran or no result was loaded.
- Do not show Imported if no real Tony/Sonic Visualiser layer/model exists.
- Do not show Editable without real editable layer and edit proof.
- Do not show Saved or Exported without inspected save/load or export evidence.
- Do not hide warnings such as possible polyphony or deferred pitch-bend mapping.

## Future UI Rule

MainWindow/UI integration must consume proof-bundle fields directly or through an equally strict adapter. It must not invent a second state machine that weakens these evidence requirements.
