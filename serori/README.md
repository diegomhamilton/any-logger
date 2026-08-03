# Serori provider-neutral package

This package contains the canonical Serori skills, references, schemas, and deterministic scripts.
ChatGPT/Codex and Claude Code adapters must consume these artifacts without changing their semantics.

## Safety boundary

The default workflow is read-only analysis. Scripts never mutate source, perform Git mutation,
install dependencies, flash hardware, or attach to a debugger. Implementation requires an accepted
architecture, concurrency/ownership review, and verification matrix.

## Skill map

- `embedded-intake`: establish target, revision, constraints, and acceptance criteria.
- `repository-map`: map files, build/configuration, runtime, and ownership candidates.
- `evidence-librarian`: validate claims and versioned provenance.
- `embedded-patterns`: select patterns and record invariants/failure modes.
- `architecture-design`: turn evidence into bounded interfaces and lifecycle decisions.
- `chibios-concurrency`: audit contexts, scheduler, synchronization, and API suffixes.
- `dma-ownership`: prove buffer ownership across DMA and consumers.
- `protocol-drivers`: review ADC/DMA, CAN, UART, SPI, and I2C state machines.
- `firmware-verification`: produce host, sanitizer, target, HIL, and soak tests.
- `implementation-worker`: apply only accepted designs, with explicit approval.
- `integration-gate`: perform the final adversarial gate; never waive deterministic failures.

