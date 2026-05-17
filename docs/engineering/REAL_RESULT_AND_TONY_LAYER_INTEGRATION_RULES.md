# Real Result and Tony Layer Integration Rules

Status: mandatory engineering rules  
Applies before: TonyLayerImporter, UI integration, MainWindow/Analyser integration, real backend workflows, selected-region replacement, save/load/export work

## 1. Purpose

This document prevents future tasks from presenting backend scaffolding, mock data, custom drawing, or partial plumbing as finished Tony functionality.

Backend infrastructure is useful only when it produces real, inspectable behavior in Tony. A feature is not complete until the requested proof chain has been demonstrated.

## 2. Absolute Prohibitions

- No fake UI: UI must not show notes, confidence, progress, layer names, or completed states unless backed by real data and real state.
- No fake Ready, Installed, or Completed states: path checks, parsed manifests, or configured settings are not runtime readiness.
- No fake `result.json`: code must not fabricate successful backend output to make a flow pass.
- No fake notes: fallback notes, sample notes, or placeholder note events are not production output.
- No custom overlay pretending to be a Tony layer: visual previews must not be called imported/editable layers unless they are real Tony/Sonic Visualiser models/layers.
- No feature-complete claims without proof: compile success, unit tests, or parsed JSON alone are not proof that a Tony workflow works.
- No silent pYIN replacement: existing pYIN behavior remains the baseline unless a task explicitly changes it with regression proof.

## 3. Required Real Result Chain

A real backend feature must prove this chain when the task scope reaches that stage:

```text
real input audio
-> real configured backend or existing Tony/pYIN path
-> real backend execution or explicitly dev/test-only mock
-> real output file(s)
-> valid UnifiedResult
-> real Tony/Sonic Visualiser model/layer where applicable
-> visible layer in Tony where applicable
-> editable through Tony mechanisms where applicable
-> save/load proof where relevant
-> export proof where relevant
```

Stopping earlier is allowed only when the task explicitly says compile-only, parser-only, discovery-only, or test-only. The final report must then say exactly where the chain stops.

## 4. Real Tony Layer Requirement

When a result is claimed as displayed or imported, it must be represented through Tony/Sonic Visualiser layer/model ownership, not a detached drawing or mock object.

Required proof before claiming layer integration:

- source inspection identifies the existing Tony/Sonic Visualiser layer/model type;
- the importer uses existing ownership paths such as `Document`/model/layer mechanisms;
- the layer is visible through Tony's normal view/pane behavior;
- editability is verified only if the layer uses an editable Tony model/layer;
- undo/redo behavior is documented if edits are supported.

## 5. Dev/Mock Backend Rule

Dev/mock backend output is allowed only for engineering proof. It must be:

- explicitly labeled dev/test-only;
- excluded from production success claims;
- routed through the same parser/validator as real backend output;
- never shown as a real installed backend;
- never used as evidence that Basic Pitch, CREPE Notes, MUSC, VioPTT, PESTO, PENN, FCPE, or pYIN integration works.

## 6. Feature Completion Evidence

A feature-complete claim must name the evidence:

- command or UI action used;
- input audio file or fixture;
- backend or pYIN path used;
- result file path if applicable;
- UnifiedResult validation outcome;
- Tony layer/model class used;
- manual edit proof if editable;
- save/load proof if the feature claims persistence;
- export proof if the feature claims export;
- screenshots/logs/test output where useful.

If any evidence is missing, the feature is partial.

## 7. Stop Conditions

Stop and report instead of continuing if:

- the output is not a real backend output or documented dev-only fixture;
- the layer type is unknown;
- the layer is not owned by Tony/Sonic Visualiser infrastructure;
- the UI state would imply readiness/completion without proof;
- save/load/export behavior is assumed but not inspected;
- Basic Pitch or another backend output shape is unverified.

## 8. Recommended Next Proof Work

Before implementing TonyLayerImporter or UI integration, complete:

- `CODEX-087A` Tony source architecture audit;
- `CODEX-087B` Tony layer/edit/save/export audit;
- `CODEX-087C` backend output truth table verification;
- `CODEX-087D` real-result acceptance rules verification.
