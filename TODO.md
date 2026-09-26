# TODO

Things noted for later. Nothing here is being worked on right now.

## Move the spec's report types into the game

`spec/src/Analysis.hpp` and `spec/src/Category.hpp` currently mirror the game by
hand: the game has no struct behind `analysis.txt` (just loose `extern` globals in
`dsda/analysis.h`) and no enum behind the category (`dsda_DetectCategory` returns
bare string literals). Once those areas are rewritten, the struct and the
`enum struct` should live in `prboom2/src/dsda/` so the writer and the spec's
parser share one definition. Keeping the duplicates until then.

## Remove undefined behavior

UBSan has been run before, and only enough UB was fixed to make the test demos sync (see the signed-overflow entry in `Compatibility.md`).
Other instances of the same kinds remain throughout the code and should be removed.
