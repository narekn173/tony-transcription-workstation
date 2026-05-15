# PRD v0.1 — Tony Fork Transcription Workstation

## 1. Product title

**Tony Fork Transcription Workstation**

Рабочее название:

```text id="53ofn3"
Tony Modern Transcription Workstation
```

Альтернативные названия для будущего:

```text id="s0yqs0"
Tony AI Transcription Workstation
Tony Multi-Engine Transcriber
Open Transcription Workstation
```

---

## 2. One-paragraph product summary

Tony Fork Transcription Workstation — это desktop-приложение на базе Tony, предназначенное для точной аудио-в-MIDI транскрипции, анализа pitch/note data, сравнения разных transcription engines, ручной коррекции нот и экспорта результата в MIDI/CSV/проектный формат. Продукт расширяет существующий Tony современными external backends: Basic Pitch/NeuralNote, CREPE Notes, MUSC Violin Transcription, VioPTT, PESTO/PENN/FCPE. Основная идея: пользователь загружает audio/stem, выбирает метод анализа или запускает несколько методов по очереди, видит реальные результаты в удобном интерфейсе, сравнивает варианты, вручную исправляет ошибки и экспортирует чистый MIDI.

Tony уже предоставляет GUI для melody annotation, pYIN-based pitch/note extraction, ручную коррекцию pitch/note track, audition и import/export pitch/note track; поэтому проект должен использовать Tony как основу редактора, а не переписывать всё с нуля. ([github.com](https://github.com/sonic-visualiser/tony))

---

## 3. Background and problem statement

Существующий Tony хорош как научный инструмент для monophonic pitch/note transcription, но его основной анализатор основан на pYIN/Vamp workflow. Это полезно, но не покрывает современные neural audio-to-MIDI методы, multi-engine comparison, violin technique-aware transcription, high-resolution violin transcription и AI-assisted correction workflow.

Сейчас современные инструменты существуют разрозненно:

| Project | Что даёт |
|---|---|
| Tony / pYIN | GUI, pitch/note layers, manual correction |
| Basic Pitch | instrument-agnostic audio-to-MIDI, polyphonic support, pitch bends |
| NeuralNote | native/plugin-oriented Basic Pitch implementation via RTNeural/ONNXRuntime |
| CREPE Notes | monophonic f0 → note segmentation/MIDI |
| MUSC Violin Transcription | high-resolution solo violin transcription |
| VioPTT | violin transcription + per-note technique labels |
| PESTO / PENN / FCPE | modern f0/pitch estimation engines |

Basic Pitch is a Python AMT library from Spotify that can generate MIDI with pitch bends and works best when analysing one instrument at a time; it also provides ONNX/CoreML/TFLite model serializations, with ONNX used by default on Windows. ([github.com](https://github.com/spotify/basic-pitch))  
NeuralNote is especially relevant as a native integration reference because it uses Basic Pitch internally with RTNeural and ONNXRuntime. ([github.com](https://github.com/DamRsn/NeuralNote))  
MUSC/MTG violin-transcription is a specialized violin transcriber that claims 5.8 ms time resolution and 10-cent frequency resolution for 44.1 kHz audio. ([github.com](https://github.com/MTG/violin-transcription/))  
VioPTT supports an end-to-end path from raw audio to MIDI plus per-note technique labels CSV. ([github.com](https://github.com/y10ab1/VioPTT))  
CREPE Notes is monophonic because its supported pitch trackers are monophonic; it also notes limitations around repeated notes at the same pitch and madmom licensing restrictions for commercial use. ([github.com](https://github.com/xavriley/crepe_notes))  
PESTO and PENN are pitch/f0 estimation tools, not complete full note-editing applications: PESTO outputs time/frequency/confidence CSV, and PENN provides pitch and periodicity estimation in PyTorch. ([github.com](https://github.com/SonyCSLParis/pesto))  
FCPE is also primarily a pitch extraction/f0 tool; its README notes that its MIDI extractor is quantized from f0 using non-neural methods. ([github.com](https://github.com/CNChTu/FCPE?utm_source=chatgpt.com))

The product problem:

```text id="j84frd"
There is no single open, Tony-like workstation where a user can load audio, run several modern transcription engines, compare their note results visually, manually correct mistakes, and export a cleaned MIDI result.
```

---

## 4. Target users

### Primary user

The first target user is **the project owner**, working locally on Windows, mainly with solo/stem material such as violin, cello, or AI-generated instrument parts that need MIDI reconstruction.

### Secondary future users

| User type | Need |
|---|---|
| Beginner music producer | Convert audio/stem into editable MIDI without deep theory |
| Sound designer | Compare algorithms and recover usable note data |
| Musician | Correct and refine transcription manually |
| Researcher / MIR user | Compare algorithm outputs transparently |
| Composer / arranger | Turn expressive violin/cello/audio phrases into MIDI sketches |
| Future open-source contributor | Add new backends through a documented backend contract |

### Product positioning

**ASSUMPTION:** The product should be positioned as:

```text id="ns9140"
A modern, local-first, multi-engine transcription workstation built on Tony.
```

It should not be positioned as:

```text id="rnvjza"
- a DAW
- a perfect automatic transcription tool
- a watermark removal tool
- a commercial Melodyne clone
- an AI music generator
```

---

## 5. User needs and pain points

### User needs

1. Load an audio file or stem.
2. Run a real transcription backend.
3. See pitch curve, notes, confidence, velocity, and technique labels where available.
4. Compare results from different algorithms.
5. Correct notes manually.
6. Re-analyze only a selected region.
7. Export clean MIDI/CSV.
8. Use the tool locally by default.
9. Have a simple UI for beginners and deeper settings for advanced users.
10. Eventually use AI Copilot for explanation, suggestions, and correction assistance.

### Pain points

| Pain point | Product response |
|---|---|
| One algorithm misses notes | Multi-engine comparison |
| Full-track transcription is too rough | Selected-region reanalysis |
| Tools output MIDI but lack correction UI | Use Tony’s editing workflow |
| Research scripts are hard for beginners | Backend Manager + unified UI |
| Python tools are fragmented | External backend adapters |
| Confidence/errors are hidden | Visual warnings and transparent status |
| User does not know which backend to use | Future AI Copilot + recommended workflow |
| Fear of fake UI / placebo functionality | Strict no-fake-results policy |

---

## 6. Product goals

### G1 — Preserve Tony as the correction/editor core

The existing Tony functionality must remain available and must not be broken.

### G2 — Add modern transcription engines

Add multiple analysis engines through a unified workflow:

```text id="c5anxs"
Load audio → choose engine → run analysis → view result → correct → export
```

### G3 — Make external backends feel native

Even if models run as external Python/ONNX processes, the user should experience them as normal analysis options inside the Tony UI.

### G4 — Support full-track and selected-region analysis

The user must be able to analyze the full file or only a selected fragment.

### G5 — Support comparison between engines

The program should allow running several engines sequentially and comparing results visually.

### G6 — Keep manual correction central

The product must not assume automatic transcription is perfect.

### G7 — Be local-first

All transcription backends should run locally by default. Cloud/API should only be used later for optional AI Copilot features.

### G8 — Prepare for future AI Copilot

AI should later help explain, compare, recommend, and suggest corrections, but not silently overwrite the user’s work.

---

## 7. Non-goals / out of scope

### MVP non-goals

The MVP must not attempt to do all future features at once.

Out of scope for MVP:

```text id="gsz8i8"
- full AI Copilot
- cloud audio upload
- commercial licensing packaging
- perfect UI redesign
- full support for every backend capability
- native/Vamp conversion for all backends
- macOS/Linux release
- full multilingual UI
- automatic professional musical arrangement correction
```

### Permanent non-goals

```text id="a47oye"
- No claim of 100% transcription accuracy.
- No claim of legal clearance of generated MIDI.
- No AI watermark removal or detector bypass claims.
- No hidden cloud upload.
- No fake completed states.
- No fake note results.
- No silent backend failure.
```

### Important product boundary

This is a **transcription and correction workstation**, not a tool for bypassing AI fingerprints or watermarks.

---

## 8. Core user workflow

### Standard workflow

```text id="5yakrt"
1. User opens audio file.
2. User chooses analysis engine.
3. User configures basic settings or accepts recommended defaults.
4. User clicks Run Analysis.
5. Program shows progress and backend status.
6. Program imports real backend output into unified result format.
7. Program displays notes, pitch curve, confidence, warnings.
8. User corrects notes manually.
9. User exports MIDI/CSV/project file.
```

### Compare workflow

```text id="py5hfy"
1. User selects “Compare Engines”.
2. User chooses compatible engines.
3. Program runs them sequentially, not all at once.
4. User sees each result as a separate layer/variant.
5. Disagreements are highlighted.
6. User previews variants.
7. User chooses a full result or copies selected notes into the main corrected layer.
```

### Selected-region workflow

```text id="rrvwa0"
1. User selects a time region.
2. User chooses backend or compare mode.
3. Program analyzes only selected region.
4. Result appears as a preview layer.
5. User accepts, rejects, or selectively applies notes.
6. Existing notes outside selected region remain untouched.
```

---

## 9. MVP scope

The user wants “everything important to work excellently,” but a professional MVP must still be staged to avoid building an unstable monster application.

### MVP definition

**MVP is not the final full application.**  
MVP means the first real vertical slice where the architecture proves itself:

```text id="jbsciw"
Tony fork can run at least one real external backend,
import the real output,
display it correctly,
allow manual correction,
and export the corrected result.
```

### MVP must include

| Feature | MVP status |
|---|---|
| Build original Tony fork | Required |
| Existing pYIN workflow preserved | Required |
| Unified Result Format | Required |
| Backend Manager foundation | Required |
| Mock/dev backend for testing architecture | Required, dev-only |
| One real external backend | Required |
| Display imported notes | Required |
| Manual correction using Tony workflow | Required |
| MIDI export | Required |
| Basic error handling | Required |
| No fake production results | Required |

### Recommended first real backend

**ASSUMPTION:** The first real backend should be **Basic Pitch / NeuralNote path**, because Basic Pitch has a CLI, can output MIDI and note-events CSV, supports Windows ONNX runtime by default, and is more general than violin-only backends. ([github.com](https://github.com/spotify/basic-pitch))

### MVP should not include yet

| Feature | Reason |
|---|---|
| Full compare mode | Needs multiple stable adapters first |
| Full selected-region replacement logic | Needs safe preview/apply system |
| VioPTT technique layer | Needs new label visualization layer |
| MUSC high-resolution pitch bends | Needs specialized import/visualization |
| AI Copilot | Future scope |
| Multilingual UI | Later after strings are externalized |
| Full UI redesign | Risky before architecture works |

---

## 10. Post-MVP scope

### Post-MVP Phase 1

```text id="7r0qho"
- CREPE Notes adapter
- selected-region analysis preview
- Basic compare mode between pYIN / Basic Pitch / CREPE Notes
```

### Post-MVP Phase 2

```text id="u2altc"
- MUSC Violin adapter
- high-resolution pitch deviation support
- pitch bend visualization/export improvements
```

### Post-MVP Phase 3

```text id="znushu"
- VioPTT adapter
- technique label layer
- technique-annotated MIDI/CSV import
```

### Post-MVP Phase 4

```text id="1sqiue"
- PESTO / PENN / FCPE f0 adapters
- shared f0-to-note segmentation layer
- f0 confidence visualization
```

### Post-MVP Phase 5

```text id="s2p1in"
- full compare/merge workflow
- voting/consensus display
- disagreement heatmap
- per-note backend provenance
```

### Post-MVP Phase 6

```text id="jbe237"
- AI Copilot via API and/or local models
- backend recommendation
- suspicious-region explanation
- playable-range / instrument feasibility checks
- suggested corrections with user confirmation
```

### Post-MVP Phase 7

```text id="ta9hhj"
- UI modernization
- simple/advanced modes
- theme support
- multilingual UI
- packaging/installer
```

---

## 11. Analysis engine capabilities

### Engine table

| Engine | Product role | Integration type | Expected output | Notes |
|---|---|---|---|---|
| pYIN Vamp | Existing classic method | Existing Vamp plugin | pitch track, note track | Must remain working |
| Basic Pitch | General audio-to-MIDI | External first; possible native ONNX later | MIDI, pitch bends, note events | Good first backend |
| NeuralNote | Native Basic Pitch reference | Native/reference backend | MIDI/audio-to-MIDI | Useful for future C++/ONNX integration |
| CREPE Notes | Monophonic f0 → notes | External first; possible Vamp/native later | MIDI/note segmentation | Monophonic only |
| MUSC Violin | High-res solo violin | External backend | MIDI + pitch deviations | Violin-specialized |
| VioPTT | Violin + technique | External backend | MIDI + per-note technique CSV | Technique layer needed |
| PESTO | f0 estimator | External f0 backend | time/frequency/confidence | Needs segmentation |
| PENN | f0/periodicity estimator | External f0 backend | pitch/periodicity | Needs segmentation |
| FCPE | f0 estimator / MIDI quantization | External f0 backend | f0 and simple MIDI | Treat cautiously |

### Product requirement

Each backend must expose:

```text id="b4axfa"
- installed / missing status
- compatible input types
- basic settings
- run button
- progress state
- output import
- error state
- logs/warnings
```

### Important clarification

The user requested that each backend should do “everything the original project can do.”  
**ASSUMPTION / PRODUCT INTERPRETATION:** In the final product, each backend should expose all capabilities that are relevant to transcription, correction, comparison, and export. Research-only training scripts, dataset preparation tools, model training workflows, and internal experiments do not need to be fully exposed in the Tony UI unless they serve the product workflow.

---

## 12. Backend comparison workflow

### Goal

Allow users to compare how different engines detect the same musical material.

### Required behavior

```text id="peifov"
- User can choose several compatible engines.
- Program runs them sequentially to avoid overloading CPU/RAM.
- Each backend result is stored as a separate result layer.
- Differences are visually highlighted.
- User can audition or inspect each result.
- User can select best full result or copy individual notes.
```

### Visual concept

```text id="0vv2a5"
Main corrected layer
├── Basic Pitch result
├── CREPE Notes result
├── pYIN result
├── MUSC result
└── VioPTT result
```

### Comparison modes

| Mode | Description | Priority |
|---|---|---|
| Whole-result comparison | Choose one backend’s full result | Early |
| Per-note comparison | Copy individual notes from backend result | Medium |
| Consensus/voting | Show notes detected by multiple engines | Later |
| Conflict heatmap | Highlight regions with disagreement | Later |
| AI explanation | Explain why region is suspicious | Future |

### Acceptance expectation

The UI must remain understandable for beginners. Advanced comparison controls should be collapsible.

---

## 13. Selected-region analysis workflow

### Goal

Allow micro-reconstruction and correction of difficult parts without re-running the entire track.

### Required behavior

```text id="e3ohdj"
- User selects time region.
- User chooses backend or compare mode.
- Program exports temporary audio slice internally.
- Backend analyzes only that region.
- Result appears as preview.
- User can apply, reject, or selectively copy notes.
```

### Safe apply rules

| Action | Behavior |
|---|---|
| Apply full preview | Replaces notes only inside selected region |
| Copy selected notes | Adds/replaces only selected notes |
| Reject preview | Leaves corrected layer unchanged |
| Keep as layer | Stores preview as comparison layer |

### Product principle

Selected-region analysis must be non-destructive by default.

---

## 14. Manual correction requirements at product level

Manual correction remains central. The product should support at minimum:

```text id="wu2zv4"
- add note
- delete note
- move note
- resize note
- change pitch
- split note
- merge notes
- adjust velocity if supported
- apply selected backend note to corrected layer
- undo/redo
```

### Future correction features

```text id="gjg9e6"
- edit pitch bend curve
- snap note to pitch curve
- smooth velocities
- clean very short ghost notes
- Kontakt-ready MIDI cleanup
- instrument range warning
```

### Product principle

Automatic analysis should produce suggestions. The user remains in control.

---

## 15. Result visualization requirements at product level

The product should show real data from the selected backend.

### Required visualization layers

| Layer | Required? | Notes |
|---|---:|---|
| Waveform | Yes | Existing Tony/SV capability |
| Pitch curve | Yes | Where backend provides f0 |
| Note blocks / piano-roll-like display | Yes | Core workflow |
| Confidence | Yes | Color/opacity or separate indicator |
| Velocity | Yes | If backend provides it |
| Pitch bends / deviations | Later | Important for violin/cello |
| Technique labels | Later | Required for VioPTT |
| Warnings/errors | Yes | Must be visible |

### Confidence display

**ASSUMPTION:** Use a simple beginner-friendly display first:

```text id="banfuj"
- normal notes = standard appearance
- low-confidence notes = warning outline / marker
- selected note panel = exact confidence value
```

This avoids visually overwhelming the user.

### Technique labels

Technique labels should appear as optional annotations above notes, not always visible by default.

Example:

```text id="gfv32n"
C4 ───── normal
D4 ───── spiccato
G4 ───── pizzicato
```

---

## 16. Export requirements at product level

### Required exports

| Format | Priority |
|---|---|
| MIDI | Required |
| CSV note events | Required |
| Project file | Required after early MVP |
| JSON unified result | Required for debugging/comparison |
| MusicXML | Future |
| WAV preview/sonified MIDI | Optional |

### MIDI export should preserve, where available:

```text id="ay2bdf"
- note start
- note end
- MIDI pitch
- velocity
- pitch bends
- tempo metadata if available
```

### Technique labels

**ASSUMPTION:** Technique labels should be exported in CSV/JSON first. MIDI technique representation can be added later using markers/text/meta events only if technically safe and compatible.

### Project file

The application should support saving analysis state:

```text id="edni4k"
- loaded audio path/reference
- corrected note layer
- backend result layers
- backend settings
- timestamps
- warnings/logs reference
```

---

## 17. AI Copilot future concept

AI Copilot is **future scope**, not MVP.

### Product role

AI Copilot should act as:

```text id="hlbub8"
assistant / explainer / workflow coordinator / correction suggester
```

Not as the primary audio-to-MIDI engine.

### Future features

```text id="btpqhu"
- recommend best backend for selected audio
- explain disagreements between backends
- highlight suspicious regions
- suggest corrections
- check playable range for violin/cello/piano/guitar/bass/voice
- help clean MIDI for a target instrument library
- generate user-readable reports
```

### AI correction principle

AI must:

```text id="qvcvvr"
1. explain the suggestion
2. show affected notes/region
3. wait for user confirmation
4. apply only confirmed changes
```

### Cloud/API policy

**ASSUMPTION:** AI Copilot may support both cloud API and local models in the future, but:

```text id="gh3ixu"
- local processing remains default
- cloud/API must be opt-in
- user must know what data is sent
- API keys must not be stored in source code
```

Microsoft secure development guidance recommends never storing credentials or secrets in the source code repository. ([learn.microsoft.com](https://learn.microsoft.com/en-us/azure/well-architected/security/secure-development-lifecycle))

### Explicit non-goal

AI Copilot must not include or advertise watermark removal, detector bypass, or legal-clearing functions.

---

## 18. UX principles

### UX direction

The interface should be:

```text id="kdj9di"
modern
minimal
clear
local-first
beginner-friendly
advanced-capable
honest about backend state
```

### UI modes

The product should support two modes:

| Mode | Description |
|---|---|
| Simple Mode | Load → choose goal → run → correct → export |
| Advanced Mode | Backend settings, comparison layers, logs, confidence thresholds, f0 data |

### Visual modernization

The UI may be modernized, but not before core architecture is stable.

Desired future UI improvements:

```text id="1elvjg"
- dark/system theme
- cleaner backend selection
- collapsible advanced panels
- clear progress states
- configurable spectrum/pitch display colors
- professional minimal design
```

### Backend Manager screen

**ASSUMPTION:** Add a Backend Manager screen.

It should show:

```text id="ngzecb"
- backend name
- installed / missing / broken
- version
- path
- runtime type: Python / ONNX / Vamp / native
- model/checkpoint status
- test button
- install/help instructions
```

### Wizard

**ASSUMPTION:** Add an optional beginner workflow:

```text id="yo6d3x"
1. Load audio
2. Choose goal
3. Choose engine
4. Run analysis
5. Review/correct
6. Export
```

Advanced users can bypass the wizard.

---

## 19. Data/privacy principles

### Local-first

The program should be local-first by default:

```text id="r8nzwp"
- local audio analysis
- local temp files
- local project files
- no cloud upload unless user explicitly enables AI/API function
```

### Internet access

**ASSUMPTION:** Only AI Copilot should need internet access, and only when cloud/API mode is enabled.

### Cloud warning

Before enabling cloud/API AI:

```text id="0y5y70"
- show clear notice
- explain what may be sent
- allow “do not show again”
- allow disabling cloud features entirely
```

### Logs

**ASSUMPTION:** Logs should exist but be privacy-conscious.

Recommended behavior:

```text id="sme4tc"
- logs enabled locally
- no automatic upload
- user can clear logs
- logs avoid storing full audio content
- logs may store paths unless privacy mode disables that
```

---

## 20. Reliability and error-handling expectations

The application must never silently pretend success.

### Required states

```text id="xuh2b1"
Idle
Ready
Analyzing
Completed
Completed with warnings
Failed
Cancelled
Backend missing
Model/checkpoint missing
Unsupported input
Low-confidence result
```

### Failure examples

| Failure | Expected UI behavior |
|---|---|
| Backend not installed | Show “Not installed” + setup/help |
| Python dependency missing | Show readable error |
| Model checkpoint missing | Ask user to locate checkpoint |
| Backend exits non-zero | Show failed state, log available |
| Output file missing | Do not import fake result |
| Invalid output schema | Show parser error |
| Timeout | Cancel process and show error |
| User cancels | Stop process safely |

### No-fake policy

Production UI must not show:

```text id="d6tf1l"
- fake notes
- fake progress
- fake completed state
- fake confidence
- fake backend availability
```

A mock backend may exist only as a development/testing tool and must be explicitly labelled as such.

---

## 21. Success metrics

### MVP success

MVP is successful when:

```text id="kgiw26"
1. Original Tony builds and runs.
2. Existing pYIN workflow still works.
3. At least one real external backend runs from the UI.
4. Real backend output is imported and displayed.
5. User can manually correct notes.
6. User can export MIDI.
7. Backend errors are visible and non-crashing.
8. No production fake states exist.
```

### Full product success

Full product is successful when:

```text id="p60mbs"
1. Multiple backends are integrated.
2. Compare mode works clearly.
3. Selected-region analysis works non-destructively.
4. VioPTT technique labels can be displayed.
5. MUSC violin output can be meaningfully visualized.
6. f0 engines can feed a segmentation layer.
7. Project files save and reload work.
8. UI is understandable to a beginner but powerful for advanced users.
9. Windows installer works.
10. License register is complete.
```

### Quality test material

**ASSUMPTION:** Test set should include:

```text id="s23axh"
- clean solo violin WAV
- clean solo cello WAV
- real recorded violin/cello phrase
- Suno-generated violin/cello stem
- noisy separated stem
- short 1–3 second difficult phrase
- repeated same-pitch notes
- vibrato/glissando phrase
- polyphonic single-instrument audio for Basic Pitch
```

---

## 22. Risks and constraints

### Technical risks

| Risk | Severity | Mitigation |
|---|---:|---|
| Tony build complexity | High | First task: build unchanged Tony |
| Python backend packaging | High | Start with external backend paths |
| Heavy dependencies | High | Backend Manager + optional installs |
| Different output formats | High | Unified Result Format |
| Compare UI complexity | Medium | Layer-based progressive design |
| Selected-region replacement bugs | Medium | Preview first, non-destructive apply |
| VioPTT/MUSC research-code fragility | High | Integrate later after stable runner |
| CPU performance | Medium | Sequential execution, cancellation |
| AMD integrated GPU support uncertain | Medium | CPU-first; GPU optional later |
| Multilingual UI complexity | Medium | Externalize strings later |

### Product risks

| Risk | Mitigation |
|---|---|
| “Everything in MVP” makes project impossible | Define vertical-slice MVP |
| UI becomes too complex | Simple/Advanced modes |
| User trusts wrong result | Confidence/warnings/comparison |
| Backends disagree heavily | Show disagreement, do not auto-merge blindly |
| Legal/commercial uncertainty | License register from the start |

### License risks

A license register is required from the start. This is especially important because:

```text id="uhm1xg"
- Tony/pYIN ecosystem has GPL-family licensing.
- CREPE Notes is GPL-3.0 and notes madmom commercial-use restrictions.
- MUSC/violin-transcription needs separate license review before redistribution.
- Basic Pitch and VioPTT are Apache-2.0.
```

CREPE Notes explicitly notes madmom-related commercial-use restrictions, so commercial use must be reviewed carefully. ([github.com](https://github.com/xavriley/crepe_notes))

---

## 23. Assumptions

These are decisions made from your “сам определи” answers.

| ID | Assumption |
|---|---|
| A1 | First release target is Windows only. |
| A2 | CPU-first is required because target machine uses AMD Ryzen 7 PRO 7840U with integrated Radeon 780M. |
| A3 | GPU acceleration is optional and future-facing. |
| A4 | MVP must be a stable vertical slice, not the full final product. |
| A5 | First real backend should be Basic Pitch/NeuralNote path. |
| A6 | Compare mode is post-MVP but high-priority. |
| A7 | Selected-region analysis is post-MVP but high-priority. |
| A8 | Backend Manager is required. |
| A9 | Mock backend is allowed only for development/testing and must never pretend to be real analysis. |
| A10 | Full multilingual UI is future scope. |
| A11 | AI Copilot is future scope and opt-in. |
| A12 | Local-first is default. |
| A13 | Cloud/API functions require explicit user confirmation. |
| A14 | Project should later be open-source on GitHub if licensing allows. |
| A15 | Commercialization is not a near-term requirement. |
| A16 | The product should avoid any watermark/fingerprint bypass positioning. |

---

## 24. Open questions for the user

These do not block the PRD, but must be answered before SRS/TDD.

### Product

1. What should the final product name be?
2. Should the product visually keep the Tony name/branding or become a clearly renamed fork?
3. Should the app focus first on monophonic solo instruments or general audio-to-MIDI?

### MVP

4. Is Basic Pitch acceptable as the first real backend?
5. Should MVP include CSV export or only MIDI export?
6. Should project save/load be required in MVP or post-MVP?

### Backend installation

7. Should the first version require the user to install Python backends manually?
8. Should the app later ship bundled backend environments?
9. Should backend models/checkpoints be downloaded manually or through the app?

### UI

10. Should Simple Mode be the default?
11. Should Advanced Mode be hidden behind a toggle?
12. Should UI modernization happen before or after first backend integration?

### AI

13. Which OpenAI/local AI providers should be supported later?
14. Should AI ever receive audio, or only symbolic note data?
15. Should AI Copilot be allowed to call backend functions automatically after confirmation?

### Licensing

16. Will the first GitHub release include all backend code or only adapters/instructions?
17. Is commercial redistribution still a real future goal?
18. Should AGPL/GPL backends be optional external dependencies only?

---

## 25. What should move to SRS

SRS should define exact, testable requirements.

Move these to SRS:

```text id="fxs7qf"
- supported input formats
- supported export formats
- exact UI states
- exact backend status messages
- exact error handling
- exact manual editing operations
- exact selected-region behavior
- exact compare mode behavior
- exact project file behavior
- exact language/i18n requirements
- exact installer requirements
```

Example SRS requirement:

```text id="ry73f8"
REQ-BACKEND-001:
The application shall display “Backend not installed” when a configured backend executable path is missing or invalid.
```

---

## 26. What should move to TDD

TDD should define how the system is built.

Move these to TDD:

```text id="r6lc0b"
- AnalysisEngineManager architecture
- IAnalysisEngine interface
- ExternalProcessRunner
- UnifiedResult schema classes
- backend adapter design
- temp file strategy
- process cancellation
- output parsing
- Tony layer importer
- confidence visualization implementation
- note merge/copy implementation
- project file serialization
- AI Copilot function-calling architecture
```

Example TDD item:

```text id="o9g4l4"
ExternalProcessRunner must support timeout, cancellation, stdout/stderr capture, exit-code handling, temp workspace cleanup, and structured error reporting.
```

---

## 27. What should move to Backend Contract

Backend Contract should define how every backend communicates with the app.

Move these to Backend Contract:

```text id="9fjl37"
- backend input fields
- backend settings format
- output JSON schema
- note event schema
- pitch curve schema
- technique label schema
- confidence schema
- warnings/errors schema
- logs schema
- version metadata
- backend capability declaration
```

Example backend capability declaration:

```json id="1c182t"
{
  "engine_id": "basic_pitch",
  "supports_full_file": true,
  "supports_region": true,
  "outputs_notes": true,
  "outputs_pitch_curve": false,
  "outputs_pitch_bends": true,
  "outputs_technique_labels": false,
  "requires_python": true,
  "requires_checkpoint": false
}
```

---

## 28. First implementation milestones

### Milestone 0 — Repository and documentation setup

```text id="ya94mm"
- Fork Tony
- Add docs folder
- Add PRD
- Add SRS draft
- Add TDD draft
- Add Backend Contract draft
- Add License Register
- Add AGENTS.md later
```

### Milestone 1 — Build original Tony unchanged

```text id="wespnz"
Goal:
Build original Tony fork without functional changes.

Success:
Tony launches and existing pYIN workflow still works.
```

### Milestone 2 — Codebase mapping

```text id="81xf3g"
Goal:
Identify where Tony handles:
- analysis
- pYIN transform
- pitch/note layers
- import/export
- main UI/menu actions
```

### Milestone 3 — Unified Result Format

```text id="fgob0g"
Goal:
Create internal data model for:
- notes
- pitch curve
- confidence
- velocity
- technique labels
- warnings
```

### Milestone 4 — Dev-only mock backend

```text id="imn849"
Goal:
Load a sample UnifiedResult JSON and display notes in Tony.

Important:
Must be clearly marked development-only.
```

### Milestone 5 — ExternalProcessRunner

```text id="8uf9th"
Goal:
Safely run external command-line backend and capture output/errors.
```

### Milestone 6 — Basic Pitch backend

```text id="bgyodl"
Goal:
Run Basic Pitch from UI, import MIDI/CSV output, display notes.
```

### Milestone 7 — Export corrected result

```text id="fo7q5h"
Goal:
Export corrected note layer as MIDI/CSV.
```

### Milestone 8 — CREPE Notes backend

```text id="xattxc"
Goal:
Add monophonic note segmentation backend.
```

### Milestone 9 — Selected-region preview

```text id="q7t061"
Goal:
Analyze only selected region and preview result before apply.
```

### Milestone 10 — Compare mode v1

```text id="tt1dwr"
Goal:
Run pYIN / Basic Pitch / CREPE Notes sequentially and compare layers.
```

### Milestone 11 — MUSC backend

```text id="e6i5f4"
Goal:
Add violin-specialized transcription backend.
```

### Milestone 12 — VioPTT backend

```text id="sjsbk8"
Goal:
Add technique-aware violin transcription and label visualization.
```

### Milestone 13 — PESTO/PENN/FCPE f0 adapters

```text id="z5065z"
Goal:
Import f0 curves and feed shared note segmentation.
```

### Milestone 14 — AI Copilot concept implementation

```text id="dp73nt"
Goal:
Add optional AI panel that explains, recommends, and suggests actions.
```

### Milestone 15 — UI modernization

```text id="m1g0gx"
Goal:
Modern minimal interface with Simple/Advanced modes and dark/system theme.
```

---

# PRD summary for non-technical reader

Ты хочешь сделать **улучшенный Tony**: программу, куда можно загрузить аудио, выбрать один из современных алгоритмов анализа, получить настоящие MIDI-ноты, увидеть их на экране, сравнить результаты разных алгоритмов, исправить ошибки руками и экспортировать чистый MIDI.

Самое важное:

```text id="rce7dp"
Tony остаётся базой и редактором.
Новые модели добавляются как внешние backends.
Все результаты приводятся к одному формату.
Никаких фейковых кнопок и фейковых нот.
Сначала local-first Windows версия.
AI Copilot — позже.
Сначала стабильный MVP: один реальный backend → отображение → коррекция → export.
Потом добавляются compare mode, selected-region analysis, MUSC, VioPTT, PESTO/PENN/FCPE и AI.
```

Главная формула проекта:

```text id="fz7s43"
Audio → choose backend → real analysis → visual notes/pitch/confidence → compare/correct → export MIDI/CSV
```
