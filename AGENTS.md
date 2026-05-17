# AGENTS.md — Tony Fork Transcription Workstation

## Document status

| Field | Value |
|---|---|
| Status | Canonical v0.1 — reviewed/stabilized |
| Last updated | 2026-05-15 |
| Applies to | Codex / AI coding agents working in this repository |


## Purpose of this file

This file is the repository-level instruction file for AI coding agents working on the Tony Fork Transcription Workstation.

It defines how agents must understand the project, which documents are authoritative, what they may change, what they must not change, how backend integrations must be implemented, and what counts as a completed task.

This file is intentionally strict. The project is complex, legacy-adjacent, audio/MIR-heavy, and vulnerable to architectural drift if an agent improvises.

---

## Project identity

This repository is a fork of Tony intended to become a modern, local-first transcription workstation.

Core product workflow:

```text
Audio → choose analysis backend → run real analysis → display notes / pitch / confidence → correct manually → export MIDI / CSV
```

Core architectural idea:

```text
Existing Tony editor + existing pYIN workflow
        ↓
Analysis Engine Manager
        ↓
Backend adapters
        ↓
BackendManifest → BackendRequest → backend execution → UnifiedResult
        ↓
Validation
        ↓
Tony pitch / note / annotation layers
```

Target analysis engines:

```text
[ pYIN Vamp plugin ]
    Existing Tony method. Must remain available.

[ Basic Pitch / NeuralNote ]
    First real external or native/ONNX backend candidate.

[ CREPE Notes ]
    External backend first; possible Vamp/native backend later.

[ MUSC Violin Transcription ]
    External backend.

[ VioPTT Violin Technique-Aware ]
    External backend with technique labels.

[ PESTO / PENN / FCPE ]
    f0 backends plus a shared note-segmentation layer.
```

---

## Source-of-truth documents

Before implementing or changing behavior, use the project documents below.

| Document | Role |
|---|---|
| `docs/00_PROJECT_INDEX.md` | Master project map, document status, reading order, and current next step. |
| `docs/02_PRD.md` | Product goals, user workflows, MVP, non-goals, assumptions. |
| `docs/03_SRS.md` | Exact software requirements and acceptance expectations. |
| `docs/04_BACKEND_CONTRACT.md` | Backend data contract: manifests, requests, UnifiedResult. |
| `docs/schemas/*.schema.json` | Machine-checkable JSON schemas. |
| `docs/examples/*.example.json` | Valid example backend manifests, requests, and results. |
| `docs/05_TDD_ARCHITECTURE.md` | Technical design and architecture. |
| `docs/engineering/REAL_RESULT_AND_TONY_LAYER_INTEGRATION_RULES.md` | Mandatory rules against fake UI/results/layers and feature-complete claims. |
| `docs/engineering/LAYER_TYPE_POLICY.md` | Mandatory output-to-layer mapping policy. |
| `docs/engineering/BACKEND_OUTPUT_TRUTH_TABLE.md` | Mandatory backend-specific output expectations and proof requirements. |
| `docs/engineering/BACKEND_TO_TONY_LAYER_MAPPING_MATRIX.md` | Mandatory backend-to-Tony-layer mapping matrix. |
| `docs/engineering/TONY_EDIT_SAVE_EXPORT_PROOF_PLAN.md` | Mandatory proof gates for layer import, edit, save/load, and export. |
| `docs/engineering/UI_VISUAL_TRUTH_STATES.md` | Mandatory visible-state truth policy. |
| `docs/engineering/PROVENANCE_METADATA_POLICY.md` | Mandatory provenance policy for imported backend results. |
| `docs/engineering/REAL_RESULT_ACCEPTANCE_CHECKLIST.md` | Final checklist before any backend/UI/layer feature-complete claim. |
| `docs/06_EXECUTION_PLAN.md` | Approved implementation sequence and stage gates. |
| `docs/07_CODEX_TASK_LIST.md` | Approved small Codex task list. |
| `docs/adr/` | Architecture Decision Records when present. |

### Precedence order

If documents conflict, follow this precedence:

```text
1. User's current explicit task
2. docs/00_PROJECT_INDEX.md for document status and reading order
3. docs/03_SRS.md for testable requirements
4. docs/04_BACKEND_CONTRACT.md + docs/schemas/ for backend data boundaries
5. docs/05_TDD_ARCHITECTURE.md for technical design
6. docs/engineering/* truth/proof documents for real-result, layer, UI, provenance, save/load, and export rules
7. docs/06_EXECUTION_PLAN.md for implementation order
8. docs/02_PRD.md for product intent
9. docs/07_CODEX_TASK_LIST.md for task prompts
10. This AGENTS.md for coding-agent behavior rules
```

If a conflict is found:

1. Do not silently choose.
2. Report the conflict.
3. Propose the smallest safe correction.
4. Do not change requirements documents unless the task explicitly asks for that.

---

## Non-negotiable project rules

1. Do not rewrite Tony from scratch.
2. Do not break existing Tony functionality.
3. Do not break or remove the existing pYIN/Vamp workflow.
4. Do not implement fake production results, fake notes, fake confidence, fake progress, or fake completed states.
5. Do not claim a backend is integrated unless it actually runs and its real output is imported.
6. Do not add backend-specific shortcuts that bypass the common Backend Contract pipeline.
7. Do not integrate multiple new backends in one task unless the task explicitly says to do so.
8. Do not combine unrelated UI redesign, backend integration, and architecture refactoring in one task.
9. Do not introduce new production dependencies without documenting why, where, license impact, and build impact.
10. Do not add cloud/API behavior unless the task explicitly requests it.
11. Do not send audio, MIDI, project files, or logs to a network service by default.
12. Do not store API keys, credentials, tokens, secrets, or private paths in source code.
13. Do not make licensing, redistribution, or commercial-use claims without checking the relevant license/source.
14. Do not position the project as watermark removal, detector bypass, legal-clearing, or copyright-evasion software.
15. Do not silently change requirements to make implementation easier.
16. Do not create fake overlays or custom drawings and call them Tony layers.
17. Do not show Ready, Installed, Completed, Imported, Editable, Saved, or Exported unless the corresponding proof gate has passed.
18. Do not call a feature complete without evidence from the Engineering Gates.
19. Before UI, layer import, MainWindow/Analyser integration, selected-region replacement, save/load, or export work, inspect the real Tony/Sonic Visualiser source path first.
20. Use real Tony/Sonic Visualiser layers and models for imported results; do not bypass them with parallel fake representations.

---

## Work style for AI agents

### One task, one focused diff

Each task should produce one small, reviewable change.

Good:

```text
Add BackendManifest parser and validation tests.
```

Bad:

```text
Add Basic Pitch, redesign UI, implement compare mode, and clean architecture.
```

### Plan before complex work

For complex or ambiguous tasks:

1. Inspect the relevant code and docs.
2. Produce a short implementation plan.
3. Identify files likely to change.
4. Identify risks.
5. Only then edit.

### Ask only when blocked

Ask a clarifying question only if the task cannot be safely completed without it.

If a reasonable assumption is already documented in PRD/SRS/TDD/Execution Plan, use the documented assumption and mark it in the final report.

### Prefer discovery over invention

Do not invent:

- build commands;
- test commands;
- dependency names;
- Tony internal class names;
- backend output formats;
- supported platform claims;
- license compatibility claims.

Inspect the repository or relevant upstream documentation instead. If not verified, mark as `UNKNOWN` or `OPEN QUESTION`.

---

## Required workflow before coding

For every coding task:

1. Read the task prompt fully.
2. Identify the task ID from `docs/07_CODEX_TASK_LIST.md` when applicable.
3. Read only the relevant project documents for the task.
4. Inspect existing code before editing.
5. Identify whether the task is documentation-only, architecture-only, UI-only, backend-only, or test-only.
6. Make the smallest useful change.
7. Preserve existing Tony/pYIN behavior unless the task explicitly changes it.
8. Run the most relevant available verification.
9. Report exactly what changed, what was verified, what remains unresolved, and the next recommended task.
10. For UI/layer/import/save/export tasks, identify the highest Engineering Gate reached and do not claim beyond it.

If confirmed build/test commands are not yet documented, do not invent them. Use the build-discovery tasks from `docs/07_CODEX_TASK_LIST.md` first.

---

## Approved implementation sequence

Follow the staged implementation order unless the user explicitly changes it:

```text
Phase 0 — Documentation baseline
Phase 1 — Build original Tony unchanged
Phase 2 — Map existing Tony analysis/import/export code paths
Phase 3 — Add architecture skeleton
Phase 4 — Add UnifiedResult data structures and validation
Phase 4A — Engineering rules and Tony layer proof gates
Phase 5 — Add dev-only mock backend vertical slice
Phase 6 — Add ExternalProcessRunner
Phase 7 — Add Basic Pitch / NeuralNote path as first real backend
Phase 8 — Harden MVP
Phase 9 — Add CREPE Notes
Phase 10 — Add selected-region preview
Phase 11 — Add compare mode v1
Phase 12 — Add MUSC backend
Phase 13 — Add VioPTT backend and technique labels
Phase 14 — Add PESTO / PENN / FCPE f0 adapters
Phase 15 — Add future AI Copilot foundation
Phase 16 — Modernize UI after core architecture works
```

Do not skip directly to later phases just because later features are more exciting.

---

## Architecture invariants

These invariants must remain true throughout the project.

### 1. Tony remains the editor core

Tony's existing correction, annotation, and pYIN-based workflow are the foundation. New systems must integrate into that foundation.

### 2. Backends are interchangeable analysis engines

A backend is not the UI. A backend is not the editor. A backend produces data that is normalized into a UnifiedResult and imported into Tony layers.

### 3. UnifiedResult is the integration boundary

All backend outputs must be converted into the common result model before UI import.

### 4. UI cannot lie

The UI must never show a successful analysis state unless a real backend succeeded or a dev-only mock is explicitly labelled as such.

### 5. Local-first by default

Local audio processing is the default. Network/API features are future, optional, explicit, and user-confirmed.

---

## Engineering Gates for Real Results and Tony Layers

Before any task touches TonyLayerImporter, UI integration, MainWindow/Analyser integration, selected-region replacement, save/load, or export behavior, Codex must read and follow:

- `docs/engineering/REAL_RESULT_AND_TONY_LAYER_INTEGRATION_RULES.md`
- `docs/engineering/LAYER_TYPE_POLICY.md`
- `docs/engineering/BACKEND_OUTPUT_TRUTH_TABLE.md`
- `docs/engineering/BACKEND_TO_TONY_LAYER_MAPPING_MATRIX.md`
- `docs/engineering/TONY_EDIT_SAVE_EXPORT_PROOF_PLAN.md`
- `docs/engineering/UI_VISUAL_TRUTH_STATES.md`
- `docs/engineering/PROVENANCE_METADATA_POLICY.md`
- `docs/engineering/REAL_RESULT_ACCEPTANCE_CHECKLIST.md`

Mandatory rules:

- inspect the relevant Tony/Sonic Visualiser source before implementation;
- no fake states, fake results, fake overlays, or fake notes;
- use real Tony/Sonic Visualiser layers/models where layer import is claimed;
- path checks are not Ready/Installed;
- loaded UnifiedResult is not Imported;
- imported is not Editable unless edit proof exists;
- exported is not proven unless an export file is created and inspected;
- preserve pYIN behavior unless explicitly tasked otherwise.
- do not claim feature completion until `REAL_RESULT_ACCEPTANCE_CHECKLIST.md` evidence is reported.

Final reports for these tasks must state the highest proof gate reached.

After CODEX-087D, the next phase is proof work, not more documentation hardening:

```text
CODEX-088 - dev/mock backend end-to-end proof
CODEX-089 - UnifiedResult to real editable Tony NoteLayer proof
CODEX-090 - edit/save/load/export proof
```

---

## Backend integration pipeline

All real backends must follow this pipeline:

```text
BackendManifest
    ↓
Backend capability check
    ↓
BackendRequest
    ↓
Backend execution
    ↓
Raw backend output
    ↓
Adapter parser
    ↓
UnifiedResult
    ↓
Schema + semantic validation
    ↓
Tony layer import
    ↓
User correction / export
```

### BackendManifest rules

A backend must declare, at minimum:

- engine ID;
- display name;
- version when detectable;
- runtime type, such as Vamp, Python, executable, native, ONNX;
- installation status;
- supported analysis modes;
- supported input formats;
- supported outputs;
- required model/checkpoint status where applicable;
- whether it supports selected-region analysis;
- whether it supports pitch curve, notes, pitch bends, confidence, velocity, or technique labels.

### BackendRequest rules

A backend request must describe:

- input audio path;
- full-file or selected-region mode;
- selected region start/end when applicable;
- backend settings;
- output directory;
- expected output types;
- cancellation/timeout policy when applicable.

### UnifiedResult rules

Before importing any result into Tony:

1. Parse raw backend output into the internal UnifiedResult model.
2. Validate against the schema where applicable.
3. Run semantic checks:
   - note start time is before note end time;
   - note times are inside analyzed audio or selected region;
   - MIDI pitch is valid when present;
   - frequency is positive when present;
   - confidence values are normalized or clearly documented;
   - required fields are present;
   - technique labels are represented only when actually produced.
4. If validation fails, mark the result as invalid and do not import fake fallback notes.

---

## External backend execution rules

External backend execution must be safe, observable, cancellable, and debuggable.

Use a structured process runner. Do not use shell-concatenated command strings.

The runner must eventually support:

- executable/script path;
- argument list as structured array;
- working directory;
- environment variables when explicitly required;
- temporary workspace isolation;
- timeout;
- cancellation;
- stdout capture;
- stderr capture;
- exit-code capture;
- structured error object;
- cleanup policy;
- log file path when logs are enabled.

### External process security

Do not:

- concatenate user file paths into shell commands;
- assume paths are safe;
- run unverified scripts automatically;
- download models silently;
- execute network commands as part of local backend analysis unless explicitly requested;
- hide stderr from the user when a backend fails.

---

## Backend-specific guidance

### pYIN / Vamp

- Existing Tony method.
- Must remain available.
- Do not replace it with neural methods.
- New engines are alternatives, not replacements.

### Basic Pitch / NeuralNote

- Recommended first real backend path unless a later task changes that decision.
- Start with external execution or an adapter path.
- Native/ONNX integration can come later.
- Do not assume pitch-curve output unless verified from actual backend output.

### CREPE Notes

- Treat as monophonic note extraction from f0/pitch data.
- External backend first.
- Possible Vamp/native integration later.
- Do not treat it as a general polyphonic transcriber.

### MUSC Violin Transcription

- Treat as specialized violin transcription.
- Integrate after the process runner and basic backend adapter flow are stable.
- Do not overgeneralize its output to all instruments unless verified.

### VioPTT

- Treat as violin transcription plus technique-label output.
- Technique labels require a dedicated optional annotation layer.
- Do not invent technique labels when backend output lacks them.

### PESTO / PENN / FCPE

- Treat as f0/pitch backends, not complete workstation-level note editors.
- They require a shared note-segmentation layer before becoming full note-analysis engines.
- Keep their raw f0/confidence output available for debugging and visualization.

---

## UI rules

### Early UI additions allowed

Early UI work should be functional and minimal:

- analysis engine selector;
- backend status indicator;
- run/cancel button;
- progress/status area;
- error/warning panel;
- result layer list;
- dev-only mock label when applicable.

### Early UI work not allowed

Do not perform major UI redesign before backend architecture is functional.

Avoid:

- fake dashboards;
- fake analysis cards;
- decorative mock results;
- hiding backend failure details;
- AI panels before AI Copilot work is explicitly scheduled;
- changes that make Tony harder to use for existing workflows.

### Future UI direction

Future UI should be:

- modern;
- minimal;
- beginner-friendly;
- clear about backend state;
- usable in Simple and Advanced modes;
- dark/system-theme capable;
- explicit about preview layers versus accepted corrected layers.

---

## Result layer and correction rules

Backend outputs should first appear as result layers or preview layers, not silently overwrite the user's corrected layer.

Recommended layer types:

```text
CorrectedLayer
BackendResultLayer
PreviewRegionLayer
PitchCurveLayer
TechniqueLabelLayer
ConfidenceOverlay
```

Selected-region analysis must be non-destructive by default:

1. Analyze selected region.
2. Show preview result.
3. Let user accept, reject, or selectively copy notes.
4. Replace only notes inside the selected region if user explicitly applies the preview.
5. Preserve notes outside the selected region.

Compare mode must not auto-merge conflicting results without explicit user action.

---

## Testing and verification expectations

Each task must include the best available verification for its scope.

### For documentation-only tasks

Verify:

- paths are correct;
- document names match the repository;
- assumptions/open questions are marked;
- no fake implementation claims are made;
- links to schemas/examples are correct.

### For schema/contract tasks

Verify:

- JSON examples validate against schemas;
- invalid examples fail where expected if tests exist;
- schema changes are reflected in `docs/04_BACKEND_CONTRACT.md`.

### For backend adapter tasks

Verify:

- backend missing state;
- backend configured state;
- backend failure state;
- successful execution with a small test file when possible;
- stdout/stderr capture;
- output parsing;
- validation before import;
- no fake fallback output.

### For UI tasks

Verify manually:

- existing Tony workflow still opens;
- pYIN path is not removed;
- new controls are disabled or show meaningful status when backend is missing;
- failed analysis is visible and honest;
- successful analysis is only shown after real success.

### For code tasks

Run the documented build/test commands when known.

If build/test commands are unknown:

- do not invent them;
- inspect the repository;
- document what was discovered;
- recommend running the appropriate build-discovery task.

---

## Documentation update rules

Update documentation when behavior, architecture, contracts, or workflow changes.

| Change type | Update |
|---|---|
| Product behavior or scope | `docs/02_PRD.md` only when explicitly requested |
| Testable requirement | `docs/03_SRS.md` |
| Backend data shape | `docs/04_BACKEND_CONTRACT.md` + schemas/examples |
| Architecture | `docs/05_TDD_ARCHITECTURE.md` |
| Implementation order | `docs/06_EXECUTION_PLAN.md` and `docs/07_CODEX_TASK_LIST.md` |
| Repeated agent mistake | `AGENTS.md` |
| Major decision | `docs/adr/ADR-xxxx-*.md` |

Do not silently edit PRD/SRS to match a shortcut taken in code.

---

## Licensing rules

Maintain a license register for Tony and every backend.

Before bundling, redistributing, publishing, or commercializing anything, verify:

- source code license;
- model/checkpoint license;
- dataset/license warning published by the backend project;
- redistribution obligations;
- commercial-use restrictions;
- compatibility with the Tony fork distribution model.

If uncertain, write:

```text
OPEN QUESTION: license review required
```

Do not claim that commercial use is safe unless verified.

---

## Security and privacy rules

The project is local-first.

Required behavior:

- no automatic network access for local backends;
- no hidden uploads;
- no cloud/API feature unless explicitly requested;
- cloud/API features must be opt-in;
- API keys must be stored outside source code;
- logs must not include full audio content;
- privacy-sensitive logs should be user-clearable;
- external process execution must avoid shell injection risks;
- temporary files should be isolated and cleaned up according to policy.

Future AI Copilot rules:

- AI Copilot is not part of MVP unless explicitly scheduled;
- AI must explain suggestions;
- AI must show affected notes/regions;
- AI must wait for user confirmation before applying changes;
- AI must not silently overwrite user work;
- AI must not be marketed as legal clearance or watermark removal.

---

## Dependency rules

Before adding a production dependency, report:

- why it is needed;
- whether it is runtime or development-only;
- license;
- platform impact;
- packaging impact;
- whether a standard-library or existing-project alternative exists;
- whether it affects Windows build complexity.

Do not add heavy Python/ML dependencies directly into the Tony build unless the task explicitly requires it.

For early backend work, prefer external user-configured backend paths over bundling full Python environments.

---

## Build and test commands

Do not invent build or test commands.

Until build discovery is complete, use placeholders:

```text
Build command: UNKNOWN — must be discovered by CODEX-010 / CODEX-011.
Test command: UNKNOWN — must be discovered by CODEX-010 / CODEX-011.
Lint/static analysis command: UNKNOWN.
```

After discovery, update this section with verified commands only.

When commands become known, include:

- Windows build prerequisites;
- exact build command;
- exact clean build command if available;
- smoke-test command or manual launch instruction;
- schema validation command if added;
- test command if available.

---

## Error handling rules

Do not swallow errors.

Every backend failure should be visible in structured form:

```text
BackendNotConfigured
BackendMissing
DependencyMissing
ModelMissing
UnsupportedInput
ExecutionFailed
TimedOut
Cancelled
OutputMissing
OutputInvalid
ValidationFailed
ImportFailed
CompletedWithWarnings
```

A failed backend run must not create a fake successful result.

---

## Commit / diff hygiene

For each task:

- keep the diff focused;
- avoid unrelated formatting churn;
- avoid broad refactors unless explicitly requested;
- preserve public behavior unless changing it is the task;
- do not update generated/binary files unless required;
- do not commit temporary outputs, private logs, model files, API keys, or local machine paths.

---

## Definition of Done

A task is complete only when:

1. The requested scope is implemented or explicitly blocked.
2. No unrelated changes were made.
3. Existing Tony/pYIN behavior is preserved unless the task explicitly changed it.
4. New behavior follows PRD, SRS, Backend Contract, TDD, and Execution Plan.
5. No fake production output was added.
6. Validation/build/test/manual verification was run where available.
7. Documentation was updated when required.
8. Remaining limitations are reported honestly.
9. The next recommended task is identified.
10. For layer/import/UI/save/export work, the Engineering Gates were followed and evidence was reported.

---

## Required final response format for Codex

At the end of every task, report:

```text
Summary:
- ...

Files changed:
- ...

Verification:
- ...

Known limitations / open questions:
- ...

Next recommended task:
- CODEX-...
```

Keep reports factual. Do not overstate success. Do not say something is fully working unless it was actually verified.
