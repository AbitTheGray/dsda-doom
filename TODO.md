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

## The mobj flags are still `#define`s, not `enum struct`

Noted 2026-09-24, after every C-style enum in `prboom2/src` had been converted.

`MF_*` (44 macros, `p_mobj.hpp:114` onwards) and `MF2_*` (51 macros,
`p_mobj.hpp:454` onwards) are the last large group of flag constants left as
macros, so the enum sweep never saw them. Both are already `uint64_t` and both
holders (`mobj_t::flags`, `mobj_t::flags2`) are `uint64_t`, so
`enum struct MobjFlag : uint64_t` satisfies `ENUM_FLAGS_FUNC`'s unsigned
requirement and keeps the width `P_SAVE_TYPE_REF` memcpy's for `mobj_t`.

Two reasons this is bigger than it looks:

- **~3600 use sites** (2567 `MF_`, 1027 `MF2_`), nearly all boolean tests of the
  form `if(mo->flags & MF_FLOAT)`. Every one has to be rewritten once `&`
  returns the enum, and that rewrite is what silently dropped an outer `!` in
  two places during the enum sweep. Do it in per-file batches and diff each
  against `git show HEAD:<file>` for negation parity; a clean build does not
  prove it.
- **`MF_` is a hybrid enum.** `MF_TRANSLATION` (`p_mobj.hpp:197`) is a two-bit
  mask with `MF_TRANSSHIFT = 26` beside it, so it needs named extractors the way
  `buttoncode_t` did, not a flag test. 15 sites shift through it.

Take `MF2_` first: half the sites and no packed field, so it is a rehearsal for
`MF_`.

### While converting, fix the signed shift in `p_inter.cpp:1005`

```c
target->flags &= ~(7 << MF_TRANSSHIFT); //no translation
```

`MF_TRANSLATION` is two bits, but the literal is `7`, so this clears bits 26-28.
Bit 28 is `MF_UNUSED2` (`p_mobj.hpp:203`), so nothing real is lost today.

The fragile part is the type: `7 << 26` is a signed `int`, so `~` produces a
negative `int` that sign-extends to `0xFFFFFFFFE3FFFFFF` when widened to the
`uint64_t` field, which is why the flags above bit 31 (`MF_TOUCHY` at
`p_mobj.hpp:213` and everything after it) survive. Write the literal as unsigned
and `~` would stay 32-bit, zeroing bits 32-63 and clearing those flags instead.
Converting to a flags enum removes the trap, because `-=` expands to `a & ~b` at
the enum's own width; the operand should be `MF_TRANSLATION` rather than `7`
either way.

