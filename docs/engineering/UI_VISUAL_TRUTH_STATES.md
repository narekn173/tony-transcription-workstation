# UI Visual Truth States

Status: mandatory UI state policy  
Applies before: UI integration, Backend Manager, run status panels, layer lists, save/export status

## 1. Purpose

Every visible state in Tony must describe what has actually happened. UI must not convert partial backend plumbing into user-facing success.

## 2. Allowed Truth States

| State | Meaning | Required evidence | Must not imply |
|---|---|---|---|
| Not configured | User has not supplied required backend settings/path | Empty or missing user setting/manifest path | Missing executable was checked |
| Missing executable | Effective executable path is absent or not a file | Executable probe result | Backend is broken after running |
| Missing model | Required model/checkpoint path is absent | Required file/model probe result | Executable is missing |
| Path checks passed | Configured paths exist at filesystem level | Availability probes passed | Backend Ready/Installed/Completed |
| Running | Process was explicitly started | ExternalProcessRunner state/event | Progress percentage unless real |
| Cancelled | Run was cancelled | cancellation result/state | Failure due to backend error |
| Failed | Process or validation failed | structured process/validation error | Completed or imported |
| Result file missing | Expected output file is absent | output handoff check | Backend produced valid output |
| Result invalid | Output exists but parse/schema/semantic validation failed | UnifiedResult loader/validator errors | Imported notes |
| UnifiedResult loaded | Valid UnifiedResult model loaded | loader success | Tony layer import |
| Imported into real layer | Data became a real Tony/Sonic Visualiser layer/model | Tony layer/model proof | Editable unless edit proven |
| Edited by user | User changed a real editable model/layer | edit action/command proof | Saved/exported |
| Exported | User exported the claimed data | export file proof | Save/load proof |

## 3. Forbidden Visual Claims

- Do not show Ready when only path checks passed.
- Do not show Installed when only a manifest was parsed.
- Do not show Completed if the backend did not run.
- Do not show Completed if the backend ran but output is missing or invalid.
- Do not show Imported if no real Tony/Sonic Visualiser layer exists.
- Do not show Editable if it is not a real editable Tony layer/model.
- Do not show Exported if no export file was created and inspected.
- Do not hide warnings/errors behind a success badge.

## 4. Recommended UI Labeling

Use conservative labels:

- `Not configured`
- `Missing executable`
- `Missing model`
- `Path checks passed`
- `Running`
- `Cancelled`
- `Failed`
- `Result file missing`
- `Result invalid`
- `UnifiedResult loaded`
- `Imported into Tony layer`
- `Edited`
- `Exported`

Avoid ambiguous labels such as:

- `Ready` before a real readiness rule exists;
- `Installed` based only on files existing;
- `Done` when validation/import has not happened;
- `Editable` for non-layer overlays.

## 5. State Transition Rule

The UI must move forward only when the required evidence exists:

```text
Not configured
-> path/model checks
-> path checks passed
-> explicit run
-> running
-> process completed/failed/cancelled
-> result handoff
-> UnifiedResult loaded
-> imported into real layer
-> edited
-> saved/exported
```

Skipping a state requires documented evidence that the skipped work was already performed.

## 6. Debug Workflow Truth-State Aliases

Debug/test workflow reports may use machine-readable stage names. Future UI may map these to user-facing labels, but must preserve the same evidence requirements.

| Debug state | Meaning |
|---|---|
| `no_audio` | No input audio path was provided |
| `backend_not_configured` | Explicit backend configuration is missing |
| `missing_executable` | Executable path is missing or invalid |
| `missing_model_or_runtime` | Required model/runtime path is missing or invalid |
| `path_checks_passed` | Filesystem path checks passed only |
| `running_backend` | A backend process was explicitly started |
| `backend_failed` | Backend process or real-run proof failed |
| `artifacts_discovered` | Output artifacts were discovered and classified |
| `result_json_written` | A real UnifiedResult-compatible JSON file was written |
| `unified_result_loaded` | UnifiedResult loader accepted the result file |
| `imported_into_real_layer` | A real Tony/Sonic Visualiser model/layer was created |
| `inserted_into_view` | The real layer was inserted into a real View/Pane |
| `edit_proof_passed` | Existing CommandHistory-safe edit proof passed |
| `save_load_proof_passed` | Real save/load proof passed |
| `export_proof_passed` | Real export proof passed |
| `completed_with_warnings` | Workflow completed but warnings remain visible |
| `failed` | Workflow failed with structured errors |
| `skipped` | Workflow intentionally skipped without claiming success |

Debug states must not be renamed to `Ready`, `Installed`, or `Completed` in UI unless the matching real evidence exists and the relevant proof gate permits that wording.

## 7. Progress Rule

Do not show fake percent progress. Use stage-based states unless a real backend provides measured progress events that can be tied to actual work performed.
