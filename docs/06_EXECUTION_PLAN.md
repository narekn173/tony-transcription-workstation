# Execution Plan v0.1 — Tony Fork Transcription Workstation

## 1. Document control

| Field | Value |
|---|---|
| Document | Execution Plan |
| Version | v0.1 |
| Product | Tony Fork Transcription Workstation |
| Source documents | `docs/02_PRD.md`, `docs/03_SRS.md`, `docs/04_BACKEND_CONTRACT.md`, `docs/05_TDD_ARCHITECTURE.md` |
| Companion document | `docs/07_CODEX_TASK_LIST.md` |
| Status | Canonical v0.1 — reviewed/stabilized |
| Primary platform | Windows |
| Primary coding workflow | Small scoped Codex tasks, one reviewable diff at a time |
| Primary implementation strategy | Preserve Tony first; add external backends through staged architecture |

---

## 2. Purpose

This Execution Plan converts the PRD, SRS, Backend Contract, and TDD into a practical implementation roadmap for Codex / vibe-coding work.

It exists to prevent the most likely failure modes:

- rewriting Tony from scratch;
- mixing UI redesign with backend integration;
- adding fake analysis output;
- integrating research backends before the infrastructure is ready;
- allowing Codex to make broad uncontrolled changes;
- losing traceability between product goals, requirements, architecture, and code tasks.

The plan defines:

- implementation phases;
- stage gates;
- task dependencies;
- review boundaries;
- Codex task rules;
- Definition of Done;
- a safe sequence from original Tony build to first real backend integration.

---

## 3. Source-of-truth documents

Codex and any human contributor must treat these documents as the current project contract:

| Document | Role |
|---|---|
| `docs/02_PRD.md` | Product intent: what the product is and why it exists. |
| `docs/03_SRS.md` | Testable software requirements. |
| `docs/04_BACKEND_CONTRACT.md` | Contract between Tony host and all analysis backends. |
| `docs/schemas/*.schema.json` | Machine-checkable schemas for backend manifest, request, and result. |
| `docs/05_TDD_ARCHITECTURE.md` | Technical architecture and component design. |
| `docs/06_EXECUTION_PLAN.md` | Implementation sequence and stage gates. |
| `docs/07_CODEX_TASK_LIST.md` | Ready-to-use Codex task prompts. |
| `AGENTS.md` | Persistent repository rules for Codex. Created and part of the approved implementation-control set. |

If a future implementation task conflicts with these documents, the task must pause and update the relevant document first.

---

## 4. External execution-planning guidance used

This plan follows current AI-coding workflow principles:

1. Complex work should be planned before coding. OpenAI Codex guidance recommends using Plan mode for complex or ambiguous tasks and using a `PLANS.md`/execution-plan template for longer multi-step work.
2. Reusable coding-agent guidance should be moved into `AGENTS.md`. OpenAI describes `AGENTS.md` as repository-level guidance covering layout, build/test commands, conventions, constraints, and Definition of Done.
3. Long execution plans should be self-contained enough that an agent can continue from the current working tree and the plan, without hidden memory.
4. GitHub Copilot repository instructions should provide project-specific guidance about how to understand, build, test, and validate changes.

References:

- OpenAI Codex best practices: https://developers.openai.com/codex/learn/best-practices
- OpenAI Codex ExecPlans: https://developers.openai.com/cookbook/articles/codex_exec_plans
- OpenAI Codex AGENTS.md guide: https://developers.openai.com/codex/guides/agents-md
- GitHub Copilot repository custom instructions: https://docs.github.com/en/copilot/how-tos/copilot-on-github/customize-copilot/add-custom-instructions/add-repository-instructions

---

## 5. Non-negotiable execution principles

### EP-001 — Preserve Tony first

No task may break existing Tony behavior intentionally. Existing pYIN workflow, loading, display, editing, and export behavior must remain intact unless a later task explicitly and safely changes it.

### EP-002 — Build before features

The original Tony fork must build unchanged before any feature task begins.

### EP-003 — Map before modifying

Codex must map real Tony source paths before implementing architecture components. The TDD uses adapter-level names until Milestone 2 verifies the actual codebase.

### EP-004 — Contract-first integration

Backends must be integrated through the Backend Contract. Any backend-specific output must be normalized to `UnifiedResult` before display/import.

### EP-005 — Real results only

Production features must never show fake notes, fake confidence, fake completion, fake backend availability, or fake analysis output.

### EP-006 — Mock is dev-only

A mock backend is allowed only to test architecture. It must be clearly labeled development-only and must never appear as a real production backend.

### EP-007 — One task, one controlled diff

Each Codex task should produce a small reviewable diff. Do not combine backend integration, UI redesign, export logic, and architecture refactoring in one task.

### EP-008 — Sequential backend execution first

Multi-engine compare must run backends sequentially by default, not concurrently, to protect CPU/RAM on the target Windows laptop.

### EP-009 — Local-first by default

No backend or AI-related task may introduce hidden network access. Cloud/API behavior is future scope and must be opt-in.

### EP-010 — Documentation changes when behavior changes

If a task changes behavior, interface, schema, status states, or workflow, it must update the relevant documentation.

---

## 6. Workstream model

Implementation is split into independent workstreams so Codex does not mix concerns.

| Workstream | Scope | Starts after |
|---|---|---|
| W0 Documentation baseline | Docs, schemas, prompts, AGENTS.md | Now |
| W1 Build and codebase mapping | Build original Tony, map source paths | W0 |
| W2 Architecture skeleton | Manager, registry, manifest loading, settings | W1 |
| W3 Contract and validation | Schema validation, parsers, example fixtures | W2 |
| W4 Dev mock vertical slice | Mock adapter → UnifiedResult → Tony result layer | W3 |
| W5 External process infrastructure | Runner, temp dirs, timeout, cancellation, logs | W3/W4 |
| W6 First real backend | Basic Pitch / NeuralNote path | W5 |
| W7 Export and MVP closure | Corrected layer export MIDI/CSV, MVP validation | W6 |
| W8 Second backend | CREPE Notes | W6/W7 |
| W9 Region workflow | selected-region preview/apply/reject | W4/W5 |
| W10 Compare workflow | sequential multi-engine result layers | W6/W8/W9 |
| W11 Specialized backends | MUSC, VioPTT, PESTO/PENN/FCPE | W10 |
| W12 Future AI Copilot | opt-in AI assistant, not MVP | W10+ |
| W13 UI modernization | Simple/Advanced, theme, polish | after MVP architecture is stable |

---

## 7. Stage gates

A phase cannot start until the previous gate passes.

| Gate | Required before proceeding |
|---|---|
| G0 Docs ready | PRD, SRS, Backend Contract, TDD, Execution Plan, Codex Task List exist. |
| G1 Build ready | Original Tony fork builds and launches unchanged. |
| G2 Codebase mapped | Analysis path, pYIN call path, layer creation, import/export, menus, settings, undo/redo are documented. |
| G3 Architecture skeleton builds | Manager/registry/settings skeleton compiles without behavior changes. |
| G4 Contract validation works | Example manifest/request/result schemas validate in tests or tooling. |
| G5 Mock vertical slice works | Dev-only mock result can be displayed as a Tony result layer. |
| G6 Runner works | Controlled external process can run/cancel/fail safely. |
| G7 First backend works | Basic Pitch result can be run, validated, displayed, corrected, and exported. |
| G8 MVP ready | Existing pYIN works, first backend works, errors are visible, MIDI/CSV export works. |
| G9 Expansion ready | CREPE Notes added and compare/region infrastructure stable. |

---

## 8. Phase roadmap

### Phase 0 — Documentation and agent setup

Goal: create stable instructions before coding.

Deliverables:

- `docs/02_PRD.md`
- `docs/03_SRS.md`
- `docs/04_BACKEND_CONTRACT.md`
- `docs/05_TDD_ARCHITECTURE.md`
- `docs/06_EXECUTION_PLAN.md`
- `docs/07_CODEX_TASK_LIST.md`
- `AGENTS.md`
- optional `.github/copilot-instructions.md`
- optional `docs/adr/`

Exit criteria:

- Documents are committed.
- Codex rules are in `AGENTS.md`.
- The first coding task references these docs.

### Phase 1 — Build original Tony unchanged

Goal: establish a clean baseline before modifications.

Tasks:

- clone/fork Tony;
- document dependencies;
- build on Windows;
- launch unchanged application;
- verify existing pYIN workflow if possible;
- create `docs/build/BUILD_WINDOWS.md` and `docs/build/BASELINE_BUILD_LOG.md`.

Exit criteria:

- Original application builds.
- Known build issues are documented.
- No feature code has been added.

### Phase 2 — Codebase mapping

Goal: map the actual Tony source tree before making architecture changes.

Map:

- application entry points;
- main window/menu/action system;
- existing pYIN analysis trigger;
- transform/analysis pipeline;
- note layer creation;
- pitch layer creation;
- import/export code;
- settings persistence;
- undo/redo or command stack;
- logging/error display patterns.

Deliverable:

- `docs/engineering/CODEBASE_MAP.md`

Exit criteria:

- Real file paths are identified.
- TDD adapter names are mapped to real integration points.
- No large feature implementation yet.

### Phase 3 — Architecture skeleton

Goal: create minimal architecture without changing production behavior.

Components:

- `AnalysisEngineManager` concept mapped into real Tony paths;
- `BackendRegistry`;
- `BackendManifestLoader`;
- `BackendSettingsStore`;
- minimal capability model.

Exit criteria:

- Code compiles.
- Existing Tony/pYIN behavior unchanged.
- No backend runs yet.

### Phase 4 — Contract validation and data model

Goal: implement enough of the Backend Contract to safely accept results.

Components:

- `UnifiedResult` internal model;
- `NoteEvent`, `PitchPoint`, `TechniqueLabel`, `Warning/Error` structures;
- schema/example validation strategy;
- parser/validator failure states.

Exit criteria:

- Valid example UnifiedResult parses.
- Invalid example fails safely.
- No invalid backend result can be imported silently.

### Phase 5 — Dev-only mock vertical slice

Goal: prove result import and display without ML backend complexity.

Components:

- dev-only mock adapter;
- reads `docs/examples/unified_result_minimal.example.json` or equivalent fixture;
- imports into a non-destructive result layer;
- labels UI as development/test only.

Exit criteria:

- Mock output appears visually.
- Mock cannot be confused with real analysis.
- Existing Tony remains stable.

### Phase 6 — External process infrastructure

Goal: safely run external CLI backends.

Components:

- `ExternalProcessRunner`;
- process launch;
- stdout/stderr capture;
- exit code handling;
- timeout;
- cancellation;
- temp workspace;
- structured errors.

Exit criteria:

- Controlled test command can succeed, fail, timeout, and cancel.
- UI receives correct status updates.

### Phase 7 — First real backend: Basic Pitch / NeuralNote path

Goal: complete MVP vertical slice using the first real backend.

Implementation notes:

- Basic Pitch is recommended first because it has CLI/output paths and Windows ONNX support in its project documentation.
- Do not integrate all backends yet.
- Do not redesign UI yet.

Exit criteria:

- Basic Pitch backend status appears.
- User can configure path/environment.
- Full-file analysis runs.
- Output is validated and imported.
- Notes are displayed.
- User can manually correct and export MIDI/CSV.

### Phase 8 — MVP hardening

Goal: stabilize before adding more backends.

Tasks:

- error handling review;
- no-fake-state audit;
- regression test pYIN workflow;
- logs/privacy review;
- small UX cleanup only.

Exit criteria:

- MVP success criteria from PRD/SRS pass.

### Phase 9 — CREPE Notes backend

Goal: add second real backend for monophonic material.

Exit criteria:

- CREPE Notes backend can run full-file analysis.
- Monophonic limitations are visible in UI/help text.
- Result imports through the same UnifiedResult pipeline.

### Phase 10 — Selected-region preview

Goal: enable non-destructive region analysis.

Exit criteria:

- User selects a region.
- Backend runs only on that region.
- Preview layer appears.
- Apply/reject/copy behavior works.
- Notes outside region are not changed.

### Phase 11 — Compare mode v1

Goal: compare pYIN, Basic Pitch, and CREPE Notes.

Exit criteria:

- User selects compatible engines.
- Backends run sequentially.
- Each result becomes a separate result layer.
- User can choose a whole result or copy selected notes.

### Phase 12 — Specialized backends

Goal: add backends that require additional visualization or semantics.

Order:

1. MUSC Violin Transcription;
2. VioPTT technique-aware backend;
3. PESTO/PENN/FCPE f0-only workflows + shared segmentation layer.

Exit criteria:

- Each backend declares capabilities correctly.
- Technique labels and f0-only outputs are represented without fake fields.
- Unsupported functions are disabled or clearly marked.

### Phase 13 — Future AI Copilot foundation

Goal: add AI-ready architecture without turning AI into the transcription engine.

Exit criteria:

- AI panel is opt-in.
- No hidden cloud upload.
- AI suggestions require confirmation.
- AI receives symbolic data by default unless future user settings explicitly allow audio.

### Phase 14 — UI modernization

Goal: improve visual design after stable functionality exists.

Exit criteria:

- Simple/Advanced modes work.
- Dark/system theme works if supported by technical stack.
- No backend behavior is changed by styling work.

---

## 9. Codex task rules

Each Codex task must include:

1. exact goal;
2. documents to read first;
3. allowed files/areas;
4. forbidden changes;
5. acceptance criteria;
6. verification commands;
7. required final report.

Codex must not:

- implement multiple phases at once;
- rewrite large Tony subsystems without explicit approval;
- add dependencies without documenting rationale;
- add fake UI output;
- silently bypass schema validation;
- change licensing headers or upstream notices without explicit review;
- claim a backend is supported before it actually runs and imports real output.

---

## 10. Branch and commit strategy

Recommended branch style:

```text
main
feature/docs-baseline
feature/build-baseline
feature/codebase-map
feature/backend-contract-validation
feature/mock-backend-vertical-slice
feature/external-process-runner
feature/basic-pitch-adapter
feature/crepe-notes-adapter
feature/region-preview
feature/compare-v1
```

Commit style:

```text
docs: add execution plan
build: document Windows baseline build
arch: add backend registry skeleton
contract: add unified result validator
backend: add dev-only mock adapter
backend: add Basic Pitch external adapter
ui: add analysis engine selector
```

Each pull request/diff should correspond to one Codex task or a small group of tightly related tasks.

---

## 11. Verification strategy

### Minimal verification per task

Every Codex task must report:

```text
- build command attempted
- test command attempted
- whether existing Tony behavior was touched
- files changed
- manual verification steps
- known limitations
```

### Task categories

| Task type | Verification |
|---|---|
| Docs-only | Markdown renders, links/paths valid, no contradiction with PRD/SRS/TDD. |
| Build setup | Clean build command documented and reproducible. |
| Architecture skeleton | Project compiles, existing behavior unchanged. |
| Parser/validator | Valid examples pass; invalid examples fail. |
| UI integration | No fake states; existing actions remain. |
| Backend adapter | Real backend output created, validated, imported. |
| Region workflow | Non-destructive behavior manually verified. |
| Compare mode | Sequential execution and separate layers verified. |

---

## 12. Risk control checklist

Before each new backend:

- Is the backend license recorded?
- Does it require Python, ONNX, CUDA, or model checkpoints?
- Does it produce notes, f0, pitch bends, labels, or only raw output?
- Is there a documented output parser?
- Can it run on CPU?
- Does it require internet? If yes, reject for local-first MVP unless explicitly approved.
- Can failure be detected reliably?
- Can output be validated before import?

Before UI changes:

- Is functionality already real?
- Is the UI state honest?
- Does the change affect existing Tony workflows?
- Can beginner mode hide advanced complexity?

Before AI work:

- Is the feature post-MVP?
- Is it opt-in?
- Does it avoid hidden audio upload?
- Are API keys stored outside source code?
- Does the user confirm changes before applying?

---

## 13. Definition of Done

A task is done only if:

- acceptance criteria are satisfied;
- project builds or build failure is explicitly unrelated and documented;
- tests/manual checks are run or clearly blocked;
- no unrelated refactor is included;
- docs are updated if behavior changed;
- no fake production result is introduced;
- existing pYIN/Tony workflow is not broken;
- Codex final report lists files changed, commands run, limitations, and next recommended task.

---

## 14. Stop conditions

Codex should stop and report instead of guessing when:

- original Tony does not build;
- real source paths cannot be identified;
- a backend output format differs from expected contract;
- a dependency requires unclear licensing review;
- a proposed change would break existing Tony behavior;
- a task requires cloud/API access before AI Copilot phase;
- a backend requires GPU-only execution for MVP;
- schema validation fails and cannot be fixed without changing the contract.

---

## 15. Next action after this plan

`AGENTS.md` now exists as part of the implementation-control set.

Next action depends on the actual repository state:

```text
If docs are not yet copied into the Tony fork repository:
    run CODEX-000.

If AGENTS.md is not yet copied into the repository:
    run CODEX-001.

If docs and AGENTS.md are already present:
    start CODEX-010: inspect repository build system.
```
