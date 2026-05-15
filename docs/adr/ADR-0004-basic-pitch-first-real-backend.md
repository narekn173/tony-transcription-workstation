# ADR-0004-basic-pitch-first-real-backend — Use Basic Pitch / NeuralNote path as first real backend

| Field | Value |
|---|---|
| Status | Accepted |
| Date | 2026-05-15 |
| Project | Tony Fork Transcription Workstation |
| Source documents | `docs/02_PRD.md`, `docs/03_SRS.md`, `docs/04_BACKEND_CONTRACT.md`, `docs/05_TDD_ARCHITECTURE.md` |

## Context

The first MVP backend should be practical, general-purpose, and able to produce MIDI/note-event output without requiring specialized violin-only workflows.

## Decision

Use Basic Pitch / NeuralNote path as the first real external/native-candidate backend after the dev-only mock backend.

## Consequences

Provides a useful vertical slice early. Specialized engines such as MUSC and VioPTT remain post-MVP phases.

## Notes

If future evidence contradicts this decision, create a new ADR that supersedes this one instead of silently editing implementation behavior.
