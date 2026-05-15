# SRS v0.1 — Tony Fork Transcription Workstation

## 1. Document control

| Field | Value |
|---|---|
| Document | Software Requirements Specification |
| Version | v0.1 |
| Product | Tony Fork Transcription Workstation |
| Source PRD | `docs/02_PRD.md` |
| Status | Draft |
| Primary platform | Windows |
| Primary processing model | Local-first |
| MVP strategy | Stable vertical slice before full feature expansion |

## 2. Purpose

This Software Requirements Specification defines the functional and non-functional software requirements for the Tony Fork Transcription Workstation. The application shall extend the existing Tony workflow with modern transcription backends, unified result display, manual correction, and export capabilities.

This SRS translates the PRD into precise, testable requirements. It does not define low-level implementation details such as C++ class names, JSON schema internals, or process-runner architecture unless those details affect externally observable behavior. Those details belong in the TDD and Backend Contract documents.

## 3. Scope

### In scope

The software shall support a local-first desktop workflow:

```text
Open audio → choose analysis engine → run real analysis → display notes/pitch/confidence → manually correct → export MIDI/CSV
```

The software shall progressively support:

- existing Tony/pYIN workflow;
- Basic Pitch / NeuralNote path;
- CREPE Notes;
- MUSC Violin Transcription;
- VioPTT Violin Technique-Aware Transcription;
- PESTO / PENN / FCPE f0 workflows;
- compare mode;
- selected-region analysis;
- future optional AI Copilot.

### Out of scope for MVP

- Full AI Copilot.
- Full multilingual UI.
- Full visual redesign.
- Bundled installers for every backend.
- Native/Vamp implementation for every backend.
- Commercial distribution readiness.
- macOS/Linux support.
- Perfect transcription claims.

## 4. Product perspective

The product is a fork or derivative workspace based on Tony. Tony remains the correction/editor core. New transcription engines are added through backend adapters and must appear to the user as integrated analysis methods inside the application.

The software shall not become a DAW, AI music generator, watermark-removal tool, or detector-bypass tool.

## 5. Definitions and abbreviations

| Term | Meaning |
|---|---|
| Tony | Existing Sonic Visualiser-based melody annotation application used as the base. |
| Backend | External or internal analysis engine that produces pitch, notes, MIDI, CSV, technique labels, or f0 data. |
| Analysis engine | User-facing method for generating transcription data. |
| pYIN | Existing classic pitch/note extraction method used by Tony. |
| Unified Result | Common internal representation of notes, pitch curve, confidence, velocity, technique labels, warnings, and metadata. |
| Corrected layer | The user-approved editable note layer intended for export. |
| Result layer | Non-destructive backend output layer. |
| Compare mode | Workflow for comparing results from multiple backends. |
| Selected-region analysis | Analysis applied only to a selected time range. |
| AI Copilot | Future assistant that explains, suggests, and coordinates actions, but does not replace transcription backends. |
| MVP | First stable vertical slice that proves the architecture with at least one real backend. |

## 6. Requirement quality rules

All requirements in this SRS should be:

- clear;
- atomic where practical;
- testable;
- traceable to PRD goals or assumptions;
- implementation-neutral unless a constraint is product-significant;
- free of unsupported claims.

## 7. Requirement priority model

| Priority | Meaning |
|---|---|
| Must | Required for MVP or required safety/integrity constraint. |
| Should | Important but may ship after MVP. |
| Could | Valuable future enhancement. |
| Won't for now | Explicitly excluded from current planning. |

## 8. Requirement status model

| Release | Meaning |
|---|---|
| MVP | First stable vertical slice. |
| Post-MVP | Required for full product but after MVP. |
| Future | Longer-term capability. |

## 9. Assumptions

| ID | Assumption |
|---|---|
| ASM-001 | Windows is the only required first platform. |
| ASM-002 | CPU-first execution is required for the initial release. |
| ASM-003 | GPU acceleration is optional and future-facing. |
| ASM-004 | Basic Pitch / NeuralNote path is the recommended first real backend unless later rejected. |
| ASM-005 | Backends are integrated progressively, not all at once. |
| ASM-006 | User-configured external backend paths are acceptable for early versions. |
| ASM-007 | Bundled backend environments are post-MVP. |
| ASM-008 | Full visual modernization is post-MVP. |
| ASM-009 | Simple Mode should be the default user-facing mode after UI modernization. |
| ASM-010 | AI Copilot is future scope and opt-in. |
| ASM-011 | Cloud/API functions are disabled by default. |
| ASM-012 | Logs are local and user-clearable. |
| ASM-013 | Mock backend is allowed only as a development/test tool. |
| ASM-014 | Commercialization is not a near-term requirement. |
| ASM-015 | A license register is required from the beginning. |

## 10. Open questions

| ID | Question | Blocking? |
|---|---|---:|
| OQ-001 | Final product name and branding. | No |
| OQ-002 | Whether the first public release includes backend code or only adapters/instructions. | No |
| OQ-003 | Whether project save/load is required in MVP or immediately after MVP. | No |
| OQ-004 | Whether AI will ever receive audio or only symbolic note data. | No |
| OQ-005 | How multilingual UI will be prioritized by language. | No |
| OQ-006 | Exact licensing strategy for GPL/AGPL backends before public distribution. | Yes before public release |

---

# 11. Functional requirements

## REQ-CORE-001
Priority: Must  
Release: MVP  
Source: PRD Goals G1-G3  
Requirement: The application shall provide a workflow for loading an audio file, selecting an analysis engine, running analysis, displaying transcription results, manually correcting results, and exporting the corrected output.  
Acceptance criteria:
- User can load an audio file supported by the application.
- User can choose at least one real analysis engine in MVP.
- User can start analysis from the UI.
- User can see the result as editable notes.
- User can export the corrected notes.

## REQ-CORE-002
Priority: Must  
Release: MVP  
Source: PRD no-fake policy  
Requirement: The application shall not display production analysis results unless they are generated by a real backend or existing Tony analysis process.  
Acceptance criteria:
- No production screen shows fake notes.
- No production screen shows fake confidence values.
- No production screen shows fake completed states.
- Development-only data is clearly labeled as development/test-only.

## REQ-CORE-003
Priority: Must  
Release: MVP  
Source: PRD local-first principle  
Requirement: The application shall run transcription backends locally by default.  
Acceptance criteria:
- No cloud/API request is required for MVP analysis.
- The application does not upload audio without explicit user action.

## REQ-CORE-004
Priority: Must  
Release: MVP  
Source: PRD MVP strategy  
Requirement: The application shall support a stable vertical-slice MVP before implementing all planned backends.  
Acceptance criteria:
- MVP includes at least one real backend.
- MVP does not require all listed backends to be implemented.
- Post-MVP features are not represented as complete production functionality.

## REQ-CORE-005
Priority: Should  
Release: Post-MVP  
Source: PRD Post-MVP scope  
Requirement: The application should allow users to keep multiple backend result layers for the same audio file.  
Acceptance criteria:
- Each backend result can be displayed separately.
- Result layers can be hidden or shown.
- User can identify which backend produced each layer.

---

# 12. Existing Tony preservation requirements

## REQ-TONY-001
Priority: Must  
Release: MVP  
Source: PRD G1  
Requirement: Existing Tony functionality shall remain available unless explicitly replaced by a documented requirement.  
Acceptance criteria:
- Original Tony application can still open supported audio files.
- Existing pYIN workflow remains accessible.
- Existing correction workflow is not removed.

## REQ-TONY-002
Priority: Must  
Release: MVP  
Source: PRD First implementation milestones  
Requirement: The first engineering milestone shall verify that the original Tony fork builds and runs before any feature work is added.  
Acceptance criteria:
- Build instructions are documented.
- Tony launches successfully.
- A build log or verification note exists.

## REQ-TONY-003
Priority: Must  
Release: MVP  
Source: PRD Goals G1, G6  
Requirement: New backend integrations shall not remove or disable manual note correction.  
Acceptance criteria:
- User can edit imported notes after backend analysis.
- User can export corrected notes after editing.

---

# 13. Backend and analysis-engine requirements

## REQ-BACKEND-001
Priority: Must  
Release: MVP  
Source: PRD Engine table  
Requirement: The application shall expose analysis engines through a unified user-facing selection workflow.  
Acceptance criteria:
- User can see available analysis engines.
- User can select an engine before analysis.
- At least one real external backend is available in MVP.

## REQ-BACKEND-002
Priority: Must  
Release: MVP  
Source: ASM-004  
Requirement: The MVP shall implement one real external backend, with Basic Pitch / NeuralNote path as the recommended first candidate unless superseded by later decision.  
Acceptance criteria:
- Backend can be launched from the UI.
- Backend output is imported into the application.
- Backend failure is shown to the user.

## REQ-BACKEND-003
Priority: Must  
Release: MVP  
Source: PRD Backend requirements  
Requirement: External backend execution shall expose a clear success, warning, failure, or cancellation state to the user.  
Acceptance criteria:
- Completed analysis is labeled completed.
- Failed analysis is labeled failed.
- Cancelled analysis is labeled cancelled.
- Missing backend is not labeled as available.

## REQ-BACKEND-004
Priority: Must  
Release: MVP  
Source: PRD Reliability  
Requirement: The application shall not silently ignore backend errors.  
Acceptance criteria:
- Non-zero backend exit produces a visible failure state.
- Missing output file produces a visible failure state.
- Invalid output produces a visible parser/import error.

## REQ-BACKEND-005
Priority: Must  
Release: MVP  
Source: ASM-006  
Requirement: Early versions shall allow the user to configure paths to external backend executables or scripts.  
Acceptance criteria:
- User can set or change backend path.
- Invalid path is detected.
- Path configuration is persisted locally.

## REQ-BACKEND-006
Priority: Should  
Release: Post-MVP  
Source: PRD Post-MVP Phase 1  
Requirement: The application should integrate CREPE Notes as an external backend before considering Vamp/native conversion.  
Acceptance criteria:
- CREPE Notes can be run from the UI.
- MIDI or note output is imported.
- Monophonic limitation is visible to the user.

## REQ-BACKEND-007
Priority: Should  
Release: Post-MVP  
Source: PRD Post-MVP Phase 2  
Requirement: The application should integrate MUSC Violin Transcription as a violin-specialized external backend.  
Acceptance criteria:
- User can run MUSC backend on compatible audio.
- Output is imported as result layer.
- Any violin-specialized limitation is visible to the user.

## REQ-BACKEND-008
Priority: Should  
Release: Post-MVP  
Source: PRD Post-MVP Phase 3  
Requirement: The application should integrate VioPTT as an external backend for violin transcription and technique labels.  
Acceptance criteria:
- User can run VioPTT on compatible audio.
- MIDI/note output is imported.
- Technique labels are imported where available.

## REQ-BACKEND-009
Priority: Should  
Release: Post-MVP  
Source: PRD Post-MVP Phase 4  
Requirement: The application should integrate PESTO, PENN, and FCPE as f0/pitch backends rather than treating them as complete note editors.  
Acceptance criteria:
- f0 curve can be imported.
- Confidence/periodicity data can be represented where available.
- Note segmentation is handled by a separate application layer or adapter.

## REQ-BACKEND-010
Priority: Should  
Release: Post-MVP  
Source: PRD Compare mode  
Requirement: The application should support sequential backend execution for multi-engine analysis to avoid overloading CPU/RAM.  
Acceptance criteria:
- Batch execution runs one backend at a time by default.
- User can cancel the batch.
- Progress identifies the currently running backend.

## REQ-BACKEND-011
Priority: Must  
Release: MVP  
Source: PRD No-fake policy  
Requirement: Development-only mock backend shall be unavailable or clearly labeled in production builds.  
Acceptance criteria:
- Mock backend is named “Development/Test Mock Backend” if visible.
- Mock backend cannot be confused with a real analysis engine.

---

# 14. Backend Manager requirements

## REQ-BM-001
Priority: Should  
Release: MVP  
Source: PRD UX principles  
Requirement: The application should provide a Backend Manager screen or panel for configuring analysis engines.  
Acceptance criteria:
- User can view backend status.
- User can configure backend path.
- User can test backend availability.

## REQ-BM-002
Priority: Must  
Release: MVP  
Source: PRD Reliability states  
Requirement: The application shall represent backend status using clear states.  
Acceptance criteria:
- Supported states include at least: Not Configured, Missing, Installed, Broken, Running, Completed, Failed.
- The user can distinguish missing backend from failed analysis.

## REQ-BM-003
Priority: Should  
Release: Post-MVP  
Source: PRD Backend Manager concept  
Requirement: Backend Manager should display backend version where the backend can report it.  
Acceptance criteria:
- Version is shown when available.
- Unknown version is shown as unknown, not fabricated.

## REQ-BM-004
Priority: Should  
Release: Post-MVP  
Source: PRD Backend Manager concept  
Requirement: Backend Manager should display runtime type for each backend.  
Acceptance criteria:
- Runtime type can show Python, ONNX/native, Vamp, or Unknown.

## REQ-BM-005
Priority: Should  
Release: Post-MVP  
Source: PRD Backend Manager concept  
Requirement: Backend Manager should provide setup/help information for missing backends.  
Acceptance criteria:
- Missing backend shows actionable setup guidance.
- Setup guidance does not claim installation was completed unless verified.

---

# 15. Unified result and result-layer requirements

## REQ-RESULT-001
Priority: Must  
Release: MVP  
Source: PRD Unified Result Format  
Requirement: The application shall normalize backend outputs into a unified internal representation before display.  
Acceptance criteria:
- Imported notes share a common application representation.
- Backend-specific output is not directly required by UI layers.

## REQ-RESULT-002
Priority: Must  
Release: MVP  
Source: PRD result visualization  
Requirement: The unified result shall support note events at minimum.  
Acceptance criteria:
- Each note event can represent start time, end time, and pitch.
- Velocity can be represented where available.
- Confidence can be represented where available.

## REQ-RESULT-003
Priority: Should  
Release: Post-MVP  
Source: PRD visualization layers  
Requirement: The unified result should support pitch curve points.  
Acceptance criteria:
- Time and frequency are representable.
- Confidence is representable where available.

## REQ-RESULT-004
Priority: Should  
Release: Post-MVP  
Source: PRD VioPTT scope  
Requirement: The unified result should support per-note technique labels.  
Acceptance criteria:
- Technique label can be attached to a note.
- Unknown/no-technique state is representable.

## REQ-RESULT-005
Priority: Should  
Release: Post-MVP  
Source: PRD Compare mode  
Requirement: Result layers should preserve backend provenance.  
Acceptance criteria:
- User can identify which backend generated a layer.
- Export/debug files include backend ID.

## REQ-RESULT-006
Priority: Should  
Release: Post-MVP  
Source: PRD Project file  
Requirement: Result layers should be saveable in a project file after project save/load is implemented.  
Acceptance criteria:
- Project reload restores result layers.
- Corrected layer is distinguishable from raw backend layers.

---

# 16. UI/UX requirements

## REQ-UI-001
Priority: Must  
Release: MVP  
Source: PRD UX principles  
Requirement: The UI shall provide a clear way to run analysis without requiring the user to interact directly with command-line backends.  
Acceptance criteria:
- User can launch analysis from the application UI.
- User does not need to type backend commands inside the application workflow.

## REQ-UI-002
Priority: Must  
Release: MVP  
Source: PRD Reliability  
Requirement: The UI shall show analysis progress and current state.  
Acceptance criteria:
- User sees when analysis is running.
- User sees when analysis completes.
- User sees when analysis fails.

## REQ-UI-003
Priority: Should  
Release: Post-MVP  
Source: PRD UX modes  
Requirement: The UI should support Simple Mode and Advanced Mode.  
Acceptance criteria:
- Simple Mode exposes only core workflow controls.
- Advanced Mode exposes backend settings, logs, and comparison controls.

## REQ-UI-004
Priority: Should  
Release: Post-MVP  
Source: PRD UI modernization  
Requirement: The UI should support system theme and dark theme after core backend workflow is stable.  
Acceptance criteria:
- User can use system theme.
- Dark theme does not reduce readability of notes/pitch/confidence.

## REQ-UI-005
Priority: Should  
Release: Post-MVP  
Source: PRD Wizard assumption  
Requirement: The UI should provide an optional beginner workflow wizard.  
Acceptance criteria:
- Wizard can guide: Load audio → Choose goal → Choose engine → Run → Review → Export.
- Advanced users can bypass the wizard.

## REQ-UI-006
Priority: Must  
Release: MVP  
Source: PRD no-fake policy  
Requirement: The UI shall not mark analysis as completed unless valid output has been produced or imported.  
Acceptance criteria:
- Backend failure cannot show Completed.
- Missing output cannot show Completed.

---

# 17. Import requirements

## REQ-IMPORT-001
Priority: Must  
Release: MVP  
Source: PRD Core workflow  
Requirement: The application shall support importing audio through the existing Tony-supported workflow for MVP.  
Acceptance criteria:
- User can open at least the audio formats already supported by the base application.
- Unsupported files show readable error.

## REQ-IMPORT-002
Priority: Should  
Release: Post-MVP  
Source: PRD user request for logical formats  
Requirement: The application should document supported audio formats explicitly.  
Acceptance criteria:
- Supported formats are listed in UI or documentation.
- Unsupported formats are not silently accepted.

## REQ-IMPORT-003
Priority: Should  
Release: Post-MVP  
Source: PRD selected-region workflow  
Requirement: The application should be able to generate temporary audio slices for selected-region analysis.  
Acceptance criteria:
- Slice start/end match selected region within documented tolerance.
- Temporary slice is deleted or managed according to privacy settings.

---

# 18. Export requirements

## REQ-EXPORT-001
Priority: Must  
Release: MVP  
Source: PRD Export requirements  
Requirement: The application shall export corrected note data as MIDI.  
Acceptance criteria:
- Exported MIDI contains corrected note start, end, and pitch data.
- Export succeeds after at least one imported backend result is corrected or accepted.

## REQ-EXPORT-002
Priority: Must  
Release: MVP  
Source: PRD Export requirements  
Requirement: The application shall export note event data as CSV or provide CSV export immediately after MVP if blocked by existing code constraints.  
Acceptance criteria:
- CSV includes note start, end, and pitch.
- Missing optional fields are represented clearly or omitted with documented behavior.

## REQ-EXPORT-003
Priority: Should  
Release: Post-MVP  
Source: PRD Export requirements  
Requirement: The application should export unified result JSON for debugging and comparison.  
Acceptance criteria:
- JSON contains backend ID and note events.
- JSON does not pretend unsupported fields exist.

## REQ-EXPORT-004
Priority: Should  
Release: Post-MVP  
Source: PRD Project file  
Requirement: The application should support project save/load.  
Acceptance criteria:
- Corrected layer persists.
- Backend result layers persist.
- Backend settings or metadata persist where appropriate.

## REQ-EXPORT-005
Priority: Could  
Release: Future  
Source: PRD Export requirements  
Requirement: The application could support MusicXML export.  
Acceptance criteria:
- MusicXML export is documented as experimental or supported.
- Exported file can be opened by a common notation program during validation.

---

# 19. Full-file analysis requirements

## REQ-CORE-006
Priority: Must  
Release: MVP  
Source: PRD Core workflow  
Requirement: The application shall support running analysis on the full loaded audio file for the selected backend.  
Acceptance criteria:
- User can select a backend and run full-file analysis.
- Result layer is produced or failure is shown.

## REQ-CORE-007
Priority: Must  
Release: MVP  
Source: PRD Reliability  
Requirement: The user shall be able to cancel a running analysis if the backend runner supports cancellation.  
Acceptance criteria:
- Cancel action is visible while analysis is running.
- Cancelled job is marked Cancelled, not Failed or Completed.

---

# 20. Selected-region analysis requirements

## REQ-REGION-001
Priority: Should  
Release: Post-MVP  
Source: PRD selected-region workflow  
Requirement: The application should allow analysis of a selected time region.  
Acceptance criteria:
- User can select a time range.
- User can run compatible backend on that range.
- Result aligns to original timeline.

## REQ-REGION-002
Priority: Should  
Release: Post-MVP  
Source: PRD safe apply rules  
Requirement: Selected-region analysis shall be non-destructive by default.  
Acceptance criteria:
- Analysis result appears as preview or separate layer.
- Existing corrected notes are not overwritten before user confirmation.

## REQ-REGION-003
Priority: Should  
Release: Post-MVP  
Source: PRD safe apply rules  
Requirement: User should be able to apply selected-region result only inside the selected time range.  
Acceptance criteria:
- Notes outside selected region remain unchanged.
- User can reject preview without changes.

## REQ-REGION-004
Priority: Should  
Release: Post-MVP  
Source: PRD micro-reconstruction use case  
Requirement: The selected-region workflow should support short difficult fragments.  
Acceptance criteria:
- UI supports selecting small regions such as 1–3 seconds.
- Backend limitations for short audio are shown if applicable.

---

# 21. Compare-mode requirements

## REQ-COMPARE-001
Priority: Should  
Release: Post-MVP  
Source: PRD compare workflow  
Requirement: The application should allow the user to run multiple compatible analysis engines sequentially.  
Acceptance criteria:
- User can choose which engines to include.
- Backends run one after another by default.
- Progress identifies the active engine.

## REQ-COMPARE-002
Priority: Should  
Release: Post-MVP  
Source: PRD compare workflow  
Requirement: The application should display each backend result as a separate layer or variant.  
Acceptance criteria:
- User can show/hide each backend result.
- User can identify backend source for each layer.

## REQ-COMPARE-003
Priority: Should  
Release: Post-MVP  
Source: PRD comparison modes  
Requirement: The application should allow the user to choose a whole backend result as the main corrected layer.  
Acceptance criteria:
- User can preview result before replacing corrected layer.
- Replacement requires confirmation.

## REQ-COMPARE-004
Priority: Should  
Release: Post-MVP  
Source: PRD comparison modes  
Requirement: The application should allow copying selected notes from a backend result into the corrected layer.  
Acceptance criteria:
- User can select one or more notes from a result layer.
- User can apply them to the corrected layer.
- Undo is available after apply.

## REQ-COMPARE-005
Priority: Could  
Release: Future  
Source: PRD comparison modes  
Requirement: The application could show consensus/voting when several engines detect similar notes.  
Acceptance criteria:
- Consensus score is based on documented matching criteria.
- User can view which engines contributed.

## REQ-COMPARE-006
Priority: Could  
Release: Future  
Source: PRD comparison modes  
Requirement: The application could highlight regions with high disagreement between engines.  
Acceptance criteria:
- Disagreement is visually marked.
- Marking is based on documented timing/pitch thresholds.

---

# 22. Manual correction requirements

## REQ-EDIT-001
Priority: Must  
Release: MVP  
Source: PRD Manual correction  
Requirement: The application shall allow user correction of imported note events.  
Acceptance criteria:
- User can change at least note pitch, start, end, and deletion/addition where supported by Tony.
- Corrected result can be exported.

## REQ-EDIT-002
Priority: Must  
Release: MVP  
Source: PRD Manual correction  
Requirement: Undo/redo shall be available for note correction operations where supported by the base application.  
Acceptance criteria:
- User can undo a note edit.
- User can redo an undone note edit.

## REQ-EDIT-003
Priority: Should  
Release: Post-MVP  
Source: PRD future correction features  
Requirement: The application should support split and merge operations for notes.  
Acceptance criteria:
- User can split a selected note.
- User can merge adjacent selected notes where valid.

## REQ-EDIT-004
Priority: Should  
Release: Post-MVP  
Source: PRD future correction features  
Requirement: The application should support velocity adjustment where backend/output format provides velocity.  
Acceptance criteria:
- User can view velocity.
- User can edit velocity.
- MIDI export preserves edited velocity.

## REQ-EDIT-005
Priority: Could  
Release: Future  
Source: PRD future correction features  
Requirement: The application could support editing pitch bend curves.  
Acceptance criteria:
- User can view pitch bend curve.
- User can edit curve points or apply smoothing.
- MIDI export preserves edited bends.

## REQ-EDIT-006
Priority: Could  
Release: Future  
Source: PRD future correction features  
Requirement: The application could provide Kontakt-ready MIDI cleanup operations.  
Acceptance criteria:
- User can run cleanup preview.
- Cleanup changes are shown before apply.
- User can undo cleanup.

---

# 23. Visualization requirements

## REQ-VIS-001
Priority: Must  
Release: MVP  
Source: PRD Visualization  
Requirement: The application shall display note events from the selected or imported result.  
Acceptance criteria:
- Notes are visible on the timeline.
- Notes align to audio time.

## REQ-VIS-002
Priority: Must  
Release: MVP  
Source: PRD Visualization  
Requirement: The application shall display waveform or existing Tony audio visualization for loaded audio.  
Acceptance criteria:
- User can inspect audio timeline.
- Notes can be visually compared with audio position.

## REQ-VIS-003
Priority: Should  
Release: Post-MVP  
Source: PRD Visualization  
Requirement: The application should display pitch curve where provided by backend or existing Tony analysis.  
Acceptance criteria:
- Pitch curve aligns to timeline.
- Missing pitch curve is shown as unavailable, not fabricated.

## REQ-VIS-004
Priority: Should  
Release: Post-MVP  
Source: PRD Confidence display  
Requirement: The application should visualize confidence without overwhelming beginner users.  
Acceptance criteria:
- Low-confidence notes are visibly marked.
- Exact confidence is available in selection/details panel.

## REQ-VIS-005
Priority: Should  
Release: Post-MVP  
Source: PRD VioPTT  
Requirement: The application should display technique labels where provided by VioPTT or future backends.  
Acceptance criteria:
- Technique labels can be shown/hidden.
- Labels are attached to relevant notes.

## REQ-VIS-006
Priority: Could  
Release: Future  
Source: PRD pitch bend/deviation  
Requirement: The application could display pitch bend or pitch deviation curves for expressive instruments.  
Acceptance criteria:
- Curves align with notes and audio timeline.
- Export behavior is documented.

---

# 24. Error handling and reliability requirements

## REQ-ERR-001
Priority: Must  
Release: MVP  
Source: PRD Reliability  
Requirement: The application shall distinguish between backend missing, backend failed, output invalid, and analysis cancelled.  
Acceptance criteria:
- Each state has a separate user-visible message.
- Logs or details are available for troubleshooting.

## REQ-ERR-002
Priority: Must  
Release: MVP  
Source: PRD Reliability  
Requirement: The application shall not crash when a backend fails.  
Acceptance criteria:
- Backend failure returns user to usable UI state.
- Existing project/audio remains open if possible.

## REQ-ERR-003
Priority: Must  
Release: MVP  
Source: PRD Reliability  
Requirement: Invalid backend output shall not be imported as valid notes.  
Acceptance criteria:
- Invalid output produces import/parser error.
- No fake replacement result is generated.

## REQ-ERR-004
Priority: Should  
Release: Post-MVP  
Source: PRD Reliability  
Requirement: The application should preserve backend stdout/stderr or equivalent logs locally for troubleshooting.  
Acceptance criteria:
- User can view or open log details.
- User can clear logs.

## REQ-ERR-005
Priority: Should  
Release: Post-MVP  
Source: PRD Reliability  
Requirement: Long-running backend tasks should support timeout or cancellation.  
Acceptance criteria:
- User can cancel when supported.
- Timeout is reported as timeout, not generic failure.

---

# 25. Privacy and security requirements

## REQ-PRIV-001
Priority: Must  
Release: MVP  
Source: PRD local-first  
Requirement: The application shall not upload audio files by default.  
Acceptance criteria:
- MVP analysis works without cloud upload.
- No hidden cloud analysis is performed.

## REQ-PRIV-002
Priority: Must  
Release: MVP  
Source: PRD cloud/API policy  
Requirement: API keys shall not be stored in source code.  
Acceptance criteria:
- Repository contains no hard-coded API key.
- User-specific API key storage is local and configurable when AI features are added.

## REQ-PRIV-003
Priority: Should  
Release: Future  
Source: PRD AI Copilot cloud policy  
Requirement: Cloud/API AI features should require explicit user opt-in.  
Acceptance criteria:
- User sees notice before enabling cloud AI.
- User can disable cloud AI.
- User can choose not to show the warning again only after seeing it.

## REQ-PRIV-004
Priority: Should  
Release: Post-MVP  
Source: PRD privacy-conscious logs  
Requirement: Logs should be local and user-clearable.  
Acceptance criteria:
- User can clear logs.
- Logs are not uploaded automatically.

## REQ-PRIV-005
Priority: Must  
Release: MVP  
Source: PRD permanent non-goals  
Requirement: The application shall not include product claims or UI language about AI watermark removal or detector bypass.  
Acceptance criteria:
- No such claims exist in UI, documentation, or onboarding text.

---

# 26. Performance requirements

## REQ-PERF-001
Priority: Must  
Release: MVP  
Source: ASM-002  
Requirement: MVP analysis workflow shall be usable on CPU-only systems.  
Acceptance criteria:
- MVP backend can run without dedicated GPU.
- If performance is slow, UI remains responsive or shows progress.

## REQ-PERF-002
Priority: Should  
Release: Post-MVP  
Source: PRD batch-run workflow  
Requirement: Multi-engine batch analysis should run engines sequentially by default.  
Acceptance criteria:
- Only one heavy backend is active at a time unless user explicitly changes setting.
- UI shows queue/progress.

## REQ-PERF-003
Priority: Should  
Release: Post-MVP  
Source: PRD cancellation  
Requirement: The UI should remain responsive during long analysis tasks.  
Acceptance criteria:
- User can cancel or inspect progress.
- Application does not appear frozen during backend execution.

## REQ-PERF-004
Priority: Could  
Release: Future  
Source: user hardware note  
Requirement: The application could support GPU acceleration where backend and hardware support it.  
Acceptance criteria:
- GPU support is optional.
- CPU fallback exists.
- Unsupported GPU is reported clearly.

---

# 27. Platform and installation requirements

## REQ-PLAT-001
Priority: Must  
Release: MVP  
Source: ASM-001  
Requirement: The first target platform shall be Windows.  
Acceptance criteria:
- Build and run instructions exist for Windows.
- MVP validation is performed on Windows.

## REQ-PLAT-002
Priority: Should  
Release: Post-MVP  
Source: PRD installer  
Requirement: The application should provide a Windows installer after the MVP architecture is stable.  
Acceptance criteria:
- Installer installs application files.
- Installer does not falsely claim optional backends are installed unless bundled and verified.

## REQ-PLAT-003
Priority: Should  
Release: Post-MVP  
Source: ASM-006/ASM-007  
Requirement: The first backend integration approach should support user-configured external backend paths before bundled backend packaging.  
Acceptance criteria:
- User can configure path.
- Backend Manager shows status.

## REQ-PLAT-004
Priority: Won't for now  
Release: Future  
Source: user answer L61  
Requirement: macOS and Linux support are not required for the first version.  
Acceptance criteria:
- No MVP acceptance criterion depends on macOS/Linux.

---

# 28. Localization requirements

## REQ-I18N-001
Priority: Could  
Release: Future  
Source: user answer J54  
Requirement: The application could support multiple UI languages, including English, Russian, Chinese, Japanese, and other popular languages.  
Acceptance criteria:
- UI strings are externalized before translation work begins.
- English remains the source language for development documentation.

## REQ-I18N-002
Priority: Should  
Release: Post-MVP  
Source: PRD multilingual scope  
Requirement: UI modernization should avoid hard-coding user-facing strings where practical.  
Acceptance criteria:
- New user-facing strings are isolated enough for future localization.

---

# 29. AI Copilot future requirements

## REQ-AI-001
Priority: Could  
Release: Future  
Source: PRD AI Copilot  
Requirement: The application could provide an AI Copilot panel for explanation, recommendation, and correction suggestions.  
Acceptance criteria:
- AI Copilot is optional.
- User can disable AI Copilot.
- AI suggestions are visually separated from confirmed edits.

## REQ-AI-002
Priority: Could  
Release: Future  
Source: PRD AI correction principle  
Requirement: AI Copilot shall not silently overwrite notes without user confirmation.  
Acceptance criteria:
- Suggested changes require explicit accept/apply action.
- User can reject suggestions.
- Applied AI suggestions are undoable.

## REQ-AI-003
Priority: Could  
Release: Future  
Source: PRD AI features  
Requirement: AI Copilot could recommend analysis backends based on user goal and available backend capabilities.  
Acceptance criteria:
- Recommendation shows reasoning.
- User can ignore recommendation.

## REQ-AI-004
Priority: Could  
Release: Future  
Source: PRD AI features  
Requirement: AI Copilot could identify suspicious regions based on backend disagreement, low confidence, or musical feasibility rules.  
Acceptance criteria:
- Suspicious region is highlighted.
- Reason is shown to user.

## REQ-AI-005
Priority: Could  
Release: Future  
Source: PRD AI features  
Requirement: AI Copilot could check playable ranges for supported instruments.  
Acceptance criteria:
- Instrument is selected by user or inferred with confirmation.
- Out-of-range notes are marked as suggestions/warnings, not automatic errors.

## REQ-AI-006
Priority: Won't for now  
Release: Future  
Source: PRD permanent non-goals  
Requirement: AI Copilot shall not be described as a watermark-removal or detector-bypass tool.  
Acceptance criteria:
- UI/documentation avoids these claims.

---

# 30. Licensing and compliance requirements

## REQ-LIC-001
Priority: Must  
Release: MVP  
Source: PRD license risks  
Requirement: The project shall maintain a license register for Tony and every backend/adapted dependency.  
Acceptance criteria:
- Register includes project name, repository URL, license, integration type, redistribution status, and notes.

## REQ-LIC-002
Priority: Must  
Release: Before public release  
Source: PRD license risks  
Requirement: Public distribution shall not occur until GPL/AGPL/backend redistribution obligations are reviewed.  
Acceptance criteria:
- License review notes exist.
- Distribution model is documented.

## REQ-LIC-003
Priority: Should  
Release: Post-MVP  
Source: PRD license risks  
Requirement: Backends with restrictive or unclear redistribution terms should be optional external dependencies unless license review approves bundling.  
Acceptance criteria:
- Optional backend is not bundled without documented decision.
- UI can point to user-configured path instead.

---

# 31. MVP acceptance criteria

The MVP is accepted only if all criteria below are satisfied:

| ID | Criterion |
|---|---|
| AC-MVP-001 | Original Tony fork builds and launches on Windows. |
| AC-MVP-002 | Existing pYIN/Tony workflow remains accessible. |
| AC-MVP-003 | At least one real external backend can be configured. |
| AC-MVP-004 | At least one real backend can be run from the UI. |
| AC-MVP-005 | Backend success/failure/cancel states are visible. |
| AC-MVP-006 | Real backend note output is imported into the application. |
| AC-MVP-007 | User can manually correct imported notes. |
| AC-MVP-008 | User can export corrected result as MIDI. |
| AC-MVP-009 | CSV or equivalent note-event export is available or explicitly deferred to immediately after MVP. |
| AC-MVP-010 | No production fake notes/results/progress states exist. |
| AC-MVP-011 | No cloud/API upload is required. |
| AC-MVP-012 | License register exists. |

---

# 32. Post-MVP acceptance criteria

| Phase | Acceptance criteria |
|---|---|
| Post-MVP 1 | CREPE Notes external backend works; selected-region preview begins. |
| Post-MVP 2 | Basic compare mode works between at least two engines. |
| Post-MVP 3 | MUSC backend produces importable violin result layer. |
| Post-MVP 4 | VioPTT backend imports technique labels. |
| Post-MVP 5 | PESTO/PENN/FCPE import f0 curves into shared flow. |
| Post-MVP 6 | Result layers can be compared and selectively applied. |
| Future | AI Copilot explains and suggests, but requires confirmation before applying edits. |

---

# 33. Traceability matrix to PRD goals

| PRD Goal | Related SRS requirements |
|---|---|
| G1 Preserve Tony | REQ-TONY-001, REQ-TONY-002, REQ-TONY-003 |
| G2 Add modern engines | REQ-BACKEND-001 through REQ-BACKEND-011 |
| G3 External backends feel native | REQ-UI-001, REQ-BM-001, REQ-BACKEND-001 |
| G4 Full/region analysis | REQ-CORE-006, REQ-REGION-001 through REQ-REGION-004 |
| G5 Compare engines | REQ-COMPARE-001 through REQ-COMPARE-006 |
| G6 Manual correction central | REQ-EDIT-001 through REQ-EDIT-006 |
| G7 Local-first | REQ-PRIV-001 through REQ-PRIV-004 |
| G8 Future AI Copilot | REQ-AI-001 through REQ-AI-006 |
| No fake results | REQ-CORE-002, REQ-UI-006, REQ-ERR-003 |
| Export workflow | REQ-EXPORT-001 through REQ-EXPORT-005 |
| License safety | REQ-LIC-001 through REQ-LIC-003 |

---

# 34. Requirements moved to TDD

The following are intentionally not specified here as implementation details and must be defined in the TDD:

- C++ class names and interfaces.
- Exact backend process runner implementation.
- Exact Tony layer importer implementation.
- Exact UI widget hierarchy.
- Exact temp-file management implementation.
- Exact threading/concurrency model.
- Exact cancellation implementation.
- Exact pitch-curve rendering implementation.
- Exact project serialization design.
- Exact AI Copilot function-calling architecture.

---

# 35. Requirements moved to Backend Contract

The following must be defined in `docs/04_BACKEND_CONTRACT.md`:

- Unified Result JSON schema.
- Backend capability declaration schema.
- Note event schema.
- Pitch curve schema.
- Technique label schema.
- Velocity/confidence schema.
- Warning/error schema.
- Backend metadata schema.
- Required adapter input parameters.
- Required adapter output files.
- Validation rules for backend output.

---

# 36. Plain-language MVP summary

The first working version should not try to include every model at once. It should prove the whole workflow:

```text
Open audio → run one real backend → see real notes → correct them → export MIDI.
```

After this works safely, the project can add CREPE Notes, MUSC, VioPTT, PESTO/PENN/FCPE, selected-region analysis, compare mode, UI modernization, and later AI Copilot.
