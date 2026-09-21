---
name: build
description: Build dsda-doom (game + spec suite) with clang-22 into prboom2/.cmake-build/claude. Use when asked to build, compile, or check that the project still compiles.
---

# Build dsda-doom

Builds the whole project from `prboom2/CMakeLists.txt` into its own tree, so the
user's CLion trees (`prboom2/.cmake-build/debug`, ...) are never touched.

All commands are run from the repository root.

## 1. Configure (only when the tree is missing)

Skip this if `prboom2/.cmake-build/claude/CMakeCache.txt` already exists - CMake
re-configures by itself when a `CMakeLists.txt` changes.

```bash
cmake -S prboom2 -B prboom2/.cmake-build/claude -G Ninja \
    -DCMAKE_C_COMPILER=/bin/clang-22 \
    -DCMAKE_CXX_COMPILER=/bin/clang++-22 \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DBUILD_SPEC=ON
```

`BUILD_SPEC=ON` pulls in `spec/` (the GTest demo suite) beside the game.

## 2. Build

```bash
cmake --build prboom2/.cmake-build/claude --parallel
```

Artifacts land in `prboom2/.cmake-build/claude/`: `dsda-doom`, `dsda-spec` and
`dsda-doom.wad`.

## Notes

- The build output is long. Pipe it to a log and read the tail when it fails:
  `... > "$CLAUDE_JOB_DIR/tmp/build.log" 2>&1; tail -n 80 "$CLAUDE_JOB_DIR/tmp/build.log"`
- To see every diagnostic instead of the first twenty, add
  `-DCMAKE_CXX_FLAGS=-ferror-limit=0` when configuring.
- To build one target only, append `--target dsda-doom` (or `dsda-spec`).
- A stale tree is thrown away by deleting `prboom2/.cmake-build/claude` and
  configuring again. Tell the user before doing that.
- Report what clang actually said. Do not summarise a failed build as "mostly
  working".
