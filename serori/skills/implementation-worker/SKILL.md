---
name: implementation-worker
description: Apply a smallest-possible firmware patch only after accepted architecture, ownership/concurrency review, and verification artifacts exist.
---

# Implementation worker

This skill is approval-gated. Require accepted intake, repository map, evidence, architecture,
concurrency/ownership review, and verification matrix. Confirm the allowed write set before editing.
Implement only the accepted design; preserve ChibiOS shared sources and generated-file ownership.
Run declared tests, record diffs and evidence, and stop on scope drift or failed prerequisites.

