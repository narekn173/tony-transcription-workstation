# ADR-0005-local-first-ai-copilot-opt-in — Local-first processing with AI Copilot opt-in

| Field | Value |
|---|---|
| Status | Accepted |
| Date | 2026-05-15 |
| Project | Tony Fork Transcription Workstation |
| Source documents | `docs/02_PRD.md`, `docs/03_SRS.md`, `docs/04_BACKEND_CONTRACT.md`, `docs/05_TDD_ARCHITECTURE.md` |

## Context

The product handles user audio, MIDI, paths, logs, and future AI assistance. Hidden cloud upload would violate the product's privacy goals.

## Decision

All transcription processing is local by default. AI Copilot is future scope and must be optional, explicit, and user-confirmed. API keys must not be stored in source code.

## Consequences

Improves privacy and trust. Cloud AI features require clear consent UX and secure local settings later.

## Notes

If future evidence contradicts this decision, create a new ADR that supersedes this one instead of silently editing implementation behavior.
