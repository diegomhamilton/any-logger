---
name: repository-map
description: Map a ChibiOS firmware repository's files, build graph, configuration, runtime threads, callbacks, DMA, synchronization, tests, and ownership candidates.
---

# Repository map

Start from an accepted or reviewed project context. Inventory project-owned versus shared/generated files.
Map Makefiles, compile inputs, HAL/RT/MCU/board/startup/linker configuration, thread creation, callbacks,
ISRs, DMA regions, queues, locks, and tests. Record unresolved constructs as unknown.
Produce `repo_map.yaml` and do not execute builds. Use `../../scripts/serori-map`.

