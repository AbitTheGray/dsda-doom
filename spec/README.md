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
- `ctest --test-dir build -L archive` - the Compet-N / SDA archive (see below).
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
- `support/archive/` - the Compet-N and SDA demo archive, never committed.
  `support/fetch_archive.py` downloads `competn.zip` and `sda.zip` from
  archive.org; unpack them into `support/archive/competn-2005/` and
  `support/archive/sda-2005/`.
- `support/archive/competn-2005.json`, `support/archive/sda-2005.json` - one entry per archive
  demo: its zip (and the zip inside it, for the monthly bundles), its lump, the
  IWAD and PWADs it needs, the time it claims (from its file name and from its
  `.txt`), the time upstream `measured` (`null` until measured) and, for PWAD
  demos, the exact `pwad_files` it was measured with.
  Regenerate them with `support/index_archive.py` (it keeps the measurements),
  then measure with `support/measure_archive.py --reference <upstream dsda-doom>`.
  A PWAD name is often shared by unrelated WADs or by several versions, so the
  measuring tries every file of that name and records the one whose time equals
  a claimed time, else the largest one that finishes a level.
  The `Competn2005` and `Sda2005` tests exist only while the archive is
  unpacked. Each replays its demo straight from the zip, with those exact PWAD
  files, and expects the `measured` time; they run as 32 GTest shards per
  archive, labelled `archive`, because CTest takes minutes to discover twenty
  thousand tests one by one.
- `support/wads/idgames/` - a private mirror of the /idgames archive, never
  committed; `support/wads/idgames/update_mirror.py` creates and updates it.
  The PWADs of archive demos come from here, extracted into
  `support/wads/pwads/<zip path>/<file>` - including those in the old PKZIP
  "shrink" and "implode" formats, which `support/pkzip.py` decodes.
- `support/archive-missing-wads.txt` - the WADs archive demos need that neither
  `support/wads/` nor the idgames mirror has; their tests are skipped.
