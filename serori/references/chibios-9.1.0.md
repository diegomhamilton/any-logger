# ChibiOS local reference: HAL 9.1.0 / kernel 8.0.0

Evidence paths are relative to `libs/ChibiOS`; the exact Git revision must be captured by intake.

## Context and API classes

- Unsuffixed APIs are normally thread-context APIs.
- `X` APIs are any-context variants; `S` APIs require the system lock; `I` APIs require interrupt/system-locked context.
- `chSysLockFromISR()` is for interrupt handlers and must surround only bounded ISR-safe work.
- An ISR must not block, allocate, format, perform storage I/O, or take a thread-only mutex.

## Threads and scheduling

Inspect `os/rt/include/chthreads.h`, `chdynamic.h`, `chschd.h`, and the selected `cfg/chconf.h`.
Record `THD_WORKING_AREA`, `THD_FUNCTION`, `chThdCreateStatic`, dynamic creation APIs, priority
expressions, `CH_CFG_TIME_QUANTUM`, `CH_CFG_ST_TIMEDELTA`, `CH_CFG_ST_FREQUENCY`, and idle-thread settings.

## HAL and protocol checks

Inspect the selected revision before classifying calls:

- ADC circular/half-buffer callbacks: ownership must prevent DMA reuse while consumed.
- CAN receive: accepted frames need consumer ownership; rejection must rearm/reset state.
- UART/Serial: distinguish transfer callbacks from buffered queues and completion semantics.
- SPI: prove start/select/transfer/unselect and DMA completion ownership.
- I2C: record timeout, error class, bus acquisition, and recovery policy.

## Configuration provenance

Treat `halconf.h`, `mcuconf.h`, `chconf.h`, board, startup, linker, and Makefile files as a
compatibility bundle. Do not edit shared ChibiOS sources to fix project configuration.

