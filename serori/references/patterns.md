# Serori pattern cards

Pattern cards are reusable engineering contracts, not drop-in implementations.
Select a card only when its applicability conditions hold. Every selected card
must be backed by repository evidence and verification tests. If a project
cannot satisfy a card's invariant, record the deviation as a decision and risk
instead of silently treating the pattern as applicable.

## BUF-01 — Bounded-occupancy ring

**Intent:** Provide a fixed-capacity FIFO with explicit full, empty, and
initialization behavior.

**Apply when:** Samples or messages are produced and consumed at different
rates, especially across an ISR/thread or thread/thread boundary.

**Invariants:** `0 <= occupancy <= capacity`; read and write indices are
initialized before use; each slot is written before it is read and released
after consumption; full and empty are distinguishable; index arithmetic cannot
overflow into an invalid slot.

**Anti-patterns:** Assuming a zeroed object is initialized; using a shared count
without a synchronization contract; overwriting unread entries without an
explicit loss policy; deriving capacity from an ambiguous macro expression.

**Failure modes:** Queue full, stale indices after restart, lost entries, data
race, and wraparound corruption.

**Evidence:** Capacity and index definitions, initialization path, producer and
consumer contexts, and the synchronization primitive or single-producer/
single-consumer proof.

**Tests:** Empty read, one item, capacity items, full write, wraparound,
concurrent producer/consumer, reset, and restart.

## OWN-01 — Descriptor payload ownership

**Intent:** Make payload lifetime and release responsibility explicit when a
descriptor crosses an API or queue boundary.

**Apply when:** A descriptor contains a pointer, buffer reference, or storage
owned by a caller, producer, consumer, or pool.

**Invariants:** Every payload has exactly one current owner; the receiver does
not outlive the payload; ownership transfer and release happen on success,
rejection, timeout, and shutdown; accepted bytes remain unchanged until the
consumer releases them.

**Anti-patterns:** Passing a pointer to stack storage; enqueueing a pointer to
mutable scratch space; copying a descriptor while assuming the payload was
copied; releasing on only the success path.

**Failure modes:** Use-after-scope, overwrite before consumption, double free,
leak, and queue-full paths that lose ownership.

**Evidence:** Descriptor definition, allocation/storage lifetime, transfer
function, and every return path that accepts or rejects the descriptor.

**Tests:** Payload mutation after enqueue, rejected enqueue, timeout, consumer
release, shutdown with pending entries, and repeated reuse.

## DMA-01 — Completed-buffer transfer

**Intent:** Transfer completed DMA storage to a consumer without allowing the
DMA engine and consumer to access the same region concurrently.

**Apply when:** ADC, serial, CAN, or other DMA transfers use a buffer, ring, or
double-buffer arrangement.

**Invariants:** DMA owns a writable region only; the consumer owns a completed
region only; handoff occurs at a defined completion boundary; cache/alignment
requirements are satisfied where relevant; the consumer returns or replaces a
region before DMA reuses it.

**Anti-patterns:** Parsing a buffer while DMA may still write it; handing off a
buffer without an ownership state; assuming an interrupt implies memory
visibility without checking the platform contract.

**Failure modes:** Torn samples, stale cache data, buffer overrun, missed
completion, and producer starvation.

**Evidence:** DMA configuration, callback/ISR contract for the pinned ChibiOS
revision, buffer layout, and handoff state machine.

**Tests:** Half/full completion, back-to-back completions, consumer delay,
buffer exhaustion, stop during transfer, and restart.

## ISR-01 — Minimal ISR handoff

**Intent:** Keep interrupt work bounded while reliably transferring event or
data ownership to a thread.

**Apply when:** An interrupt callback must notify a worker or enqueue a small
descriptor.

**Invariants:** ISR work is bounded and non-blocking; no formatting, storage
I/O, unbounded loops, or allocation occurs in the ISR; interrupt status is
acknowledged according to the platform contract; failures are observable.

**Anti-patterns:** Calling a blocking API from interrupt context; doing a full
payload copy or protocol encode without a measured bound; silently dropping a
notification when the queue is full.

**Failure modes:** Deadlock, interrupt latency spikes, missed events, queue
overflow, and unsafe context/API use.

**Evidence:** Callback context, every called API's context requirements, bound
on work, and handoff mechanism.

**Tests:** Burst interrupts, queue-full notification, interrupt during shutdown,
and latency measurement under load.

## Q-01 — Explicit overflow policy

**Intent:** Define what happens when a queue, pool, or ring cannot accept more
work.

**Apply when:** Production can outrun consumption or storage can become
temporarily unavailable.

**Invariants:** Full condition is detected before overwrite; rejection,
discard-oldest, blocking, backpressure, or coalescing is an explicit decision;
loss is counted or otherwise observable; ownership is resolved on failure.

**Anti-patterns:** Silent overwrite; unbounded retry; blocking an ISR; treating
an error return as proof that the payload was released.

**Failure modes:** Silent data loss, deadlock, memory leak, and unbounded
latency.

**Evidence:** Capacity calculation, rate/burst assumptions, return contract,
counter/event, and operator-visible policy.

**Tests:** Sustained overload, burst overload, recovery after full, and all
rejected-item release paths.

## THREAD-01 — Separate instance state

**Intent:** Ensure each live thread has independent working storage and a valid
argument for its entire lifetime.

**Apply when:** Multiple worker instances or repeated start/stop cycles exist.

**Invariants:** Each thread has a distinct working area and state; the argument
points to live instance data; initialization completes before start; teardown
does not reclaim state until the thread exits.

**Anti-patterns:** Reusing one static working area; passing the address of a
stack-local argument; storing instance state in an accidental global.

**Failure modes:** Corrupted stacks, cross-instance interference, use-after-free,
and restart races.

**Evidence:** Thread creation, working-area declarations, argument ownership,
and stop/join behavior.

**Tests:** Two simultaneous instances, distinct arguments, stop/start, and
creation failure cleanup.

## SCHED-01 — Bounded ready worker

**Intent:** Keep a ready worker from starving peers or violating latency and
power budgets.

**Apply when:** A high-priority thread processes queued work or polling input.

**Invariants:** Per-iteration work has a declared bound; the worker blocks,
yields, or completes when no work exists; queue depth and service rate are
compatible; priority is justified by a measured deadline.

**Anti-patterns:** Busy-waiting on an empty queue; unbounded batch draining;
raising priority to hide missed deadlines.

**Failure modes:** CPU starvation, missed deadlines, thermal/power increase,
and unbounded backlog.

**Evidence:** Priority, wake-up mechanism, work bound, rate budget, and timing
measurements.

**Tests:** Empty idle behavior, maximum batch, sustained load, and lower-priority
progress.

## SYNC-01 — Mutex ownership

**Intent:** Protect shared state with a lock whose ownership and release rules
are auditable.

**Apply when:** Multiple thread contexts access mutable state and a mutex is
permitted by the platform contract.

**Invariants:** Mutex is initialized before use; only the owning thread unlocks;
every lock path releases it; critical sections are bounded; priority-inheritance
requirements are understood.

**Anti-patterns:** Unlocking from a different context; returning while locked;
holding a mutex across blocking I/O; using a mutex in an ISR.

**Failure modes:** Deadlock, priority inversion, recursive misuse, and stale
state after partial initialization.

**Evidence:** Lock scope, call graph, context classification, and failure-path
cleanup.

**Tests:** Contention, timeout/error path, cancellation/shutdown, and priority
inversion scenario where applicable.

## TIME-01 — Timing contract

**Intent:** Make periods, timeouts, tick conversion, and deadline behavior
explicit and testable.

**Apply when:** Scheduling, debounce, retry, sampling, or communication depends
on time.

**Invariants:** Units are explicit; conversion uses the configured frequency and
rounding policy; zero and maximum values are defined; timeout expiry cannot
silently become an infinite wait; jitter tolerance is stated.

**Anti-patterns:** Mixing milliseconds and ticks; assuming a tick is always one
millisecond; using a polling delay as a deadline guarantee.

**Failure modes:** Early/late timeouts, overflow, busy loops, and drift.

**Evidence:** Configuration, conversion code, API contract for the pinned
ChibiOS revision, and measured timing.

**Tests:** Zero, one-tick, large, expired, and repeated-period cases.

## LIFE-01 — Quiesce, drain, restart

**Intent:** Stop producers before retiring consumers or resources and make
restart state deterministic.

**Apply when:** Drivers, queues, DMA, threads, or backends can be stopped and
started.

**Invariants:** New production is disabled before teardown; in-flight work is
drained, cancelled, or explicitly discarded; callbacks cannot target retired
state; resources are released once; restart reinitializes all state.

**Anti-patterns:** Freeing a buffer while a callback can run; killing a thread
without resolving queued ownership; assuming a second start is equivalent to
initialization.

**Failure modes:** Use-after-free, stale callbacks, leaked resources, and
duplicate workers.

**Evidence:** State transitions, stop ordering, callback disable/acknowledge,
and resource ownership.

**Tests:** Stop while idle, active, full, and mid-transfer; repeated restart;
and teardown after partial initialization.

## MEM-01 — Static or pool allocation policy

**Intent:** Bound memory use and make allocation failure and lifetime behavior
explicit.

**Apply when:** Firmware has known object counts, fixed payload sizes, or
long-lived runtime constraints.

**Invariants:** Allocation source and maximum count are declared; failure is
handled; object lifetime and release owner are explicit; fragmentation and
alignment requirements are acceptable.

**Anti-patterns:** Unbounded heap allocation in a long-running path; allocation
from an ISR; assuming allocation cannot fail after startup.

**Failure modes:** Exhaustion, fragmentation, leak, alignment fault, and
unpredictable latency.

**Evidence:** Pool/heap configuration, size/count calculation, failure path,
and high-water measurements.

**Tests:** Exhaustion, release/reuse, partial initialization, and maximum
concurrency.

## MEM-02 — Checked byte copy

**Intent:** Ensure every byte copy has valid source, destination, and length
contracts.

**Apply when:** Protocol payloads or descriptors are copied between buffers.

**Invariants:** Source contains the requested bytes; destination capacity covers
the complete copy; length arithmetic cannot wrap; overlapping regions use a
defined operation; null/zero-length behavior is defined.

**Anti-patterns:** Trusting a payload length without checking capacity; copying
`sizeof(pointer)` when the payload size was intended; relying on a terminator in
binary data.

**Failure modes:** Buffer overrun, truncation, information disclosure, and
corruption from integer wraparound.

**Evidence:** Length origin, capacity declaration, validation branch, and copy
API contract.

**Tests:** Zero, exact fit, one over capacity, maximum representable length,
null input, and malformed protocol length.

## CFG-01 — Versioned configuration

**Intent:** Keep target identity, build configuration, and platform APIs aligned.

**Apply when:** Firmware behavior depends on board, MCU, toolchain, ChibiOS,
HAL, linker, or generated configuration files.

**Invariants:** Board and MCU agree; toolchain and startup files agree; exact
ChibiOS revision is recorded; configuration headers are the ones used by the
build; incompatible changes fail or are visible.

**Anti-patterns:** Treating a HAL component version as the full ChibiOS version;
relying on filenames instead of the checkout revision; mixing configuration
from another board.

**Failure modes:** Wrong register/API assumptions, link failures, undefined
runtime behavior, and irreproducible builds.

**Evidence:** Build command, config paths, board/MCU identity, ChibiOS tag or
commit, and source/configuration hashes.

**Tests:** Clean build, configuration-diff check, and a pinned-revision rebuild.

## DBG-01 — Symbol-backed debug

**Intent:** Ensure debug symbols, flashed image, source, and linker inputs refer
to the same build.

**Apply when:** Diagnosing faults with an ELF, map file, debugger, BIN, or HEX.

**Invariants:** ELF and flashed image derive from the same source/configuration
revision; symbols match addresses; linker script and optimization settings are
recorded; build identity is recoverable.

**Anti-patterns:** Debugging a stale ELF; using BIN/HEX alone to infer source
locations; mixing symbols from a different linker layout.

**Failure modes:** False source locations, misleading stack traces, and wasted
debugging effort.

**Evidence:** Build commit, artifact hashes, map/ELF metadata, linker inputs,
and flash procedure.

**Tests:** Reproducible build hash where practical and symbol/address sanity
check on the target image.

## STACK-01 — Measured stack margin

**Intent:** Demonstrate that each thread has sufficient stack for its worst
observed or bounded workload.

**Apply when:** Thread stacks are statically sized or memory is constrained.

**Invariants:** Stack size is declared; high-water usage is measured under the
maximum relevant path; a safety margin is defined; overflow detection is
enabled where available.

**Anti-patterns:** Choosing stack size by habit; measuring only idle usage;
ignoring interrupt/nested-call contribution.

**Failure modes:** Silent stack corruption, sporadic faults, and memory waste.

**Evidence:** Working-area size, measurement method, workload, and margin
calculation.

**Tests:** Maximum message, error, logging, nested-callback, and restart paths.

## FSM-01 — Driver-state admission

**Intent:** Make legal API calls and lifecycle transitions explicit for a driver.

**Apply when:** A peripheral or backend has initialized, active, stopping, or
stopped states.

**Invariants:** Every public operation declares allowed states; invalid calls
fail predictably; transitions are serialized; callbacks cannot observe a state
that has already been retired.

**Anti-patterns:** Inferring state from a non-null pointer; allowing start twice;
silently accepting I/O before initialization; changing state in unrelated code.

**Failure modes:** Partial initialization, duplicate resources, invalid hardware
access, and restart races.

**Evidence:** State diagram or enum, transition code, API guards, and teardown
ordering.

**Tests:** Every valid transition, every invalid call, partial-start failure,
stop during activity, and repeated start/stop.

