---
name: integration-gate
description: Perform the final adversarial Serori gate over artifacts, evidence, ownership, context legality, tests, configuration, and source scope.
---

# Integration gate

Run `../../scripts/serori-review` after deterministic checks. Critical findings fail. In strict mode,
unknown/stale evidence, missing ownership release, illegal context calls, unresolved build inputs,
failed tests, and forbidden writes fail. LLM prose may explain findings but cannot waive or downgrade
deterministic results. Emit a gate report with pass, conditional, or fail and explicit blockers.

