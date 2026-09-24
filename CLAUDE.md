# Working agreement

## Go slow
We are rewriting a project from old C to Modern C++. We do not need to solve everything at once.

## KISS
Keep everything simple and readable.
Prefer the obvious solution over the clever one. Do not add abstraction, indirection or configuration that the current task does not need.

## Do not start or reload the project without explaining why
Do not run any CMake commands unless necessary.

## Explain shell commands before running them
Every Bash (or other shell) call gets a leading comment stating the goal of that command.

## No `using namespace` in global scope
Use of `using namespace` is allowed only inside a scope that does not pollute anything outside of it - it should not affect anything outside the current file.
Avoid `using namespace std` as it decreases readability.

## Maintain compatibility
The whole point is to keep maximum compatibility with the original project while rewriting it into a readable C++ code.

## Use Exceptions
It is OK to use exceptions if it is not to control code flow.
In performance-critical code, or where exceptions do not make sense, use `std::expected`. Do not let `std::expected` propagate too far (in too many parents).

## Git
Do not create branches, worktrees or commits unless asked.
Explain what you are doing before running any git command.
Unless explicitly asked, do not switch to worktrees. If you really have to, pause, explain and ask.

## Platform Independence
Do not use platform-specific code.
Our compiler-of-choice is CLang, for both Windows and Linux.

## Be explicit about integer size and signess.
Use `int32_t` instead of `int`.
Do not use the `signed` and `unsigned` keywords, we have `int32_t` and `uint32_t` for that (and equivalents to other sizes).

## Use Modern C++26
Use features from C++26 where possible.

## Follow RAII
Resource Acquisition Is Initialization is necessary to have a good memory safety and ownership. Use it where possible.

## Header file guard
Do not use `#ifndef`+`#define` to prevent multiple isntances of a header, use `#pragma once`

## C++ enums instead of C one
Use `enum struct` (not `enum class`) for structs.
Add helper functions to "flags" enum using `ENUM_FLAGS_FUNC(enum_name)` macro and use `Bit<uint8_t>(0u)` to get a specific bit (it is just `1 << 0u` but type-checked).
It needs `prboom2/src/cpp/Util.hpp`.
Always specify underlaying type.
`ENUM_FLAGS_FUNC` requires an unsigned underlying type, and it has to sit outside any `extern "C"` block.
If an enum mixes flag bits with a packed field (a mask plus a shift), give it named extractor functions instead of repeating the mask-and-shift at every use.
A list terminated by a bare `-1` or `0` gets a named enumerator (`End`, `None`) so the list stays typed.

## Use ranges and views
Use modern ranges and views where possible. They give us capabilities and readability of LINQ in C++.

## Use `std::array` instead of C-style array
Do not use C-style arrays, use other containers (especially `std::array`) where possible.

## Use `std::inplace_vector` instead of an array and "used indices"
When you have a fixed-length array and index into it indicating how many entries we've used so far, that is mimicking an Inplace Vector.
We have access to `std::inplace_vector` so use that.

## Implement `std::hash` and `std::less`
For types where it makes sense, let's implement a hashing and comparison function using `DOOM_STD_HASH(a_type, a_implementation)` (input is as `value`) and `DOOM_STD_LESS(a_type, a_implementation)` (inputs are `left` and `right`, both `a_type`).
It needs `prboom2/src/cpp/Util.hpp`.

## Use `std::string_view` and `std::string` instead of raw `const char*`
Use owning `std::string` if we own it, and `std::string_view` if we don't but can trust its lifetime.
`std::string_view` is especially good for procedure arguments as it allows some processing in parent without allocating new strings.

## Implement `std::formatter` override
For types where it makes sense, let's implement a formatter so it is easier to write it out.
```cpp
template<>
struct std::formatter<a_type, char>
{
	template<class ParseContext>
	constexpr typename ParseContext::iterator parse(ParseContext& ctx)
	{
		// Argument processing here
	}
	template<typename FormatContext>
	typename FormatContext::iterator format(const a_type& value, FormatContext& ctx) const
	{
		// Formatting here
	}
};
```
It is OK to just call `DOOM_STD_FORMATTER` if you have a function to call. If that function is a `to_string`, use `DOOM_STD_FORMATTER_TOSTRING`.
It needs `prboom2/src/cpp/Util.hpp`.

## Explicitly deleted copy and move
Use `DOOM_NO_COPY` and `DOOM_NO_MOVE` if you want to explicitly disable copy and/or move (both construction and assignment).

## Not implemented exception
If we do not have time to implement something, put `NOT_IMPLEMENTED` there. It contains an exception and is an explicit mark for us.

## Do not pre-declare variables
Do not start procedures with declaring all variables. That is an old thing from C, we do not want that.
Not even declaring the object for `for` loop, unless it needs to survive the loop's scope.

## Use C++ casts, not C casts
Avoid using C-style casts like `(int)`, be explicit like `static_cast<int>`.

## Type the holder, not the use site
When a variable, field or parameter only ever holds one enum, change its type to that enum.
Converting at every use is the wrong fix - it keeps the old type and adds noise.
Only convert where the integer really is the storage or interface format.

## Use `std::to_underlying` to get the number out of an enum
Where a conversion is genuinely needed, use `std::to_underlying(value)` rather than `static_cast`.
It needs `<utility>`.

## Never include a header from inside `extern "C"`
Every header here guards its own linkage, so the wrapper is redundant - and it gives the standard library C linkage, which fails to compile.
Put the includes above the `extern "C"` block.
