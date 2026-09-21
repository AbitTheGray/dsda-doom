# TODO

Things noted for later. Nothing here is being worked on right now.

## Move the spec's report types into the game

`spec/src/Analysis.hpp` and `spec/src/Category.hpp` currently mirror the game by
hand: the game has no struct behind `analysis.txt` (just loose `extern` globals in
`dsda/analysis.h`) and no enum behind the category (`dsda_DetectCategory` returns
bare string literals). Once those areas are rewritten, the struct and the
`enum struct` should live in `prboom2/src/dsda/` so the writer and the spec's
parser share one definition. Keeping the duplicates until then.

## A missing autoload WAD crashes instead of exiting

Noted 2026-09-21, dsda-doom v0.29.4.

A dangling symlink in `~/.dsda-doom/autoload/doom-all/` gave this, exit code 255:

```
W_AddFile: couldn't open /home/abit/.dsda-doom/autoload/doom-all/DOOM2.WAD
The game has crashed!
Please report the following information: Segmentation fault (0x0000)
```

Autoload entries are not checked for existence before being queued
(`d_main.c:1884`), unlike `-file`, so a dangling one reaches `I_Error` in
`W_AddFile` (`w_wad.c:132`). The fault itself is in one of the exit handlers
`I_SafeExit` then runs (`SDL/i_main.c:149`) — which one is unidentified.
`-verbose` logs each handler's name, so the last line before the crash should
name it. Unknown whether this is pre-existing upstream.

It matters because the crash lands after `analysis.txt` is written but before any
tic plays, so the demo suite read a report of a run that never happened and
reported ~35 assertion failures instead of one error. The suite now passes
`-noautoload`, which hides this from the tests but does not fix it for normal play.

To reproduce: symlink something nonexistent into `~/.dsda-doom/autoload/doom-all/`
and start with a Doom IWAD and no `-noautoload`.
