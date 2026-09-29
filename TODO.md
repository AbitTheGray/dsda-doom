# TODO

The final goal is to separate the gameplay part from the window. We want to be able to run the game headless, to render it into a video stream (using `ffmpeg`) without a window...
For that, we need to eliminate global-scope variables and everything `extern`.
We should end up with a library and an executable projects.

Things below are noted for later.
Nothing here is being worked on right now.

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

13 source files still use C's `FILE*` (`M_OpenFile`/`fopen`, `fprintf`, `fclose`).
Replace them with `std::ofstream` / `std::ifstream`, which close themselves (RAII), and format the text with `std::format` instead of `printf`-style strings.

The written files must not change:
- keep each file's mode: `"wb"` becomes `std::ios::binary` (e.g. `levelstat.txt` writes `\r\n` on every platform), `"w"` stays text;
- print flags as `{:d}`, because `std::format` writes a `bool` as `true`/`false`;
- `M_OpenFile` converts the UTF-8 name to wide on Windows; open streams from a `std::filesystem::path` built with `std::u8string` so non-ASCII paths keep working.

## `dsda_StringConfig` storage and return type

`dsda_StringConfig` still returns `const char*`.
The value is a `char*` in the `dsda_config_value_t` union, managed by hand (`Z_Strdup`/`Z_Free`) and replaced by `dsda_UpdateStringConfig` and `dsda_HackStringConfig`.
Most of its 22 callers need a zero-terminated string: `sscanf` (resolutions in `SDL/i_video.cpp`), `strcasecmp` (`MUSIC/portmidiplayer.cpp`, `SDL/i_sound.cpp`), `strncpy`/`M_CopyText`/`Z_Strdup` (menu, console), `M_remove`/`M_CheckWritableDir`, `parsecommand` (capture), and the FluidSynth, PortMidi and SDL settings; several keep the pointer in a `const char*` global.

Plan: store the value as a `std::string` (replacing the union), return a `std::string_view` that is valid until that config is updated, and convert the callers step by step.
Where a C API needs a zero-terminated string, the caller makes a `std::string` from the view; these are read at startup or when a setting changes, so the copies are cheap.

## Buffers as `std::span`

Functions that take a buffer as a pointer plus a separate length should take one `std::span` (or `std::string_view` for text), so the two cannot disagree and callers pass what they hold.
- **Read-only data** (`std::span<const std::byte>` or `std::span<const unsigned char>`): `dsda_ParseUDMF` (`dsda/udmf.hpp`), `ParseUMapInfo` (`umapinfo.hpp`), `G_StartDemoPlayback`, `G_ReadDemoHeaderEx` (`g_game.hpp`), `dsda_AttachPlaybackStream` (`dsda/playback.hpp`), `dsda_WriteToDemo`, `dsda_WriteQueueToDemo`, `dsda_WriteTicToDemo`, `dsda_DemoMarkerPosition` (`dsda/demo.hpp`), `I_RegisterSong` (`i_sound.hpp`), `ReadPWADTable` (`wadtbl.hpp`).
  Most callers pass `W_LumpByNum(lump)` with `W_LumpLength(lump)`; a helper returning a lump as a span would cover them.
- **Text**: `Scanner(const char* data, int length = -1)` (`-1` means "measure it") and the DEHACKED key lookups `dsda_GetDehSFXIndex`, `dsda_GetDehMusicIndex` take `std::string_view`.
- **Output buffers** (`std::span<char>`, written with `FormatTo` from `cpp/Util.hpp`): the HUD components' `(char* str, size_t max_size)` (`dsda/hud_components/*.hpp`) and `M_getcwd`.

`memio.hpp` mirrors C's `fread`/`fwrite` on purpose and stays as it is.
Mind the length types: several take `int`, the span's size is `size_t`.

## Enums still written as `#define`

Groups of `#define`s that are really an enum or a set of flags, to become `enum struct`s (see the enum rules in `CLAUDE.md`).
Done so far: `MTF_*` (now `MapThingFlag`), `UDMF_TF_*` (`UdmfThingFlag`), and `SKILL4`/`SKILL5` (local constants, because `gameskill` is an open index).
Convert a few groups per batch, then build and run the spec suite.

Candidates found by scanning for 3+ adjacent numeric `#define`s with a shared prefix; check each one, some may turn out to be plain constants:
- **Flags:** `SCROLL_` (`dsda/scroll.hpp`: two flag sets and the untracked `THRUST_` group share `scroll_t::flags`, which is part of the savegame layout, so one enum cannot type that field), `RDC_` (`r_draw.cpp`: only used in `#if` inside `r_drawcolumn.inl`/`r_drawflush.inl`, which `r_draw.cpp` includes once per pipeline - it needs those files turned into templates with `if constexpr`, not just an enum).
- **Flags with a packed field** (need extractor functions): `AFLAG_` (`doomdef.hpp`: `ticcmd_t::arti` holds an artifact id, these two flags around it, or the sentinels `0xff`/`HexenCount`, and it is a demo byte, so it needs a design first).
- **Plain enumerations:** `PRB_MB_` (`e6y.hpp`; really a packed Windows `MessageBox` type - a button set plus `DEFBUTTON` bits - and only `PRB_MB_OK` is used).

The scan misses two-entry groups like the old `SKILL4`/`SKILL5`, so expect a few more.

Found by a later scan that also counts 2-entry groups; not yet checked:
- **Larger groups:** `UDMF_SCROLL_`/`UDMF_THRUST_` (`dsda/udmf.hpp`: the UDMF side of the `SCROLL_`/`THRUST_` flags above, best done together with them).
- **Single-player intermission states** (`wi_stuff.cpp`): the `SP_KILLS`...`SP_PAUSE` `#define`s are unused; `sp_state` is stepped with `++` and odd values are the pauses between counters (`sp_state & 1`), so it needs a design first, not just an enum.
- **Checked, plain constants:** `USE_*_AMMO_*` (`doomdef.hpp`) and `AMMO_*_WIMPY`/`_HEFTY` (`p_mobj.hpp`) are ammo amounts, the rest of `MENU_MOUSE_` (`m_mouse.inl`), `DM_`/`SP_` coordinates (`wi_stuff.cpp`) and `TALLY_` (`hexen/in_lude.cpp`) are layout sizes, `SORCBALL_`/`SORC_DEFENSE_`/`KORAX_` (`p_enemy.cpp`) are speeds, heights, times and TIDs, `STAIR_` (`p_floor.cpp`) and `LUMP_NOT_FOUND` (`w_wad.hpp`) are a sector type, a queue size and a sentinel index, `OPL_` (`MUSIC/opl.hpp`) are OPL register addresses and sizes, and the `ST_` sizes (`st_stuff.cpp`), `GENMIDI_NUM_` (`MUSIC/oplplayer.cpp`), `SS_` (`hexen/sn_sonix.cpp`), `MAX_ACS_` (`hexen/p_acs.hpp`) and `LIGHTNING_SPECIAL`/`LIGHTNING_SPECIAL2` (`hexen/p_anim.cpp`, line specials) are counts and ids; none of them is an enum.
- **Two-entry groups:** `LUMP_STATIC`/`LUMP_PRBOOM` (`w_wad.hpp`, lump flags), `BF_FAILURE`/`BF_SUCCESS` (`dsda/brute_force.cpp`, a success result), `PL_SKYFLAT_` (`r_plane.hpp`: two flag bits packed with a sky index in one `int`, so it needs extractor functions), `MOBJ_NULL`/`MOBJ_XX_PLAYER` (`hexen/sv_save.cpp`: negative sentinels in the saved mobj index, so the savegame format is involved).

## Remaining `#define`s

Go through all `#define`s and convert each one to either a variable or a function.
A `#define` that only names a value (a size, a speed, a coordinate, like the `TALLY_` or `KORAX_` constants) becomes a `constexpr` variable.
A function-like `#define` becomes an `inline` (or `constexpr`) function.
Only what really has to be a macro (e.g. build configuration tested in `#if`, or text pasting) stays one.

## Review keypad digits in typed input

Keypad keys are `0x100 +` their character (`KeyCode::Keypad1` is `0x100 + '1'`), and three places treat them differently, all kept exactly as upstream for now:
- **Cheats** (`M_FindCheats`, `m_cheat.cpp`): the key is truncated to a `char`, so keypad digits type digits (`idclev` on the keypad works).
- **Numeric setup entry** (`M_SetupCommonSelectResponder`, `m_menu.cpp`): only the low byte is tested with `isdigit` and stored, so keypad digits are accepted. The keypad minus is not, because the `'-'` test compares the whole code.
- **Weapon-number entry** (`M_WeaponResponder`, `m_menu.cpp`): the whole code minus `'0'` must be 1 to 9, so keypad digits are rejected.

Decide whether the inconsistency is a bug. If it is, fix it (probably one helper that maps keypad digits and minus to their characters) and log the change in `Compatibility.md`.

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

## Explain archive-listed vs measured times

In `spec/support/archive/*.json`, the time upstream `measured` differs from the listed `time` for 985 Compet-N and 1500 SDA demos (433 and 1151 of them measured `00:00`, i.e. no level finished).
The suite compares against `measured`, so nothing fails, but the differences are unexplained.
Sort them into causes - a wrong PWAD version, a demo that does not finish a level on purpose or desyncs, a zip whose one listed time covers several demos (e.g. SDA's `0-a/10lvls.zip`), a different way of counting time (e.g. Compet-N's `doom/built/ep2-0428.zip`: listed 4:28, measured 5:02) - and record the cause per demo, so the real desyncs stand out.
