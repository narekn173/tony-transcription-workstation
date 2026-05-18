# Structured Provenance Persistence Strategy

Status: CODEX-094 strategy and conservative code boundary.

This document records the current provenance persistence proof level for
backend-generated Tony/Sonic Visualiser layers. It intentionally does not claim
full structured metadata persistence, because the inspected Tony/Sonic
Visualiser save/load path does not currently expose a generic structured
metadata store for layers or models.

## Source Inspected

- `svapp/framework/Document.cpp`
- `svapp/framework/Document.h`
- `svapp/framework/SVFileReader.cpp`
- `svapp/framework/SVFileReader.h`
- `svgui/layer/Layer.cpp`
- `svgui/layer/Layer.h`
- `svgui/layer/NoteLayer.cpp`
- `svgui/layer/NoteLayer.h`
- `svgui/layer/FlexiNoteLayer.cpp`
- `svgui/layer/FlexiNoteLayer.h`
- `svcore/data/model/Model.cpp`
- `svcore/data/model/Model.h`
- `svcore/data/model/NoteModel.h`
- `svcore/base/Event.h`
- `svcore/base/PropertyContainer.h`
- `main/backend/TonyLayerImporter.cpp`
- `main/backend/TonyLayerImporter.h`
- `main/backend/test/TestBackendTypes.h`

## Current Provenance Proof

CODEX-093 proved durable provenance-derived identity through real
Tony/Sonic Visualiser fields:

- `NoteModel::objectName`
- `Layer::objectName`
- `Layer::presentationName`

Those fields are written by the existing XML save path and restored by
`SVFileReader`. The proof confirms backend ID, backend version, run ID, result
JSON filename, request JSON filename, input audio filename, selected region,
test-only/dev-mock flags, warning summary, and confidence summary can survive
save/load when encoded into the durable identity string.

CODEX-094 adds explicit result flags:

- `durableIdentityPersisted=true` only when the importer has attached the
  durable identity to a real imported layer that was inserted into a real
  `View`/`Pane` path.
- `structuredProvenancePersisted=false` because no full structured provenance
  store has been proven.

The importer also emits a warning with code
`structured_provenance_persistence_deferred` when provenance input exists and
only the durable identity path is available.

## Why Structured Persistence Is Deferred

The inspected source supports known XML attributes, not an arbitrary metadata
bag:

- `Model::toXml` writes model identity and model-specific attributes.
- `NoteModel::toXml` writes sparse note model data and known note model
  attributes.
- `Layer::toXml` writes layer type, layer object name, model reference, and
  `presentationName`.
- `SVFileReader::readLayer` restores known layer attributes and passes all
  attributes to `Layer::setProperties`, but `NoteLayer` and `FlexiNoteLayer`
  restore only their known display properties.
- `PropertyContainer` exposes fixed UI property names and integer values. It
  is not a generic persisted metadata map.
- `Event::label` and `Event::uri` can persist per-event text/URI, but using
  note labels or URIs for whole-layer backend provenance would mix layer
  metadata into editable musical data and is not an honest structured layer
  provenance store.
- Qt dynamic object properties were not found in the XML save/load path and
  must not be treated as persistent metadata.

Therefore adding arbitrary structured provenance today would require modifying
core XML persistence, introducing a sidecar/linking mechanism, or abusing
musical note fields. None of those is safe to claim as solved in CODEX-094.

## Required Structured Provenance Fields

A future structured mechanism should be able to preserve, where safe:

- backend ID;
- backend name;
- backend version;
- adapter version;
- run ID, request ID, and result ID;
- input audio stable reference or redacted path;
- selected region start/end;
- request JSON path;
- result JSON path;
- model/checkpoint path or redacted reference;
- backend settings summary or hash;
- confidence and warning/error summary;
- test-only/dev-mock flag;
- production-transcription flag;
- run timestamp;
- user edit status;
- exported/corrected-layer relationship.

## Candidate Future Designs

### Option 1: Tony/SV XML Metadata Extension

Add explicit provenance attributes or a nested provenance element to the
session XML for layers/models, then extend `SVFileReader` to restore it.

Requirements:

- define a schema/stable field list;
- avoid storing secrets or raw audio content;
- prove save/load round trip for all fields;
- prove older sessions still load;
- keep provenance distinct from editable note events.

Risk: touches core `Document`, `Layer`, model, or `SVFileReader` behavior and
therefore requires a dedicated task with regression proof.

### Option 2: Document-Owned Provenance Registry

Create a document-owned registry keyed by stable layer/model identity or run
ID, serialize that registry in the session, and restore it with the document.

Requirements:

- stable layer/model linkage across save/load;
- behavior when layers are duplicated, removed, copied, or exported;
- command/update semantics for user edit status;
- tests for missing or stale provenance entries.

Risk: requires new document-level state and lifecycle rules.

### Option 3: Sidecar Provenance File

Write a sidecar JSON file keyed by run ID and layer identity.

Requirements:

- explicit user/session linking;
- clear behavior if the sidecar file is missing, moved, or stale;
- privacy policy for paths and settings;
- no claim of session persistence unless the session records the link and
  reload validates it.

Risk: easy to lose or desynchronize. This should not be treated as solved until
session linking and reload validation are proven.

## Current Policy

Until a structured mechanism is implemented and proven:

- durable identity is the highest proven provenance persistence level;
- `structuredProvenancePersisted` must remain false;
- UI or reports must not claim full structured provenance persistence;
- full paths, backend settings, checkpoint paths, and user edit status must be
  reported as deferred unless a future task proves them;
- dev/mock provenance must remain test-only/debug-only;
- imported layers must still use real Tony/Sonic Visualiser models and layers.

## Recommended Next Task

Recommended next task: CODEX-095 - design a real Tony/Sonic Visualiser XML
provenance extension or document-owned provenance registry, including
backward-compatible save/load tests, before any UI claims full provenance.
