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
