# Basic Pitch User Setup Preflight Dialog Plan

Status: CODEX-116 source-backed UX/configuration plan
Runtime behavior changed: no
Production UI added: no
Production transcription support: not claimed

## Purpose

This plan defines the smallest safe transition from debug-only Basic Pitch configuration/status to a guarded user-facing setup/preflight dialog.

The future dialog must inspect configuration and readiness. It must not run Basic Pitch, import results, create notes, or claim production readiness until later gates pass.

## Proposed Dialog Concept

Future action label:

```text
Analysis -> Basic Pitch Setup / Preflight...
```

CODEX-117 should add the dialog skeleton only. It should be separate from:

- `Analyse Now!`;
- pYIN option toggles;
- debug manual handoff actions;
- existing save/export actions.

The dialog should show that Basic Pitch is still guarded and not production-ready until all production-readiness gates pass.

## Fields

The setup/preflight dialog should include these fields or read-only status rows.

| Field | Initial behavior |
|---|---|
| Basic Pitch command/executable path | User-editable or browseable later; validated as path/command text, not Ready |
| Command mode | Optional later field for direct executable vs Python/module invocation |
| Input audio source | Derived from the current loaded audio file or shown missing; no selected-region support until implemented |
| Output directory | User-selectable directory; validated for existence/writability later |
| Result JSON path | Derived from output directory as `basic_pitch_result.json` unless an advanced override is later approved |
| Manual run confirmation | Shown as required for later run tasks; CODEX-117 should not run |
| Debug/test-only indicator | Visible while the workflow is still transitioning |
| Production readiness status | Rendered from `BasicPitchProductionReadinessPreflight` |
| Missing requirements | Derived from preflight gates and manual-run status |
| Warnings | Must include pitch-bend, polyphony, debug-only, and unsupported-output warnings |
| Proof/evidence summary | Shows available debug proof without converting it into production readiness |

## Honest States The Dialog May Show

| State | Meaning |
|---|---|
| Not configured | Required command/audio/output information is absent or invalid |
| Missing command | Basic Pitch command/executable path is empty or invalid |
| Missing input audio | No real current audio input is available or selected |
| Missing output directory | No output workspace is selected or accepted |
| Path checks passed | Local path checks passed; this is not Ready/Installed/Completed |
| Manual run blocked | Run cannot be started because preflight or confirmation is missing |
| Manual run allowed | All later manual-run preconditions are present, but this still is not production-ready |
| Debug proof available | Existing debug proof evidence exists and may be inspected |
| Production not ready | One or more mandatory production gates remain blocked |
| Warnings present | User-visible warnings must be reviewed before any run/import |

## Labels The Dialog Must Not Show

The setup/preflight dialog must not show:

- Ready;
- Installed;
- Completed;
- production-ready;
- Imported;
- Visible;
- Editable;
- Save verified;
- Export verified;
- fake percentage progress;
- fake result path created;
- fake notes;
- fake overlays.

It may show "configured" only after validation and only with wording that configured paths do not prove installation or successful analysis.

## Persistence Policy

Persistence should not be added until a later task designs and tests stale-path behavior. When persistent settings are added:

- use a dedicated Basic Pitch settings group or backend settings store, not `Analyser`;
- persist command path only after validation and clear/reset support exists;
- persist default output directory only if the user opts in or if it follows a known app preference pattern;
- do not persist one-off input audio paths by default;
- do not persist derived `result.json` paths by default;
- revalidate all stored paths every time the dialog opens;
- show stale paths as stale/missing, not configured;
- avoid logging private absolute paths in user-facing proof docs unless the user explicitly opens the proof bundle;
- preserve structured argument execution for Windows paths with spaces;
- never treat a stored command path as Installed or Ready.

## Preflight Validation Mapping

Future dialog rows should map to `BasicPitchProductionReadinessPreflight` gates.

| Dialog concern | Preflight gate |
|---|---|
| Command path or runtime | `command_runtime` |
| Current audio file or selected input | `audio_input` |
| Output directory | `output_directory` |
| User confirmation or opt-in | `manual_run_permission` |
| Run preflight complete | `manual_run_permission` / `artifact_discovery` |
| Result schema status | `result_validation` |
| Import/layer policy status | `layer_mapping`, `layer_import`, `visibility` |
| Edit behavior | `edit`, `undo_redo` |
| Persistence/export | `save_load`, `export` |
| User-visible warnings | `warning_visibility`, `pitch_bend_policy`, `polyphony_policy` |
| Progress/cancel | `progress_cancel` |
| Recovery wording | `error_recovery` |
| User docs | `documentation` |
| Test coverage | `regression_protection` |

`BasicPitchDebugManualRunStatus` can still supply env/config-style fields during the transition, but production setup must eventually use user-selected settings.

## Future Run And Import Flow

The intended user-facing flow is:

```text
User opens audio
-> user opens Basic Pitch setup/preflight
-> command/runtime path is selected or validated
-> audio input/full-file scope is confirmed
-> output directory is selected or accepted
-> preflight gates update
-> user explicitly starts run in a later task
-> UI shows stage-based progress/logs only
-> artifacts are discovered
-> result.json is written and validated
-> warnings are shown
-> user confirms import
-> real Tony/SV NoteLayer is inserted
-> post-import proof can be inspected
-> user keeps, discards, renames, edits, saves, or exports through real Tony paths
```

CODEX-117 should stop at setup/preflight display. It must not include a working run button.

## How Future UI Consumes Existing Models

### `BasicPitchDebugManualRunStatus`

Use during transition to show:

- required env/config keys;
- missing command/audio/output;
- explicit opt-in;
- derived result JSON path;
- debug/manual run allowed or skipped.

Do not use it as production readiness by itself.

### `BasicPitchProductionReadinessPreflight`

Use as the production gate list:

- show gate display name;
- show status;
- show user-facing message;
- show technical message in details;
- show evidence reference;
- show whether the gate blocks production readiness.

This is the main source for "Production not ready" state.

### `BasicPitchDebugCombinedRunImportAction` Report

Use only as debug proof evidence:

- manual handoff status;
- result JSON status;
- imported layer status;
- warning preservation.

Do not convert debug success into production-ready status.

### `BasicPitchDebugPostImportProofAction` Report

Use only as post-import proof evidence:

- real NoteModel/NoteLayer;
- Document ownership;
- Pane/View insertion;
- edit and undo/redo proof;
- save/load proof;
- CSV export proof and row count.

Do not use it to claim unsupported export formats or pitch-bend/polyphony production handling.

## User-Facing Text Requirements

The setup/preflight dialog should include wording equivalent to:

```text
Basic Pitch setup/preflight
This workflow is guarded and not yet production-ready.
Configured paths do not mean Basic Pitch has run or completed.
No backend process will run from this dialog.
```

If missing configuration:

```text
Basic Pitch is not configured.
Choose a command, audio input, and output directory before running.
No backend was run.
```

If path checks pass:

```text
Path checks passed.
This does not mean Basic Pitch is installed, ready, or completed.
```

If debug proof exists:

```text
Debug proof evidence is available.
Debug proof does not remove production blockers.
```

Warnings must remain visible:

```text
Possible polyphony remains unresolved.
Pitch-bend mapping is deferred.
```

## Risks And Unsafe Areas

| Risk | Required mitigation |
|---|---|
| MainWindow coupling | Keep MainWindow thin; delegate state to preflight/dialog models |
| Analyser/pYIN regression | Do not call `analyseNow()` or `Analyser`; do not write `Analyser` settings |
| Persistent stale paths | Revalidate every display; show stale paths as missing |
| Windows path quoting | Use structured command/argument lists, never shell strings |
| Stale result.json | Derive/display path, but do not import unless loader validation passes |
| Temp/output cleanup | Define retention/cleanup policy before production runs |
| User confusion | Keep "not production-ready" and "debug evidence" labels visible |
| Progress/cancel | Stage-based only until real progress/cancel exists |
| Pitch-bend/polyphony | Warnings block production policy until solved |
| Privacy | Avoid persisting or over-displaying private audio paths unnecessarily |

## CODEX-117 Acceptance Criteria

The next implementation task may add a setup/preflight dialog skeleton only if it proves:

- no Basic Pitch backend process runs;
- no `result.json` is created;
- no Tony layer is imported;
- no Ready/Installed/Completed state is created;
- no fake progress is shown;
- pYIN and `Analyser` behavior are unchanged;
- `productionReady=false` remains visible from preflight;
- missing command/audio/output are displayed honestly;
- the dialog consumes a model/formatter instead of duplicating state logic in MainWindow.

## Proposed Next CODEX Sequence

```text
CODEX-117 - guarded user-facing Basic Pitch setup/preflight dialog skeleton, no run
CODEX-118 - setup dialog model tests and path validation
CODEX-119 - user-selected audio/output preflight
CODEX-120 - explicit run button with no fake progress
CODEX-121 - result validation/import confirmation
CODEX-122 - progress/log/cancel hardening
CODEX-123 - pitch-bend/polyphony policy plan or implementation boundary
```

## What Remains Deferred

- Production Basic Pitch execution from UI.
- Persistent settings writes.
- Selected-region workflow.
- Real progress and cancellation UI.
- Result import confirmation UX.
- Keep/discard/rename imported layer UX.
- Production pitch-bend mapping.
- Production polyphony policy.
- End-user documentation.
