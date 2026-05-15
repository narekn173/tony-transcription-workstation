# ADR-0002-external-backends-first — Use external backends first

| Field | Value |
|---|---|
| Status | Accepted |
| Date | 2026-05-15 |
| Project | Tony Fork Transcription Workstation |
| Source documents | `docs/02_PRD.md`, `docs/03_SRS.md`, `docs/04_BACKEND_CONTRACT.md`, `docs/05_TDD_ARCHITECTURE.md` |

## Context

Most target engines are Python/PyTorch/ONNX/research-code projects. Native/Vamp integration for all engines would be expensive and risky as a first step.

## Decision

Integrate modern engines first as external backends behind BackendAdapter and ExternalProcessRunner. Native/Vamp ports may be considered later after behavior is proven.

## Consequences

Faster MVP and lower risk. Requires robust process management, validation, error handling, and backend configuration UI.

## Notes

If future evidence contradicts this decision, create a new ADR that supersedes this one instead of silently editing implementation behavior.
