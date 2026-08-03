# Getting started with Serori

Serori is the provider-neutral ChibiOS firmware analysis package under [`serori/`](serori/).
It provides skills, references, artifact schemas, deterministic inspection scripts, and adapters
for ChatGPT/Codex and Claude Code.

## ChibiOS version terminology

- `21.11` is the ChibiOS release line.
- `9.1.0` is the ChibiOS/HAL component version used by that release line.
- The local HAL header reports `CH_HAL_VERSION "9.1.0"`.
- The exact supported dependency must be identified by a ChibiOS tag or Git commit.

Do not treat `9.1.0` as the overall ChibiOS release. Pin and record the exact 21.11 patch/tag or
commit before making API or scheduling claims.

```bash
git -C libs/ChibiOS describe --tags --always --dirty
git -C libs/ChibiOS rev-parse HEAD
```

## Required intake information

- board and MCU;
- exact ChibiOS 21.11 patch/tag or commit;
- toolchain;
- RAM/flash and latency budgets;
- acquisition rates;
- required protocols;
- logging/storage backend;
- definition of done and out-of-scope items.

## First local run

```bash
mkdir -p artifacts

./serori/scripts/serori-intake \
  --project-root . \
  --chibios libs/ChibiOS \
  --board Nucleo-F303RE \
  --mcu STM32F303RE \
  --protocol adc \
  --protocol dma \
  --protocol can \
  --protocol serial \
  --output artifacts/project-context.json

./serori/scripts/serori-map \
  --project-root . \
  --output artifacts/repo-map.json
```

Validate evidence against the pinned checkout:

```bash
./serori/scripts/serori-evidence \
  --project-root . \
  --path libs/ChibiOS/os/hal/include/hal.h \
  --path libs/ChibiOS/os/rt/include/chthreads.h \
  --path docs/issue_review_report.md \
  --output artifacts/evidence.json
```

## Skill order

1. `embedded-intake`
2. `repository-map`
3. `evidence-librarian`
4. `embedded-patterns`
5. `architecture-design`
6. `chibios-concurrency`, `dma-ownership`, and `protocol-drivers`
7. `firmware-verification`
8. `implementation-worker`, only after approval
9. `integration-gate`

The default mode is read-only analysis. Article text, a successful compile, or an unverified static
analysis result is not proof of firmware safety.

