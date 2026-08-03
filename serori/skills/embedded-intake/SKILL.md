---
name: embedded-intake
description: Establish a bounded ChibiOS firmware task context, target identity, revision, constraints, protocols, and acceptance criteria before analysis or implementation.
---

# Embedded intake

Read the task, repository, `libs/ChibiOS`, and user-provided constraints. Produce `project_context.yaml`.
Record board/MCU/toolchain and ChibiOS revision as explicit, detected, or unknown; never promote filenames to facts.
Capture memory/latency budgets, execution contexts, protocols, out-of-scope items, and definition of done.
Stop in strict mode when target identity, ChibiOS identity, or acceptance criteria are missing.
Use `../../scripts/serori-intake`; do not recommend architecture or edit source.

