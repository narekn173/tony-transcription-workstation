# Backend Contract v0.1 — Tony Fork Transcription Workstation

## 1. Document control

| Field | Value |
|---|---|
| Document | Backend Contract |
| Version | v0.1 |
| Product | Tony Fork Transcription Workstation |
| Source documents | `docs/02_PRD.md`, `docs/03_SRS.md` |
| Status | Draft |
| Primary platform | Windows |
| Primary integration model | External backend adapters first |
| Primary data format | UTF-8 JSON |
| Schema style | JSON Schema-compatible, contract-first |

---

## 2. Purpose

This document defines the contract between the Tony fork host application and all transcription/analysis backends.

The contract exists so that Tony can treat different engines consistently, even when their internal implementations are very different.

Target engines include:

- existing pYIN Vamp workflow;
- Basic Pitch / NeuralNote;
- CREPE Notes;
- MUSC Violin Transcription;
- VioPTT Violin Technique-Aware Transcription;
- PESTO / PENN / FCPE f0 workflows.

The contract defines:

- backend capability declaration;
- backend request format;
- backend result format;
- note/pitch/technique schemas;
- warnings/errors;
- validation rules;
- full-file and selected-region semantics;
- compare-mode compatibility;
- privacy and external process constraints.

---

## 3. Scope and non-scope

### 3.1 In scope

This contract covers:

- how the host discovers backend capabilities;
- how the host asks a backend to run analysis;
- how a backend reports status, warnings, errors, and results;
- how results are normalized into a unified format before display/import;
- how selected-region analysis is represented;
- how compare mode stores multiple backend results;
- how f0-only backends feed a note-segmentation layer;
- how validation prevents fake, malformed, or unsafe imports.

### 3.2 Out of scope

This document does **not** define:

- C++ class names;
- Qt widget layout;
- thread implementation;
- exact command-line syntax for each original upstream project;
- backend installation instructions;
- UI styling;
- AI Copilot prompts;
- legal license decisions;
- musical correction algorithms;
- native/Vamp porting details.

Those belong in TDD, AGENTS.md, execution plans, license register, or future AI Copilot specs.

---

## 4. Design principles

### CONTRACT-001 — Contract-first integration

The host application must treat each backend through a stable contract rather than backend-specific ad hoc parsing inside UI code.

### CONTRACT-002 — Real results only

Production backends must not return fake notes, fake confidence, fake progress, or fake completed states. Development-only mock results must be explicitly labeled as development/test data.

### CONTRACT-003 — Capability-driven UI

The UI must not assume every backend supports every feature. The backend must declare its capabilities, and the UI must enable/disable features accordingly.

### CONTRACT-004 — Local-first by default

Backend analysis is local by default. Cloud/API behaviors are outside the MVP backend contract and must be opt-in if later added.

### CONTRACT-005 — Progressive enhancement

A backend may return minimal note data first. Additional fields such as pitch bends, technique labels, confidence, and provenance may be added when available.

### CONTRACT-006 — Non-destructive selected-region workflow

Selected-region analysis must produce a preview/result layer first. It must not overwrite the corrected layer unless the user explicitly applies changes.

### CONTRACT-007 — Validation before import

The host must validate backend output before importing it into Tony layers.

### CONTRACT-008 — Explicit units

All units must be explicit and stable:

| Field type | Unit / range |
|---|---|
| Time | seconds from start of original audio |
| Duration | seconds |
| Frequency | Hz |
| MIDI pitch | integer 0–127 |
| Velocity | integer 0–127 |
| Confidence | float 0.0–1.0 |
| Cents deviation | cents relative to equal-tempered MIDI pitch |

---

## 5. Contract model overview

The host application should communicate with each backend adapter through three main objects:

```text
BackendManifest
    declares what the backend can do

BackendRequest
    tells the backend what to analyze and where to write output

UnifiedResult
    returns normalized notes/pitch/labels/warnings/errors to the host
```

Recommended flow:

```text
1. Host loads backend manifest.
2. Host shows backend as available/missing/broken.
3. User selects backend and analysis mode.
4. Host creates BackendRequest JSON.
5. Host runs backend adapter or internal backend.
6. Backend writes UnifiedResult JSON and optional files.
7. Host validates UnifiedResult.
8. Host imports result into result layer or preview layer.
9. User manually accepts/corrects/export results.
```

---

## 6. Engine identifiers and categories

### 6.1 Stable engine IDs

Engine IDs must be lowercase snake_case and stable across versions.

| Engine ID | Category | Initial integration type | Notes |
|---|---|---|---|
| `pyin_vamp` | note_transcription | existing internal/Vamp | Existing Tony/pYIN workflow |
| `basic_pitch` | note_transcription | external first | General audio-to-MIDI |
| `neuralnote` | note_transcription | external/native reference | Native Basic Pitch path reference |
| `crepe_notes` | note_transcription | external first | Monophonic f0-to-notes |
| `musc_violin` | note_transcription | external | Specialized violin transcription |
| `vioptt` | note_transcription_technique | external | Violin notes + technique labels |
| `pesto` | f0_estimation | external | f0 curve; needs segmentation |
| `penn` | f0_estimation | external | f0/periodicity; needs segmentation |
| `fcpe` | f0_estimation | external | f0 curve; simple MIDI may be treated cautiously |
| `dev_mock` | development_test | internal/dev-only | Must never be exposed as real production analysis |

### 6.2 Engine category enum

Allowed categories:

```json
[
  "note_transcription",
  "note_transcription_technique",
  "f0_estimation",
  "segmentation",
  "postprocessing",
  "development_test"
]
```

---

## 7. Versioning and compatibility rules

### 7.1 Contract version

Every manifest, request, and result must include:

```json
"contract_version": "0.1"
```

### 7.2 Compatibility rule

For v0.x development:

- breaking changes are allowed only with explicit document update;
- schema files must be versioned;
- examples must be updated with the schema.

### 7.3 Breaking changes

Breaking changes include:

- removing a required field;
- changing field type;
- changing time units;
- changing confidence scale;
- changing status enum names;
- changing engine ID semantics;
- changing selected-region coordinate behavior.

### 7.4 Forward compatibility

The host should ignore unknown optional fields if the contract version is compatible and required fields validate.

### 7.5 Backward compatibility

Backends should not depend on host behavior not declared in this contract.

---

## 8. File-system and runtime assumptions

### 8.1 Workspace model

The host should create a temporary workspace per analysis request.

Example:

```text
<app_temp>/analysis/<request_id>/
    request.json
    result.json
    backend_stdout.log
    backend_stderr.log
    outputs/
        notes.mid
        notes.csv
        pitch.csv
```

### 8.2 Path conventions

- Paths in JSON should use absolute paths for files created by the host.
- Paths should use UTF-8 strings.
- Windows paths are allowed.
- Backends must not write outside the provided workspace unless explicitly configured.

### 8.3 Runtime execution model

Initial external backends should be launched by the host through a backend adapter.

Recommended generic adapter invocation:

```text
backend_adapter --request <path/to/request.json> --result <path/to/result.json>
```

This invocation is a normalized adapter contract. The adapter may internally call the original upstream tool with its own CLI.

### 8.4 CPU/GPU assumptions

Initial version is CPU-first. Device preference is represented in the request, but backends may ignore unsupported devices and must report the actual device used.

---

## 9. Backend manifest schema

### 9.1 Purpose

A `BackendManifest` tells the host what a backend is, whether it can be configured, what it can output, and what runtime it requires.

### 9.2 Required fields

| Field | Type | Required | Description |
|---|---:|---:|---|
| `contract_version` | string | yes | Contract version, e.g. `0.1` |
| `engine_id` | string | yes | Stable engine ID |
| `display_name` | string | yes | User-facing backend name |
| `engine_version` | string/null | yes | Upstream/backend version if known |
| `adapter_version` | string | yes | Adapter version |
| `category` | string | yes | Engine category enum |
| `runtime` | object | yes | Runtime requirements |
| `capabilities` | object | yes | Supported features |
| `inputs` | object | yes | Input constraints |
| `outputs` | object | yes | Output capabilities |
| `settings_schema` | object | no | Backend-specific configurable parameters |
| `license` | object | yes | License metadata |
| `status_probe` | object | no | Optional command/check info |

### 9.3 Runtime object

```json
{
  "type": "python_cli",
  "requires_python": true,
  "requires_model_files": false,
  "supports_cpu": true,
  "supports_cuda": false,
  "supports_directml": false,
  "supports_rocm": false,
  "internet_required": false
}
```

Allowed runtime `type` values:

```json
[
  "internal",
  "vamp_plugin",
  "python_cli",
  "native_cli",
  "onnx_native",
  "adapter_cli",
  "development_test"
]
```

### 9.4 Capabilities object

```json
{
  "supports_full_file": true,
  "supports_region": true,
  "supports_batch": false,
  "outputs_notes": true,
  "outputs_pitch_curve": false,
  "outputs_pitch_bends": true,
  "outputs_velocity": true,
  "outputs_confidence": true,
  "outputs_technique_labels": false,
  "outputs_warnings": true,
  "can_run_offline": true
}
```

### 9.5 Input constraints object

```json
{
  "audio_formats": ["wav"],
  "preferred_formats": ["wav"],
  "mono_required": false,
  "max_channels": 2,
  "sample_rates_hz": [44100],
  "requires_resampling": "adapter_or_host"
}
```

Allowed `requires_resampling` values:

```json
["none", "host", "adapter", "adapter_or_host", "unknown"]
```

### 9.6 License object

```json
{
  "name": "Apache-2.0",
  "source_url": "https://github.com/example/project",
  "redistribution_status": "allowed_with_notice",
  "notes": "Review before bundling."
}
```

Allowed `redistribution_status` values:

```json
[
  "allowed_with_notice",
  "restricted",
  "unknown_review_required",
  "external_dependency_only"
]
```

---

## 10. Backend request schema

### 10.1 Purpose

A `BackendRequest` is created by the host for a single analysis operation.

### 10.2 Required fields

| Field | Type | Required | Description |
|---|---:|---:|---|
| `contract_version` | string | yes | Contract version |
| `request_id` | string | yes | Unique request ID |
| `created_at` | string | yes | ISO 8601 timestamp |
| `engine_id` | string | yes | Target backend |
| `analysis_mode` | string | yes | `full_file`, `region`, or `f0_then_segmentation` |
| `input_audio` | object | yes | Source audio info |
| `region` | object/null | yes | Region info or null |
| `output` | object | yes | Paths and requested outputs |
| `settings` | object | yes | Backend-specific settings |
| `runtime` | object | yes | Runtime preferences |
| `host_context` | object | no | Optional host metadata |

### 10.3 Analysis modes

Allowed `analysis_mode` values:

```json
[
  "full_file",
  "region",
  "f0_then_segmentation"
]
```

Compare mode is represented by multiple separate requests, one per backend, coordinated by the host.

### 10.4 Input audio object

```json
{
  "path": "C:/audio/input.wav",
  "original_path": "C:/audio/input.mp3",
  "format": "wav",
  "duration_sec": 12.345,
  "sample_rate_hz": 44100,
  "channels": 1,
  "channel_policy": "mono_mixdown",
  "hash_sha256": null
}
```

Allowed `channel_policy` values:

```json
["as_is", "mono_mixdown", "left", "right", "unknown"]
```

### 10.5 Region object

For full-file analysis:

```json
"region": null
```

For selected-region analysis:

```json
{
  "start_sec": 10.000,
  "end_sec": 13.000,
  "coordinate_system": "original_audio_time",
  "apply_policy": "preview_only"
}
```

Allowed `apply_policy` values:

```json
["preview_only", "replace_region_after_user_confirmation", "add_as_layer"]
```

All returned note and pitch times must use `original_audio_time`, not slice-relative time.

### 10.6 Output object

```json
{
  "workspace_dir": "C:/temp/analysis/req_001",
  "result_json_path": "C:/temp/analysis/req_001/result.json",
  "requested_outputs": ["notes", "pitch_curve", "midi", "csv", "warnings"],
  "keep_intermediate_files": true
}
```

Allowed `requested_outputs` values:

```json
[
  "notes",
  "pitch_curve",
  "pitch_bends",
  "velocity",
  "confidence",
  "technique_labels",
  "midi",
  "csv",
  "warnings",
  "logs"
]
```

### 10.7 Runtime preferences

```json
{
  "device_preference": "cpu",
  "max_threads": null,
  "timeout_sec": 1800,
  "allow_internet": false,
  "cancel_token_path": "C:/temp/analysis/req_001/cancel.flag"
}
```

Allowed `device_preference` values:

```json
["cpu", "cuda", "directml", "rocm", "auto"]
```

---

## 11. Backend operation/state model

### 11.1 Host-visible states

Allowed operation states:

```json
[
  "idle",
  "ready",
  "running",
  "completed",
  "completed_with_warnings",
  "failed",
  "cancelled",
  "backend_missing",
  "model_missing",
  "unsupported_input",
  "invalid_output"
]
```

### 11.2 Long-running behavior

External analysis may take long enough that the UI must remain responsive. The host should treat each backend run as a long-running operation with progress/log updates where available.

### 11.3 Progress object

Backends may optionally write progress to stdout or a progress file. If supported, progress should be normalized as:

```json
{
  "percent": 42.5,
  "stage": "running_model",
  "message": "Estimating notes"
}
```

Progress is optional. Lack of progress events must not imply failure.

---

## 12. Unified result envelope

### 12.1 Purpose

`UnifiedResult` is the normalized output object that the host imports into Tony layers.

### 12.2 Required top-level fields

| Field | Type | Required | Description |
|---|---:|---:|---|
| `contract_version` | string | yes | Contract version |
| `result_id` | string | yes | Unique result ID |
| `request_id` | string | yes | Request this result answers |
| `created_at` | string | yes | ISO 8601 timestamp |
| `engine` | object | yes | Engine metadata |
| `status` | string | yes | Result status |
| `audio` | object | yes | Audio metadata |
| `region` | object/null | yes | Region or null |
| `summary` | object | yes | Counts and high-level info |
| `notes` | array | yes | Note events; may be empty |
| `pitch_curve` | array | yes | Pitch points; may be empty |
| `pitch_bends` | array | yes | Pitch bend/deviation data; may be empty |
| `technique_labels` | array | yes | Technique labels; may be empty |
| `files` | array | yes | Optional output files; may be empty |
| `warnings` | array | yes | Non-fatal warnings |
| `errors` | array | yes | Fatal/non-fatal structured errors |
| `provenance` | object | yes | Audit trail |

### 12.3 Engine metadata object

```json
{
  "engine_id": "basic_pitch",
  "display_name": "Basic Pitch",
  "engine_version": null,
  "adapter_version": "0.1.0",
  "runtime_type": "python_cli",
  "device_used": "cpu"
}
```

### 12.4 Summary object

```json
{
  "note_count": 42,
  "pitch_point_count": 0,
  "pitch_bend_count": 12,
  "technique_label_count": 0,
  "mean_confidence": 0.82,
  "low_confidence_count": 3,
  "duration_analyzed_sec": 12.345
}
```

---

## 13. Note event schema

### 13.1 Required fields

| Field | Type | Required | Description |
|---|---:|---:|---|
| `id` | string | yes | Stable within result |
| `start_sec` | number | yes | Note start, original audio time |
| `end_sec` | number | yes | Note end, original audio time |
| `midi_pitch` | integer/null | yes | 0–127 or null if unknown |
| `frequency_hz` | number/null | yes | Representative frequency if available |
| `velocity` | integer/null | yes | 0–127 or null |
| `confidence` | number/null | yes | 0.0–1.0 or null |
| `source` | object | yes | Backend/provenance source |
| `flags` | array | yes | Note-level flags |

### 13.2 Optional fields

| Field | Type | Description |
|---|---:|---|
| `label` | string | Note name or backend label |
| `channel` | integer/null | MIDI channel if available |
| `pitch_bend_ref` | string/null | ID linking to pitch bend/deviation data |
| `technique_ref` | string/null | ID linking to technique label |
| `articulation` | string/null | Future articulation value |
| `user_edit_state` | string | For host use after import |

### 13.3 Note example

```json
{
  "id": "note_0001",
  "start_sec": 1.240,
  "end_sec": 1.680,
  "midi_pitch": 64,
  "frequency_hz": 329.63,
  "velocity": 82,
  "confidence": 0.91,
  "label": "E4",
  "channel": null,
  "pitch_bend_ref": null,
  "technique_ref": null,
  "source": {
    "engine_id": "basic_pitch",
    "raw_event_id": null
  },
  "flags": []
}
```

### 13.4 Note validation rules

- `start_sec` must be `>= 0`.
- `end_sec` must be `> start_sec`.
- For region results, notes must fall inside the requested region unless explicitly flagged as `boundary_overlap`.
- `midi_pitch` must be integer 0–127 when present.
- `velocity` must be integer 0–127 when present.
- `confidence` must be 0.0–1.0 when present.
- The host must reject or quarantine notes with invalid timing.

---

## 14. Pitch curve schema

### 14.1 Purpose

`pitch_curve` stores f0/pitch estimates over time.

### 14.2 Pitch point fields

| Field | Type | Required | Description |
|---|---:|---:|---|
| `time_sec` | number | yes | Time in original audio coordinates |
| `frequency_hz` | number/null | yes | Estimated f0 in Hz; null for unvoiced |
| `confidence` | number/null | yes | 0.0–1.0 or null |
| `voiced` | boolean/null | yes | Whether point is voiced |
| `midi_pitch_float` | number/null | no | Continuous MIDI pitch if available |
| `cents_deviation` | number/null | no | Cents from nearest equal-tempered MIDI pitch |

### 14.3 Pitch point example

```json
{
  "time_sec": 1.250,
  "frequency_hz": 330.1,
  "confidence": 0.88,
  "voiced": true,
  "midi_pitch_float": 64.02,
  "cents_deviation": 2.1
}
```

### 14.4 Pitch curve validation rules

- `time_sec` must be monotonically non-decreasing.
- `frequency_hz` must be positive when present.
- Unvoiced points should set `frequency_hz` to null and `voiced` to false.
- The host may downsample dense pitch curves for display, but must preserve original data if saved in project files.

---

## 15. Pitch bend/deviation schema

### 15.1 Purpose

`pitch_bends` represents pitch deviations linked to notes or continuous bend curves.

### 15.2 Pitch bend object

```json
{
  "id": "bend_0001",
  "note_id": "note_0001",
  "unit": "cents",
  "points": [
    { "time_sec": 1.240, "value": 0.0 },
    { "time_sec": 1.300, "value": 12.5 },
    { "time_sec": 1.360, "value": -8.2 }
  ],
  "confidence": 0.75
}
```

Allowed `unit` values:

```json
["cents", "midi_pitch_float", "pitch_wheel_14bit"]
```

### 15.3 Validation rules

- `points` must be ordered by time.
- `note_id` should reference an existing note when note-linked.
- Host may choose whether to export bends to MIDI depending on export settings.

---

## 16. Technique label schema

### 16.1 Purpose

`technique_labels` stores per-note or time-region playing technique labels, initially for VioPTT.

### 16.2 Technique label object

```json
{
  "id": "tech_0001",
  "note_id": "note_0001",
  "start_sec": 1.240,
  "end_sec": 1.680,
  "technique": "pizzicato",
  "confidence": 0.87,
  "source": {
    "engine_id": "vioptt",
    "raw_label": "pizzicato"
  }
}
```

### 16.3 Allowed initial technique values

Initial values:

```json
[
  "normal",
  "pizzicato",
  "spiccato",
  "flageolet",
  "no_technique",
  "unknown",
  "other"
]
```

### 16.4 Validation rules

- If `note_id` is present, it should reference an existing note.
- If no `note_id` is present, `start_sec` and `end_sec` define the time region.
- Unknown backend labels should map to `other` or `unknown` while preserving `raw_label`.

---

## 17. Confidence, velocity, and timing conventions

### 17.1 Confidence

Confidence values must be normalized to 0.0–1.0.

If backend confidence is not directly comparable, the adapter must document the mapping in `provenance.confidence_mapping`.

Example:

```json
"confidence_mapping": "Backend onset probability copied directly to note confidence."
```

### 17.2 Velocity

Velocity must be integer 0–127.

If backend produces amplitude or loudness instead of MIDI velocity, adapter must map it and document mapping.

### 17.3 Timing

All times are seconds from the start of the original loaded audio file.

For selected-region analysis, backend adapters must convert slice-relative times back to original audio time before writing `UnifiedResult`.

---

## 18. Warning and error schema

### 18.1 Warning object

```json
{
  "code": "LOW_CONFIDENCE_REGION",
  "severity": "warning",
  "message": "Low confidence between 10.2s and 11.0s.",
  "start_sec": 10.2,
  "end_sec": 11.0,
  "details": {
    "mean_confidence": 0.31
  }
}
```

### 18.2 Error object

```json
{
  "code": "BACKEND_OUTPUT_MISSING",
  "severity": "error",
  "message": "Backend completed but did not produce the expected MIDI or JSON output.",
  "recoverable": false,
  "details": {
    "expected_path": "C:/temp/analysis/req_001/result.json"
  }
}
```

### 18.3 Severity enum

```json
["info", "warning", "error", "fatal"]
```

### 18.4 Standard error codes

| Code | Meaning |
|---|---|
| `BACKEND_NOT_CONFIGURED` | No executable/path configured |
| `BACKEND_MISSING` | Configured backend not found |
| `MODEL_MISSING` | Required model/checkpoint missing |
| `UNSUPPORTED_INPUT` | Input audio unsupported |
| `RUNTIME_DEPENDENCY_MISSING` | Python/package/native dependency missing |
| `BACKEND_TIMEOUT` | Run exceeded timeout |
| `BACKEND_CANCELLED` | User cancelled operation |
| `BACKEND_FAILED` | Backend returned non-zero exit code |
| `BACKEND_OUTPUT_MISSING` | Expected output missing |
| `INVALID_OUTPUT_SCHEMA` | Output failed validation |
| `LOW_CONFIDENCE_RESULT` | Result exists but confidence is low |
| `PARTIAL_RESULT` | Result incomplete but usable |
| `INTERNAL_ADAPTER_ERROR` | Adapter failed before/after backend run |

### 18.5 Error handling rule

If `status` is `failed`, `errors` must contain at least one error object.

---

## 19. Provenance and audit fields

### 19.1 Purpose

Provenance allows the host and user to understand where a result came from.

### 19.2 Provenance object

```json
{
  "host_app_version": null,
  "adapter_command": "basic_pitch_adapter --request request.json --result result.json",
  "backend_command_redacted": "basic-pitch ...",
  "settings_used": {
    "onset_threshold": 0.5
  },
  "input_hash_sha256": null,
  "created_by": "backend_adapter",
  "confidence_mapping": null,
  "velocity_mapping": null,
  "notes": []
}
```

### 19.3 Privacy rule

Commands should redact secrets and unnecessary personal paths where possible. API keys must never appear in provenance/logs.

---

## 20. Optional output file references

Backends may produce optional files.

### 20.1 File reference object

```json
{
  "kind": "midi",
  "path": "C:/temp/analysis/req_001/outputs/notes.mid",
  "description": "Raw backend MIDI output",
  "generated_by": "basic_pitch",
  "imported": true
}
```

Allowed `kind` values:

```json
[
  "midi",
  "csv_notes",
  "csv_pitch",
  "json_raw",
  "log_stdout",
  "log_stderr",
  "audio_slice",
  "other"
]
```

---

## 21. Per-engine capability mapping

### 21.1 Initial capability expectations

| Engine | Notes | Pitch curve | Pitch bends | Velocity | Confidence | Technique | Region | First priority |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| `pyin_vamp` | yes | yes | no/limited | unknown | yes/limited | no | likely yes | existing |
| `basic_pitch` | yes | no/limited | yes | yes | yes | no | adapter-supported | MVP |
| `neuralnote` | yes | no/limited | yes | yes | yes | no | adapter-supported | post-MVP/native study |
| `crepe_notes` | yes | maybe via f0 | no/limited | yes | maybe | no | adapter-supported | post-MVP 1 |
| `musc_violin` | yes | likely yes | yes/deviation | maybe | maybe | no | adapter-supported | post-MVP 2 |
| `vioptt` | yes | maybe | unknown | yes | maybe | yes | adapter-supported | post-MVP 3 |
| `pesto` | no direct notes | yes | no | no | yes | no | adapter-supported | post-MVP 4 |
| `penn` | no direct notes | yes | no | no | yes/periodicity | no | adapter-supported | post-MVP 4 |
| `fcpe` | simple/derived | yes | no | no | maybe | no | adapter-supported | post-MVP 4 |

**ASSUMPTION:** Region support for external tools may be implemented by the host/adapter slicing audio, even when the original backend does not natively accept a time range.

### 21.2 Capability declaration is authoritative

The capability table above is a planning expectation. Runtime UI must use each backend manifest as the authoritative source.

---

## 22. Full-file analysis contract

### 22.1 Request

For full-file analysis:

```json
"analysis_mode": "full_file",
"region": null
```

### 22.2 Result

The backend returns note/pitch times relative to original audio start.

### 22.3 Import behavior

Host imports result as a new result layer. The user may copy/apply it to the corrected layer.

---

## 23. Selected-region analysis contract

### 23.1 Request

For selected-region analysis:

```json
"analysis_mode": "region",
"region": {
  "start_sec": 10.0,
  "end_sec": 13.0,
  "coordinate_system": "original_audio_time",
  "apply_policy": "preview_only"
}
```

### 23.2 Adapter behavior

If the backend cannot analyze a region natively, the adapter may create a temporary audio slice.

The adapter must convert all returned timestamps back to original audio time.

### 23.3 Result behavior

The result must include the same `region` object.

### 23.4 Host behavior

Host must not overwrite corrected notes automatically. Region result appears as preview/result layer first.

---

## 24. Compare-mode contract

### 24.1 Compare model

Compare mode is a host-level workflow, not a single backend request.

The host creates one `BackendRequest` per selected engine:

```text
compare_run_id
├── request basic_pitch
├── request crepe_notes
└── request pyin_vamp
```

### 24.2 Result grouping

Each `UnifiedResult` may include:

```json
"compare_run_id": "cmp_2026_001"
```

This field is optional but recommended for compare workflows.

### 24.3 Per-note provenance

Every note must include source engine metadata so the host can show which engine produced which note.

### 24.4 Consensus is host logic

Voting, consensus, and disagreement highlighting are not backend responsibilities. Backends only return results.

---

## 25. f0-backend + segmentation contract

### 25.1 Purpose

PESTO, PENN, and FCPE-like backends may output f0 curves rather than final note events. These results must be passed through a segmentation layer before they become note events.

### 25.2 f0-only result

An f0 backend may return:

```json
"notes": [],
"pitch_curve": [ ... ],
"summary": {
  "note_count": 0,
  "pitch_point_count": 1234
}
```

### 25.3 Segmentation request

A segmentation backend may receive a `BackendRequest` with:

```json
"analysis_mode": "f0_then_segmentation"
```

The request settings should include reference to f0 result file or in-memory result ID.

### 25.4 Segmentation output

The segmentation layer returns normal `notes` and may preserve original f0 curve.

---

## 26. Validation rules

### 26.1 Required validation before import

The host must validate:

- top-level required fields;
- `contract_version` compatibility;
- `request_id` match;
- `engine.engine_id` match;
- status enum;
- note timing;
- note pitch range;
- confidence range;
- velocity range;
- pitch curve monotonic order;
- region bounds;
- file paths remain inside workspace unless allowed.

### 26.2 Quarantine behavior

If validation fails:

- do not import result into normal layers;
- show `invalid_output` state;
- store raw result for debugging if safe;
- show validation error to user;
- keep application usable.

### 26.3 Partial result behavior

If output is partially valid:

- backend may set `status` to `completed_with_warnings`;
- warnings must explain missing/partial data;
- host may import valid portions only if safe.

---

## 27. Exit codes and failure handling

### 27.1 Standard adapter exit codes

| Exit code | Meaning |
|---:|---|
| 0 | Success |
| 1 | General backend failure |
| 2 | Invalid request JSON |
| 3 | Backend not configured/missing |
| 4 | Model/checkpoint missing |
| 5 | Unsupported input |
| 6 | Timeout |
| 7 | Cancelled |
| 8 | Output validation failed inside adapter |
| 9 | Dependency/runtime missing |
| 10 | Permission/path error |

### 27.2 Exit code vs result JSON

Whenever possible, adapter should still write a `UnifiedResult` with `status: failed` and structured errors, even when exit code is non-zero.

If no result JSON exists, host creates a host-side failure object.

---

## 28. Privacy and local-first constraints

### 28.1 Local-first

Backends must run locally unless explicitly documented otherwise.

### 28.2 Internet

`allow_internet` defaults to false. Backends must not require internet for normal local transcription unless their manifest states it and the user enables it.

### 28.3 Audio privacy

Backends must not upload audio in the MVP contract.

### 28.4 Logs

Logs are local. They should not include secrets. User should be able to clear logs.

---

## 29. Security constraints for external process execution

### 29.1 Process safety

The host/adapter must:

- pass arguments safely as argument arrays, not string-concatenated shell commands;
- avoid shell execution where possible;
- enforce timeout;
- support cancellation;
- capture stdout/stderr;
- restrict workspace paths;
- reject unsafe or missing executable paths;
- avoid running arbitrary untrusted commands as backends.

### 29.2 Path safety

The adapter must treat all paths as data, not executable script content.

### 29.3 Secret safety

API keys and credentials are not part of this MVP backend contract and must not be logged.

---

## 30. Backward/forward compatibility rules

### 30.1 Required field stability

Required fields cannot be removed within a compatible contract version.

### 30.2 Optional field expansion

New optional fields may be added if existing validators ignore unknown fields.

### 30.3 Enum expansion

Enum expansion may break strict clients. New enum values require document update and validator update.

### 30.4 Raw backend outputs

Raw outputs may be stored as optional files but must not be treated as the canonical import format unless no normalized result exists.

---

## 31. Example backend manifests

### 31.1 Basic Pitch manifest example

```json
{
  "contract_version": "0.1",
  "engine_id": "basic_pitch",
  "display_name": "Basic Pitch",
  "engine_version": null,
  "adapter_version": "0.1.0",
  "category": "note_transcription",
  "runtime": {
    "type": "python_cli",
    "requires_python": true,
    "requires_model_files": false,
    "supports_cpu": true,
    "supports_cuda": false,
    "supports_directml": false,
    "supports_rocm": false,
    "internet_required": false
  },
  "capabilities": {
    "supports_full_file": true,
    "supports_region": true,
    "supports_batch": false,
    "outputs_notes": true,
    "outputs_pitch_curve": false,
    "outputs_pitch_bends": true,
    "outputs_velocity": true,
    "outputs_confidence": true,
    "outputs_technique_labels": false,
    "outputs_warnings": true,
    "can_run_offline": true
  },
  "inputs": {
    "audio_formats": ["wav"],
    "preferred_formats": ["wav"],
    "mono_required": false,
    "max_channels": 2,
    "sample_rates_hz": [44100],
    "requires_resampling": "adapter_or_host"
  },
  "outputs": {
    "primary": ["notes", "midi", "csv_notes"],
    "optional": ["pitch_bends", "warnings", "logs"]
  },
  "settings_schema": {},
  "license": {
    "name": "Apache-2.0",
    "source_url": "https://github.com/spotify/basic-pitch",
    "redistribution_status": "allowed_with_notice",
    "notes": "Review before bundling."
  }
}
```

---

## 32. Example request

```json
{
  "contract_version": "0.1",
  "request_id": "req_20260515_0001",
  "created_at": "2026-05-15T12:00:00Z",
  "engine_id": "basic_pitch",
  "analysis_mode": "full_file",
  "input_audio": {
    "path": "C:/Users/User/audio/input.wav",
    "original_path": "C:/Users/User/audio/input.wav",
    "format": "wav",
    "duration_sec": 12.345,
    "sample_rate_hz": 44100,
    "channels": 1,
    "channel_policy": "as_is",
    "hash_sha256": null
  },
  "region": null,
  "output": {
    "workspace_dir": "C:/Temp/tony_analysis/req_20260515_0001",
    "result_json_path": "C:/Temp/tony_analysis/req_20260515_0001/result.json",
    "requested_outputs": ["notes", "midi", "csv", "warnings", "logs"],
    "keep_intermediate_files": true
  },
  "settings": {},
  "runtime": {
    "device_preference": "cpu",
    "max_threads": null,
    "timeout_sec": 1800,
    "allow_internet": false,
    "cancel_token_path": "C:/Temp/tony_analysis/req_20260515_0001/cancel.flag"
  },
  "host_context": {
    "host_app": "Tony Fork Transcription Workstation",
    "host_version": null
  }
}
```

---

## 33. Example minimal result

```json
{
  "contract_version": "0.1",
  "result_id": "res_20260515_0001",
  "request_id": "req_20260515_0001",
  "created_at": "2026-05-15T12:01:00Z",
  "engine": {
    "engine_id": "basic_pitch",
    "display_name": "Basic Pitch",
    "engine_version": null,
    "adapter_version": "0.1.0",
    "runtime_type": "python_cli",
    "device_used": "cpu"
  },
  "status": "completed",
  "audio": {
    "path": "C:/Users/User/audio/input.wav",
    "duration_sec": 12.345,
    "sample_rate_hz": 44100,
    "channels": 1
  },
  "region": null,
  "summary": {
    "note_count": 1,
    "pitch_point_count": 0,
    "pitch_bend_count": 0,
    "technique_label_count": 0,
    "mean_confidence": 0.91,
    "low_confidence_count": 0,
    "duration_analyzed_sec": 12.345
  },
  "notes": [
    {
      "id": "note_0001",
      "start_sec": 1.240,
      "end_sec": 1.680,
      "midi_pitch": 64,
      "frequency_hz": 329.63,
      "velocity": 82,
      "confidence": 0.91,
      "label": "E4",
      "channel": null,
      "pitch_bend_ref": null,
      "technique_ref": null,
      "source": {
        "engine_id": "basic_pitch",
        "raw_event_id": null
      },
      "flags": []
    }
  ],
  "pitch_curve": [],
  "pitch_bends": [],
  "technique_labels": [],
  "files": [],
  "warnings": [],
  "errors": [],
  "provenance": {
    "host_app_version": null,
    "adapter_command": "basic_pitch_adapter --request request.json --result result.json",
    "backend_command_redacted": "basic-pitch ...",
    "settings_used": {},
    "input_hash_sha256": null,
    "created_by": "backend_adapter",
    "confidence_mapping": null,
    "velocity_mapping": null,
    "notes": []
  }
}
```

---

## 34. Acceptance criteria

### AC-CONTRACT-001

Every backend manifest must validate against the backend manifest schema before the backend appears as available.

### AC-CONTRACT-002

Every backend request must include `contract_version`, `request_id`, `engine_id`, `analysis_mode`, `input_audio`, `region`, `output`, `settings`, and `runtime`.

### AC-CONTRACT-003

Every backend result must include `contract_version`, `result_id`, `request_id`, `engine`, `status`, `audio`, `summary`, `notes`, `pitch_curve`, `warnings`, `errors`, and `provenance`.

### AC-CONTRACT-004

Invalid result data must not be imported into normal Tony layers.

### AC-CONTRACT-005

Selected-region results must use original audio time coordinates.

### AC-CONTRACT-006

Failed backends must produce a visible failure state and must not show as completed.

### AC-CONTRACT-007

Development mock results must be marked as development/test only.

### AC-CONTRACT-008

Backends must not require internet access unless explicitly declared and enabled.

---

## 35. Open questions

1. Should JSON Schema files use Draft 2020-12 as the formal schema dialect?
2. Should the host require SHA-256 hashes for input audio in production builds?
3. Should backend adapters be shipped inside the repo or maintained as separate packages?
4. Should MIDI export use pitch bends by default or only when enabled?
5. Should VioPTT technique labels be exported to MIDI markers or only JSON/CSV initially?
6. Should external backends be sandboxed more strictly in future releases?
7. Should compare-mode consensus become part of this contract later, or remain host-only logic?
8. Should the project eventually define a stable plugin SDK for third-party backends?

---

## 36. What moves to TDD

The following belong in TDD, not this contract:

- C++/Qt class design;
- `AnalysisEngineManager` implementation;
- `ExternalProcessRunner` implementation;
- threading/cancellation implementation;
- temp workspace cleanup implementation;
- Tony layer importer design;
- JSON validation library choice;
- MIDI parser/export library choice;
- exact f0-to-note segmentation algorithm;
- exact UI panels and menus.

---

## 37. What moves to AGENTS.md / Codex tasks

AGENTS.md should instruct Codex/agents to:

- read this contract before backend work;
- never create fake production results;
- keep each task small;
- add/update examples when schema changes;
- run schema validation tests;
- avoid unrelated UI refactors during backend contract implementation;
- document files changed and verification steps.

---

## 38. Plain-language summary

This Backend Contract says how Tony will talk to every analysis model. Each backend must declare what it can do, accept a standard request, and return a standard result. The result can contain notes, pitch curves, pitch bends, confidence, technique labels, files, warnings, and errors. Tony must validate everything before showing it as real analysis. This allows Basic Pitch, CREPE Notes, MUSC, VioPTT, PESTO, PENN, FCPE, and future engines to work through one clean system instead of many incompatible one-off integrations.
