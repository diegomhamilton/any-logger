---
name: dma-ownership
description: Prove DMA buffer lifetime, half/full completion transfer, reuse, release, cache, queue-full, and error behavior in ChibiOS firmware.
---

# DMA ownership

For every DMA region identify peripheral owner, completed region, transfer event, consumer owner,
release/rearm event, and error/reset path. A circular or ping-pong half must not be reused while the
logger/backend can read it. ISR handoff is bounded and non-blocking. If copying is selected, verify
capacity and copy completion; if tokens are selected, verify release on success and rejection.

