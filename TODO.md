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

## Enums still written as `#define`

Groups of `#define`s that are really an enum or a set of flags, to become `enum struct`s (see the enum rules in `CLAUDE.md`).
Done so far: `MTF_*` (now `MapThingFlag`), `UDMF_TF_*` (`UdmfThingFlag`), and `SKILL4`/`SKILL5` (local constants, because `gameskill` is an open index).
Convert a few groups per batch, then build and run the spec suite.

Candidates found by scanning for 3+ adjacent numeric `#define`s with a shared prefix; check each one, some may turn out to be plain constants:
- **Flags:** `ML_` (`doomdata.hpp`), `CF_` (`d_player.hpp`), `CF_` (`dsda/console.cpp`), `WIF_` (`d_items.hpp`), `DF_` (`dsda/demo.cpp`), `XC_` (`dsda/excmd.hpp`), `VF_` (`dsda/map_format.hpp`), `WI_SHOW_NEXT_` (`dsda/mapinfo.hpp`), `PAUSE_` (`dsda/pause.hpp`), `SCROLL_` (`dsda/scroll.hpp`), `SI_` (`dsda/skill_info.hpp`), `UDMF_ML_`, `UDMF_SF_`, `UDMF_SECF_` (`dsda/udmf.hpp`), `DEMOHEADER_` (`g_game.cpp`), `PT_` (`p_maputl.hpp`), `SHARDSPAWN_` (`p_pspr.cpp`), `STAIR_`, `TELF_` and the other groups after `NO_CRUSH` (`p_spec.hpp`), `NO_TOPTEXTURES`... (`r_defs.hpp`), `SF_`, `RF_` (`r_defs.hpp`), `RDC_` (`r_draw.cpp`), `TI_` (`d_deh.cpp`), `S_` menu item flags (`m_menu.cpp`, ~1800 uses).
- **Flags with a packed field** (need extractor functions): `HML_`/`ZML_` (`doomdata.hpp`, the `SPAC` bits), `AFLAG_` (`doomdef.hpp`), `ZDOOM_*_MASK` (`p_spec.hpp`).
- **Plain enumerations:** `MCMD_` (`dsda/mapinfo/hexen.cpp`), `PRB_MB_` (`e6y.hpp`), `GLDWF_`, `SKY_` (`gl_intern.hpp`), `WGLSTATE_` (`p_floor.cpp`), `LIGHT_SEQUENCE*` (`p_spec.hpp`), `SIL_` (`r_defs.hpp`), `KEYD_` (`doomdef.hpp`).

The scan misses two-entry groups like the old `SKILL4`/`SKILL5`, so expect a few more.

## Old PKZIP compression methods in zip loading

The game unpacks zips given to `-file` through libzip (`dsda_UnzipFile` in `dsda/zipfile.cpp`), and libzip reads neither of two methods that 1990s zips still use:
- **"Shrink" (method 1):** LZW with codes growing from 9 to 13 bits and a "partial clear" that frees the entries nothing else extends. 54 files in the idgames mirror and the Compet-N archive use it, 10 of them WADs.
- **"Implode" (method 6):** a 4 or 8 KB sliding window with Shannon-Fano coded literals, lengths and distances. 782 files use it, WADs and demos among them.

Such a zip cannot be loaded today: `-file levels/doom/p-r/pggm.zip` stops with `dsda_WriteZippedFilesToDest: Failed to open zipped file PGGM.WAD.`
The game should decode both itself for the members libzip reports as unsupported (`zip_compression_method_supported`).
`spec/support/pkzip.py` is a working reference: it decodes all 836 of those files with matching CRC-32s, the two shrunk WADs `JACKINBX.WAD` and `PGGM.WAD` byte for byte as Info-ZIP's `unzip` does.
The trap it documents: a shrink entry is only (the code it extends, one byte) and its string must be rebuilt from that chain when used, because a partial clear can free a code that later entries still extend.
The spec suite's archive test has the same gap (it reads demos through libzip too) and skips those demos for now.

Approach - keep libzip for the container and decode only these members ourselves:
1. Ask libzip whether it can unpack a member: `zip_compression_method_supported(method, 0)`.
2. If not, open it with `zip_fopen_index(..., ZIP_FL_COMPRESSED)`, which hands over the raw compressed bytes.
3. Unshrink or explode them, then compare the result with the CRC-32 from `zip_stat`; a mismatch is an error, never silently wrong data.

Why not the alternatives:
- **Adding the methods to libzip:** it has no public API to register decompressors, so it would mean a fork (breaking the system and vcpkg packages we build against) or an upstream contribution we cannot count on.
- **Another library:** libarchive recognises methods 1 and 6 but does not decompress them, and neither does minizip-ng; 7-Zip does, but it is a large LGPL C++ codebase, not a library.

The decoders: a C++ port of `spec/support/pkzip.py` (about 250 lines), taking a `std::span` of bytes and returning `std::expected<std::vector<std::byte>, ...>` - corrupt data is an expected outcome here, not an exceptional one.
Hans Wennborg's write-up (https://www.hanshq.net/zip2.html) and its C code, hwzip, are a good second reference and also cover "Reduce" (methods 2-5), which neither archive uses; check hwzip's license before copying any of it.

Build it as one component that both the game (`dsda/zipfile.cpp`) and the spec suite (`ArchiveTest`) use, so one implementation closes both gaps.
Test it with spec unit tests that decode known files and compare CRC-32s - the two shrunk WADs above and all four implode variants (4 or 8 KB window, with or without a literal tree) - skipped when the idgames mirror is absent; the archive test then replays the demos it skips today.
