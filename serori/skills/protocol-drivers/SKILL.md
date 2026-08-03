---
name: protocol-drivers
description: Review ChibiOS ADC/DMA, CAN, UART/Serial, SPI, and I2C protocol drivers as state machines with timeout, retry, ownership, and recovery policies.
---

# Protocol drivers

For each protocol map init/start/ready/active/stop/error/restart states and the context of callbacks.
Record blocking versus I-class try APIs, buffer ownership, framing/backpressure, timeouts, retries,
and recovery. ADC requires completed-buffer ownership; CAN rejection must rearm/reset receiver state;
SPI must balance select/unselect; I2C must define bus recovery. Validate exact symbols against local ChibiOS.

