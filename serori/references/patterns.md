# Serori pattern cards

Each card requires intent, applicability, invariants, anti-patterns, failure modes, evidence, and tests.

| ID | Pattern | Core invariant |
|---|---|---|
| BUF-01 | bounded occupancy ring | `0 <= occupancy <= capacity`; initialization and synchronization are explicit |
| OWN-01 | descriptor payload ownership | accepted bytes remain valid until completion/release |
| DMA-01 | completed-buffer transfer | DMA never writes a region still owned by the consumer |
| ISR-01 | minimal ISR handoff | ISR performs bounded non-blocking enqueue/signaling only |
| Q-01 | explicit overflow policy | rejection/discard is observable and ownership is resolved |
| THREAD-01 | separate instance state | every live thread has its own working area and valid argument |
| SCHED-01 | bounded ready worker | high-priority work blocks/yields or completes within a budget |
| SYNC-01 | mutex ownership | lock is initialized, released on every path, and used where inheritance matters |
| TIME-01 | timing contract | timeouts and periods use configured frequency/tickless semantics |
| LIFE-01 | quiesce/drain/restart | producers stop before consumers/resources are retired |
| MEM-01 | static/pool policy | allocation source, failure, lifetime, and fragmentation policy are explicit |
| MEM-02 | checked byte copy | source validity and destination capacity cover the complete copy |
| CFG-01 | versioned configuration | target, board, toolchain, ChibiOS revision, and config files agree |
| DBG-01 | symbol-backed debug | ELF, image, source revision, and linker inputs match |
| STACK-01 | measured stack margin | high-water usage leaves a declared safety margin |
| FSM-01 | driver state admission | every API call is valid for the current driver state |

