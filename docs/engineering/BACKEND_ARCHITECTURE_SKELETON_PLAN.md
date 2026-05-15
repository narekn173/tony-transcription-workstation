# CODEX-030 Backend Architecture Skeleton Plan

Status: proposal only  
Repository: `narekn173/tony-transcription-workstation`  
Branch: `default`  
Date: 2026-05-15

This document proposes the concrete source-code architecture for adding the backend infrastructure skeleton. It is intentionally limited to a compile-only skeleton plan. It does not integrate Basic Pitch, run external backends, modify existing pYIN behavior, redesign the UI, or refactor Tony/Sonic Visualiser internals.

## 1. Scope

The next implementation slice should add backend infrastructure types and empty/safe orchestration boundaries that compile but are not wired into Tony's current analysis menu or pYIN execution path.

The skeleton should prepare for:

- engine discovery and registry state;
- backend adapter boundaries;
- backend request/result data models;
- external process execution abstraction;
- conceptual schema-backed result validation;
- future Tony layer import through existing `Document` ownership APIs.

The skeleton should not yet:

- expose new production UI controls;
- run Basic Pitch, Python, ONNX, CREPE, MUSC, VioPTT, PESTO, PENN, or FCPE;
- route `MainWindow::analyseNow()` through the new manager;
- replace or wrap `Analyser::addAnalyses()`;
- import backend results into Tony layers;
- report fake progress, fake notes, fake confidence, or fake completion.

## 2. Documents and Code Map Read

Inputs reviewed for this plan:

- `AGENTS.md`
- `docs/00_PROJECT_INDEX.md`
- `docs/02_PRD.md`
- `docs/03_SRS.md`
- `docs/04_BACKEND_CONTRACT.md`
- `docs/05_TDD_ARCHITECTURE.md`
- `docs/engineering/CODEBASE_MAP.md`

Key codebase facts from `docs/engineering/CODEBASE_MAP.md`:

- Existing pYIN analysis is centered in `main/Analyser.cpp`, especially `Analyser::addAnalyses()`.
- Analysis menu/actions are in `main/MainWindow.cpp`, especially `MainWindow::setupAnalysisMenu()` and `MainWindow::analyseNow()`.
- Pitch and note layers are currently produced by pYIN through `Document::createDerivedLayers(...)` and then assigned to `TimeValueLayer` and `FlexiNoteLayer` instances.
- Existing import/export logic lives mostly in `main/MainWindow.cpp` and framework/file I/O helpers.
- `svapp/framework/Document.*` owns model/layer lifecycle and should be used later rather than bypassed.
- Existing pYIN/Vamp setup in `main/main.cpp` and `main/Analyser.cpp` is high risk and should not be changed in the skeleton.

## 3. Proposed Source Layout

Add a Tony-specific backend architecture folder under `main/` rather than under `svcore`, `svgui`, or `svapp`.

Recommended root:

```text
main/backend/
```

Rationale:

- Backend orchestration is product-specific to this Tony fork.
- It should not become part of the Sonic Visualiser framework libraries yet.
- It keeps the first diff localized and reviewable.
- It avoids risky changes to `svapp/framework/Document.*`, `svgui/layer/*`, `svcore/transform/*`, and pYIN plugin code.

Recommended subfolders:

```text
main/backend/core/
main/backend/contract/
main/backend/adapters/
main/backend/runner/
main/backend/import/
```

The first compile-only skeleton can use the subfolders above, or it can keep all files directly under `main/backend/` if the maintainer wants fewer Meson source entries. The subfolder layout is cleaner for long-term growth.

## 4. New Folders and Files to Add

### Compile-only files for CODEX-031

These are recommended for the immediate skeleton implementation:

```text
main/backend/core/AnalysisEngineManager.h
main/backend/core/AnalysisEngineManager.cpp
main/backend/core/BackendRegistry.h
main/backend/core/BackendRegistry.cpp
main/backend/core/BackendTypes.h
main/backend/core/BackendTypes.cpp

main/backend/adapters/BackendAdapter.h
main/backend/adapters/BackendAdapter.cpp

main/backend/contract/BackendManifest.h
main/backend/contract/BackendManifest.cpp
main/backend/contract/BackendRequest.h
main/backend/contract/BackendRequest.cpp
main/backend/contract/UnifiedResult.h
main/backend/contract/UnifiedResult.cpp
main/backend/contract/ResultValidator.h
main/backend/contract/ResultValidator.cpp

main/backend/runner/ExternalProcessRunner.h
main/backend/runner/ExternalProcessRunner.cpp

main/backend/import/TonyLayerImporter.h
main/backend/import/TonyLayerImporter.cpp
```

### Files to defer

Do not add these yet unless a later task explicitly asks for them:

```text
main/backend/adapters/BasicPitchExternalAdapter.*
main/backend/adapters/CrepeNotesExternalAdapter.*
main/backend/adapters/MuscViolinExternalAdapter.*
main/backend/adapters/VioPttExternalAdapter.*
main/backend/adapters/PestoExternalAdapter.*
main/backend/adapters/PennExternalAdapter.*
main/backend/adapters/FcpeExternalAdapter.*
main/backend/adapters/PyinVampAdapter.*
main/backend/ui/*
```

Reason: backend-specific adapters and UI wiring would risk implying integration before the shared contract pipeline exists.

## 5. Proposed Classes and Interfaces

### `BackendTypes`

File:

```text
main/backend/core/BackendTypes.h
```

Purpose: central small value types and enums shared by the skeleton.

Recommended compile-only contents:

- `BackendId`
- `AnalysisRunId`
- `BackendRuntimeType`
- `BackendAvailabilityState`
- `AnalysisRunState`
- `AnalysisMode`
- `BackendErrorCode`
- `BackendCapabilityFlags`
- lightweight structs such as `BackendError`, `BackendWarning`, `AnalysisRegion`, `AnalysisRunSummary`

The first implementation can use Qt value types such as `QString`, `QStringList`, `QDateTime`, `QUrl`, `QVariantMap`, and `QJsonObject` because the existing application is Qt-based.

### `BackendManifest`

Files:

```text
main/backend/contract/BackendManifest.h
main/backend/contract/BackendManifest.cpp
```

Purpose: in-memory representation of `BackendManifest` from the Backend Contract.

Compile-only behavior:

- hold fields such as contract version, engine ID, display name, runtime type, capability flags, supported outputs, and license metadata;
- expose getters and an `isValidShapeForSkeleton()` style placeholder if useful;
- do not load files from disk yet;
- do not claim runtime availability.

### `BackendRequest`

Files:

```text
main/backend/contract/BackendRequest.h
main/backend/contract/BackendRequest.cpp
```

Purpose: in-memory representation of a single backend analysis request.

Compile-only behavior:

- model the conceptual fields from `docs/04_BACKEND_CONTRACT.md`: request ID, engine ID, mode, input audio path, optional region, output workspace, settings, runtime preferences;
- provide construction helpers only if they do not touch Tony runtime state;
- do not write request JSON yet unless a later contract task asks for serialization.

### `UnifiedResult`

Files:

```text
main/backend/contract/UnifiedResult.h
main/backend/contract/UnifiedResult.cpp
```

Purpose: internal C++ result model corresponding to `UnifiedResult`.

Recommended nested or related structs:

- `EngineMetadata`
- `UnifiedNoteEvent`
- `UnifiedPitchPoint`
- `UnifiedPitchBend`
- `UnifiedTechniqueLabel`
- `UnifiedOutputFileRef`
- `UnifiedResultSummary`
- `UnifiedResult`

Compile-only behavior:

- store data only;
- no parsing yet;
- no Tony layer conversion yet;
- no fake sample results.

### `ResultValidator`

Files:

```text
main/backend/contract/ResultValidator.h
main/backend/contract/ResultValidator.cpp
```

Purpose: validate `UnifiedResult` before any future import.

Compile-only behavior:

- expose an interface such as `validate(const UnifiedResult&, const BackendRequest&)`;
- return a structured validation report;
- implement only minimal deterministic semantic checks if desired, such as empty result object detection;
- keep JSON Schema validation as a documented TODO for the next contract-validation task.

### `BackendAdapter`

Files:

```text
main/backend/adapters/BackendAdapter.h
main/backend/adapters/BackendAdapter.cpp
```

Purpose: abstract boundary implemented later by pYIN, external process, native ONNX, and dev mock adapters.

Conceptual interface:

```cpp
class BackendAdapter {
public:
    virtual ~BackendAdapter();

    virtual QString engineId() const = 0;
    virtual BackendManifest manifest() const = 0;
    virtual BackendAvailabilityState checkAvailability() const = 0;
    virtual bool supportsMode(AnalysisMode mode) const = 0;
};
```

Do not include real `run()` behavior in the first skeleton unless it only returns a safe `NotImplemented` result. Running belongs after the manager, runner, validator, and dev mock steps are scheduled.

### `AnalysisEngineManager`

Files:

```text
main/backend/core/AnalysisEngineManager.h
main/backend/core/AnalysisEngineManager.cpp
```

Purpose: future orchestration service that owns the list of engines and coordinates analysis runs.

Compile-only behavior:

- construct with or own a `BackendRegistry`;
- expose read-only engine listing APIs;
- expose placeholder run methods that return `NotImplemented` or `unsupported` without side effects;
- no connection to `MainWindow` yet;
- no calls into `Analyser` yet;
- no external process execution yet.

### `BackendRegistry`

Files:

```text
main/backend/core/BackendRegistry.h
main/backend/core/BackendRegistry.cpp
```

Purpose: registry for known backend adapters and manifests.

Compile-only behavior:

- hold adapter pointers or references conceptually;
- support registration of adapters in memory;
- support listing manifests/statuses;
- do not probe user machines yet;
- do not load backend manifests from disk yet unless limited to inert built-in placeholder metadata.

### `ExternalProcessRunner`

Files:

```text
main/backend/runner/ExternalProcessRunner.h
main/backend/runner/ExternalProcessRunner.cpp
```

Purpose: future isolated process execution wrapper.

Compile-only behavior:

- define request/result structs for process execution;
- define safe API shape using executable path plus argument list;
- return `NotImplemented` without launching a process;
- do not include Basic Pitch invocation;
- do not use shell command strings.

### `TonyLayerImporter`

Files:

```text
main/backend/import/TonyLayerImporter.h
main/backend/import/TonyLayerImporter.cpp
```

Purpose: future bridge from validated `UnifiedResult` to Tony/Sonic Visualiser layers.

Compile-only behavior:

- define an importer class and result types;
- document that actual import must use `svapp/framework/Document` ownership APIs;
- do not include `Document`, `Layer`, `NoteModel`, `FlexiNoteLayer`, or `TimeValueLayer` manipulation yet unless unavoidable;
- return `NotImplemented` for import calls.

## 6. `AnalysisEngineManager` and `BackendAdapter` Relationship

`AnalysisEngineManager` should orchestrate analysis at a high level. `BackendAdapter` should encapsulate engine-specific behavior.

Recommended relationship:

```text
AnalysisEngineManager
    owns or references BackendRegistry
        owns or references BackendAdapter instances
```

Responsibilities:

| Component | Owns | Must not own |
|---|---|---|
| `AnalysisEngineManager` | run state, engine selection, queue decisions, validation/import sequencing | backend-specific CLI syntax, raw output parsing in UI |
| `BackendRegistry` | adapter registration, manifest/status cache | long-running execution |
| `BackendAdapter` | backend-specific manifest, availability check, request adaptation | Tony layer mutation, UI widgets |
| `ExternalProcessRunner` | process lifecycle for external adapters | backend semantics, result validation |
| `ResultValidator` | contract/schema/semantic validation | process execution, UI state |
| `TonyLayerImporter` | future layer import boundary | backend execution or schema validation |

The manager should eventually perform this sequence:

```text
1. Select adapter by engine_id.
2. Ask adapter for manifest and availability.
3. Build or receive BackendRequest.
4. Run adapter.
5. Receive raw or normalized output location.
6. Parse and validate UnifiedResult.
7. Hand validated result to TonyLayerImporter.
8. Report completion only after validation and import succeed.
```

For the skeleton, steps 4 through 8 should remain unimplemented or return explicit `NotImplemented`/`Unsupported` states.

## 7. `ExternalProcessRunner` Isolation

`ExternalProcessRunner` should be isolated under:

```text
main/backend/runner/
```

It should be a generic process utility, not a Basic Pitch runner.

Safe API principles:

- executable path is a `QString` or `QFileInfo`-checked path;
- arguments are a `QStringList`;
- working directory is explicit;
- environment overrides are explicit;
- timeout is explicit;
- stdout/stderr capture is explicit;
- cancellation is explicit;
- result includes exit code, process error, timed-out flag, cancelled flag, stdout path/text, stderr path/text.

Skeleton-only recommended types:

```text
ExternalProcessRequest
ExternalProcessResult
ExternalProcessState
ExternalProcessRunner
```

The skeleton must not:

- call `QProcess::start()` yet unless a later runner task asks for it;
- accept shell command strings;
- concatenate user paths into commands;
- download models;
- probe Python environments;
- run any backend executable.

Later implementation can use Qt process APIs because Tony is already Qt-based, but the first skeleton should make the safe API shape visible before behavior is added.

## 8. `ResultValidator` and Backend Contract Schemas

`ResultValidator` should conceptually validate against the contract in three layers:

```text
JSON parse validation
    -> JSON Schema validation
        -> semantic validation
```

Schema sources:

```text
docs/schemas/backend_manifest.schema.json
docs/schemas/backend_request.schema.json
docs/schemas/unified_result.schema.json
```

For the compile-only skeleton:

- define `ValidationIssue`, `ValidationReport`, and `ResultValidator`;
- include method names that clearly match future schema validation;
- do not add a JSON Schema dependency yet;
- do not duplicate full schema logic in C++ by hand;
- document dependency choice as deferred.

Future validation responsibilities:

- required top-level fields;
- `contract_version` compatibility;
- request/result engine ID consistency;
- valid status enum;
- note `end_sec > start_sec`;
- MIDI pitch 0 to 127 when present;
- velocity 0 to 127 when present;
- confidence 0.0 to 1.0 when present;
- positive frequency when present;
- pitch curve time monotonicity;
- selected-region bounds;
- file paths staying within the run workspace unless explicitly allowed.

Invalid results must never reach `TonyLayerImporter`.

## 9. Future `TonyLayerImporter` Design

`TonyLayerImporter` should be designed later as the only bridge from validated backend results into Tony/Sonic Visualiser layers.

It should not touch current Tony layer logic in the skeleton.

Future import principles:

- receive only a validated `UnifiedResult`;
- preserve `Document` ownership and model/layer lifecycle;
- use existing `Document` APIs such as `createImportedLayer(...)`, `createEmptyLayer(...)`, `addNonDerivedModel(...)`, and `addLayerToView(...)` where appropriate;
- create result/preview layers before changing corrected layers;
- preserve backend provenance so users can identify source engine;
- avoid silently overwriting the current corrected note layer;
- use `CommandHistory` for user-visible/apply operations when edits become undoable.

Deferred design questions:

- whether backend note result layers should be `FlexiNoteLayer`, `NoteLayer`, or a dedicated result-layer wrapper;
- how backend confidence should be displayed without changing core layer rendering too early;
- how pitch curves should be represented for f0-only backends;
- how selected-region preview/apply should interact with existing pYIN candidate layer behavior;
- how backend result layers should be saved in project/session files.

## 10. Required Meson Build File Changes

Current `meson.build` builds Tony app-specific files through `tony_main_files` and app MOC headers through `tony_main_moc_files`.

The compile-only skeleton should add new `.cpp` files to `tony_main_files`.

Proposed additions:

```meson
tony_main_files = [
  'main/main.cpp',
  'main/Analyser.cpp',
  'main/MainWindow.cpp',
  'main/NetworkPermissionTester.cpp',

  'main/backend/core/AnalysisEngineManager.cpp',
  'main/backend/core/BackendRegistry.cpp',
  'main/backend/core/BackendTypes.cpp',
  'main/backend/adapters/BackendAdapter.cpp',
  'main/backend/contract/BackendManifest.cpp',
  'main/backend/contract/BackendRequest.cpp',
  'main/backend/contract/UnifiedResult.cpp',
  'main/backend/contract/ResultValidator.cpp',
  'main/backend/runner/ExternalProcessRunner.cpp',
  'main/backend/import/TonyLayerImporter.cpp',
]
```

If any new class uses `QObject`, its header must also be added to `tony_main_moc_files`:

```meson
tony_main_moc_files = qt.preprocess(
  moc_headers: [
    'main/MainWindow.h',
    'main/Analyser.h',
    'main/backend/core/AnalysisEngineManager.h',
    'main/backend/runner/ExternalProcessRunner.h',
  ])
```

Recommended skeleton choice:

- avoid `QObject` in most value/model classes;
- use `QObject` only if `AnalysisEngineManager` or `ExternalProcessRunner` needs signals in the immediate skeleton;
- if no signals are implemented yet, avoid MOC additions to keep the diff simpler.

No new external dependencies should be added in CODEX-031.

No schema validation library should be added in CODEX-031. That choice belongs to the contract validation task.

## 11. Compile-only Now vs Deferred

### Compile-only now

The next code task should implement only:

- class declarations and minimal constructors/destructors;
- enum/value types;
- simple in-memory registry methods;
- explicit `NotImplemented` or `Unsupported` return values for run/import paths;
- comments only where needed to explain intentional non-integration;
- Meson source list additions;
- build verification.

### Must remain unimplemented now

The next code task must not implement:

- Basic Pitch adapter;
- any external backend invocation;
- pYIN wrapping or rerouting;
- UI controls or menu changes;
- result JSON parsing from disk;
- full schema validation;
- Tony layer creation/import;
- selected-region backend analysis;
- compare mode;
- settings UI or Backend Manager UI;
- mock backend visible in production;
- project/session serialization of backend results.

## 12. Safety Constraints

Non-negotiable constraints for the skeleton:

- Existing pYIN/Vamp workflow must remain untouched.
- `main/Analyser.cpp` should not be modified in the skeleton.
- `main/MainWindow.cpp` should not be modified in the skeleton unless a later task explicitly asks to wire UI.
- `main/main.cpp` Vamp path setup must not be modified.
- `svapp/framework/Document.*` must not be modified.
- `svgui/layer/*` must not be modified.
- `svcore/transform/*` and `svcore/plugin/*` must not be modified.
- `pyin/*` and `chp/*` must not be modified.
- No backend process should run.
- No backend availability should be fabricated.
- No successful analysis state should be emitted by skeleton code.
- No new production dependency should be added.
- No network behavior should be added.
- No UI redesign should be included.

## 13. Risks

| Risk | Impact | Mitigation |
|---|---|---|
| Skeleton accidentally changes pYIN behavior | Breaks baseline Tony workflow | Do not wire manager into `MainWindow` or `Analyser` yet |
| Architecture classes become too generic | Hard to integrate with Tony later | Keep module in `main/backend/` and trace to Backend Contract |
| External runner API permits unsafe command strings | Security risk | Design argument-list API from the start |
| Validator duplicates schemas incorrectly | Divergence from contract | Defer full schema validation dependency and keep schema files authoritative |
| Importer touches layer internals too early | Breaks editing/rendering behavior | Keep `TonyLayerImporter` compile-only with no layer mutation |
| Meson source additions break Windows build | Blocks progress | Keep files dependency-light and verify build after CODEX-031 |
| QObject/MOC use creates unnecessary complexity | Build friction | Avoid `QObject` unless signals are needed immediately |

## 14. Recommended CODEX-031 Implementation Shape

Recommended next implementation task:

```text
CODEX-031 - Add compile-only backend architecture skeleton
```

Task boundaries:

- add the `main/backend/` folder tree;
- add the compile-only C++ headers and source files listed above;
- update `meson.build` source lists only;
- do not connect the new classes to `MainWindow`, `Analyser`, or pYIN;
- build on Windows;
- run existing Meson tests;
- report any compile blockers.

Acceptance criteria:

- Tony still builds;
- existing tests still pass;
- no pYIN code path changed;
- no new UI appears;
- no external process can run from the skeleton;
- skeleton APIs make the future manager/adapter/validator/importer boundaries clear.

## 15. Recommended Next Task

Proceed with `CODEX-031`: add the compile-only backend architecture skeleton exactly within the safety boundaries above.

If CODEX-031 reveals Meson or include-path friction, fix only the compile issue and do not broaden the task into backend behavior or UI integration.
