<!--
Sync Impact Report
- Version change: 1.0.0 -> 1.1.0
- Modified principles:
  - I. Deterministic Simulation First -> I. Preserve Gameplay Baseline While Hardening Determinism
  - II. Replay Artifacts as Canonical Debug Surface -> II. Replay Artifacts and First-Divergence Diagnostics Are Canonical
  - III. Tests and Deterministic Verification Gates (NON-NEGOTIABLE) -> III. Verification Gates and Reproducible Evidence (NON-NEGOTIABLE)
  - IV. Stable Identity and Ordering Contracts -> IV. Stable Identity, Ordering, and Logical-Frame Contracts
  - V. Schema Discipline and Backward Compatibility -> V. Scope Discipline, Toolchain Reality, and License/Attribution Integrity
- Added sections:
  - Operational Constraints and Baseline Assumptions
  - Delivery Workflow and Phase Gates
- Removed sections:
  - None
- Templates requiring updates:
  - ✅ .specify/templates/plan-template.md (already contains Constitution Check gate placeholder)
  - ✅ .specify/templates/spec-template.md (already supports mandatory measurable outcomes and requirements)
  - ✅ .specify/templates/tasks-template.md (already supports test and validation tasks)
  - ✅ .specify/extensions/git/commands/*.md (no constitution-specific wording conflict found)
- Deferred TODOs:
  - TODO(RATIFICATION_DATE): Original adoption date unknown; set when first formally approved.
- Traceability Appendix:
  - Principle I: README.md (project scope and baseline), sopwith3/docs/roadmap.md (baseline gameplay and phased determinism focus)
  - Principle II: sopwith3/docs/phase2-replay-model-decision.md (artifact schema, first-divergence contract), sopwith3/docs/roadmap.md (golden-state outputs)
  - Principle III: sopwith3/docs/phase0-deterministic-replay-memo.md (repeatability criteria), sopwith3/docs/roadmap.md (phase exit criteria with verification evidence)
  - Principle IV: sopwith3/docs/phase2-replay-model-decision.md (entity identity, ordering, logical-frame cadence)
  - Principle V: README.md (licensing and attribution obligations), Toolchain-Readme.md (Windows/MSYS2/SDL maintained build path), sopwith3/docs/roadmap.md (in/out-of-scope boundaries)
  - Operational Constraints section: Toolchain-Readme.md, sopwith3/STYLEGUIDE.md, sopwith3/docs/findings.md, sopwith3/docs/roadmap.md
  - Delivery Workflow and Phase Gates section: .specify/templates/plan-template.md, .specify/templates/spec-template.md, .specify/templates/tasks-template.md, sopwith3/docs/phase1-build-run-smoke.md, sopwith3/docs/roadmap.md
-->
# Sopwith 3.5 Constitution

## Core Principles

### I. Preserve Gameplay Baseline While Hardening Determinism
Changes MUST preserve established gameplay behavior (single-player baseline first) while improving
build reliability, determinism, and maintainability. Large gameplay rewrites MUST NOT be introduced
before deterministic replay baselines are stable. Rationale: this repository is a C++ foundation
effort, not a gameplay redesign.

### II. Replay Artifacts and First-Divergence Diagnostics Are Canonical
The project MUST treat replay artifacts, golden-state outputs, and first-divergence reports as the
primary regression and parity-debugging surface. Deterministic validation outputs MUST be
machine-diffable and include explicit frame/row/field context. Replay workflow changes MUST update
related usage docs in the same change. Rationale: transparent artifacts enable fast diagnosis for
maintainers and AI-assisted workflows.

### III. Verification Gates and Reproducible Evidence (NON-NEGOTIABLE)
Changes affecting simulation, replay, timing, RNG, or serialization/comparison logic MUST include
reproducible evidence before merge. At minimum, validation MUST prove repeatability for same input
and seed across repeated runs and MUST verify first-divergence behavior on mismatch. If automation
is temporarily unavailable, the change MUST include a time-bounded exception and follow-up task.
Rationale: determinism and replay confidence require enforceable evidence, not assumptions.

### IV. Stable Identity, Ordering, and Logical-Frame Contracts
Replay-producing code MUST define and preserve stable entity identities, deterministic row ordering,
and logical-frame sampling semantics independent of render cadence or wall-clock timing. Any change to
identity, ordering, or cadence rules MUST be treated as schema-impacting and documented as such.
Rationale: comparability depends on strict temporal and structural contracts.

### V. Scope Discipline, Toolchain Reality, and License/Attribution Integrity
Repository work MUST stay within declared scope: Windows/MSYS2 SDL baseline reliability, deterministic
replay/testing infrastructure, and diagnostics. Out-of-scope items (for this repo), including the
future production C# implementation, MUST remain separate. Contributions MUST preserve required
license texts and upstream attribution notices. Rationale: explicit scope and legal integrity protect
project focus and redistribution compliance.

## Operational Constraints and Baseline Assumptions

- Current maintained build path is Windows 11 with MSYS2 MinGW64 and SDL1.2; changes to build
  scripts/tooling MUST keep this path reliable and documented.
- Deterministic capture cadence MUST align to logical simulation frames, not render cadence.
- Artifact formats intended for diagnosis SHOULD remain text-diff-friendly by default; if alternate
  encodings are added, equivalent diagnosability MUST be preserved.
- Network/multiplayer code may be preserved, but deterministic baseline acceptance targets are
  single-player first unless explicitly expanded.
- Style updates in legacy C/C++ sources SHOULD follow observed local conventions unless a planned
  formatter/policy migration is approved.

## Delivery Workflow and Phase Gates

Every feature spec and implementation plan MUST include a Constitution Check that references these
principles. Task plans MUST include deterministic verification tasks when simulation behavior changes.
Pull requests MUST document impacted schema/version behavior, scope impact, and verification evidence.
Roadmap phase exits MUST be backed by checked-in evidence docs (for example phase reports, model
decisions, and reproducible run/build notes). Reviews MUST block merges for unresolved constitution
violations unless an explicit, time-bounded exception is approved.

## Governance

This constitution supersedes conflicting local habits and informal workflow notes for Sopwith 3.5.
Amendments require: (1) a documented proposal, (2) explicit reviewer approval, (3) updates to affected
templates/docs in the same change, and (4) an updated Sync Impact Report at the top of this file.
Versioning policy: MAJOR for incompatible governance or principle redefinitions/removals; MINOR for new
principles/sections or materially expanded obligations; PATCH for clarifications and editorial changes.
Compliance review is mandatory for each plan and pull request, and violations MUST be tracked to closure.

**Version**: 1.1.0 | **Ratified**: TODO(2026-04-21): Original adoption date unknown. | **Last Amended**: 2026-04-21
