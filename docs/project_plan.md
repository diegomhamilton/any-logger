# any-logger Project Plan

## Purpose

`any-logger` is a portable C logging core for embedded and application projects. Platform-specific code supplies init/start/stop/write/wait/signal/lock/unlock behavior; the core coordinates channels and queues data writes.

## Current Shape

- Public API: `inc/logger.h`.
- Channel model: `inc/channel.h`.
- Queue primitive: `inc/buffer.h`.
- Core implementation: `src/logger.c`.
- Local pthread example: `examples/test_logger`.
- ChibiOS-oriented example/project folders already exist.

## First Milestone

Make the core logger bookkeeping predictable and testable.

Scope:

- Channel registration returns IDs from `1..no_channels`.
- Registration fails cleanly when full.
- Unregistration clears the correct bit and removes the channel pointer.
- Async write behavior is covered by at least one small local test or example.

## Starter Findings

- `register_new_channel` could skip the last valid ID and return an out-of-range ID when channels were full.
- `logger_unregister` cleared the wrong bit because it used a right shift instead of a left shift.
- The example compiles as an integration smoke test, but it does not assert behavior yet.

## Next Tasks

- Add a small C test target for channel registration/unregistration.
- Decide whether API functions should return operation status instead of silent `void` for failures.
- Document ownership expectations for `data_t.data` passed to `logger_write_async`.
- Decide the intended behavior when `write_buffer` is full.
