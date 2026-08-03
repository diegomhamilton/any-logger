# PLAY Embedded research basis

These articles are conceptual inputs. Exact API behavior is always checked against the selected
ChibiOS checkout and recorded with a revision.

## Threading and configuration

Sources: [beginner guide](https://playembedded.org/blog/mastering-multithreading-with-chibios-a-beginners-guide/),
[complete reference](https://playembedded.org/blog/the-complete-reference-for-multithreading-in-chibios-rt/),
[parametric threads](https://playembedded.org/parametric-threads-with-chibios/).

- Treat `halconf.h`, `chconf.h`, `mcuconf.h`, board, startup, linker, and Makefile inputs as one
  versioned compatibility set.
- Establish `halInit()`, `chSysInit()`, driver activation, and thread creation order.
- Prefer static working areas when topology and lifetimes are bounded; include RTOS metadata,
  alignment, and measured stack high-water in the budget.
- A shared `THD_FUNCTION` still requires one working area and one valid argument object per live
  instance. Never pass an expired local or temporary through `void *arg`.
- Scheduling depends on priority, `CH_CFG_TIME_QUANTUM`, tick frequency, and tickless configuration.
  A loop must block, sleep, wait, or explicitly yield when it cannot complete in a bounded interval.
- Stop/restart is a lifecycle, not a status flag: quiesce producers, retire workers, release
  resources, and admit at most one worker per logical resource.

## Debugging and instrumentation

Source: [ChibiStudio debugging](https://playembedded.org/debugging-stm32-chibistudio/).

- Debug an ELF that matches the flashed image and source revision; BIN/HEX alone do not provide
  source-level symbols.
- Validate both reduced-optimization debug builds and optimized builds for timing-sensitive code.
- Classify API legality by Init, Thread, S-Locked, I-Locked, ISR, or unknown context.
- Use state checks, assertions, stack fill/high-water, trace, and runtime statistics only with their
  memory/timing/configuration costs recorded. Unsupported instrumentation is `unknown` or `blocked`.
- Debugger-written peripheral state is external test manipulation, not firmware evidence.

## Pointers and memory

Source: [C pointers](https://playembedded.org/demystifying-c-pointers/#24_Memory_and_memory_fragmentation).

- `void *` is a transport mechanism, not an ownership contract. Track type, bounds, mutability,
  lifetime, owner before/after transfer, and release/rejection behavior.
- `memcpy` does not validate bounds or establish ownership. Check destination capacity, source
  validity, overlap semantics, and size arithmetic.
- Static allocation or fixed-size pools are preferred for bounded firmware resources, but dynamic
  allocation is permitted only with an explicit failure, lifetime, and fragmentation policy.
- Allocation in ISR/callback paths is prohibited by default.

## Version caveat

The local checkout exposes HAL component version 9.1.0. That is not the overall ChibiOS release
number: use ChibiOS 21.11 as the release-line reference and capture the exact Git tag/commit during
intake. Article examples may use older releases, stale trace-mask values, illustrative typos, or
different configuration. Preserve article claims as `version-dependent` until local headers/source
confirm them.
