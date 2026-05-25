# Project Index v0.2 — Tony Fork Transcription Workstation

**Status:** Canonical master project map  
**Last updated:** 2026-05-15  
**Purpose:** Defines the authoritative documents, reading order, current implementation-control files, and immediate next step for the Tony Fork Transcription Workstation project.

---

## 1. Project identity

**Working name:** Tony Fork Transcription Workstation  
**Base application:** Tony / Sonic Visualiser ecosystem  
**Primary platform:** Windows first  
**Processing model:** Local-first  
**Implementation strategy:** Preserve Tony first; add modern analysis engines through staged backend architecture.

Core workflow:

```text
Audio → choose backend → real analysis → visual notes / pitch / confidence → compare / correct → export MIDI / CSV
```

Target engine architecture:

```text
[ pYIN Vamp plugin ]
    existing Tony method; must remain working

[ Basic Pitch / NeuralNote ]
    first real backend candidate; external or native ONNX later

[ CREPE Notes ]
    external backend first; possible Vamp/native later

[ MUSC Violin Transcription ]
    external backend

[ VioPTT Violin Technique-Aware ]
    external backend with technique labels

[ PESTO / PENN / FCPE ]
    f0 backends + shared note segmentation layer
```

---

## 2. Canonical documents

These files are the current source-of-truth set. Read them in this order.

| Order | File | Status | Purpose |
|---:|---|---|---|
| 1 | `docs/00_PROJECT_INDEX.md` | Canonical v0.2 | Master map, document status, reading order, next step |
| 2 | `docs/02_PRD.md` | Canonical v0.1 | Product vision, goals, MVP, post-MVP roadmap |
| 3 | `docs/03_SRS.md` | Canonical v0.1 | Testable software requirements |
| 4 | `docs/04_BACKEND_CONTRACT.md` | Canonical v0.1 | Backend data contract and integration boundary |
| 5 | `docs/05_TDD_ARCHITECTURE.md` | Canonical v0.1 | Technical architecture blueprint |
| 6 | `docs/engineering/REAL_RESULT_AND_TONY_LAYER_INTEGRATION_RULES.md` | Mandatory engineering rules | No fake UI/results/layers; real Tony proof chain |
| 7 | `docs/engineering/LAYER_TYPE_POLICY.md` | Mandatory engineering rules | Output-to-layer mapping policy |
| 8 | `docs/engineering/BACKEND_OUTPUT_TRUTH_TABLE.md` | Mandatory engineering rules | Backend-specific output truth table |
| 9 | `docs/engineering/BACKEND_TO_TONY_LAYER_MAPPING_MATRIX.md` | Mandatory engineering rules | Backend output to real Tony layer mapping matrix |
| 10 | `docs/engineering/BASIC_PITCH_ADAPTER_AUDIT.md` | Mandatory engineering audit | Basic Pitch CLI/artifact facts and conservative adapter contract |
| 11 | `docs/engineering/BASIC_PITCH_OUTPUT_CONVERSION_PLAN.md` | Mandatory engineering audit | Basic Pitch fixture-backed CSV-to-UnifiedResult conversion boundary |
| 12 | `docs/engineering/BASIC_PITCH_REAL_ARTIFACT_DISCOVERY.md` | Mandatory engineering audit | Manual/test-only Basic Pitch artifact discovery harness |
| 13 | `docs/engineering/BASIC_PITCH_REAL_ARTIFACT_TO_UNIFIED_RESULT_PROOF.md` | Mandatory engineering audit | Manual/test-only Basic Pitch artifact-to-UnifiedResult proof |
| 14 | `docs/engineering/BASIC_PITCH_UNIFIED_RESULT_JSON_HANDOFF_PROOF.md` | Mandatory engineering audit | Manual/test-only Basic Pitch result.json handoff and loader/reporter proof |
| 15 | `docs/engineering/BASIC_PITCH_REAL_RUN_HANDOFF_PROOF.md` | Mandatory engineering audit | Manual opt-in Basic Pitch real-run-to-result.json handoff proof |
| 16 | `docs/engineering/BASIC_PITCH_RESULT_TO_TONY_LAYER_PROOF.md` | Mandatory engineering audit | Manual/test-only Basic Pitch result.json-to-real-Tony-layer proof |
| 17 | `docs/engineering/BASIC_PITCH_LAYER_SAVE_LOAD_EXPORT_PROOF.md` | Mandatory engineering audit | Manual/test-only Basic Pitch layer save/load/export proof |
| 18 | `docs/engineering/PROOF_BUNDLE_POLICY.md` | Mandatory engineering rules | Evidence bundle and truth-state rules for debug/UI workflows |
| 19 | `docs/engineering/BASIC_PITCH_DEBUG_WORKFLOW_PROOF.md` | Mandatory engineering audit | Debug-only Basic Pitch workflow coordinator and proof bundle |
| 20 | `docs/engineering/BASIC_PITCH_DEBUG_UI_CONSUMPTION_MODEL.md` | Mandatory engineering audit | Compile-only UI-consumption truth-state adapter |
| 21 | `docs/engineering/BASIC_PITCH_MAINWINDOW_DEBUG_UI_INTEGRATION_PLAN.md` | Mandatory engineering audit | Source audit and plan for future debug-only MainWindow consumption |
| 22 | `docs/engineering/BASIC_PITCH_MAINWINDOW_DEBUG_UI_ACTION_PROOF.md` | Mandatory engineering audit | First debug-only MainWindow action proof and limitations |
| 23 | `docs/engineering/BASIC_PITCH_DEBUG_CONFIGURATION_ENTRY_PROOF.md` | Mandatory engineering audit | Debug-only Basic Pitch manual-run configuration/status boundary |
| 24 | `docs/engineering/BASIC_PITCH_DEBUG_MANUAL_RUN_ACTION_PROOF.md` | Mandatory engineering audit | Explicit debug-only Basic Pitch manual-run handoff action boundary |
| 25 | `docs/engineering/BASIC_PITCH_DEBUG_POST_RUN_IMPORT_ACTION_PROOF.md` | Mandatory engineering audit | Explicit debug-only Basic Pitch result.json-to-real-layer import action boundary |
| 26 | `docs/engineering/BASIC_PITCH_DEBUG_COMBINED_RUN_IMPORT_ACTION_PROOF.md` | Mandatory engineering audit | Explicit debug-only Basic Pitch manual handoff plus import action boundary |
| 27 | `docs/engineering/BASIC_PITCH_DEBUG_POST_IMPORT_EDIT_SAVE_EXPORT_PROOF.md` | Mandatory engineering audit | Debug-only Basic Pitch post-import edit/save/load/export proof summary |
| 28 | `docs/engineering/BASIC_PITCH_PRODUCTION_READINESS_CHECKLIST.md` | Mandatory engineering gate | Guarded checklist before removing Basic Pitch debug-only label |
| 29 | `docs/engineering/BASIC_PITCH_DEBUG_TO_PRODUCTION_TRANSITION_PLAN.md` | Mandatory engineering plan | Safe sequence from debug Basic Pitch proof path to guarded user workflow |
| 30 | `docs/engineering/TONY_EDIT_SAVE_EXPORT_PROOF_PLAN.md` | Mandatory engineering rules | Proof gates for import/edit/save/export claims |
| 31 | `docs/engineering/UI_VISUAL_TRUTH_STATES.md` | Mandatory engineering rules | Honest visible backend/result UI states |
| 32 | `docs/engineering/PROVENANCE_METADATA_POLICY.md` | Mandatory engineering rules | Provenance requirements for imported results |
| 33 | `docs/engineering/REAL_RESULT_ACCEPTANCE_CHECKLIST.md` | Mandatory engineering rules | Final acceptance checklist for feature-complete claims |
| 34 | `docs/06_EXECUTION_PLAN.md` | Canonical v0.1 | Implementation phases, stage gates, review rules |
| 35 | `docs/07_CODEX_TASK_LIST.md` | Canonical v0.1 | Safe small Codex tasks in implementation order |
| 36 | `AGENTS.md` | Canonical v0.1 | Persistent coding-agent rules |

### Interpretation

```text
docs/02_PRD.md
    = what product should exist and why

docs/03_SRS.md
    = what the software must do in testable terms

docs/04_BACKEND_CONTRACT.md
    = how Tony host/core and backends communicate

docs/05_TDD_ARCHITECTURE.md
    = how the system is technically structured

docs/engineering/*.md
    = mandatory proof rules before layer/import/UI/save/export work

docs/06_EXECUTION_PLAN.md
    = in what order implementation should proceed

docs/07_CODEX_TASK_LIST.md
    = exact small task prompts for Codex

AGENTS.md
    = standing rules for AI coding agents
```

---

## 3. Contract-supporting artifacts

These files support `docs/04_BACKEND_CONTRACT.md` and should be treated as normative when implementing backend integration.

| File | Status | Purpose |
|---|---|---|
| `docs/schemas/backend_manifest.schema.json` | Contract support | Validates backend capability manifests |
| `docs/schemas/backend_request.schema.json` | Contract support | Validates backend execution requests |
| `docs/schemas/unified_result.schema.json` | Contract support | Validates backend analysis results |
| `docs/examples/backend_manifest_basic_pitch.example.json` | Example | Valid example Basic Pitch manifest |
| `docs/examples/backend_request_full_file.example.json` | Example | Valid example full-file backend request |
| `docs/examples/unified_result_minimal.example.json` | Example | Minimal valid UnifiedResult example |
| `docs/diagrams/05_context_diagram.md` | Supporting | Context/architecture diagram |

Rule:

```text
Backends must not bypass the Backend Contract.
Every real backend output must be normalized to UnifiedResult before Tony layer import.
```

---

## 3A. Mandatory engineering proof documents

These documents are required reading before TonyLayerImporter, UI integration, MainWindow/Analyser integration, real backend workflow, selected-region replacement, save/load, or export work.

| File | Mandatory before | Purpose |
|---|---|---|
| `docs/engineering/REAL_RESULT_AND_TONY_LAYER_INTEGRATION_RULES.md` | All importer/UI/backend workflow claims | Defines no-fake rules and real end-to-end proof chain |
| `docs/engineering/LAYER_TYPE_POLICY.md` | TonyLayerImporter and display work | Defines how notes, f0, bends, confidence, labels, and annotations map to Tony concepts |
| `docs/engineering/BACKEND_OUTPUT_TRUTH_TABLE.md` | Backend adapter and importer work | Defines backend-specific expected outputs, risks, and proof |
| `docs/engineering/BACKEND_TO_TONY_LAYER_MAPPING_MATRIX.md` | Backend adapter and importer work | Defines allowed, deferred, and forbidden backend-to-layer mappings |
| `docs/engineering/BASIC_PITCH_ADAPTER_AUDIT.md` | Basic Pitch adapter, manifest, request, and converter work | Defines verified Basic Pitch CLI/artifact facts, deferred claims, and contract boundaries |
| `docs/engineering/BASIC_PITCH_OUTPUT_CONVERSION_PLAN.md` | Basic Pitch output parser and converter work | Defines fixture-backed CSV-to-UnifiedResult mapping and limitations |
| `docs/engineering/BASIC_PITCH_REAL_ARTIFACT_DISCOVERY.md` | Basic Pitch real artifact discovery and conversion work | Defines opt-in discovery, artifact classification, and no-fake-success constraints |
| `docs/engineering/BASIC_PITCH_REAL_ARTIFACT_TO_UNIFIED_RESULT_PROOF.md` | Basic Pitch artifact conversion work | Defines discovered note-events CSV selection and in-memory UnifiedResult conversion proof |
| `docs/engineering/BASIC_PITCH_UNIFIED_RESULT_JSON_HANDOFF_PROOF.md` | Basic Pitch result-file handoff work | Defines artifact-to-result.json-to-loader/reporter proof and limitations |
| `docs/engineering/BASIC_PITCH_REAL_RUN_HANDOFF_PROOF.md` | Manual Basic Pitch real-run proof | Defines explicit env opt-in, real process execution, artifact discovery, and result.json handoff verification |
| `docs/engineering/BASIC_PITCH_RESULT_TO_TONY_LAYER_PROOF.md` | Basic Pitch result-to-layer proof | Defines manual/test-only Basic Pitch-shaped result.json loading and real Tony/SV NoteLayer import proof |
| `docs/engineering/BASIC_PITCH_LAYER_SAVE_LOAD_EXPORT_PROOF.md` | Basic Pitch layer persistence/export proof | Defines manual/test-only Basic Pitch-shaped imported NoteLayer save/load/export proof |
| `docs/engineering/PROOF_BUNDLE_POLICY.md` | Debug workflow and future UI integration | Defines evidence bundle contents and forbids fake progress/state claims |
| `docs/engineering/BASIC_PITCH_DEBUG_WORKFLOW_PROOF.md` | Debug Basic Pitch workflow integration | Defines debug-only Basic Pitch workflow truth states and proof-bundle reporting |
| `docs/engineering/BASIC_PITCH_DEBUG_UI_CONSUMPTION_MODEL.md` | Debug Basic Pitch UI consumption | Defines compile-only user-visible truth-state and action enablement mapping |
| `docs/engineering/BASIC_PITCH_MAINWINDOW_DEBUG_UI_INTEGRATION_PLAN.md` | Debug Basic Pitch MainWindow planning | Defines source-audited future MainWindow insertion points and forbidden UI claims |
| `docs/engineering/BASIC_PITCH_MAINWINDOW_DEBUG_UI_ACTION_PROOF.md` | Debug Basic Pitch MainWindow action work | Defines the first debug-only action boundary, displayed fields, and remaining limitations |
| `docs/engineering/BASIC_PITCH_DEBUG_CONFIGURATION_ENTRY_PROOF.md` | Debug Basic Pitch manual-run configuration work | Defines manual-run env/config requirements, missing-key reporting, and skipped/allowed status |
| `docs/engineering/BASIC_PITCH_DEBUG_MANUAL_RUN_ACTION_PROOF.md` | Debug Basic Pitch manual-run action work | Defines the explicit user-triggered manual handoff action and its no-import/no-production limits |
| `docs/engineering/BASIC_PITCH_DEBUG_POST_RUN_IMPORT_ACTION_PROOF.md` | Debug Basic Pitch post-run import action work | Defines existing result.json loading and real Tony/SV layer import from an explicit debug action |
| `docs/engineering/BASIC_PITCH_DEBUG_COMBINED_RUN_IMPORT_ACTION_PROOF.md` | Debug Basic Pitch combined run/import action work | Defines explicit manual handoff plus real Tony/SV layer import from one debug-only action |
| `docs/engineering/BASIC_PITCH_DEBUG_POST_IMPORT_EDIT_SAVE_EXPORT_PROOF.md` | Debug Basic Pitch post-import proof work | Defines edit/undo/save/load/export proof summary reporting for imported Basic Pitch layers |
| `docs/engineering/BASIC_PITCH_PRODUCTION_READINESS_CHECKLIST.md` | Basic Pitch production transition work | Defines gates that must pass before removing debug-only labels or claiming production Basic Pitch support |
| `docs/engineering/BASIC_PITCH_DEBUG_TO_PRODUCTION_TRANSITION_PLAN.md` | Basic Pitch production transition work | Defines the safe CODEX-115 through CODEX-122 transition sequence |
| `docs/engineering/TONY_EDIT_SAVE_EXPORT_PROOF_PLAN.md` | Edit/save/load/export claims | Defines proof gates and required evidence |
| `docs/engineering/UI_VISUAL_TRUTH_STATES.md` | UI state/status work | Defines honest visible states and forbidden claims |
| `docs/engineering/PROVENANCE_METADATA_POLICY.md` | Imported result layers and persistence | Defines required provenance metadata and privacy rules |
| `docs/engineering/REAL_RESULT_ACCEPTANCE_CHECKLIST.md` | All backend/UI/layer completion claims | Defines the final evidence checklist before feature-complete claims |

Rule:

```text
No task may claim imported, editable, saved, exported, ready, installed, completed, or feature complete unless the matching proof gate and final acceptance checklist item has passed.
```

---

## 4. ADRs

Architecture decisions are stored in `docs/adr/`.

| ADR | Status | Decision |
|---|---|---|
| `docs/adr/ADR-0001-use-tony-fork-not-new-app.md` | Accepted | Use Tony fork instead of building a new application from scratch |
| `docs/adr/ADR-0002-external-backends-first.md` | Accepted | Use external backends first |
| `docs/adr/ADR-0003-unified-result-contract.md` | Accepted | Normalize all backend output to UnifiedResult |
| `docs/adr/ADR-0004-basic-pitch-first-real-backend.md` | Accepted | Use Basic Pitch / NeuralNote path as first real backend |
| `docs/adr/ADR-0005-local-first-ai-copilot-opt-in.md` | Accepted | Keep local-first processing; AI Copilot is future opt-in |

Rule:

```text
If a major architectural decision changes, create a new ADR that supersedes the old one.
Do not silently change architecture in code without updating ADRs/docs.
```

---

## 5. Prompt/reference files

Prompt files are not product requirements. They are regeneration tools.

| File | Status | Use |
|---|---|---|
| `docs/03_SRS_PROMPT.md` | Prompt/reference | Use only to regenerate/update SRS |
| `docs/04_BACKEND_CONTRACT_PROMPT.md` | Prompt/reference | Use only to regenerate/update Backend Contract |
| `docs/05_TDD_ARCHITECTURE_PROMPT.md` | Prompt/reference | Use only to regenerate/update TDD |
| `docs/06_EXECUTION_PLAN_PROMPT.md` | Prompt/reference | Use only to regenerate/update Execution Plan |
| `docs/08_AGENTS_PROMPT.md` | Prompt/reference | Use only to regenerate/update AGENTS.md |

Rule:

```text
Do not give prompt files to Codex as source-of-truth unless the task is specifically to update that document.
```

---

## 6. Legacy and snapshot files

| File type | Status | Rule |
|---|---|---|
| `Интеграция crepe_notes в Tony.txt` | Historical/context | Useful background only; not source-of-truth |
| `*.zip` packages | Delivery snapshots | Individual files in `/docs` and root `AGENTS.md` are authoritative |

Rule:

```text
If a ZIP and an individual file differ, use the individual file unless a newer package is explicitly declared canonical.
```

---

## 7. Non-negotiable rules

These apply to all coding tasks.

```text
1. Do not rewrite Tony from scratch.
2. Do not break existing Tony/pYIN behavior.
3. Do not show fake production analysis results.
4. Do not claim perfect transcription.
5. Do not claim watermark removal, detector bypass, legal clearance, or copyright-evasion functionality.
6. Prefer local processing by default.
7. Use external backends first; native/Vamp ports can come later.
8. Validate backend outputs through the Backend Contract.
9. Treat selected-region changes as non-destructive preview first.
10. Keep manual correction central.
11. Do not store API keys or secrets in source code.
12. Do not introduce large dependencies without documentation and review.
13. Do not mix unrelated UI redesign, backend integration, and architecture refactoring in one task.
```

---

## 8. MVP interpretation

The MVP is a stable vertical slice, not the complete final application.

MVP target:

```text
Original Tony builds unchanged
Existing pYIN still works
Backend infrastructure exists
At least one real external backend runs from UI
Real output is imported into Tony layers
User can manually correct notes
User can export MIDI / CSV
No fake production states exist
```

Recommended first real backend:

```text
Basic Pitch / NeuralNote path
```

---

## 9. Correct reading order for humans

```text
1. docs/00_PROJECT_INDEX.md
2. docs/02_PRD.md
3. docs/03_SRS.md
4. docs/04_BACKEND_CONTRACT.md
5. docs/schemas/*.schema.json
6. docs/examples/*.example.json
7. docs/05_TDD_ARCHITECTURE.md
8. docs/engineering/REAL_RESULT_AND_TONY_LAYER_INTEGRATION_RULES.md
9. docs/engineering/LAYER_TYPE_POLICY.md
10. docs/engineering/BACKEND_OUTPUT_TRUTH_TABLE.md
11. docs/engineering/BACKEND_TO_TONY_LAYER_MAPPING_MATRIX.md
12. docs/engineering/BASIC_PITCH_ADAPTER_AUDIT.md
13. docs/engineering/BASIC_PITCH_OUTPUT_CONVERSION_PLAN.md
14. docs/engineering/BASIC_PITCH_REAL_ARTIFACT_DISCOVERY.md
15. docs/engineering/BASIC_PITCH_REAL_ARTIFACT_TO_UNIFIED_RESULT_PROOF.md
16. docs/engineering/BASIC_PITCH_UNIFIED_RESULT_JSON_HANDOFF_PROOF.md
17. docs/engineering/BASIC_PITCH_REAL_RUN_HANDOFF_PROOF.md
18. docs/engineering/BASIC_PITCH_RESULT_TO_TONY_LAYER_PROOF.md
19. docs/engineering/BASIC_PITCH_LAYER_SAVE_LOAD_EXPORT_PROOF.md
20. docs/engineering/BASIC_PITCH_DEBUG_WORKFLOW_PROOF.md
21. docs/engineering/BASIC_PITCH_DEBUG_UI_CONSUMPTION_MODEL.md
22. docs/engineering/BASIC_PITCH_MAINWINDOW_DEBUG_UI_INTEGRATION_PLAN.md
23. docs/engineering/BASIC_PITCH_MAINWINDOW_DEBUG_UI_ACTION_PROOF.md
24. docs/engineering/BASIC_PITCH_DEBUG_CONFIGURATION_ENTRY_PROOF.md
25. docs/engineering/BASIC_PITCH_DEBUG_MANUAL_RUN_ACTION_PROOF.md
26. docs/engineering/BASIC_PITCH_DEBUG_POST_RUN_IMPORT_ACTION_PROOF.md
27. docs/engineering/BASIC_PITCH_DEBUG_COMBINED_RUN_IMPORT_ACTION_PROOF.md
28. docs/engineering/BASIC_PITCH_DEBUG_POST_IMPORT_EDIT_SAVE_EXPORT_PROOF.md
29. docs/engineering/BASIC_PITCH_PRODUCTION_READINESS_CHECKLIST.md
30. docs/engineering/BASIC_PITCH_DEBUG_TO_PRODUCTION_TRANSITION_PLAN.md
31. docs/engineering/TONY_EDIT_SAVE_EXPORT_PROOF_PLAN.md
32. docs/engineering/UI_VISUAL_TRUTH_STATES.md
33. docs/engineering/PROVENANCE_METADATA_POLICY.md
34. docs/engineering/REAL_RESULT_ACCEPTANCE_CHECKLIST.md
35. docs/adr/*.md
36. docs/06_EXECUTION_PLAN.md
37. docs/07_CODEX_TASK_LIST.md
38. AGENTS.md
```

---

## 10. Correct reading order for Codex / AI coding agents

For coding work, give Codex:

```text
AGENTS.md
docs/00_PROJECT_INDEX.md
docs/02_PRD.md
docs/03_SRS.md
docs/04_BACKEND_CONTRACT.md
docs/schemas/*.schema.json
docs/examples/*.example.json
docs/05_TDD_ARCHITECTURE.md
docs/engineering/REAL_RESULT_AND_TONY_LAYER_INTEGRATION_RULES.md
docs/engineering/LAYER_TYPE_POLICY.md
docs/engineering/BACKEND_OUTPUT_TRUTH_TABLE.md
docs/engineering/BACKEND_TO_TONY_LAYER_MAPPING_MATRIX.md
docs/engineering/BASIC_PITCH_ADAPTER_AUDIT.md
docs/engineering/BASIC_PITCH_OUTPUT_CONVERSION_PLAN.md
docs/engineering/BASIC_PITCH_REAL_ARTIFACT_DISCOVERY.md
docs/engineering/BASIC_PITCH_REAL_ARTIFACT_TO_UNIFIED_RESULT_PROOF.md
docs/engineering/BASIC_PITCH_UNIFIED_RESULT_JSON_HANDOFF_PROOF.md
docs/engineering/BASIC_PITCH_REAL_RUN_HANDOFF_PROOF.md
docs/engineering/BASIC_PITCH_RESULT_TO_TONY_LAYER_PROOF.md
docs/engineering/BASIC_PITCH_LAYER_SAVE_LOAD_EXPORT_PROOF.md
docs/engineering/BASIC_PITCH_DEBUG_WORKFLOW_PROOF.md
docs/engineering/BASIC_PITCH_DEBUG_UI_CONSUMPTION_MODEL.md
docs/engineering/BASIC_PITCH_MAINWINDOW_DEBUG_UI_INTEGRATION_PLAN.md
docs/engineering/BASIC_PITCH_MAINWINDOW_DEBUG_UI_ACTION_PROOF.md
docs/engineering/BASIC_PITCH_DEBUG_CONFIGURATION_ENTRY_PROOF.md
docs/engineering/BASIC_PITCH_DEBUG_MANUAL_RUN_ACTION_PROOF.md
docs/engineering/BASIC_PITCH_DEBUG_POST_RUN_IMPORT_ACTION_PROOF.md
docs/engineering/BASIC_PITCH_DEBUG_COMBINED_RUN_IMPORT_ACTION_PROOF.md
docs/engineering/BASIC_PITCH_DEBUG_POST_IMPORT_EDIT_SAVE_EXPORT_PROOF.md
docs/engineering/BASIC_PITCH_PRODUCTION_READINESS_CHECKLIST.md
docs/engineering/BASIC_PITCH_DEBUG_TO_PRODUCTION_TRANSITION_PLAN.md
docs/engineering/TONY_EDIT_SAVE_EXPORT_PROOF_PLAN.md
docs/engineering/UI_VISUAL_TRUTH_STATES.md
docs/engineering/PROVENANCE_METADATA_POLICY.md
docs/engineering/REAL_RESULT_ACCEPTANCE_CHECKLIST.md
docs/adr/*.md
docs/06_EXECUTION_PLAN.md
docs/07_CODEX_TASK_LIST.md
```

Do not give Codex ZIP packages or prompt files as primary context.

---

## 11. Current implementation status

Current project status:

```text
Backend infrastructure foundation exists through CODEX-086.
Real-result engineering rules and Tony layer proof gates exist through CODEX-087D.
Tony source architecture, layer/edit/save/export paths, and backend output mappings have been audited.
Dev/mock pipeline, real Tony layer, edit, save/load, export, provenance identity, and Basic Pitch fixture conversion proof work exists through CODEX-096.
CODEX-097 adds a manual/test-only Basic Pitch real artifact discovery harness.
CODEX-098 adds a manual/test-only Basic Pitch discovered note-events CSV to in-memory UnifiedResult proof.
CODEX-099 adds Basic Pitch artifact-to-real-result.json handoff through the existing loader/reporter boundary.
CODEX-100 adds a manual opt-in real Basic Pitch run-to-result.json proof harness.
CODEX-101 adds a Basic Pitch-shaped result.json-to-real-Tony-layer proof boundary.
CODEX-102 adds Basic Pitch-shaped imported NoteLayer save/load/export proof through real Tony/SV session XML and CSV export APIs.
CODEX-103 adds a debug-only Basic Pitch workflow boundary with truth states and proof-bundle reporting.
CODEX-104 adds a compile-only Basic Pitch debug workflow UI-consumption adapter.
CODEX-105 adds a source-audited MainWindow debug UI integration plan without runtime wiring.
CODEX-106 adds the first explicit debug-only MainWindow action that displays Basic Pitch workflow truth states and proof-bundle fields without production claims.
CODEX-107 hardens the Basic Pitch debug UI report details with explicit mode, configuration, skipped/not-configured, artifact, and proof-bundle status.
CODEX-108 adds explicit debug-only Basic Pitch manual-run configuration/status reporting.
CODEX-109 adds an explicit debug-only Basic Pitch manual handoff action.
CODEX-110 adds an explicit debug-only Basic Pitch existing result.json import action.
CODEX-111 adds an explicit debug-only combined manual handoff plus real Tony/SV layer import action.
CODEX-112 adds debug-only post-import edit/undo/save/load/export proof reporting to the combined Basic Pitch debug path.
CODEX-113 tightens the combined debug UI/report flow so the post-import proof appears as a dedicated truth-gated section.
CODEX-114 adds the guarded production-readiness checklist and debug-to-production transition plan for Basic Pitch.
```

Therefore the next work must advance through the guarded Basic Pitch transition gates without weakening the debug-only truth-state guarantees.

---

## 12. Immediate next action

Current next action after CODEX-114:

```text
CODEX-115 - add the Basic Pitch production-readiness preflight model, still with no production UI or runtime behavior changes
```

This supersedes the earlier baseline setup sequence below for the current repository state. Do not start production Basic Pitch UI integration, selected-region replacement, or feature-complete claims until `docs/engineering/BASIC_PITCH_PRODUCTION_READINESS_CHECKLIST.md` gates are satisfied through the transition sequence in `docs/engineering/BASIC_PITCH_DEBUG_TO_PRODUCTION_TRANSITION_PLAN.md`.

Historical baseline setup sequence:

If the actual Tony fork repository does not yet contain the documents, first run:

```text
CODEX-000 — Add project documentation baseline
CODEX-001 — Create/verify repository AGENTS.md
CODEX-002 — Create ADR directory and initial ADRs
```

If the documents already exist in the repository, start with:

```text
CODEX-010 — Inspect repository build system
CODEX-011 — Build original Tony unchanged
CODEX-012 — Minimal baseline runtime check
```

Do not start backend implementation until original Tony builds and the existing pYIN workflow is verified.

---

## 13. Documents still missing for later phases

These are not blockers for CODEX-010, but they should be created before their phases require them.

| File | Priority | Purpose |
|---|---|---|
| `docs/01_RESEARCH_MAP.md` | High before backend implementation | Verified backend links, licenses, capabilities, install notes |
| `docs/09_LICENSE_REGISTER.md` | High before public release or bundled backend work | License review table |
| `docs/10_TEST_PLAN.md` | High before MVP hardening | Test fixtures and acceptance/regression tests |
| `docs/11_BUILD_WINDOWS.md` | Created during CODEX-010/011 | Real Windows build instructions |
| `docs/12_CODEBASE_MAP.md` | Created during CODEX-020/022 | Real Tony source-code locations |
| `docs/08_AI_COPILOT_SPEC.md` | Future | AI Copilot design before AI implementation |

---

## 14. Approval checklist before feature coding

```text
[ ] Original Tony source repository is cloned/forked.
[ ] docs/ files are present in the repository.
[ ] AGENTS.md is present in the repository.
[ ] CODEX-010 has mapped the build system.
[ ] CODEX-011 has built original Tony unchanged or documented exact blockers.
[ ] CODEX-012 has verified baseline runtime behavior.
[ ] CODEX-020/021/022 have mapped relevant Tony source paths.
[ ] No backend integration task has started before build/source mapping.
```

---

## 15. Short Russian summary

Проект снова стабилизирован.

Каноническая цепочка теперь такая:

```text
00_PROJECT_INDEX → PRD → SRS → Backend Contract → TDD → ADRs → Execution Plan → Codex Task List → AGENTS.md
```

Следующий реальный шаг для Codex:

```text
Сначала перенести документы в настоящий Tony fork repository.
Потом собрать оригинальный Tony без изменений.
Потом составить карту кода Tony.
Только потом добавлять backend infrastructure.
```
