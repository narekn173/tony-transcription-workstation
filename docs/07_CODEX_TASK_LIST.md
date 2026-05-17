# Codex Task List v0.1 — Tony Fork Transcription Workstation

## 1. Document control

| Field | Value |
|---|---|
| Document | Codex Task List |
| Version | v0.1 |
| Product | Tony Fork Transcription Workstation |
| Source documents | `docs/02_PRD.md`, `docs/03_SRS.md`, `docs/04_BACKEND_CONTRACT.md`, `docs/05_TDD_ARCHITECTURE.md`, `docs/06_EXECUTION_PLAN.md` |
| Status | Canonical v0.1 — reviewed/stabilized |
| Purpose | Ready-to-use small Codex prompts in safe implementation order |

---

## 2. How to use this file

Use **one task at a time**. Do not paste the whole file into Codex and ask it to implement everything.

Workflow:

```text
1. Make sure project docs are in the repository.
2. Confirm `AGENTS.md` is present and current.
3. Give Codex one task from this document.
4. Review the diff.
5. Build/test.
6. Commit only if correct.
7. Move to the next task.
```

Each task is intentionally small. If Codex proposes doing more than the task allows, reject or ask it to split the change.

---

## 3. Global prompt header for every Codex task

Paste this before each task unless `AGENTS.md` already contains equivalent instructions.

```text
You are working on a fork of Tony called Tony Fork Transcription Workstation.

Before changing code, read these documents:
- docs/02_PRD.md
- docs/03_SRS.md
- docs/04_BACKEND_CONTRACT.md
- docs/05_TDD_ARCHITECTURE.md
- docs/06_EXECUTION_PLAN.md

Core rules:
- Do not rewrite Tony from scratch.
- Preserve existing Tony/pYIN behavior.
- Do not introduce fake analysis output or fake completed states.
- Do not integrate multiple backends in one task.
- Do not redesign UI unless the task explicitly asks for UI design work.
- Use the Backend Contract and UnifiedResult for all backend results.
- Before TonyLayerImporter, UI, MainWindow/Analyser, real backend workflow, selected-region replacement, save/load, or export work, read docs/engineering/*.md and follow the Engineering Gates.
- Keep the change small and reviewable.
- If you are unsure, inspect the codebase and report findings before implementing.

At the end, report:
- files changed
- build/test commands run
- manual verification steps
- known limitations
- next recommended task
```

---

## 4. Phase 0 — Documentation and repository setup

### CODEX-000 — Add project documentation baseline

```text
Task ID: CODEX-000
Title: Add project documentation baseline

Goal:
Add the existing project planning documents to the repository without changing application code.

Read first:
- docs/02_PRD.md
- docs/03_SRS.md
- docs/04_BACKEND_CONTRACT.md
- docs/05_TDD_ARCHITECTURE.md
- docs/06_EXECUTION_PLAN.md

Allowed:
- add docs/ files
- add docs/schemas/ files
- add docs/examples/ files
- add docs/diagrams/ files

Not allowed:
- no C++ changes
- no build system changes
- no UI changes

Acceptance criteria:
1. Documents exist in the repository under docs/.
2. JSON schema files and examples are included.
3. No application behavior changes.

Verification:
- List the added files.
- Confirm no source code files were changed.
```

### CODEX-001 — Create AGENTS.md for Codex rules

```text
Task ID: CODEX-001
Title: Create repository AGENTS.md

Goal:
Create AGENTS.md that gives Codex persistent project-specific instructions.

Read first:
- docs/02_PRD.md
- docs/03_SRS.md
- docs/04_BACKEND_CONTRACT.md
- docs/05_TDD_ARCHITECTURE.md
- docs/06_EXECUTION_PLAN.md
- docs/07_CODEX_TASK_LIST.md

Allowed:
- create AGENTS.md
- optionally create .github/copilot-instructions.md if requested

Not allowed:
- no application code changes
- no build system changes

AGENTS.md must include:
- project overview
- required docs to read
- no-fake-results policy
- preserve Tony/pYIN rule
- one-task-one-diff rule
- Definition of Done
- build/test command placeholders if not yet known
- rule to update build commands after baseline build is mapped

Acceptance criteria:
1. AGENTS.md exists.
2. It is concise but specific.
3. It references the docs and no-fake policy.
4. It does not contain invented build commands.
```

### CODEX-002 — Create ADR directory and initial ADRs

```text
Task ID: CODEX-002
Title: Create initial Architecture Decision Records

Goal:
Create docs/adr/ and record the initial architecture decisions already established in the project documents.

Allowed:
- create docs/adr/ADR-0001-tony-fork.md
- create docs/adr/ADR-0002-external-backends-first.md
- create docs/adr/ADR-0003-unified-result-contract.md
- create docs/adr/ADR-0004-basic-pitch-first-backend.md
- create docs/adr/ADR-0005-local-first-cpu-first.md

Not allowed:
- no code changes
- no new architecture decisions beyond what documents already support

Acceptance criteria:
1. ADRs include Status, Context, Decision, Consequences.
2. Decisions align with PRD/SRS/TDD.
3. Uncertain items are marked as Proposed, not Accepted.
```

---

## 5. Phase 1 — Build original Tony unchanged

### CODEX-010 — Inspect repository build system

```text
Task ID: CODEX-010
Title: Inspect Tony build system

Goal:
Inspect the forked Tony repository and document how it is supposed to build on Windows.

Read first:
- AGENTS.md
- docs/06_EXECUTION_PLAN.md

Allowed:
- inspect files
- create docs/build/BUILD_SYSTEM_MAP.md

Not allowed:
- no source code changes
- no dependency installation scripts yet
- no feature implementation

Acceptance criteria:
1. Document identifies build system files and dependency references.
2. Document lists likely Windows build prerequisites.
3. Document marks unknowns clearly.
4. No app behavior changes.
```

### CODEX-011 — Build original Tony unchanged

```text
Task ID: CODEX-011
Title: Build original Tony unchanged

Goal:
Attempt to build the unmodified fork and document exact commands/results.

Allowed:
- run build commands
- create docs/build/BASELINE_BUILD_LOG.md
- create docs/build/BUILD_WINDOWS.md

Not allowed:
- no feature work
- no backend integration
- no UI changes
- no source code fixes unless separately approved

Acceptance criteria:
1. Build command(s) are documented.
2. Build result is documented.
3. If build fails, exact errors and likely missing prerequisites are documented.
4. No feature implementation is attempted.
```

### CODEX-012 — Minimal baseline runtime check

```text
Task ID: CODEX-012
Title: Verify baseline Tony runtime behavior

Goal:
Run the unmodified application if build succeeds and document baseline behavior.

Allowed:
- create docs/build/BASELINE_RUNTIME_CHECK.md
- add screenshots only if the repo workflow supports them

Not allowed:
- no code changes
- no backend changes

Acceptance criteria:
1. Application launch status is documented.
2. Existing audio open behavior is documented if testable.
3. Existing pYIN/analysis workflow status is documented if testable.
4. Known limitations are listed.
```

---

## 6. Phase 2 — Codebase mapping

### CODEX-020 — Map source tree and main application entry points

```text
Task ID: CODEX-020
Title: Map Tony source tree and entry points

Goal:
Create a codebase map before modifying architecture.

Allowed:
- inspect source code
- create docs/engineering/CODEBASE_MAP.md

Not allowed:
- no source code changes
- no architecture implementation

Map:
- source directories
- application entry point
- main window class/file
- menu/action system
- build system organization
- settings/config persistence locations

Acceptance criteria:
1. CODEBASE_MAP.md lists real file paths.
2. No guessed file paths are presented as facts.
3. Unverified hypotheses are labeled TODO/UNKNOWN.
```

### CODEX-021 — Map existing pYIN analysis workflow

```text
Task ID: CODEX-021
Title: Map existing pYIN analysis workflow

Goal:
Find the exact code path that triggers existing pYIN analysis and creates pitch/note layers.

Allowed:
- inspect source code
- update docs/engineering/CODEBASE_MAP.md or create docs/engineering/PYIN_WORKFLOW_MAP.md

Not allowed:
- no behavior changes
- no new backend code

Map:
- UI action/menu that triggers analysis
- transform/analysis call path
- pYIN plugin references
- pitch layer creation
- note layer creation
- error/status handling

Acceptance criteria:
1. Real source files/functions/classes are identified.
2. Integration points for future AnalysisEngineManager are proposed but not implemented.
3. Existing behavior is unchanged.
```

### CODEX-022 — Map import/export and note editing paths

```text
Task ID: CODEX-022
Title: Map import/export and note editing paths

Goal:
Identify how Tony imports/exports note/pitch data and handles note editing.

Allowed:
- inspect source code
- create docs/engineering/IMPORT_EXPORT_EDITING_MAP.md

Not allowed:
- no code changes

Map:
- MIDI export path if present
- CSV/export path if present
- note editing operations
- undo/redo or command stack
- layer model relevant to imported backend results

Acceptance criteria:
1. Real paths are documented.
2. Gaps are marked as UNKNOWN.
3. Recommendations are limited to future tasks.
```

---

## 7. Phase 3 — Architecture skeleton

### CODEX-030 — Propose concrete source locations for new architecture

```text
Task ID: CODEX-030
Title: Propose concrete source locations for backend architecture

Goal:
Using the completed codebase maps, propose where new architecture files should live.

Read first:
- docs/engineering/CODEBASE_MAP.md
- docs/engineering/PYIN_WORKFLOW_MAP.md
- docs/05_TDD_ARCHITECTURE.md

Allowed:
- create docs/engineering/NEW_ARCHITECTURE_FILE_PLAN.md

Not allowed:
- no source code implementation yet

Acceptance criteria:
1. File plan maps TDD component names to real repo locations.
2. It explains why each location fits existing Tony structure.
3. It lists any build-system changes that would be needed later.
```

### CODEX-031 — Add architecture skeleton without behavior changes

```text
Task ID: CODEX-031
Title: Add AnalysisEngineManager skeleton

Goal:
Add minimal architecture skeleton that compiles but does not change production behavior.

Allowed:
- add AnalysisEngineManager skeleton
- add BackendRegistry skeleton
- add BackendManifestLoader skeleton
- add BackendSettingsStore skeleton
- update build files only as needed for these classes

Not allowed:
- no real backend execution
- no UI redesign
- no changes to existing pYIN workflow
- no fake analysis results

Acceptance criteria:
1. Project builds.
2. Existing pYIN workflow remains unchanged.
3. Skeleton classes are small and documented.
4. No user-visible fake backend appears.
```

### CODEX-032 — Add backend status enum and capability model

```text
Task ID: CODEX-032
Title: Add backend status and capability model

Goal:
Add internal types for backend status and capability declaration.

Allowed:
- backend status enum/model
- capability model aligned with BackendManifest
- basic unit tests if test framework exists

Not allowed:
- no UI display yet unless tiny debug-only logging is needed
- no external process execution

Acceptance criteria:
1. Model supports installed/missing/broken/not configured/running/failed/completed states.
2. Capability fields align with docs/schemas/backend_manifest.schema.json.
3. Code builds.
```

---

## 8. Phase 4 — Contract validation and UnifiedResult

### CODEX-040 — Add UnifiedResult data structures

```text
Task ID: CODEX-040
Title: Add UnifiedResult data structures

Goal:
Implement internal data structures for UnifiedResult without importing into Tony layers yet.

Allowed:
- add UnifiedResult, NoteEvent, PitchPoint, PitchBend, TechniqueLabel, Warning/Error structures
- parser skeleton
- tests with minimal example if test framework exists

Not allowed:
- no UI changes
- no backend execution
- no layer import yet

Acceptance criteria:
1. Structures represent fields from docs/schemas/unified_result.schema.json.
2. Invalid/missing required fields are handled safely.
3. Code builds.
```

### CODEX-041 — Add result parser/validator for example JSON

```text
Task ID: CODEX-041
Title: Parse and validate UnifiedResult examples

Goal:
Load and validate example UnifiedResult JSON files.

Allowed:
- parser/validator implementation
- add test fixtures if needed
- use existing JSON support if available

Not allowed:
- no new heavy dependency unless justified and documented
- no UI changes
- no fake backend behavior

Acceptance criteria:
1. Valid example parses successfully.
2. Malformed example fails with structured error.
3. No invalid result can be marked completed.
4. Code builds/tests run.
```

### CODEX-042 — Add BackendManifest loader for static manifests

```text
Task ID: CODEX-042
Title: Add BackendManifest loader

Goal:
Load backend capability manifests from JSON files.

Allowed:
- manifest loader
- validation against expected fields
- documentation update if manifest location is chosen

Not allowed:
- no backend execution
- no UI redesign

Acceptance criteria:
1. Valid manifest loads.
2. Invalid manifest is rejected.
3. Backend appears as unavailable if manifest is missing/invalid.
4. Code builds.
```

---

## 9. Phase 4A — Engineering rules and Tony layer proof gates

These tasks are upcoming control tasks. Do not mark them implemented until they have been explicitly run and reviewed.

### CODEX-087A — Tony source architecture audit

```text
Task ID: CODEX-087A
Title: Audit Tony source architecture before layer integration

Goal:
Inspect the real Tony/Sonic Visualiser source paths that will matter for TonyLayerImporter and future UI/import work.

Read first:
- AGENTS.md
- docs/00_PROJECT_INDEX.md
- docs/engineering/REAL_RESULT_AND_TONY_LAYER_INTEGRATION_RULES.md
- docs/engineering/LAYER_TYPE_POLICY.md
- docs/engineering/TONY_EDIT_SAVE_EXPORT_PROOF_PLAN.md
- docs/engineering/CODEBASE_MAP.md

Allowed:
- inspect source code
- create/update docs/engineering source-audit documentation

Not allowed:
- no C++ behavior changes
- no layer import implementation
- no UI

Acceptance criteria:
1. Real source files/classes for document ownership, layer creation, view insertion, command history, and model lifecycle are identified.
2. Unknowns are marked.
3. Risky files/classes are listed.
```

### CODEX-087B — Tony layer/edit/save/export audit

```text
Task ID: CODEX-087B
Title: Audit Tony layer edit save export paths

Goal:
Map how real Tony/Sonic Visualiser note and pitch layers are edited, saved, loaded, and exported.

Allowed:
- inspect source code
- create/update docs/engineering audit documentation

Not allowed:
- no C++ behavior changes
- no importer implementation
- no UI

Acceptance criteria:
1. Note layer edit paths are identified.
2. Pitch/f0 layer paths are identified.
3. Save/load paths are identified.
4. MIDI/CSV/export paths are identified.
5. Proof requirements are documented before implementation.
```

### CODEX-087C — Backend output truth table verification

```text
Task ID: CODEX-087C
Title: Verify backend output truth table

Goal:
Review backend-specific expected outputs and update the truth table with verified local or documented evidence.

Allowed:
- inspect docs/examples/backend notes
- update docs/engineering/BACKEND_OUTPUT_TRUTH_TABLE.md
- create backend integration notes if needed

Not allowed:
- no backend integration
- no fake output
- no UI

Acceptance criteria:
1. Each backend entry distinguishes verified facts from planning assumptions.
2. f0-only backends are not treated as note backends.
3. Basic Pitch polyphony risk remains explicit.
```

### CODEX-087D — Real-result acceptance rules verification

```text
Task ID: CODEX-087D
Title: Verify real-result acceptance rules

Goal:
Review all real-result, layer, UI state, provenance, and edit/save/export proof rules before implementation resumes.

Allowed:
- documentation review
- small documentation corrections

Not allowed:
- no C++ behavior changes
- no UI
- no backend integration

Acceptance criteria:
1. Engineering Gates are consistent across AGENTS, Project Index, TDD, Execution Plan, and task list.
2. No rule permits fake Ready/Completed/Imported/Editable/Exported claims.
3. Next importer task has clear proof prerequisites.
```

### CODEX-088 — Dev/mock backend end-to-end proof

```text
Task ID: CODEX-088
Title: Prove dev/mock backend end-to-end path

Goal:
Use a clearly dev/test-only result to prove the parser/import path without claiming real backend success.

Not allowed:
- no production success claims
- no fake real backend
- no pYIN behavior changes

Acceptance criteria:
1. Mock/dev output is labeled test-only.
2. It uses the same UnifiedResult loader/validator path as real results.
3. Highest proof gate reached is reported.
```

### CODEX-089 — UnifiedResult to real editable Tony NoteLayer proof

```text
Task ID: CODEX-089
Title: Prove UnifiedResult imports to real editable Tony NoteLayer

Goal:
Import validated UnifiedResult notes into a real Tony/Sonic Visualiser editable note layer.

Not allowed:
- no custom overlay pretending to be a layer
- no fake notes
- no save/export claims unless proven

Acceptance criteria:
1. Real Tony/Sonic Visualiser layer/model class is used.
2. Imported notes are visible.
3. Edit proof is completed before claiming editable.
4. pYIN behavior remains unchanged.
```

### CODEX-090 — Save/load/export proof

```text
Task ID: CODEX-090
Title: Prove save load export for imported result layers

Goal:
Prove that imported or accepted backend result data survives save/load and exports correctly where claimed.

Not allowed:
- no fake export files
- no persistence claims without reload proof
- no pYIN behavior changes

Acceptance criteria:
1. Save/load proof is documented.
2. Export file is created and inspected.
3. Provenance limitations are documented.
4. Unsupported export paths are reported honestly.
```

---

## 10. Phase 5 — Dev-only mock vertical slice

### CODEX-050 — Add dev-only mock backend adapter

```text
Task ID: CODEX-050
Title: Add dev-only mock backend adapter

Goal:
Create a development-only backend adapter that reads a known UnifiedResult example.

Allowed:
- mock adapter
- dev/test-only registration
- docs explaining it is not real analysis

Not allowed:
- no production menu item that looks real
- no fake completed state in normal UI
- no external process execution

Acceptance criteria:
1. Mock adapter is clearly labeled dev-only.
2. It reads a real JSON fixture.
3. It produces a UnifiedResult through the same parser as real backends.
4. It cannot be mistaken for a production backend.
```

### CODEX-051 — Import mock UnifiedResult into a Tony result layer

```text
Task ID: CODEX-051
Title: Import mock UnifiedResult into Tony layer

Goal:
Use the mock result to prove that UnifiedResult can be displayed in Tony.

Allowed:
- add TonyLayerImporter prototype
- create a non-destructive result layer from NoteEvent data
- minimal debug UI/menu only if necessary and clearly labeled

Not allowed:
- no real backend work
- no UI redesign
- no overwrite of user-corrected layer

Acceptance criteria:
1. Notes from the fixture appear in a result layer.
2. Layer is non-destructive.
3. The result can be removed without affecting existing data.
4. Existing pYIN behavior remains working.
```

---

## 11. Phase 6 — External process infrastructure

### CODEX-060 — Add ExternalProcessRunner skeleton

```text
Task ID: CODEX-060
Title: Add ExternalProcessRunner skeleton

Goal:
Create a safe process runner abstraction for external backend execution.

Allowed:
- ExternalProcessRunner class/module
- command + args model
- stdout/stderr capture
- exit code capture
- structured result object

Not allowed:
- no Basic Pitch integration yet
- no real ML backend execution
- no UI redesign

Acceptance criteria:
1. Runner can execute a simple controlled command.
2. stdout/stderr/exit code are captured.
3. Errors are structured.
4. Code builds.
```

### CODEX-061 — Add timeout, cancellation, and temp workspace support

```text
Task ID: CODEX-061
Title: Add ExternalProcessRunner timeout and cancellation

Goal:
Make external process execution safe for long-running backends.

Allowed:
- timeout support
- cancellation support
- temp workspace creation/cleanup
- structured cancelled/timeout states

Not allowed:
- no backend-specific logic yet

Acceptance criteria:
1. Long-running test process can be cancelled.
2. Timeout produces a timeout error state.
3. Temp files are cleaned or explicitly retained for debugging when configured.
4. Code builds/tests run.
```

### CODEX-062 — Wire runner status to minimal analysis status UI

```text
Task ID: CODEX-062
Title: Wire process status to minimal UI status

Goal:
Expose honest status states for backend execution.

Allowed:
- minimal status display
- statuses: idle, running, completed, failed, cancelled, timed out
- no visual redesign

Not allowed:
- no fake progress percentage unless real progress exists
- no real backend integration yet

Acceptance criteria:
1. Running command shows running state.
2. Success shows completed state.
3. Failure shows failed state with readable message.
4. Cancel shows cancelled state.
```

---

## 12. Phase 7 — First real backend: Basic Pitch / NeuralNote path

### CODEX-070 — Inspect Basic Pitch CLI/output behavior locally

```text
Task ID: CODEX-070
Title: Inspect Basic Pitch CLI and output behavior

Goal:
Determine exact command-line invocation and output files for Basic Pitch in the local environment.

Allowed:
- documentation-only investigation
- create docs/backends/BASIC_PITCH_INTEGRATION_NOTES.md
- run Basic Pitch only if available locally

Not allowed:
- no app integration yet
- no assumptions presented as facts

Acceptance criteria:
1. Exact command format is documented if verified.
2. Output files are documented if verified.
3. Missing installation/dependency issues are documented.
4. Unknowns remain marked UNKNOWN.
```

### CODEX-071 — Add Basic Pitch backend manifest/configuration

```text
Task ID: CODEX-071
Title: Add Basic Pitch backend manifest and configuration

Goal:
Add Basic Pitch as a configurable backend without running analysis yet.

Allowed:
- Basic Pitch manifest
- backend path setting
- status detection: not configured/missing/available
- Backend Manager entry if Backend Manager UI exists

Not allowed:
- no analysis run yet
- no fake available status
- no automatic download/install

Acceptance criteria:
1. Basic Pitch appears as not configured until path/environment is set.
2. Missing executable/environment is shown honestly.
3. No analysis result is produced yet.
```

### CODEX-072 — Run Basic Pitch full-file analysis through ExternalProcessRunner

```text
Task ID: CODEX-072
Title: Run Basic Pitch full-file analysis

Goal:
Run Basic Pitch on a full audio file through the external runner.

Allowed:
- BasicPitchExternalAdapter
- request generation
- temp workspace output collection
- structured errors

Not allowed:
- no selected-region logic yet
- no compare mode
- no UI redesign

Acceptance criteria:
1. User can run Basic Pitch on a full file when configured.
2. Backend output files are detected.
3. Failure is shown honestly.
4. No output is imported unless validation succeeds.
```

### CODEX-073 — Convert Basic Pitch output to UnifiedResult

```text
Task ID: CODEX-073
Title: Convert Basic Pitch output to UnifiedResult

Goal:
Normalize Basic Pitch output into the UnifiedResult format.

Allowed:
- output parser/converter
- notes, velocity, pitch bends where available
- warnings for unavailable fields

Not allowed:
- no invented pitch curve if backend did not provide one
- no fake confidence values

Acceptance criteria:
1. Converted result validates against UnifiedResult expectations.
2. Missing fields are omitted or marked as unavailable, not invented.
3. Result imports through existing validator.
```

### CODEX-074 — Display Basic Pitch result and export corrected layer

```text
Task ID: CODEX-074
Title: Display Basic Pitch result and export corrected MIDI/CSV

Goal:
Complete the first real backend MVP vertical slice.

Allowed:
- import Basic Pitch UnifiedResult into result layer
- allow copy/accept into corrected layer if existing workflow supports it
- export corrected note layer to MIDI/CSV using existing Tony export paths or minimal adapter

Not allowed:
- no compare mode
- no selected-region analysis
- no AI Copilot
- no full UI redesign

Acceptance criteria:
1. Basic Pitch output appears visually as real notes.
2. User can manually correct notes using Tony workflow where supported.
3. User can export MIDI.
4. CSV export works if implemented by existing path or small adapter.
5. Existing pYIN workflow still works.
```

---

## 13. Phase 8 — MVP hardening

### CODEX-080 — MVP no-fake and error-state audit

```text
Task ID: CODEX-080
Title: MVP no-fake and error-state audit

Goal:
Audit the MVP for fake states, silent failures, and unclear backend errors.

Allowed:
- small fixes to error messages/status states
- documentation updates
- add tests if possible

Not allowed:
- no new backend
- no UI redesign

Acceptance criteria:
1. No UI state claims completion without real validated output.
2. Backend missing/failure/timeout/cancel states are distinct.
3. Errors are readable.
4. Audit findings are documented.
```

### CODEX-081 — Regression check existing Tony/pYIN behavior

```text
Task ID: CODEX-081
Title: Regression check existing Tony/pYIN behavior

Goal:
Verify new backend infrastructure did not break existing Tony behavior.

Allowed:
- tests or manual verification document
- small regression fixes if directly caused by recent changes

Not allowed:
- no new features

Acceptance criteria:
1. Existing audio open path still works.
2. Existing pYIN workflow still works or known limitation is documented.
3. Existing export behavior is not regressed.
4. Findings are documented.
```

---

## 14. Phase 9 — CREPE Notes backend

### CODEX-091A — Inspect CREPE Notes CLI/output behavior

```text
Task ID: CODEX-091A
Title: Inspect CREPE Notes CLI and output behavior

Goal:
Document verified local integration behavior for CREPE Notes.

Allowed:
- create docs/backends/CREPE_NOTES_INTEGRATION_NOTES.md
- inspect CLI help/docs
- run locally if installed

Not allowed:
- no app integration yet

Acceptance criteria:
1. Verified command/output behavior is documented.
2. Monophonic limitation is documented.
3. Dependency and licensing notes are documented.
```

### CODEX-091B — Add CREPE Notes external adapter

```text
Task ID: CODEX-091B
Title: Add CREPE Notes external adapter

Goal:
Integrate CREPE Notes as a second real backend through the same Backend Contract path.

Allowed:
- manifest/configuration
- external runner invocation
- output parsing to UnifiedResult
- full-file analysis

Not allowed:
- no Vamp/native port yet
- no compare mode changes beyond making result layers available
- no fake pitch/note fields

Acceptance criteria:
1. CREPE Notes can be configured.
2. It runs when available.
3. Output validates as UnifiedResult.
4. UI shows monophonic-use warning/help.
5. Existing Basic Pitch and pYIN behavior still works.
```

---

## 15. Phase 10 — Selected-region preview

### CODEX-100 — Add selected-region request plumbing

```text
Task ID: CODEX-100
Title: Add selected-region request plumbing

Goal:
Represent selected time ranges in BackendRequest and UI flow without applying results yet.

Allowed:
- region selection extraction from existing Tony UI if available
- BackendRequest region fields
- validation of region start/end

Not allowed:
- no destructive replacement of notes
- no compare mode

Acceptance criteria:
1. Selected region can be represented as start/end seconds.
2. Invalid regions are rejected.
3. Full-file behavior still works.
```

### CODEX-101 — Add selected-region preview layer

```text
Task ID: CODEX-101
Title: Add selected-region preview layer

Goal:
Run an available backend on a selected region and show result as preview.

Allowed:
- temporary audio slice or backend region request based on adapter capability
- preview result layer
- reject/discard preview

Not allowed:
- no automatic overwrite
- no multi-engine compare yet

Acceptance criteria:
1. Region analysis does not affect notes outside the region.
2. Result is preview-only until user applies it.
3. User can reject preview.
4. Failure does not modify existing notes.
```

### CODEX-102 — Add apply/copy selected-region behavior

```text
Task ID: CODEX-102
Title: Add selected-region apply/copy behavior

Goal:
Let the user apply or copy selected-region preview results safely.

Allowed:
- apply full preview to selected region
- copy selected notes from preview
- undo/redo integration if available

Not allowed:
- no automatic merge without confirmation

Acceptance criteria:
1. Apply affects only selected region.
2. Copy selected notes works if layer editing supports it.
3. Undo/redo works or limitation is documented.
4. User confirmation is required before destructive replacement.
```

---

## 16. Phase 11 — Compare mode v1

### CODEX-110 — Add sequential compare queue

```text
Task ID: CODEX-110
Title: Add sequential compare queue

Goal:
Run multiple selected compatible engines sequentially.

Allowed:
- compare controller
- queue engine runs one after another
- cancellation for remaining queue

Not allowed:
- no concurrent multi-backend execution
- no AI best-result decision

Acceptance criteria:
1. User can select compatible engines.
2. Engines run sequentially.
3. Progress shows current engine.
4. User can cancel remaining runs.
```

### CODEX-111 — Add compare result layers

```text
Task ID: CODEX-111
Title: Add compare result layers

Goal:
Display each backend result as a separate named layer.

Allowed:
- result layer naming by backend
- visibility toggles if supported
- whole-layer accept/copy behavior

Not allowed:
- no advanced voting/heatmap yet

Acceptance criteria:
1. Each backend result is visually distinguishable by name/status.
2. User can inspect one result at a time.
3. User can choose a whole result as corrected layer or copy from it if supported.
```

### CODEX-112 — Add basic disagreement marking

```text
Task ID: CODEX-112
Title: Add basic compare disagreement marking

Goal:
Highlight regions where backend results differ significantly.

Allowed:
- simple time-window disagreement markers
- basic note count/pitch mismatch summary

Not allowed:
- no complex consensus algorithm unless separately designed
- no AI explanation

Acceptance criteria:
1. Obvious disagreements are marked.
2. Markers are informational only.
3. No automatic correction is applied.
```

---

## 17. Phase 12 — Specialized backends

### CODEX-120 — Add MUSC integration notes before implementation

```text
Task ID: CODEX-120
Title: Document MUSC integration behavior

Goal:
Study MUSC Violin Transcription locally/docs-first before implementation.

Allowed:
- create docs/backends/MUSC_INTEGRATION_NOTES.md
- document input/output/checkpoints/dependencies/license concerns

Not allowed:
- no app integration yet

Acceptance criteria:
1. Verified or documented output format is known.
2. Required dependencies/checkpoints are listed.
3. Integration risk is assessed.
```

### CODEX-121 — Add MUSC external adapter

```text
Task ID: CODEX-121
Title: Add MUSC external adapter

Goal:
Integrate MUSC as violin-specialized backend after core infrastructure is stable.

Allowed:
- manifest/configuration
- external runner invocation
- result conversion to UnifiedResult

Not allowed:
- no invented technique labels
- no fake pitch deviations

Acceptance criteria:
1. MUSC runs when configured.
2. Output validates.
3. Missing unsupported fields are omitted or warned.
4. UI identifies it as violin-specialized.
```

### CODEX-122 — Add VioPTT integration notes before implementation

```text
Task ID: CODEX-122
Title: Document VioPTT integration behavior

Goal:
Study VioPTT outputs before implementation.

Allowed:
- create docs/backends/VIOPTT_INTEGRATION_NOTES.md
- document MIDI, CSV, technique label outputs
- document dependencies/checkpoints/license

Not allowed:
- no app integration yet

Acceptance criteria:
1. Technique label format is documented if available.
2. Output fields map to UnifiedResult technique_labels.
3. Unknowns are marked.
```

### CODEX-123 — Add VioPTT external adapter and technique labels

```text
Task ID: CODEX-123
Title: Add VioPTT adapter and technique label layer

Goal:
Integrate VioPTT and display per-note technique labels where available.

Allowed:
- VioPTT adapter
- technique label parsing
- optional technique annotation layer

Not allowed:
- no fake technique labels
- no AI interpretation of technique labels

Acceptance criteria:
1. VioPTT runs when configured.
2. Notes import correctly.
3. Technique labels display only when present.
4. Technique labels export to CSV/JSON if supported.
```

### CODEX-124 — Add f0-only backend integration design

```text
Task ID: CODEX-124
Title: Design PESTO/PENN/FCPE f0 integration

Goal:
Design the shared f0-to-note segmentation path before implementing f0-only backends.

Allowed:
- create docs/backends/F0_BACKENDS_DESIGN.md
- map f0/time/confidence outputs to pitch_curve
- define segmentation interface

Not allowed:
- no f0 backend implementation yet
- no fake note segmentation

Acceptance criteria:
1. PESTO/PENN/FCPE are treated as f0 engines, not full note engines unless verified otherwise.
2. Segmentation layer interface is documented.
3. Output limitations are clear.
```

---

## 18. Phase 13 — Future AI Copilot

### CODEX-130 — Create AI Copilot design document only

```text
Task ID: CODEX-130
Title: Create AI Copilot design document

Goal:
Design future AI Copilot behavior without implementing API calls yet.

Allowed:
- create docs/08_AI_COPILOT_SPEC.md
- define user confirmation model
- define local-first/cloud opt-in policy
- define allowed actions

Not allowed:
- no API key storage
- no OpenAI API integration yet
- no audio upload
- no automatic correction application

Acceptance criteria:
1. AI is positioned as assistant/coordinator, not transcription engine.
2. User confirmation is required before changes.
3. Cloud/API is opt-in.
4. Watermark/fingerprint bypass is explicitly out of scope.
```

---

## 19. Phase 14 — UI modernization

### CODEX-140 — Create UI modernization proposal only

```text
Task ID: CODEX-140
Title: Create UI modernization proposal

Goal:
Plan UI modernization after functional MVP exists.

Allowed:
- create docs/ui/UI_MODERNIZATION_PLAN.md
- propose Simple/Advanced mode structure
- propose Backend Manager layout
- propose dark/system theme approach

Not allowed:
- no UI implementation yet
- no backend behavior change

Acceptance criteria:
1. Proposal preserves existing Tony workflow.
2. It does not overload beginner UI.
3. Advanced controls are collapsible or separated.
4. Implementation is split into future small tasks.
```

---

## 20. Review checklist for every Codex diff

Before accepting a Codex change, check:

```text
[ ] Did it stay within the task scope?
[ ] Did it avoid unrelated refactors?
[ ] Does the project build?
[ ] Did it preserve existing Tony/pYIN behavior?
[ ] Did it avoid fake analysis output?
[ ] For UI/layer/import/save/export work, did it follow the Engineering Gates and report the highest proof gate reached?
[ ] Did it use Backend Contract / UnifiedResult where required?
[ ] Are errors visible and honest?
[ ] Are docs updated if behavior changed?
[ ] Did Codex report files changed and commands run?
[ ] Is the next task still logically valid?
```

---

## 21. Recommended immediate next action

Do **not** start `CODEX-010` until `AGENTS.md` exists and the project documents have been added to the actual repository.

Next recommended step:

```text
If the repository does not yet contain the planning documents, run CODEX-000.
If AGENTS.md is missing in the repository, run CODEX-001.
If docs and AGENTS.md already exist, start CODEX-010: inspect repository build system.
```
