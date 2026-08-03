---
name: firmware-verification
description: Design evidence-backed host, sanitizer, static, target, HIL, debug-instrumentation, and soak tests for embedded firmware contracts.
---

# Firmware verification

Translate each invariant into a test with level, fixture/command, expected result, evidence, and status.
Cover initialization, queue overflow, pointer lifetime, DMA reuse, ISR legality, scheduler starvation,
priority inversion, allocation exhaustion, driver state transitions, backend failure, stop/restart,
stack margin, and optimized builds. A successful compile does not prove runtime ownership safety.

