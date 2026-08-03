# Embedded systems patterns from Making Embedded Systems

This reference maps the embedded design patterns discussed by Elecia White in
*Making Embedded Systems: Design Patterns for Great Software* (O'Reilly, 2011)
to bounded Serori firmware guidance. The page numbers below are the book's
printed page numbers.

These are design options, not mandates. Select a pattern only after defining
execution context, ownership, memory limits, timing limits, lifecycle, and the
tests that will demonstrate the required invariant.

## Book map

| Pattern or technique | Book location | Serori use |
|---|---:|---|
| Layered architecture, encapsulation, delegation | Ch. 2, pp. 15-18 | Boundaries between hardware, transport, logger, and application code |
| Adapter / wrapper | Ch. 2, pp. 19-22 | Translate a generic interface to a board or peripheral implementation |
| Singleton | Ch. 2, pp. 28-29 | Use only for deliberately unique, explicit static resources |
| Model-View-Controller sandbox | Ch. 2, pp. 28-31 | Exercise algorithms and inputs on a host without hardware |
| Command handler and Command pattern | Ch. 3, pp. 64-70 | Decouple parsing and invocation from the receiver of an operation |
| Facade | Ch. 4, pp. 86-87 | Expose a small stable interface over a complex subsystem |
| Dependency injection | Ch. 4, pp. 96-98 | Supply backend, synchronization, timing, or storage dependencies |
| State pattern and finite-state machines | Ch. 5, pp. 116-123 | Make lifecycle and driver admission explicit |
| Publish/Subscribe (Observer) | Ch. 5, pp. 141-142 | Deliver typed events to loosely coupled consumers |
| Watchdog / fail-safe recovery | Ch. 5, pp. 143-145 | Detect unrecoverable stalls and enter a safe restart path |
| Flyweight and Flyweight Factory | Ch. 6, pp. 191-193 | Share immutable descriptors and bounded storage |
| Factory method | Ch. 6, pp. 192-193 | Select a concrete implementation behind a generic interface |
| Strategy | Ch. 6, pp. 194-195 | Select interchangeable processing or transport algorithms |
| Template Method | Ch. 6, pp. 194-195 | Keep a fixed pipeline while allowing bounded step variation |

The book also presents interrupts, timers, schedulers, polling, error handling,
and communication layers as recurring embedded techniques. They should be
combined with the existing Serori cards for ISR handoff, scheduling, timing,
ownership, and protocol drivers.

## Pattern cards

### ESP-01 - Adapter and Facade boundary

**Intent:** Translate or simplify a subsystem interface without exposing board,
register, transport, or backend details to its caller.

**Applicability:** Use an adapter when two interfaces have different shapes or
semantics. Use a facade when callers need only a small, stable subset of a
larger subsystem. A logger backend, board HAL, or peripheral driver is a common
boundary.

**Interface:** Define a small, typed interface whose calls state context
requirements, result values, timeout behavior, and ownership of buffers. Keep
translation at the boundary; do not leak a backend handle through a generic
`void *` without a lifetime contract.

**Invariants:** The adapter preserves the required meaning of each operation;
the facade does not expose mutable subsystem state; invalid state and backend
failure are reported; every acquired resource has a matching release.

**Ownership:** The boundary must say whether input is borrowed for the call,
retained until completion, or copied. Returned handles and buffers have an
explicit release owner.

**Anti-patterns:** A facade that silently drops errors, an adapter that changes
units or byte order without documenting it, and a generic interface that hides
an unsafe blocking or ISR-incompatible operation.

**Failure modes:** Wrong protocol translation, stale handles after stop, hidden
blocking in an ISR, backend errors collapsed into success, and buffer lifetime
ending before asynchronous completion.

**Evidence:** White, Ch. 2, pp. 15-22; Ch. 4, pp. 86-87.

**Verification:** Test every translation boundary with known-good and malformed
inputs; test invalid lifecycle calls; inject backend errors; verify buffer
validity through completion; run context checks for ISR and thread callers.

### ESP-02 - Injected dependency set

**Intent:** Supply variable dependencies at initialization so policy and
hardware-specific behavior can be replaced without changing the consumer.

**Applicability:** Use for logger backends, locks, signals, clocks, storage,
transport, and host fakes. Prefer explicit construction-time injection in C;
function tables and a context pointer are sufficient.

**Interface:** A dependency table contains only the operations and state needed
by the consumer. Initialization validates required entries and records whether
each operation is legal from thread, callback, or ISR context.

**Invariants:** Dependencies remain valid for the consumer's complete lifetime;
required operations are non-null; injected callbacks obey the declared context,
blocking, timing, and ownership contracts.

**Ownership:** The owner of the dependency table and context outlives every
consumer using it. Initialization either transfers ownership explicitly or
records that the dependency is borrowed.

**Anti-patterns:** Partially initialized callback tables, mutable global service
locators, callbacks that capture stack objects, and injecting a blocking API into
an ISR-facing path.

**Failure modes:** Null callback calls, use-after-free of a context object,
deadlock from an incompatible lock, and tests that pass only because a global
backend remains installed.

**Evidence:** White, Ch. 4, pp. 96-98; the logger backend example in Ch. 2,
pp. 22-28.

**Verification:** Construct with missing and incompatible dependencies; replace
the backend with a deterministic fake; verify initialization rejects invalid
tables; run lifecycle, concurrency, and context-specific tests.

### ESP-03 - Command dispatcher

**Intent:** Decouple recognition and invocation of a request from the receiver
that performs the action.

**Applicability:** Use for diagnostic consoles, protocol commands, maintenance
operations, and scripted tests. A static command table is preferred when the
command set is known at build time.

**Interface:** Each command entry has a stable name or opcode, argument parser,
permission/state predicate, handler, help text, and result mapping. The invoker
does not depend on receiver internals.

**Invariants:** Unknown or malformed commands do not invoke a receiver; each
command has one deterministic dispatch path; handlers execute in an allowed
context and return an observable result; argument bounds are checked before use.

**Ownership:** The parser owns input storage until parsing completes. A handler
may borrow arguments for the call or receive an owned copy; asynchronous
commands must transfer or copy arguments explicitly.

**Anti-patterns:** Long `strcmp` chains with inconsistent validation, handlers
that parse directly from a reusable RX buffer after return, and command paths
that block an ISR or high-priority callback.

**Failure modes:** Prefix collisions, unchecked numeric overflow, stale argument
storage, command execution in the wrong state, and macro commands that partially
complete without reporting which operation failed.

**Evidence:** White, Ch. 3, pp. 64-70.

**Verification:** Test every command and malformed argument class; fuzz the
parser; test unknown commands and duplicate names; test state and permission
rejection; verify asynchronous argument lifetime and macro failure reporting.

### ESP-04 - Explicit state machine

**Intent:** Make behavior depend on current state and event while making every
allowed transition visible and testable.

**Applicability:** Use for peripheral protocols, logger lifecycle, connection
management, retries, and any flow chart with meaningful history. Use a
table-driven form when states and events are numerous or similarly shaped.

**Interface:** Define a finite set of states, events, entry/exit actions,
housekeeping behavior, and a transition function or table. Invalid events have
an explicit policy: reject, ignore, defer, or fault.

**Invariants:** The current state is always valid; every state/event combination
has a defined outcome; entry and exit actions occur exactly once per transition;
no event causes a resource operation outside its admitted state.

**Ownership:** State context owns timers, pending operations, and retry counters.
On transition, resources are either retained by the context or released before
the next state begins. Events queued across a transition have a documented
validity policy.

**Anti-patterns:** Hidden transitions spread across callbacks, default branches
that silently do the wrong thing, timers surviving a state exit, and a state
enum used without validating external input.

**Failure modes:** Deadlock in a state, lost or duplicated event, re-entry of an
operation after timeout, stale DMA/transport completion, and incomplete stop or
restart transitions.

**Evidence:** White, Ch. 5, pp. 116-123; communication state machines in Ch. 6,
pp. 178-188.

**Verification:** Generate or enumerate the complete state/event matrix; test
every transition and invalid event; inject timeout, duplicate completion, and
backend failure; verify entry/exit counts, resource release, and restart safety.

### ESP-05 - Publish/Subscribe event delivery

**Intent:** Allow a publisher to deliver typed information to multiple consumers
without knowing their concrete implementations.

**Applicability:** Use for scheduler ticks, channel events, health notifications,
and system messages. Do not use when a direct call gives a simpler bounded
dependency or when delivery latency cannot be controlled.

**Interface:** Define event type, payload size, subscriber registration lifetime,
delivery context, queue capacity, ordering, and unsubscribe behavior. Prefer
static subscriber tables or bounded queues.

**Invariants:** A published event is either delivered, rejected, or accounted for
according to policy; a subscriber cannot be called after unsubscribe; publisher
execution remains within its time and blocking budget.

**Ownership:** The event payload is copied into subscriber-owned or queue-owned
storage, or remains valid until all subscribers acknowledge completion. A raw
pointer payload requires an explicit reference or release protocol.

**Anti-patterns:** Unbounded subscriber lists, callbacks that remove themselves
while dispatching without a defined rule, silent queue overflow, and publishing
borrowed DMA memory.

**Failure modes:** Use-after-unsubscribe, fan-out latency spikes, event loss,
recursive publication, and one slow subscriber blocking unrelated consumers.

**Evidence:** White, Ch. 5, pp. 141-142.

**Verification:** Test registration and removal during dispatch; fill every
queue; verify ordering and overflow accounting; test slow and failing subscribers;
run payload lifetime and concurrent publish/consume tests.

### ESP-06 - Strategy and bounded pipeline variation

**Intent:** Select interchangeable algorithms behind a stable interface, or keep
a fixed processing pipeline while allowing selected steps to vary.

**Applicability:** Use Strategy for encoding, filtering, compression, transport,
or signal-processing algorithms. Use a template-style pipeline when ordering and
lifecycle are fixed but one or more steps need a test or product variant.

**Interface:** A strategy has a typed input/output contract, resource budget,
error model, and context pointer. A pipeline documents step order and which
steps may be replaced.

**Invariants:** Every strategy satisfies the same input, output, and ownership
contract; selection cannot produce an incompatible implementation; a pipeline
does not skip required cleanup when a step fails.

**Ownership:** The selected strategy does not retain borrowed input unless the
interface grants retention. Output ownership and scratch-buffer lifetime are
explicit, especially for DMA or asynchronous processing.

**Anti-patterns:** Runtime selection without a validation step, function pointers
with undocumented calling context, strategies that allocate unpredictably, and
using a state machine when only an algorithm choice is needed.

**Failure modes:** Incompatible output format, scratch-buffer aliasing,
algorithm-specific timing overruns, and replacement during an in-flight call.

**Evidence:** White, Ch. 6, pp. 194-195.

**Verification:** Run a common conformance suite against every strategy; measure
worst-case execution and memory; test replacement only at a quiescent boundary;
inject each strategy's error and partial-output behavior.

### ESP-07 - Factory and flyweight storage

**Intent:** Separate generic use from concrete instance creation, and share
immutable or interchangeable objects instead of allocating duplicates.

**Applicability:** Use factories for backend selection and fixed registries. Use
flyweights for descriptors, protocol definitions, glyphs, lookup entries, or
other repeated immutable data. Prefer static tables or fixed pools in firmware.

**Interface:** A factory accepts a bounded identifier and returns a typed handle
with an explicit lifetime. A flyweight table defines valid indices and whether
returned objects are read-only.

**Invariants:** Invalid identifiers cannot create an invalid object; the factory
does not exceed its pool; shared objects are immutable while referenced; all
handles identify an object of the promised interface.

**Ownership:** Static flyweights remain valid for the program lifetime. Pool
objects have a clear allocate/release owner and cannot be returned while in use.
Factory-created resources are released through the matching factory or owner.

**Anti-patterns:** Hidden heap allocation in a factory, returning pointers into
reusable scratch storage, mutable global flyweights, and treating a generic
handle as a concrete type without validation.

**Failure modes:** Pool exhaustion, aliasing-induced corruption, stale handles,
fragmentation, and factory selection that succeeds but returns an uninitialized
backend.

**Evidence:** White, Ch. 6, pp. 191-193; static and pool allocation guidance in
Ch. 8, pp. 216-233.

**Verification:** Exhaust and recover every pool; test invalid identifiers and
double release; verify read-only sharing; instrument allocation source and peak
occupancy; run handle lifetime and restart tests.

### ESP-08 - Watchdog and fail-safe recovery

**Intent:** Detect an unrecoverable system stall and restore the system to a
known safe state.

**Applicability:** Use for products that must recover without an operator. The
watchdog is a last-resort mechanism, not a substitute for ordinary error
handling, queue policies, or protocol timeouts.

**Interface:** Define the health conditions that permit servicing, the service
owner, timeout, reset cause capture, safe-state actions, and boot-time recovery
policy. Service it from one point that demonstrates required subsystems are
progressing.

**Invariants:** A stuck required subsystem eventually stops the service condition;
reset cause is retained or reported; recovery is bounded; outputs and external
resources enter the declared safe state.

**Ownership:** One system-level supervisor owns watchdog servicing. Subsystems
publish bounded health evidence but do not independently service the watchdog.
Reset diagnostics remain valid until they are persisted or consumed.

**Anti-patterns:** Servicing from a periodic interrupt that runs while the main
system is stuck, scattering service calls through arbitrary delays, and enabling
the watchdog without a debugger/bring-up policy.

**Failure modes:** A false healthy signal, reset loops, lost reset diagnostics,
unsafe outputs during restart, and a timeout too short for legitimate worst-case
work.

**Evidence:** White, Ch. 5, pp. 143-145; power and sleep interaction in Ch. 10,
pp. 290-296.

**Verification:** Inject a stalled worker and verify reset; test each reset cause;
measure service timing under worst-case load; test recovery after repeated
resets; verify safe outputs and persisted diagnostics.

### ESP-09 - Host sandbox and model/view separation

**Intent:** Exercise embedded algorithms using deterministic host inputs and
observable outputs while keeping hardware access outside the algorithm under
test.

**Applicability:** Use for parsers, state machines, data processing, command
handlers, and lifecycle logic that can run without registers or interrupts.

**Interface:** The model owns algorithm state, the view supplies input and
records output, and the controller translates between them. Hardware adapters
are replaced by deterministic fakes with the same contract.

**Invariants:** A test run is reproducible from its input; algorithm behavior is
independent of wall-clock and hardware state unless explicitly injected; every
output and error is observable.

**Ownership:** The test harness owns input fixtures and captured output. Fakes
must not retain pointers to short-lived fixture data unless the test grants that
lifetime.

**Anti-patterns:** Host tests that reproduce the hardware implementation instead
of testing the contract, hidden real-time dependencies, and assertions that only
inspect final state while missing intermediate transitions.

**Failure modes:** False confidence from unrealistic fakes, nondeterministic
ordering, platform-dependent integer behavior, and tests that cannot expose
buffer or lifecycle violations.

**Evidence:** White, Ch. 2, pp. 28-31.

**Verification:** Replay deterministic fixtures; test malformed and boundary
inputs; compare event traces; run sanitizer builds; supplement host tests with
target or HIL tests for context, timing, DMA, and register behavior.

## Deliberate non-patterns

Singletons can be useful for a physically unique resource, but Serori should
prefer an explicit instance passed to callers. This makes ownership, lifecycle,
test isolation, and concurrency review visible. Do not add a Singleton card just
to justify a global logger or service locator.

Likewise, Adapter, Facade, Factory, Strategy, and Dependency Injection may be
implemented together in C through a bounded function table and context pointer.
Record the separate contracts only when their invariants or verification tests
differ; avoid creating layers that add indirection without reducing coupling or
clarifying ownership.

## Relationship to existing Serori cards

| Book-derived concern | Existing Serori card(s) to reuse |
|---|---|
| Bounded queues and event delivery | `BUF-01`, `Q-01` |
| Async payloads and flyweight/pool lifetime | `OWN-01`, `DMA-01`, `MEM-01`, `MEM-02` |
| ISR-facing adapters and publishers | `ISR-01` |
| Worker and scheduler budgets | `THREAD-01`, `SCHED-01`, `TIME-01` |
| Stop, restart, and watchdog lifecycle | `LIFE-01`, `FSM-01` |
| Locks injected into adapters | `SYNC-01` |
| Target-specific implementations | `CFG-01`, `DBG-01` |

No pattern card is complete until its context legality, ownership transfer,
overflow or failure policy, and verification evidence are recorded for the
specific target.
