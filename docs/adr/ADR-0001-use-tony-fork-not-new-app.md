# ADR-0001-use-tony-fork-not-new-app — Use Tony fork instead of a new application

| Field | Value |
|---|---|
| Status | Accepted |
| Date | 2026-05-15 |
| Project | Tony Fork Transcription Workstation |
| Source documents | `docs/02_PRD.md`, `docs/03_SRS.md`, `docs/04_BACKEND_CONTRACT.md`, `docs/05_TDD_ARCHITECTURE.md` |

## Context

The product needs an existing pitch/note correction workflow, audio viewing, and pYIN-based transcription baseline. Building a new DAW-like editor from scratch would delay useful backend integration.

## Decision

Use Tony as the editor/core foundation. Preserve existing Tony and pYIN behavior. Add new analysis engines through adapters and the Backend Contract.

## Consequences

Reduces scope and risk. Requires careful integration with existing Tony architecture and licensing constraints.

## Notes

If future evidence contradicts this decision, create a new ADR that supersedes this one instead of silently editing implementation behavior.
