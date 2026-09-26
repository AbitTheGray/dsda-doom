# TODO

The final goal is to separate the gameplay part from the window. We want to be able to run the game headless, to render it into a video stream (using `ffmpeg`) without a window...
For that, we need to eliminate global-scope variables and everything `extern`.
We should end up with a library and an executable projects.

Things below are noted for later.
Nothing here is being worked on right now.

## Move the spec's report types into the game

The report struct is already shared (`dsda/analysis_report.hpp`), but
`spec/src/Category.hpp` still mirrors the game by hand: the game has no enum behind
the category (`dsda_DetectCategory` returns bare string literals), and its tracker
state is loose `extern` globals in `dsda/analysis.hpp`. The output of
`analysis.txt` (and the category in the text file) must not change. Two steps:

1. **Shared category enum.** Move `Category` and its `to_string` into the game.
   `dsda_DetectCategory` returns the enum; its two callers (`dsda_WriteAnalysis`,
   `text_file.cpp`) print it with `to_string`. Its side effects on the tracker
   (clearing `almost_reality`, `stroller`, `weapon_collector`; setting `nomo`,
   `respawn`, `fast`) must stay.
2. **Tracker globals into structs.** The 27 `extern`s in `dsda/analysis.hpp` are
   the running state, not the report. Group them by how they are reset, so each
   reset becomes `= {}` without resetting anything new: the run stats
   (`dsda_ResetAnalysis`), the per-map kill tracking (`kills_on_map`,
   `100k_on_map`, `100k_note_shown`) and the other note flags.

## Tables left on C designators

These still use clang's C99 designator extension (`[x] = ...`), because `EnumArray` / `DesignatedArray` do not fit them yet.
Once they are gone, `-Wno-c99-designator` can be dropped from `cmake/DsdaTargetFeatures.cmake`.

- **`fake_contrast_list` in `m_menu.cpp`.** The menu takes it as a `nullptr`-terminated `const char**`, and the terminator sits past the enum's range.
- **`doom_S_sfx` in `sounds.cpp`.** It would be an `EnumArray<sfxinfo_t, SfxId, SfxId::DoomCount>` (its `[500]`...`[699]` are `SfxId::Fre000`...`Fre199`), but `sfxinfo_t::link` is a raw pointer into the table itself (`dschgun` links to the pistol, and every `doom_disambiguated_sfx` entry links into it), which a constant initializer cannot form.
  Plan: make `link` an `SfxId` (`SfxId::None` = no link) and resolve it in `I_GetSfxLumpNum` (its only reader) through the table passed to `dsda_InitializeSFX`, not through `S_sfx`.
  Once DEHEXTRA grows the sound table, `dsda/sfx.cpp` moves `S_sfx` to a heap copy that DEHACKED edits, while the pointers keep reading the original static table; resolving through `S_sfx` would change which name a link reads.
  At the same time, drop the `typedef struct sfxinfo_struct sfxinfo_t;` and declare it as `struct sfxinfo_t`.
- **`components_template` in `dsda/exhud.cpp`.** Its entries mix positional fields with `.default_vpt = ...`, which is a C extension in C++ as well.

## Remove undefined behavior

UBSan has been run before, and only enough UB was fixed to make the test demos sync (see the signed-overflow entry in `Compatibility.md`).
Other instances of the same kinds remain throughout the code and should be removed.

## C file IO to C++ streams

About 15 source files still use C's `FILE*` (`M_OpenFile`/`fopen`, `fprintf`, `fclose`).
Replace them with `std::ofstream` / `std::ifstream`, which close themselves (RAII), and format the text with `std::format` instead of `printf`-style strings.

The written files must not change:
- keep each file's mode: `"wb"` becomes `std::ios::binary` (e.g. `levelstat.txt` writes `\r\n` on every platform), `"w"` stays text;
- print flags as `{:d}`, because `std::format` writes a `bool` as `true`/`false`;
- `M_OpenFile` converts the UTF-8 name to wide on Windows; open streams from a `std::filesystem::path` built with `std::u8string` so non-ASCII paths keep working.

`dsda_WriteAnalysis` in `dsda/analysis.cpp` is already converted and shows the pattern.
