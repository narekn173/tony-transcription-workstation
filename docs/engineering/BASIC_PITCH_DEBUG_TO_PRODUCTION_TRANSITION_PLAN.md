# Basic Pitch Debug To Production Transition Plan

Status: CODEX-114 guarded transition plan
Scope: documentation, audit, and design only
Runtime behavior changed: no

## Purpose

The Basic Pitch path now has a strong debug/test-only proof chain. This plan defines how to transition from that chain toward a guarded user-facing workflow without turning debug proof states into production claims.

The transition must remain incremental. No future task may jump directly from the current debug actions to "production Basic Pitch support" without passing the gates in `BASIC_PITCH_PRODUCTION_READINESS_CHECKLIST.md`.

## Current Debug Chain

The current debug path is:

```text
Analysis -> Debug: Run Basic Pitch Manual Handoff and Import...
manual env/config preflight
-> optional real Basic Pitch process when explicitly configured
-> artifact discovery
-> recognized csv_note_events artifact
-> UnifiedResult conversion
-> real result.json
-> BackendRunResultLoader / UnifiedResultFileLoader
-> TonyLayerImporter / BasicPitchResultToTonyLayerProof
-> real Document-owned NoteLayer
-> optional real Pane/View insertion
-> BasicPitchDebugPostImportProofAction
-> edit / undo-redo / save-load / CSV export proof summary
```

This chain proves mechanics and truth-state reporting. It is still not production UI.

## Proven Debug Boundaries To Reuse

Future production work should reuse these boundaries rather than rewriting state logic in MainWindow:

| Boundary | Reuse role |
|---|---|
| `BasicPitchDebugManualRunStatus` | Configuration/preflight status shape, but production UI needs persistent user configuration instead of env-only setup. |
| `BasicPitchArtifactDiscovery` | External process artifact discovery and classification. |
| `BasicPitchArtifactToUnifiedResult` | Recognized artifact to in-memory `UnifiedResult`. |
| `BasicPitchUnifiedResultHandoff` | Parsed artifact to real `result.json` plus loader handoff. |
| `BasicPitchDebugPostRunImportAction` / `BasicPitchResultToTonyLayerProof` | Existing result import to real Tony/SV layer. |
| `BasicPitchDebugPostImportProofAction` / `BasicPitchLayerPersistenceExportProof` | Edit, undo/redo, save/load, and CSV export evidence. |
| `BasicPitchDebugWorkflowUiModel` and report formatters | Conservative truth-state mapping and proof-bundle presentation model. |

Future production UI may wrap or generalize these boundaries, but it must not loosen their evidence requirements.

## Transition Principles

- Keep pYIN and `Analyser` as the protected baseline.
- Keep Basic Pitch separate from `Analyse Now` until a later task explicitly designs a safe shared analysis-entry strategy.
- Do not persist configuration until missing, stale, and invalid settings behavior is designed.
- Do not show Ready, Installed, Completed, Imported, Visible, Editable, Saved, or Exported unless the matching evidence exists.
- Do not show percent progress until real Basic Pitch progress events exist.
- Do not hide polyphony or pitch-bend limitations behind a success message.
- Do not delete debug actions until production UI has equivalent or stronger proof-bundle access.

## Proposed CODEX Sequence

### CODEX-115 - Production-Readiness Preflight Model

Status: implemented as `BasicPitchProductionReadinessPreflight`.

Goal: add a compile-only or backend-only model that evaluates production-readiness gates without adding production UI.

Must prove:

- configuration completeness;
- executable/runtime presence;
- audio input readiness;
- output workspace readiness;
- known unsupported items such as pitch-bend mapping and polyphony policy;
- no Ready/Installed/Completed mutation.

Still forbidden:

- no production MainWindow action;
- no Basic Pitch execution by default;
- no persistent settings writes unless explicitly designed.

CODEX-115 result:

- production gates are represented as code-level gate records;
- debug proof can appear as `debug_only` evidence;
- `productionReady` remains false while the preflight boundary is debug/test-only or any production blocker remains;
- no Ready, Installed, or Completed backend mutation is created.

### CODEX-116 - Debug-To-User Configuration UX Source Audit

Status: implemented by:

- `BASIC_PITCH_USER_CONFIGURATION_UX_SOURCE_AUDIT.md`
- `BASIC_PITCH_USER_SETUP_PREFLIGHT_DIALOG_PLAN.md`

Goal: audit existing Tony settings/dialog patterns and design the safest production configuration entry.

Must document:

- where persistent Basic Pitch settings should live;
- whether QSettings is appropriate;
- how stale command/audio/output paths are shown;
- how settings can be cleared;
- how private paths are avoided in logs/docs;
- what UI wording distinguishes Configured from Ready.

### CODEX-117 - First Guarded Setup Dialog, No Run

Goal: add a user-facing setup dialog or panel that validates configuration but cannot run Basic Pitch.

Must prove:

- missing command/audio/output fields are visible;
- configured state does not imply Ready/Installed;
- no backend process runs;
- no result.json is created;
- pYIN/Analyser unchanged.

### CODEX-118 - Setup Dialog Model Tests And Path Validation

Goal: harden the setup/preflight dialog model and validate path/configuration fields without running analysis.

Must prove:

- missing command, audio, and output fields are truthfully represented;
- stale or invalid paths do not become Configured/Ready;
- path checks use structured values rather than shell strings;
- no backend process runs;
- no fake result or layer is created.

### CODEX-119 - User-Selected Audio/Output Preflight

Goal: let the user pick or confirm audio scope and output workspace for Basic Pitch without running analysis.

Must prove:

- full-file vs selected-region scope is explicit;
- selected-region support remains disabled or preview-only until implemented;
- invalid paths and unsupported scope block run;
- no fake result or layer is created.

### CODEX-120 - Guarded Manual Run From User Flow

Goal: expose an explicit user-triggered Basic Pitch run after preflight passes.

Must prove:

- real process execution through structured arguments;
- honest running/failed/succeeded states;
- stage-based progress only;
- logs/stdout/stderr visible;
- no import until result validation succeeds;
- cancellation remains hidden unless real cancellation is implemented.

### CODEX-121 - Import Result With User Confirmation

Goal: import a validated Basic Pitch result only after user confirmation.

Must prove:

- user sees warnings before import;
- import creates a real Document-owned layer;
- View/Pane insertion is real;
- user can keep, discard, or rename the imported layer;
- existing notes are not overwritten silently.

### CODEX-122 - Progress, Log, Cancel, And Error Recovery Hardening

Goal: harden long-running process UX.

Must prove:

- cancellation if shown really terminates or stops the process path;
- failure preserves logs and does not create success states;
- retry does not reuse stale artifacts incorrectly;
- temp/output cleanup policy is explicit.

### CODEX-123 - Pitch-Bend And Polyphony Policy Implementation Plan

Goal: design and then implement production policy for Basic Pitch-specific output limitations.

Must decide:

- whether overlapping notes remain in a single note layer, split into layers/voices, or are warned/rejected;
- how pitch bends are represented, preserved, saved, and exported;
- what warnings remain visible if some output is unsupported;
- whether export sidecars are needed.

## Implementation Versus Audit Split

| Step | Type | Runtime behavior? |
|---|---|---|
| CODEX-115 | Backend/model preflight | Implemented; no production UI, no run |
| CODEX-116 | Audit/design | Implemented; no runtime behavior |
| CODEX-117 | Guarded setup UI | UI skeleton only, no backend run |
| CODEX-118 | Setup model/path validation | UI/model only, no backend run |
| CODEX-119 | Audio/output preflight UX | UI/model only, no backend run unless explicitly deferred |
| CODEX-120 | User-triggered run | Yes, guarded real process |
| CODEX-121 | User-confirmed import | Yes, guarded real layer import |
| CODEX-122 | Hardening | Yes, process/status/error improvements |
| CODEX-123 | Output policy | Design first, then implementation after proof |

## Production User Workflow Target

The eventual production workflow should be:

```text
User opens audio
-> user opens Basic Pitch setup/status
-> UI validates Basic Pitch command/runtime
-> user selects full-file or allowed region scope
-> user chooses/accepts output workspace
-> user starts run explicitly
-> UI shows real stage state, logs, and failures
-> artifacts are discovered
-> result.json is written and validated
-> warnings are shown
-> user confirms import
-> real Tony/SV NoteLayer is inserted
-> user edits notes with undo/redo
-> project save/load preserves notes
-> supported export formats are enabled only after proof
```

## Required User-Facing Messages

Future production UI must include wording equivalent to:

- "Configured" means paths/settings are present and validated; it does not mean Basic Pitch has completed analysis.
- "Running" means a real Basic Pitch process is executing.
- "Result loaded" means a real result file was written and loaded.
- "Imported" means notes became a real Tony/Sonic Visualiser layer.
- "Visible" means that layer was inserted into the current view.
- "Pitch-bend mapping is deferred" until solved.
- "Possible polyphony" until production policy is solved.
- "CSV export verified" only for the CSV path actually proven.

## What Must Stay Forbidden Until Proven

- Running Basic Pitch automatically on startup.
- Running Basic Pitch from normal `Analyse Now`.
- Replacing pYIN behavior.
- Creating fake `result.json` for failed runs.
- Creating fake notes for missing output.
- Drawing custom overlays and calling them layers.
- Showing fake progress percentages.
- Showing global Ready, Installed, or Completed states from path checks.
- Hiding stdout/stderr or validation errors.
- Silently dropping pitch bends or overlap/polyphony information.
- Overwriting existing corrected notes without preview and confirmation.

## Production Readiness Review Checklist

Before any task claims the transition has reached production:

- [ ] All gates in `BASIC_PITCH_PRODUCTION_READINESS_CHECKLIST.md` are addressed.
- [ ] The final UI text still follows `UI_VISUAL_TRUTH_STATES.md`.
- [ ] The proof bundle remains available for failures and successes.
- [ ] pYIN and `Analyser` behavior are regression-tested.
- [ ] Windows paths with spaces are tested.
- [ ] AppData/temp directory behavior is tested or documented.
- [ ] Pitch-bend and polyphony warnings are visible.
- [ ] Unsupported export formats are disabled or labeled unproven.
- [ ] Tests and manual proof use real local Basic Pitch at least once before release claims.

## Next Task

Recommended next task: CODEX-117 - guarded user-facing Basic Pitch setup/preflight dialog skeleton, no run.
