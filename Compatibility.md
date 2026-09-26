# Compatibility

Deliberate differences in behaviour from upstream dsda-doom.
Anything not listed here should behave exactly as upstream does.

## Fix signed integer overflow
From official [MR 999](https://github.com/kraflab/dsda-doom/pull/999).

Overly-active compiler could optimize integer overflow, causing a demo desync for one of tested demo files.
The fix is somewhat simple (use unsigned `angle_t` and convert it to signed only at the end) but necessary.

There are other similar Undefined Behaviors throughout the code. We fixed just enough to have working demo (and a little on top of it), not all instances.

## Unreadable autoload files are skipped

A `.wad`, `.lmp` or `.zip` found in an autoload directory that cannot be read, a
dangling symlink or a file without read permission, is skipped with a warning:

```
Skipping unreadable /home/user/.dsda-doom/autoload/doom-all/DOOM2.WAD
```

Upstream stops the game instead: `W_AddFile: couldn't open ...` for a WAD, or the
not-found error from `I_RequireZip` for a ZIP. On POSIX the WAD case also
segfaulted while exiting, which has been fixed separately.

Only files found by scanning the autoload directories are affected. A file named
on the command line (`-file`, `-iwad`, ...) that cannot be opened still ends the
game, as upstream.

Code: `IsReadableOrWarn` in `prboom2/src/d_main.cpp`, called from
`LoadWADsAtPath` and `LoadZIPsAtPath`.

## Game controller button names are bounds-checked

Upstream checks the button number against `sizeof(button_names)`, the table's size in bytes rather than its number of entries.
A button number from 23 up to the byte size read past the table; it now gets the name `misc`, like any unknown button.
Only the name shown in the menu is affected, not input or demos.

Code: `dsda_GameControllerButtonName` in `prboom2/src/dsda/game_controller.cpp`.

# To report upstream

Behaviour that looks like an upstream bug, kept as it is here for compatibility.

## `analysis.txt` writes `signature -1`

Every flag in `analysis.txt` is written as 0 or 1, and `signature` reads like one of
them. It has a third value: `-1` when the
demo's `FEATURES` lump is malformed or its signature does not match (`0` means there
is no lump, `1` a valid signature). A reader that treats it as a flag takes `-1` as
set, so a demo with a bad signature passes as signed. Our own spec suite did exactly
that until it was fixed.

We still write `-1`. Upstream could document the three values, or split them into
separate keys.

Code: `DemoEx_GetFeatures` in `prboom2/src/dsda/exdemo.cpp` sets the value
(`Signature` in `prboom2/src/dsda/exdemo_signature.hpp`), and `dsda_WriteAnalysis`
in `prboom2/src/dsda/analysis.cpp` writes it.
