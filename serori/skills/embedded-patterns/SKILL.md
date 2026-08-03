---
name: embedded-patterns
description: Select or author evidence-backed embedded firmware pattern cards for buffers, ownership, DMA, ISR handoff, scheduling, memory, protocols, and lifecycle.
---

# Embedded patterns

Read `../../references/patterns.md` and relevant ChibiOS references. A card must state intent,
applicability, interface, invariants, ownership, anti-patterns, failure modes, and verification tests.
Prefer bounded static storage or fixed pools when lifetimes are known. Treat pointer and DMA payload
lifetime as an explicit contract, not an implication of `void *` or `memcpy`.

