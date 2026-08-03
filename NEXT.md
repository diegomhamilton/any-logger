# Next steps and human review

The Serori package is scaffolded, but it still requires human review before source changes or release
decisions. There is no formal GitHub review yet because the branch has not been successfully pushed.

## Human review checklist

- Confirm the canonical ChibiOS 21.11 patch/tag/commit.
- Review the scope and activation boundaries of all 11 skills.
- Approve the artifact schemas and `draft` → `reviewed` → `accepted` transitions.
- Validate PLAY Embedded guidance against the pinned ChibiOS checkout.
- Review pattern-card invariants for DMA, ISR context, pointers, buffers, scheduling, and lifecycle.
- Define ownership rules for `serori-check-ownership`.
- Define permitted host, sanitizer, target, HIL, and soak profiles for `serori-test`.
- Approve the conditions under which `implementation-worker` may modify source.
- Review ChatGPT/Codex and Claude Code adapter equivalence.

## Current gates

`implementation-worker` requires accepted intake, repository map, evidence, architecture,
concurrency/ownership review, and verification artifacts.

`integration-gate` must fail on critical findings, stale or missing evidence, unresolved ownership,
illegal context calls, failed tests, unresolved build inputs, or forbidden writes.

The current ownership checker and test runner intentionally return `BLOCKED` until explicit rules and
test profiles are supplied. They must not claim that unconfigured analysis or tests prove firmware
safety.

## Recommended bounded first change

Use `any-logger` as the first fixture:

1. Intake and map the STM32F303RE/Nucleo project.
2. Pin and record the ChibiOS 21.11 revision.
3. Validate queue, ISR, DMA, CAN, and lifecycle evidence.
4. Produce an architecture record for one ownership-safe enqueue path.
5. Produce tests for payload lifetime, queue-full behavior, ISR handoff, and stop/restart.
6. Request human approval before implementation.
7. Apply the smallest patch and run the integration gate.

## Outstanding package correction

The documentation now distinguishes the ChibiOS 21.11 release line from the HAL 9.1.0 component
version. That correction is currently uncommitted and should be included in the next package commit.

