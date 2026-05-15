# ADR-0003-unified-result-contract — Normalize all backend output to UnifiedResult

| Field | Value |
|---|---|
| Status | Accepted |
| Date | 2026-05-15 |
| Project | Tony Fork Transcription Workstation |
| Source documents | `docs/02_PRD.md`, `docs/03_SRS.md`, `docs/04_BACKEND_CONTRACT.md`, `docs/05_TDD_ARCHITECTURE.md` |

## Context

Each backend emits different formats: MIDI, CSV, f0 curves, pitch bends, labels, or logs. UI and correction logic would become fragile if every backend bypassed a common model.

## Decision

All backend outputs must be converted into the Backend Contract's UnifiedResult before display/import into Tony layers.

## Consequences

Enables interchangeable backends, compare mode, validation, and safer UI. Requires adapters/parsers per backend.

## Notes

If future evidence contradicts this decision, create a new ADR that supersedes this one instead of silently editing implementation behavior.
