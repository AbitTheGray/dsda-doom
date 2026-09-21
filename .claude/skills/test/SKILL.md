---
name: test
description: Build dsda-doom and run the GTest demo regression suite through ctest. Use when asked to run the tests, the specs, or to check that demos still sync.
---

# Build and run the spec suite

The suite (`spec/`, GTest via ctest) replays recorded demos through the freshly
built game and compares the reports it writes. It is how we know a rewrite did
not break demo compatibility.

All commands are run from the repository root.

## 1. Build first

Follow the `build` skill: configure `prboom2/.cmake-build/claude` if it is
missing, then

```bash
cmake --build prboom2/.cmake-build/claude --parallel
```

The tests run the binaries from that tree, so a failed build means there is
nothing to test - stop and report the compiler errors.

## 2. Run the tests

```bash
ctest --test-dir prboom2/.cmake-build/claude -L spec --output-on-failure -j"$(nproc)"
```

`-L spec` is the default run: sync, analysis and category, ~55 demos. Every test
gets its own working directory, so `-j` is safe.

Other selections:

- `-L heretic` - the 1000+ demo Heretic archive. Slow, so only on request.
- `-R Sync` - one group by name.
- no `-L` at all - everything, Heretic included.

## Notes

- The IWADs live in `spec/support/wads`. A test whose WAD or demo is missing is
  reported as *skipped*, not failed - do not read skips as passes, and say how
  many were skipped.
- A failing demo prints the expected and actual time or category. Quote those
  numbers; they are the whole point of the suite.
- The suite passes `-noautoload` so the game ignores the developer's own
  `~/.dsda-doom`. If a test starts depending on host state, that is a bug in the
  suite, not something to work around here.
