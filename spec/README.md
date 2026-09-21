# dsda-doom spec files

The demo regression suite. It replays recorded demos through the built game and
checks the reports the game writes (`levelstat.txt` and `analysis.txt`). A demo
only replays correctly when the playsim still behaves exactly as it did when the
demo was recorded, so this is how we know a change did not break compatibility.

How to run the specs:

1) Put `DOOM2.WAD`, `DOOM.WAD`, `HERETIC.WAD`, `HEXEN.WAD`, `Valiant.wad`, and `rush.wad` in `spec/support/wads`.
2) Configure with the suite enabled: `cmake -S prboom2 -B build -DBUILD_SPEC=ON`
3) Build it: `cmake --build build`
4) Run it: `ctest --test-dir build --output-on-failure`

A test whose IWAD, PWAD or demo is not on disk is reported as skipped rather
than failed, and the suite prints which WADs it could not find before it starts.
A partial set of WADs still gives a useful run.

Selecting what to run:

- `ctest --test-dir build -L spec` - sync, analysis and category (55 demos).
- `ctest --test-dir build -L heretic` - the 1000+ demo Heretic archive.
- `ctest --test-dir build -j8` - spread the work over cores. Every test gets its
  own working directory, so they do not fight over the report files.
- `ctest --test-dir build -R Sync` - one group, by name.

Layout:

- `src/` - the suite itself.
- `support/wads/` - the IWADs you supply, plus the checked-in `analysis_test.wad`
  that the purpose-built analysis demos are recorded against.
- `support/lmps/` - the recorded demos.
- `support/lmps/heretic/list.txt` - one line per Heretic demo, as
  `description|expected time|path|extra arguments`. The suite generates one test
  per line, so adding a demo means adding a line here.
