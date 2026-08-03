---
name: chibios-concurrency
description: Review ChibiOS ISR/thread contexts, API suffix legality, scheduler configuration, blocking, synchronization, priority inversion, and lifecycle safety.
---

# ChibiOS concurrency

Read `../../references/chibios-9.1.0.md` and the selected checkout. Classify every relevant path as
thread, ISR, callback, system-locked, or unknown. Unsuffixed calls are not presumed ISR-safe; check
X/S/I variants and lock pairing. Reject blocking, allocation, formatting, storage I/O, and thread
mutexes in ISR paths. Inspect `CH_CFG_TIME_QUANTUM`, tickless settings, priorities, wait points,
mutex ownership, and restart behavior. Unknown is not safe in strict mode.

