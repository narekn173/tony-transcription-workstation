# TDD / Architecture v0.1 — Tony Fork Transcription Workstation

## 1. Document control

| Field | Value |
|---|---|
| Document | Technical Design Document / Architecture |
| Version | v0.1 |
| Product | Tony Fork Transcription Workstation |
| Source documents | `docs/02_PRD.md`, `docs/03_SRS.md`, `docs/04_BACKEND_CONTRACT.md` |
| Related schemas | `docs/schemas/backend_manifest.schema.json`, `docs/schemas/backend_request.schema.json`, `docs/schemas/unified_result.schema.json` |
| Status | Draft |
| Primary platform | Windows |
| Primary implementation strategy | Tony fork, external backend adapters first |
| Primary UI strategy | Preserve Tony workflow first; modernize after architecture is stable |
| Primary processing model | Local-first, CPU-first, sequential multi-engine execution by default |

---

## 2. Purpose

This document defines the technical architecture for the Tony Fork Transcription Workstation.

It translates the PRD and SRS into a concrete design for:

- preserving existing Tony functionality;
- adding a capability-driven analysis engine system;
- running external transcription backends safely;
- validating and importing backend results through the Backend Contract;
- displaying results in Tony-style note/pitch layers;
- enabling future compare mode, selected-region analysis, and AI Copilot without redesigning the core later.

This TDD is intended for:

- the project owner;
- Codex / AI coding agents;
- future open-source contributors;
- reviewers who need to understand why the architecture is staged and local-first.

---

## 3. Scope and non-scope

### 3.1 In scope

This document covers the technical architecture for:

- `AnalysisEngineManager`;
- backend discovery and configuration;
- backend capability manifests;
- external process execution;
- request generation;
- temporary workspaces;
- validation of backend output;
- `UnifiedResult` parsing;
- import of results into Tony layers;
- result layer management;
- full-file analysis;
- selected-region analysis;
- compare mode;
- f0-only backend workflows;
- error handling;
- security/privacy constraints;
- testing and phased implementation.

### 3.2 Non-scope

This document does not define:

- exact pixel-level UI design;
- final visual theme;
- final product branding;
- complete native/Vamp implementation for all backends;
- complete AI Copilot prompt design;
- commercial licensing strategy;
- exact upstream backend installation manuals;
- low-level Tony source locations until the fork is inspected in Milestone 2.

When exact Tony source classes are not yet verified, this document uses **adapter-level architecture names**. Codex must map these names onto the real Tony source tree during the codebase-mapping milestone.

---

## 4. Source documents and traceability

### 4.1 Source documents

| Document | Role in architecture |
|---|---|
| `docs/02_PRD.md` | Product goals, MVP scope, post-MVP phases, no-fake policy, UX principles, local-first policy |
| `docs/03_SRS.md` | Testable requirements and requirement IDs |
| `docs/04_BACKEND_CONTRACT.md` | Backend manifest, request, result, validation, error, provenance, compare and region contracts |
| JSON schemas | Machine-checkable contract validation |

### 4.2 Traceability rule

Every architectural module must trace to at least one of:

- PRD goal;
- SRS requirement ID;
- Backend Contract section;
- explicit security/privacy constraint.

If a proposed module cannot be traced, it should not be implemented yet.

---

## 5. Architecture principles

### ARCH-001 — Preserve Tony first

Do not rewrite Tony from scratch. Existing Tony functionality remains the baseline and must be verified before feature work.

Traceability: `REQ-TONY-001`, `REQ-TONY-002`, PRD G1.

### ARCH-002 — Add engines through adapters, not UI-specific hacks

Each backend must be integrated through a backend adapter and the Backend Contract. UI code must not directly parse arbitrary backend outputs.

Traceability: `REQ-BACKEND-001`, `REQ-RESULT-001`, Backend Contract `CONTRACT-001`.

### ARCH-003 — Real results only

Production code must not display fake notes, fake confidence, fake progress, or fake completed states.

Traceability: `REQ-CORE-002`, `REQ-BACKEND-011`, `REQ-UI-006`, Backend Contract `CONTRACT-002`.

### ARCH-004 — Capability-driven UI

The UI must enable features based on the backend manifest. If a backend does not support pitch bends, technique labels, region analysis, or pitch curves, the UI must not pretend that it does.

Traceability: Backend Contract `CONTRACT-003`.

### ARCH-005 — Local-first, cloud-optional later

All MVP analysis runs locally. AI/cloud features are future opt-in and must be architecturally isolated.

Traceability: `REQ-CORE-003`, `REQ-PRIV-001`, `REQ-PRIV-003`.

### ARCH-006 — Non-destructive editing and analysis

Backend results are imported as result/preview layers. They do not overwrite the corrected layer unless the user explicitly applies them.

Traceability: `REQ-REGION-002`, `REQ-COMPARE-002`, Backend Contract `CONTRACT-006`.

### ARCH-007 — Keep the UI responsive

Long-running analysis must not block the GUI thread. External processes must support progress, cancellation where possible, and failure reporting.

Traceability: `REQ-PERF-003`, `REQ-CORE-007`, `REQ-ERR-005`.

### ARCH-008 — Validate before import

Every backend output must be schema-validated and semantically checked before it can create Tony layers.

Traceability: `REQ-ERR-003`, Backend Contract `CONTRACT-007`.

### ARCH-009 — CPU-first, sequential multi-engine execution

MVP must work on CPU-only systems. Compare mode should run engines sequentially by default to avoid overloading the machine.

Traceability: `REQ-PERF-001`, `REQ-PERF-002`.

### ARCH-010 — Future-native path without blocking MVP

External process adapters are the first implementation path. Native/ONNX/Vamp integrations can be added later behind the same adapter interface.

Traceability: PRD MVP and post-MVP phases.

---

## 6. Context and current Tony constraints

### 6.1 Existing system role

Tony is the base application and editor. It already provides a Sonic Visualiser-based environment for melody annotation, pitch/note visualisation, manual correction, and pYIN-based analysis.

The fork must treat this existing capability as the foundation, not as disposable prototype code.

### 6.2 Constraints

| Constraint | Architectural consequence |
|---|---|
| Tony is a desktop C++/Qt/Sonic Visualiser-based application | New UI/components should integrate into the existing Qt workflow first |
| pYIN exists through Vamp workflow | Keep pYIN as first-class `pyin_vamp` engine |
| Modern backends are heterogeneous | Use external process adapters and Backend Contract |
| Backends produce different outputs | Normalize to `UnifiedResult` before display |
| User wants Windows-first local use | Design CPU-first, local external process execution |
| Some backends are research projects | Keep them optional and capability-driven |
| Licensing may restrict bundling | Separate adapter code from optional external dependencies |

### 6.3 Key assumption

**ASSUMPTION-TDD-001:** The first engineering implementation will not modify core Tony rendering until the codebase-mapping milestone identifies the safest insertion points.

---

## 7. Target architecture overview

### 7.1 High-level structure

```text
Tony UI / Existing editor
    ↓
Analysis UI actions
    ↓
AnalysisEngineManager
    ↓
BackendRegistry + BackendManifestLoader
    ↓
BackendAdapter interface
    ├── PyinVampAdapter
    ├── BasicPitchExternalAdapter
    ├── CrepeNotesExternalAdapter
    ├── MuscViolinExternalAdapter
    ├── VioPttExternalAdapter
    ├── F0ExternalAdapter: PESTO / PENN / FCPE
    └── FutureNativeOnnxAdapter
    ↓
ExternalProcessRunner / NativeRunner
    ↓
Backend output files
    ↓
ResultValidator + UnifiedResultParser
    ↓
TonyLayerImporter
    ↓
ResultLayerManager / CorrectedLayer
    ↓
ExportService
```

### 7.2 Architectural idea

The application has two sides:

1. **Tony editor side** — audio loading, visualisation, note editing, export, existing pYIN behavior.
2. **Modern backend side** — external/native engines that produce `UnifiedResult` data.

The bridge is:

```text
Backend Contract + UnifiedResult + TonyLayerImporter
```

That bridge is the most important part of the architecture.

---

## 8. C4-style architecture views

### 8.1 System Context view

```text
[User]
  | loads audio, selects engine, edits notes, exports MIDI
  v
[Tony Fork Transcription Workstation]
  | local process execution
  v
[External Backend Environments]
  - Basic Pitch / NeuralNote
  - CREPE Notes
  - MUSC Violin
  - VioPTT
  - PESTO / PENN / FCPE

Optional future:
[Tony Fork] --> [AI Copilot API or local LLM]
```

### 8.2 Container view

```text
Tony Fork Desktop Application
├── Existing Tony Core
│   ├── audio loading/playback
│   ├── waveform/pitch/note display
│   ├── manual correction
│   └── existing pYIN workflow
│
├── Analysis Engine Subsystem
│   ├── AnalysisEngineManager
│   ├── BackendRegistry
│   ├── BackendSettingsStore
│   └── BackendAdapter implementations
│
├── Execution Subsystem
│   ├── ExternalProcessRunner
│   ├── TempWorkspaceManager
│   ├── ProgressTracker
│   └── CancellationController
│
├── Contract/Import Subsystem
│   ├── BackendRequestBuilder
│   ├── ResultValidator
│   ├── UnifiedResultParser
│   └── TonyLayerImporter
│
├── Result Management Subsystem
│   ├── ResultLayerManager
│   ├── CorrectedLayerCoordinator
│   ├── CompareController
│   └── RegionAnalysisController
│
├── UI Subsystem
│   ├── Analysis method selector
│   ├── Backend Manager panel
│   ├── Analysis progress dialog/panel
│   └── Result/compare controls
│
└── Support Subsystem
    ├── ExportService
    ├── LoggingService
    ├── LicenseRegistry
    ├── SecurityGuard
    └── Future AICopilotBridge
```

### 8.3 Component view — Analysis Engine Subsystem

```text
AnalysisEngineManager
├── listEngines()
├── getEngineManifest(engineId)
├── validateEngineAvailability(engineId)
├── runFullFileAnalysis(engineId, audioRef, settings)
├── runRegionAnalysis(engineId, audioRef, region, settings)
├── cancelRun(runId)
└── emit runStateChanged / progressChanged / resultReady / runFailed

BackendRegistry
├── built-in engine definitions
├── user-configured backend locations
├── manifest cache
└── backend availability state

BackendAdapter
├── buildRequest()
├── run()
├── cancel()
├── parseOutput()
└── reportCapabilities()
```

### 8.4 Runtime view — full-file analysis

```text
User clicks Run Analysis
  ↓
Analysis UI asks AnalysisEngineManager
  ↓
Manager checks BackendRegistry + manifest
  ↓
BackendRequestBuilder creates request JSON
  ↓
TempWorkspaceManager creates run workspace
  ↓
BackendAdapter invokes ExternalProcessRunner
  ↓
ExternalProcessRunner starts backend process
  ↓
ProgressTracker updates UI
  ↓
Backend writes UnifiedResult JSON / output files
  ↓
ResultValidator validates schema + semantics
  ↓
UnifiedResultParser loads result
  ↓
TonyLayerImporter creates result layer
  ↓
ResultLayerManager stores layer with provenance
  ↓
UI shows completed/warning/failure state
```

### 8.5 Runtime view — selected-region analysis

```text
User selects time range
  ↓
RegionAnalysisController checks backend supports region OR host can slice audio
  ↓
TempWorkspaceManager creates audio slice if needed
  ↓
BackendRequestBuilder sets mode = selected_region
  ↓
Backend runs on slice or original+region
  ↓
Result timestamps normalized to original audio timeline
  ↓
Result imported as preview layer
  ↓
User applies/rejects/copies selected notes
```

### 8.6 Runtime view — compare mode

```text
User selects Compare Engines
  ↓
CompareController builds run queue
  ↓
Runs engine 1 → validates → stores layer
  ↓
Runs engine 2 → validates → stores layer
  ↓
Runs engine N → validates → stores layer
  ↓
ResultLayerManager groups layers by compare session
  ↓
UI shows backend variants and disagreements
  ↓
User chooses/copies/merges manually
```

---

## 9. Module design

## 9.1 AnalysisEngineManager

### Responsibility

Central service responsible for engine discovery, availability checks, analysis run orchestration, cancellation, and result handoff.

### Public responsibilities

```text
- list available engines
- expose engine status
- create analysis runs
- enforce sequential compare queue
- route full-file and selected-region requests
- emit state/progress/result/error events
- prevent UI from talking directly to external scripts
```

### Inputs

- loaded audio reference;
- selected engine ID;
- analysis mode;
- optional selected region;
- engine settings;
- output preferences.

### Outputs

- run state updates;
- progress updates;
- `UnifiedResult` object after validation;
- structured failure object.

### Must not

```text
- parse arbitrary backend-specific output directly in UI code;
- mark a run completed without valid result/import;
- block the GUI thread;
- overwrite corrected layer directly.
```

### Acceptance criteria

| ID | Criterion |
|---|---|
| AC-TDD-AEM-001 | UI can request engine list without knowing backend implementation details. |
| AC-TDD-AEM-002 | Running backend emits `running`, `completed`, `failed`, or `cancelled`. |
| AC-TDD-AEM-003 | Invalid backend output never reaches TonyLayerImporter. |
| AC-TDD-AEM-004 | Existing pYIN workflow remains accessible. |

---

## 9.2 BackendRegistry

### Responsibility

Stores known engines, user-configured paths, manifest locations, cached availability status, runtime type, and setup state.

### Data sources

```text
- built-in engine definitions shipped with the app
- user settings / config
- backend manifest files
- runtime detection checks
```

### Engine status enum

```text
not_configured
missing
installed
broken
unsupported
ready
running
failed
```

### Persistence

**ASSUMPTION-TDD-002:** On Qt, backend paths and user preferences should initially use the existing Tony settings mechanism if available; otherwise use Qt `QSettings` or a project-local JSON config. The final choice must be made after codebase mapping.

---

## 9.3 BackendManifestLoader

### Responsibility

Loads and validates `BackendManifest` files according to `backend_manifest.schema.json`.

### Behavior

```text
1. Load manifest JSON.
2. Validate against schema.
3. Validate semantic fields: engine_id, contract_version, capabilities.
4. Store manifest in registry.
5. Mark backend broken if manifest is invalid.
```

### Failure behavior

Invalid manifest must not crash the app. It must produce a Backend Manager status message.

---

## 9.4 BackendSettingsStore

### Responsibility

Stores backend-specific settings, executable paths, Python environment paths, model/checkpoint paths, and user preferences.

### Security constraints

- No API keys in source code.
- No secrets in logs.
- Cloud/API keys are future scope and must be stored using OS-appropriate secure storage when implemented.

### MVP storage fields

```text
engine_id
backend_executable_path
python_interpreter_path optional
working_directory optional
model_checkpoint_path optional
last_validated_at
last_known_version
user_enabled true/false
```

---

## 9.5 BackendAdapter interface

### Responsibility

Backend-specific adapter boundary. Each engine integration implements this interface, whether it uses an external process, existing Vamp path, or future native/ONNX code.

### Conceptual interface

```cpp
class IBackendAdapter {
public:
    virtual EngineId engineId() const = 0;
    virtual BackendManifest manifest() const = 0;
    virtual BackendAvailability checkAvailability() = 0;
    virtual BackendRequest buildRequest(const AnalysisRunContext&) = 0;
    virtual AnalysisRunHandle run(const BackendRequest&) = 0;
    virtual void cancel(AnalysisRunHandle) = 0;
    virtual ParsedBackendOutput collectOutput(AnalysisRunHandle) = 0;
};
```

This is conceptual. The exact C++ signature must follow Tony coding style after codebase mapping.

### Adapter types

| Adapter type | Used for |
|---|---|
| ExistingAnalysisAdapter | pYIN/Vamp already in Tony |
| ExternalProcessAdapter | Basic Pitch, CREPE Notes, MUSC, VioPTT, PESTO/PENN/FCPE |
| NativeOnnxAdapter | Future Basic Pitch/NeuralNote-style native path |
| SegmentationAdapter | f0 curve → notes |
| MockDevAdapter | development-only contract tests |

---

## 9.6 ExternalProcessRunner

### Responsibility

Runs external backend commands safely and asynchronously.

### Required capabilities

```text
- start process with explicit executable path and argument list
- set working directory
- pass environment variables only when required
- capture stdout
- capture stderr
- write/read request and result files
- emit progress events when available
- support cancellation/termination
- support timeout policy
- return exit code and structured run summary
```

### Qt implementation guidance

**ASSUMPTION-TDD-003:** Use Qt process APIs for external backend execution because the application is Qt-based. The runner must not block the GUI thread. If process orchestration or parsing becomes heavy, use a worker object in a background thread.

### Security constraints

```text
- do not build shell commands by string concatenation
- pass arguments as argument list
- validate executable path
- restrict working directory to run workspace or configured backend directory
- avoid inheriting unnecessary environment variables
- never log secrets
```

### Exit handling

| Process outcome | Host behavior |
|---|---|
| exit code 0 + valid result | import result |
| exit code 0 + invalid/missing result | failed: invalid output |
| non-zero exit code + error JSON | show structured error |
| non-zero exit code only | show stderr/log summary |
| timeout | terminate and mark timeout |
| user cancel | terminate and mark cancelled |

---

## 9.7 BackendRequestBuilder

### Responsibility

Creates `BackendRequest` JSON according to `backend_request.schema.json`.

### Inputs

```text
- analysis mode: full_file / selected_region / f0_segmentation
- audio source path
- optional selected region
- engine ID
- output workspace paths
- runtime preferences: CPU/GPU, timeout, threads
- backend settings
```

### Timestamp rule

All result timestamps must resolve to seconds from the start of the original audio, even if a temporary audio slice is created.

---

## 9.8 TempWorkspaceManager

### Responsibility

Creates isolated per-run directories for request JSON, output JSON, optional MIDI/CSV, logs, and temporary audio slices.

### Workspace layout

```text
<app-data>/runs/<run_id>/
    request.json
    result.json
    backend_stdout.log
    backend_stderr.log
    input_slice.wav optional
    raw_outputs/
    normalized/
```

### Cleanup policy

| Artifact | Default behavior |
|---|---|
| request/result JSON | keep while project/session active |
| stdout/stderr logs | keep locally, user-clearable |
| temp audio slices | delete after successful import unless debug mode enabled |
| raw backend MIDI/CSV | keep if user enables “save raw backend outputs” |

**ASSUMPTION-TDD-004:** MVP may keep run artifacts during session for debugging, with a manual cleanup option. A later privacy mode can auto-delete more aggressively.

---

## 9.9 ResultValidator

### Responsibility

Validates backend output before it can be imported.

### Validation layers

```text
1. JSON parse validation
2. JSON Schema validation
3. Contract version compatibility
4. Required fields validation
5. Semantic validation
6. Timeline validation
7. Capability/output consistency validation
```

### Semantic checks

```text
- note start_sec >= 0
- note end_sec > start_sec
- midi_pitch between 0 and 127
- velocity between 0 and 127 if present
- confidence between 0.0 and 1.0 if present
- frequency_hz > 0 if present
- events within audio duration, with tolerance
- selected-region results do not claim changes outside region unless explicitly allowed
```

### Quarantine behavior

Invalid results are quarantined in the run workspace and are not imported.

---

## 9.10 UnifiedResultParser

### Responsibility

Converts validated `UnifiedResult` JSON into internal C++ data structures.

### Internal models

```text
UnifiedResultModel
EngineMetadata
NoteEventModel
PitchPointModel
PitchBendModel
TechniqueLabelModel
WarningModel
ErrorModel
ProvenanceModel
```

### Rule

Parsing must not depend on UI components.

---

## 9.11 TonyLayerImporter

### Responsibility

Creates Tony/Sonic Visualiser-compatible note/pitch/result layers from `UnifiedResultModel`.

### Initial import priority

1. Note events.
2. Backend provenance.
3. Warnings/errors visible in run panel.
4. Pitch curve where supported.
5. Technique labels post-MVP.
6. Pitch bends/deviations future.

### Import behavior

| Result type | Import behavior |
|---|---|
| MVP note events | Create result note layer |
| Full-file run | Create result layer spanning full audio |
| Selected-region run | Create preview layer limited to selected region |
| Compare run | Create separate result layer per backend |
| Corrected application | Copy/replace notes only after user action |

### Mapping uncertainty

**OPEN QUESTION-TDD-001:** The exact Tony internal layer classes and import APIs must be confirmed during codebase mapping. Until confirmed, `TonyLayerImporter` is an architectural boundary, not a finalized class name.

---

## 9.12 ResultLayerManager

### Responsibility

Manages backend result layers, preview layers, compare groups, and the corrected layer relationship.

### Required concepts

```text
CorrectedLayer
ResultLayer
PreviewLayer
CompareSession
BackendProvenance
LayerVisibility
LayerLockState
```

### Non-destructive rule

Result layers are read-only or semi-read-only by default. User edits should occur on the corrected layer, or through explicit “copy/apply” actions.

---

## 9.13 BackendManager UI

### Responsibility

User-facing configuration and status surface for available engines.

### MVP fields

```text
engine name
engine ID
status
configured path
runtime type
last validation result
test button
help/setup message
```

### Post-MVP fields

```text
version
license summary
model/checkpoint status
supported features
CPU/GPU availability
raw logs shortcut
```

### UI rule

Backend Manager must distinguish `not configured`, `missing`, `broken`, `installed`, and `ready`.

---

## 9.14 Analysis run UI

### Responsibility

Allows user to select an engine, choose settings, run/cancel analysis, and inspect status.

### MVP UI elements

```text
Analysis Method dropdown
Run Analysis button
Cancel button
Status/progress area
Error/warning area
```

### Post-MVP UI elements

```text
Compare Engines button
Selected Region mode
Advanced settings drawer
Raw output/log viewer
```

---

## 9.15 CompareController

### Responsibility

Runs multiple compatible engines sequentially and groups results.

### Behavior

```text
1. Validate selected engines.
2. Build queue.
3. Run one engine at a time.
4. Store each valid result layer.
5. Record failures without aborting entire compare session unless user chooses.
6. Show summary at end.
```

### Future comparison logic

```text
- note matching by onset tolerance, pitch, duration overlap
- consensus count
- disagreement highlighting
- per-note provenance display
```

**ASSUMPTION-TDD-005:** MVP does not implement full compare mode. It prepares data structures so compare mode can be added without refactoring backend integration.

---

## 9.16 RegionAnalysisController

### Responsibility

Coordinates analysis of selected time ranges.

### Core behavior

```text
- read selected region from Tony UI
- check backend region capability
- create audio slice if required
- normalize result timestamps to original audio timeline
- import as preview layer
- apply only after user confirmation
```

### Safety rule

Selected-region apply must never modify notes outside the selected region.

---

## 9.17 ExportService

### Responsibility

Exports corrected note data and optionally backend result data.

### MVP exports

```text
- MIDI from corrected layer
- CSV note events, if feasible in MVP or immediately after
```

### Post-MVP exports

```text
- UnifiedResult JSON
- backend-specific raw outputs
- project file
- MusicXML future
```

### Export principle

The corrected layer is the default export source. Raw backend layers are exportable only when user chooses them.

---

## 9.18 LoggingService

### Responsibility

Captures local debug information without uploading data.

### Log categories

```text
backend_runner
backend_stdout
backend_stderr
result_validation
import_errors
ui_actions minimal
```

### Privacy rules

- Logs are local.
- User can clear logs.
- Logs must not contain API keys.
- Logs should avoid storing full audio content.

---

## 9.19 LicenseRegistry

### Responsibility

Tracks licenses and redistribution status of Tony and all optional backends/dependencies.

### Minimum fields

```text
component_name
upstream_url
license
redistribution_status
bundled true/false
notes
review_status
```

### Architecture consequence

Backends with restrictive or unclear licenses should be optional external dependencies until license review allows bundling.

---

## 9.20 SecurityGuard

### Responsibility

Centralizes checks for external process execution, path validation, secret handling, network policy, and safe logging.

### MVP checks

```text
validate executable path
validate working directory
reject dangerous missing/empty paths
avoid shell invocation
sanitize log messages
prevent production mock backend display
```

---

## 9.21 Future AICopilotBridge

### Responsibility

Future bridge between UI, symbolic note data, backend capabilities, and optional cloud/local AI.

### Scope boundary

AI Copilot is not part of MVP. Architecture reserves a boundary so it can later:

```text
- read symbolic analysis summaries
- recommend backends
- explain disagreements
- suggest corrections
- call safe application actions after user confirmation
```

### Non-goal

AI Copilot must not be an audio-to-MIDI replacement and must not be framed as watermark removal or detector bypass.

---

## 10. Data model mapping from Backend Contract to Tony layers

### 10.1 Contract data to host model

| Backend Contract field | Internal model | UI/layer target |
|---|---|---|
| `engine` metadata | `EngineMetadata` | Run panel, layer provenance |
| `notes[]` | `NoteEventModel` | Note/result layer |
| `pitch_curve[]` | `PitchPointModel` | Pitch curve layer |
| `pitch_bends[]` | `PitchBendModel` | Future pitch-bend display/export |
| `technique_labels[]` | `TechniqueLabelModel` | Future technique annotation layer |
| `warnings[]` | `WarningModel` | Run panel/layer warning badges |
| `errors[]` | `ErrorModel` | Failure state/log viewer |
| `provenance` | `ProvenanceModel` | Compare/audit/export metadata |

### 10.2 Layer model

```text
CorrectedLayer
    editable, default export source

ResultLayer
    backend output, read-only by default

PreviewLayer
    temporary result from selected-region analysis

CompareLayerGroup
    group of result layers generated by one compare session
```

### 10.3 Time normalization

All internal events use seconds from original audio start.

For selected-region runs:

```text
backend_local_time + region.start_sec = original_time
```

Unless backend already returns original timestamps and declares this in output metadata.

---

## 11. Full-file analysis design

### 11.1 Preconditions

```text
- audio file is loaded
- selected backend is ready
- backend supports full_file mode
- output workspace is available
```

### 11.2 Flow

```text
User → Analysis UI → AnalysisEngineManager → BackendAdapter → Runner → Validator → Importer → ResultLayerManager → UI
```

### 11.3 Failure points

| Failure | Handling |
|---|---|
| no audio loaded | disable run or show message |
| backend not configured | open Backend Manager |
| backend missing | show setup help |
| run fails | show failed state and logs |
| result invalid | quarantine output and show validation error |
| import fails | keep result file, show import error |

---

## 12. Selected-region analysis design

### 12.1 Preconditions

```text
- audio file is loaded
- user has selected a valid time region
- selected backend supports region analysis OR host can create a slice
```

### 12.2 Slice strategy

Some backends may not support original-file-with-region arguments. The host can create a temporary audio slice and then normalize timestamps back to the original audio timeline.

**OPEN QUESTION-TDD-002:** The best audio-slicing path depends on existing Tony/Sonic Visualiser APIs. If not available, a small local audio slicing utility may be required.

### 12.3 Preview strategy

Selected-region results are imported as preview layers. The corrected layer changes only when the user applies the result.

### 12.4 Apply modes

| Mode | Behavior |
|---|---|
| Replace region | Remove corrected notes inside region and insert preview notes |
| Copy selected | Copy only selected preview notes |
| Keep as layer | Keep preview without changing corrected layer |
| Reject | Delete preview layer |

---

## 13. Compare-mode design

### 13.1 Compare session object

```text
CompareSession
    session_id
    audio_file_id
    region optional
    selected_engines[]
    result_layer_ids[]
    failures[]
    created_at
```

### 13.2 Sequential run queue

Compare mode must run engines sequentially by default.

Reasons:

- CPU-first requirement;
- avoids memory spikes from multiple Python/ML processes;
- easier progress and error reporting;
- easier cancellation semantics.

### 13.3 Compare visual model

MVP/post-MVP v1:

```text
- show separate layers
- allow toggling layer visibility
- allow choosing a whole layer as corrected result
```

Later:

```text
- per-note copy
- consensus/voting
- disagreement highlighting
- AI explanation
```

### 13.4 Note matching for future consensus

Future matching should consider:

```text
pitch match or near match
onset tolerance
duration overlap
confidence
backend provenance
```

No automatic merge should happen without user confirmation.

---

## 14. f0-backend + note segmentation design

### 14.1 Problem

PESTO, PENN, and FCPE are primarily f0/pitch engines. They should not be treated as complete note editors.

### 14.2 Architecture

```text
F0 Backend Adapter
    ↓
UnifiedResult with pitch_curve only
    ↓
SegmentationAdapter
    ↓
UnifiedResult with notes + pitch_curve
    ↓
TonyLayerImporter
```

### 14.3 Segmentation settings

Future segmentation layer may expose:

```text
minimum note duration
pitch stability threshold
onset sensitivity
confidence threshold
merge tolerance
split tolerance
```

### 14.4 MVP status

f0 segmentation is post-MVP. The MVP should not block on this.

---

## 15. Backend adapter strategy by engine

### 15.1 pYIN Vamp adapter

| Field | Design |
|---|---|
| Type | Existing Tony/Vamp workflow wrapper |
| MVP status | Must remain available |
| Contract role | Expose as `pyin_vamp` engine in registry |
| Output | Existing pitch/note layers or normalized result if feasible |
| Risk | Existing code path may not map cleanly to new `UnifiedResult` initially |

### 15.2 Basic Pitch / NeuralNote adapter

| Field | Design |
|---|---|
| Type | External backend first; native ONNX later |
| MVP status | Recommended first real backend |
| Request | full-file audio path, output directory, runtime preference CPU |
| Output | MIDI and/or note-events CSV normalized to `UnifiedResult` |
| UI | Engine selector + Backend Manager config |
| Risk | Python/ONNX environment setup |

### 15.3 CREPE Notes adapter

| Field | Design |
|---|---|
| Type | External backend first; possible Vamp/native later |
| Status | Post-MVP Phase 1 |
| Output | MIDI/note segmentation |
| Scope | Monophonic material |
| Risk | dependency complexity and repeated same-pitch note limitations |

### 15.4 MUSC Violin adapter

| Field | Design |
|---|---|
| Type | External backend |
| Status | Post-MVP Phase 2 |
| Output | violin-specialized transcription data normalized to notes/pitch deviations |
| Scope | solo violin first; cello support not assumed unless proven |
| Risk | research-code fragility and model/checkpoint management |

### 15.5 VioPTT adapter

| Field | Design |
|---|---|
| Type | External backend |
| Status | Post-MVP Phase 3 |
| Output | MIDI + technique labels CSV normalized to notes + technique labels |
| Scope | violin technique-aware transcription |
| Risk | technique label UI and model dependency complexity |

### 15.6 PESTO / PENN / FCPE adapters

| Field | Design |
|---|---|
| Type | External f0 backends |
| Status | Post-MVP Phase 4 |
| Output | pitch curve/f0 data |
| Additional layer | note segmentation required |
| Risk | f0-to-note conversion quality and parameter design |

---

## 16. Threading, process, cancellation, and progress design

### 16.1 UI thread rule

The UI thread must not perform blocking backend execution, long parsing, or long import operations.

### 16.2 External process model

```text
AnalysisEngineManager
    starts run
ExternalProcessRunner
    owns QProcess-like object
ProgressTracker
    forwards progress/log updates
ResultValidator
    runs after process completion
TonyLayerImporter
    imports on safe application thread/path according to Tony requirements
```

### 16.3 Cancellation

Cancellation should:

```text
1. request graceful termination if backend supports it;
2. terminate process after timeout;
3. mark run as cancelled;
4. keep partial logs;
5. not import partial invalid output unless explicitly marked valid partial result by contract.
```

### 16.4 Progress

Progress sources:

```text
- backend structured progress JSON, if available
- stdout progress parsing, if adapter supports it
- indeterminate progress when backend cannot report progress
```

The UI must show honest progress. Indeterminate is better than fake percentage.

---

## 17. File system, temp workspace, settings, and logs

### 17.1 Config locations

**ASSUMPTION-TDD-006:** Use platform-appropriate app data/config directories rather than storing user settings in the source tree.

### 17.2 Runtime workspace

Each run gets a unique run directory.

```text
runs/<run_id>/request.json
runs/<run_id>/result.json
runs/<run_id>/logs/
runs/<run_id>/raw_outputs/
runs/<run_id>/temp_audio/
```

### 17.3 Project files

Project save/load is post-MVP. When implemented, project files should reference:

```text
- audio file reference
- corrected layer
- result layers
- backend run metadata
- analysis settings
- warnings/errors summary
```

### 17.4 Log retention

Logs should be user-clearable. Debug-mode raw output retention should be explicit.

---

## 18. Error handling and state model

### 18.1 Run state enum

```text
idle
ready
queued
running
cancelling
cancelled
completed
completed_with_warnings
failed
```

### 18.2 Backend availability state enum

```text
not_configured
missing
installed
ready
broken
unsupported
```

### 18.3 Error categories

```text
backend_missing
backend_not_configured
backend_dependency_missing
backend_process_failed
backend_timeout
user_cancelled
output_missing
output_invalid
schema_validation_failed
semantic_validation_failed
import_failed
unsupported_mode
privacy_blocked
```

### 18.4 Error display principle

User-facing errors should be short and actionable. Detailed logs should be available separately.

Example:

```text
Basic Pitch failed because the configured Python environment could not import basic_pitch.
Open Backend Manager to fix the path, or view logs.
```

---

## 19. Security and privacy architecture

### 19.1 Threat model summary

| Asset | Threat | Mitigation |
|---|---|---|
| Audio files | unintended upload | local-first; cloud opt-in only |
| API keys | source-code leakage | never store in source; future secure storage |
| External command execution | command injection | no shell concatenation; argument list only |
| User file paths | log leakage | privacy-conscious logs; user-clearable |
| Backend outputs | malformed/malicious data | schema and semantic validation |
| Temp audio slices | leftover sensitive files | cleanup policy |
| UI trust | fake success states | no-fake policy and validation before import |

### 19.2 Process execution security

Rules:

```text
- only run configured executable paths
- do not execute arbitrary text from backend manifests
- pass args separately, not via shell string
- validate paths before execution
- restrict working directory
- capture logs safely
- never include secrets in logs
```

### 19.3 Network policy

MVP backends are local. Network access is not required by the host for analysis. Future AI Copilot/cloud features must be explicit opt-in.

### 19.4 Mock backend policy

Mock/dev backend must be:

```text
- unavailable in production build OR clearly labeled as Development/Test
- impossible to confuse with real analysis
- excluded from default user workflow
```

---

## 20. Performance architecture

### 20.1 CPU-first design

The initial product must be usable on CPU-only systems.

### 20.2 Sequential multi-engine design

Compare mode defaults to sequential execution.

### 20.3 Large-file strategy

For long audio:

```text
- show progress or indeterminate state
- allow cancellation
- avoid loading excessive intermediate data into UI at once
- keep raw backend outputs on disk until parsed
```

### 20.4 Future GPU support

GPU support should be backend-specific and capability-driven. The UI should not expose GPU options unless the backend manifest declares support.

---

## 21. UI integration strategy

### 21.1 MVP UI integration

Add minimal UI surfaces:

```text
Analysis Method selector
Run Analysis
Cancel Analysis
Backend Manager entry
Run status panel/dialog
Result layer imported into existing Tony display
```

### 21.2 Post-MVP UI integration

```text
Compare Engines panel
Selected Region analysis button
Layer comparison controls
Confidence display controls
Technique label toggle
Advanced backend settings
```

### 21.3 Design rule

Do not perform full visual redesign before the first real backend path works.

---

## 22. Export architecture

### 22.1 Export source model

```text
CorrectedLayer = default export source
ResultLayer = exportable only by explicit user selection
PreviewLayer = not exported unless applied or saved intentionally
```

### 22.2 Export paths

MVP:

```text
CorrectedLayer → MIDI
CorrectedLayer → CSV if feasible
```

Post-MVP:

```text
ResultLayer → UnifiedResult JSON
CompareSession → bundle of JSON/CSV outputs
Project → full session save/load
```

### 22.3 Validation before export

Before export, note events should be checked for:

```text
valid pitch
valid start/end
duration > 0
no impossible NaN/invalid values
```

---

## 23. Testing and validation strategy

### 23.1 Test levels

| Level | Tests |
|---|---|
| Unit | manifest parsing, request building, result validation, time normalization |
| Integration | ExternalProcessRunner with controlled test backend, Basic Pitch adapter, import pipeline |
| UI/manual | load audio, run analysis, see notes, cancel backend, handle failure |
| Regression | existing Tony pYIN workflow still works |
| Contract | JSON examples validate against schemas |
| Security | invalid paths, malformed outputs, secret/log checks |

### 23.2 MVP acceptance tests

```text
1. Build original Tony fork.
2. Verify existing pYIN workflow.
3. Configure first external backend path.
4. Run full-file analysis on test WAV.
5. Validate result JSON.
6. Import note events as result layer.
7. Manually edit notes.
8. Export MIDI.
9. Backend failure does not crash app.
10. UI does not show completed state for invalid output.
```

### 23.3 Test fixtures

Recommended fixture set:

```text
short clean solo violin WAV
short clean solo cello WAV
short polyphonic single-instrument Basic Pitch test
empty/silent audio
invalid backend output JSON
missing result file
backend exits non-zero
long-running dummy backend for cancellation
```

### 23.4 Contract validation

The JSON examples under `docs/examples/` must continue to validate against schemas under `docs/schemas/`.

---

## Real Result and Tony Layer Integration Quality Gates

Before TonyLayerImporter, UI integration, MainWindow/Analyser integration, Basic Pitch workflow, selected-region replacement, save/load, or export behavior is implemented, the engineering proof documents under `docs/engineering/` are mandatory.

Required documents:

- `docs/engineering/REAL_RESULT_AND_TONY_LAYER_INTEGRATION_RULES.md`
- `docs/engineering/LAYER_TYPE_POLICY.md`
- `docs/engineering/BACKEND_OUTPUT_TRUTH_TABLE.md`
- `docs/engineering/TONY_EDIT_SAVE_EXPORT_PROOF_PLAN.md`
- `docs/engineering/UI_VISUAL_TRUTH_STATES.md`
- `docs/engineering/PROVENANCE_METADATA_POLICY.md`

### Mandatory end-to-end proof chain

Future real-backend features must prove the highest relevant part of this chain:

```text
real audio
-> real backend execution or existing pYIN path
-> real backend output file(s)
-> valid UnifiedResult
-> real Tony/Sonic Visualiser model/layer
-> visible layer in Tony
-> editable through Tony mechanisms where claimed
-> save/load where claimed
-> export where claimed
```

Compile-only tasks, parser-only tasks, and dev/mock tasks may stop earlier, but their final reports must state exactly where they stop.

### Quality gate rules

- Path checks are not Ready or Installed.
- Backend process success is not result import.
- Loaded UnifiedResult is not a Tony layer.
- A custom overlay is not a Tony/Sonic Visualiser layer.
- Editable requires a real editable Tony model/layer and edit proof.
- Saved/exported requires file/session evidence.
- f0 is not automatically notes.
- Technique labels are not notes.
- Polyphonic backend output must not be blindly forced into one monophonic Tony note layer.
- pYIN behavior remains the baseline and must be regression checked when relevant.

---

## 24. Build, packaging, and installation strategy

### 24.1 MVP build strategy

```text
1. Build original Tony fork unchanged.
2. Add backend architecture behind feature flags/menu items.
3. Use user-configured external backend paths.
4. Do not bundle large Python/ML environments in MVP.
```

### 24.2 Backend installation strategy

MVP:

```text
User installs backend separately.
User configures path in Backend Manager.
App validates the path and reports status.
```

Post-MVP:

```text
Optional guided setup.
Optional bundled environments where licensing/size allow.
Optional native ONNX for selected backends.
```

### 24.3 Windows installer

A Windows installer is post-MVP, after backend architecture is stable.

---

## 25. Phased implementation plan

### Phase 0 — Documentation baseline

Deliverables:

```text
docs/02_PRD.md
docs/03_SRS.md
docs/04_BACKEND_CONTRACT.md
docs/05_TDD_ARCHITECTURE.md
```

### Phase 1 — Build Tony unchanged

Goal: prove original fork builds and runs.

Deliverables:

```text
BUILD_WINDOWS.md
build log
known build issues
```

### Phase 2 — Codebase mapping

Goal: identify exact Tony insertion points.

Map:

```text
analysis path
pYIN transform call
note layer creation
pitch layer creation
import/export code
main window menu/actions
settings persistence
undo/redo integration
```

### Phase 3 — Architecture skeleton

Deliver:

```text
AnalysisEngineManager skeleton
BackendRegistry skeleton
BackendManifestLoader
BackendSettingsStore
no production behavior changes yet
```

### Phase 4 — Contract validation skeleton

Deliver:

```text
ResultValidator
UnifiedResultParser
schema validation for example results
```

### Phase 4A — Engineering rules and Tony layer proof gates

Goal: codify proof requirements before Tony layer import, UI state, real backend workflow, selected-region replacement, save/load, or export work.

Deliver:

```text
real-result and Tony layer integration rules
layer type policy
backend output truth table
edit/save/export proof plan
UI visual truth states
provenance metadata policy
```

### Phase 5 — Dev-only mock adapter

Deliver:

```text
MockDevAdapter reads known valid UnifiedResult JSON
TonyLayerImporter creates result layer
mock clearly marked dev-only
```

### Phase 6 — ExternalProcessRunner

Deliver:

```text
runner starts controlled test process
captures stdout/stderr
handles exit codes
supports cancellation
```

### Phase 7 — First real backend: Basic Pitch / NeuralNote path

Deliver:

```text
BasicPitchExternalAdapter
Backend Manager configuration
full-file analysis
validated import
MIDI export from corrected layer
```

### Phase 8 — CREPE Notes

Deliver:

```text
CrepeNotesExternalAdapter
monophonic use-case documentation
compare with pYIN/Basic Pitch later
```

### Phase 9 — Selected-region preview

Deliver:

```text
RegionAnalysisController
preview layer
apply/reject/copy behavior
```

### Phase 10 — Compare mode v1

Deliver:

```text
sequential queue
multi-result layers
whole-layer choose/copy behavior
```

### Phase 11+ — Specialized backends

Deliver:

```text
MUSC adapter
VioPTT adapter + technique layer
PESTO/PENN/FCPE f0 adapters + segmentation
AI Copilot future bridge
```

---

## 26. ADRs to create

Create an `docs/adr/` directory and record these decisions:

| ADR | Title | Status |
|---|---|---|
| ADR-0001 | Use Tony fork instead of new app from scratch | Proposed/Accepted |
| ADR-0002 | External backends first, native/Vamp later | Proposed/Accepted |
| ADR-0003 | Use Backend Contract + UnifiedResult | Proposed/Accepted |
| ADR-0004 | Basic Pitch / NeuralNote path as first real backend | Proposed |
| ADR-0005 | CPU-first and sequential compare execution | Proposed |
| ADR-0006 | Mock backend allowed only as dev/test tool | Proposed/Accepted |
| ADR-0007 | Local-first with future opt-in AI/cloud | Proposed/Accepted |
| ADR-0008 | Result layers non-destructive by default | Proposed/Accepted |

### ADR template

```md
# ADR-000X — Title

## Status
Proposed | Accepted | Deprecated | Superseded

## Context
What issue forces this decision?

## Decision
What is the decision?

## Consequences
Positive and negative consequences.

## Alternatives considered
What else was considered and rejected?
```

---

## 27. Traceability matrix to SRS requirements

| Architecture element | SRS requirements |
|---|---|
| AnalysisEngineManager | REQ-CORE-001, REQ-BACKEND-001, REQ-BACKEND-003, REQ-CORE-006, REQ-CORE-007 |
| BackendRegistry | REQ-BACKEND-005, REQ-BM-001, REQ-BM-002 |
| BackendManifestLoader | REQ-BM-002, REQ-RESULT-001, Backend Contract manifest rules |
| BackendSettingsStore | REQ-BACKEND-005, REQ-PLAT-003, REQ-PRIV-002 |
| ExternalProcessRunner | REQ-BACKEND-003, REQ-BACKEND-004, REQ-ERR-001, REQ-ERR-002, REQ-ERR-005, REQ-PERF-003 |
| ResultValidator | REQ-RESULT-001, REQ-ERR-003, Backend Contract validation rules |
| UnifiedResultParser | REQ-RESULT-001, REQ-RESULT-002, REQ-RESULT-003, REQ-RESULT-004 |
| TonyLayerImporter | REQ-VIS-001, REQ-VIS-002, REQ-EDIT-001 |
| ResultLayerManager | REQ-CORE-005, REQ-COMPARE-002, REQ-RESULT-005 |
| RegionAnalysisController | REQ-REGION-001, REQ-REGION-002, REQ-REGION-003, REQ-REGION-004 |
| CompareController | REQ-COMPARE-001 through REQ-COMPARE-006, REQ-PERF-002 |
| BackendManager UI | REQ-BM-001 through REQ-BM-005, REQ-UI-001 |
| Analysis run UI | REQ-UI-001, REQ-UI-002, REQ-UI-006 |
| ExportService | REQ-EXPORT-001 through REQ-EXPORT-005 |
| LoggingService | REQ-ERR-004, REQ-PRIV-004 |
| SecurityGuard | REQ-PRIV-001 through REQ-PRIV-005 |
| LicenseRegistry | REQ-LIC-001 through REQ-LIC-003 |
| AICopilotBridge | REQ-AI-001 through REQ-AI-006 |

---

## 28. Open questions

| ID | Question | Blocks MVP? |
|---|---|---|
| OQ-TDD-001 | Exact Tony source classes for note/pitch/result layer import must be mapped. | Yes, for implementation |
| OQ-TDD-002 | Exact audio slicing implementation path must be confirmed. | No, post-MVP |
| OQ-TDD-003 | Exact settings storage mechanism in Tony must be confirmed. | No, can adapt |
| OQ-TDD-004 | Whether MVP CSV export is easy through existing code or requires new exporter. | No, MIDI can be first |
| OQ-TDD-005 | Exact Basic Pitch invocation path on target Windows system. | Yes, for first backend |
| OQ-TDD-006 | Whether to expose pYIN as normalized `UnifiedResult` immediately or keep legacy path initially. | No, but affects compare mode |
| OQ-TDD-007 | License status of each backend before bundling. | No for local-only MVP, yes before public distribution |

---

## 29. Professional TDD checklist

Before Codex implementation starts, this document should be checked for:

```text
- clear scope and non-scope
- traceability to SRS requirements
- stable module boundaries
- no fake production states
- external process safety
- validation-before-import rule
- non-destructive selected-region design
- sequential compare design
- local-first privacy design
- test strategy
- phased implementation plan
```

---

## 30. Non-technical summary

The application should not be rebuilt from zero. Tony remains the editor. New transcription tools are added behind a clean backend system.

The core technical idea is:

```text
External backend → Backend Contract → UnifiedResult → Tony layer → manual correction → export
```

The first real version should prove one complete path:

```text
Open audio → run Basic Pitch-style backend → validate result → display notes → correct → export MIDI
```

After that, the same architecture can add CREPE Notes, MUSC, VioPTT, PESTO/PENN/FCPE, selected-region analysis, compare mode, and later AI Copilot.

The most important rules are:

```text
Do not break Tony.
Do not fake results.
Do not block the UI.
Validate before import.
Keep backend results non-destructive.
Run locally by default.
Build in small, testable phases.
```

---

## 31. Reference sources used for architecture methodology

- Google Technical Writing — design documents should define scope and non-scope clearly: https://developers.google.com/tech-writing/one/documents
- C4 Model — context/container/component style architecture visualization: https://c4model.com/
- ADR guidance — lightweight records for architecturally significant decisions: https://adr.github.io/
- Microsoft SDL threat modeling — define security requirements, create diagrams, identify threats, mitigate, validate: https://www.microsoft.com/en-us/securityengineering/sdl/threatmodeling
- Qt QProcess — starts external programs and communicates with them: https://doc.qt.io/qt-6/qprocess.html
- Qt QThread — supports worker objects and separate threads: https://doc.qt.io/qt-6/qthread.html
- Qt Model/View — separates data from views: https://doc.qt.io/qt-6/model-view-programming.html
- Qt QSettings — persistent platform-independent application settings: https://doc.qt.io/qt-6/qsettings.html
