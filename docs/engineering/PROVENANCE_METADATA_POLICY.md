# Provenance Metadata Policy

Status: mandatory provenance policy  
Applies before: TonyLayerImporter, result layers, save/load/export, compare mode

## 1. Purpose

Imported backend results must carry enough provenance for users and future developers to know where the data came from, what was analyzed, and what can be trusted.

## 2. Required Provenance Fields

When a backend result is imported into a Tony/Sonic Visualiser layer or saved as a project/session artifact, preserve where possible:

- backend name;
- backend ID;
- backend version if available;
- adapter version if available;
- input audio file path or stable reference;
- selected region start/end if applicable;
- backend settings used;
- model/checkpoint path if relevant;
- effective executable path if safe to store;
- result file path;
- request file path if relevant;
- confidence summary and warning/error codes;
- run timestamp;
- run ID/request ID/result ID;
- device/runtime used if reported;
- user-edit status where possible;
- source layer/result relationship in compare mode.

## 3. Privacy Rules

- Do not store secrets, API keys, tokens, or credentials.
- Avoid logging full commands if they include private paths or secrets; use redacted forms where needed.
- Do not store audio content in logs.
- Keep provenance local unless a future cloud feature explicitly asks for opt-in sharing.

## 4. User Edit Status

Where possible, distinguish:

- raw backend result;
- imported result layer;
- user-edited result;
- accepted/corrected layer;
- exported data.

Do not imply the raw backend output remains unchanged after user edits.

## 5. Save/Load Implications

Before claiming save/load support:

- provenance must survive session/project save and reload where possible;
- durable identity through model/layer names is only partial provenance, not
  full structured provenance;
- full structured provenance may be claimed only after a source-backed metadata
  mechanism is proven through save/load;
- missing backend executables after reload must not invalidate existing saved notes;
- re-running analysis must be explicit and must use current settings or clearly preserved previous settings;
- unavailable provenance fields must be shown as unknown, not fabricated.

Current CODEX-094 status:

- `NoteModel::objectName`, `Layer::objectName`, and
  `Layer::presentationName` are proven persistence-compatible for durable
  provenance-derived identity.
- full structured provenance persistence is deferred and must report
  `structuredProvenancePersisted=false` until a real XML/registry/sidecar
  mechanism is designed, linked, saved, reloaded, and tested.
- See `docs/engineering/STRUCTURED_PROVENANCE_PERSISTENCE_STRATEGY.md`.

## 6. Export Implications

Before claiming export support:

- identify whether export uses corrected layer, result layer, or preview layer;
- export should include provenance in sidecar JSON/CSV metadata when feasible;
- MIDI-only export may lose provenance and should not be described as preserving full backend metadata;
- warning/error context should remain available in project/session data or logs.

## 7. Compare Mode Implications

For compare mode, each result layer must preserve:

- backend ID/name;
- run ID;
- input audio reference;
- selected region if any;
- result status;
- warnings/errors;
- whether the user copied or accepted data from that result.

Compare views must not merge provenance silently.
