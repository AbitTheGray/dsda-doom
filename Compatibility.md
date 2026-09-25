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
