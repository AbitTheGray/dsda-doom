// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <format>
#include <functional>
#include <limits>
#include <span>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <utility>

/**
 * Get number with only specified bit set.
 * @pre `v` is less than the number of bits of `T`
 */
template<typename T>
requires std::is_integral_v<T>
&&
std::is_unsigned_v<T>
[[nodiscard]]
inline constexpr T(Bit)(const T v) noexcept
{
	assert(std::cmp_less(v, std::numeric_limits < T > ::digits));
	return static_cast < T > (1u) << v;
}
static_assert(Bit<uint8_t>(0u) == 0b0000'0001u);
static_assert(Bit<uint8_t>(1u) == 0b0000'0010u);
static_assert(Bit<uint8_t>(2u) == 0b0000'0100u);
static_assert(Bit<uint8_t>(3u) == 0b0000'1000u);
static_assert(Bit<uint8_t>(4u) == 0b0001'0000u);
static_assert(Bit<uint8_t>(5u) == 0b0010'0000u);
static_assert(Bit<uint8_t>(6u) == 0b0100'0000u);
static_assert(Bit<uint8_t>(7u) == 0b1000'0000u);
/**
 * Get number with specified number of bits set.
 * @note Always least significant bits
 * @pre `v` is less than the number of bits of `T`
 */
template<typename T>
requires std::is_integral_v<T>
&&
std::is_unsigned_v<T>
[[nodiscard]]
inline constexpr T(Bits)(const T v) noexcept
{
	return Bit(v) - 1u;
}
static_assert(Bits<uint8_t>(0u) == 0b0000'0000u);
static_assert(Bits<uint8_t>(1u) == 0b0000'0001u);
static_assert(Bits<uint8_t>(2u) == 0b0000'0011u);
static_assert(Bits<uint8_t>(3u) == 0b0000'0111u);
static_assert(Bits<uint8_t>(4u) == 0b0000'1111u);
static_assert(Bits<uint8_t>(5u) == 0b0001'1111u);
static_assert(Bits<uint8_t>(6u) == 0b0011'1111u);
static_assert(Bits<uint8_t>(7u) == 0b0111'1111u);

/**
 * Get `value % modulo`, always in range `[0, modulo)`.
 * @pre `modulo` is positive
 */
template<typename TV, typename TM>
requires std::is_integral_v<TV>
&&
std::is_integral_v<TM>
[[nodiscard]]
inline constexpr TV PositiveModulo(const TV value, const TM modulo) noexcept
{
	assert(modulo > 0);
	return static_cast<TV>(((value % modulo) + modulo) % modulo);
}
static_assert(PositiveModulo(0, 2) == 0);
static_assert(PositiveModulo(1, 2) == 1);
static_assert(PositiveModulo(3, 2) == 1);
static_assert(PositiveModulo(0, 4) == 0);
static_assert(PositiveModulo(3, 4) == 3);
static_assert(PositiveModulo(-1, 4) == 3);
static_assert(PositiveModulo(4, 4) == 0);
static_assert(PositiveModulo(-4, 4) == 0);

/**
 * `std::format` into a fixed-size C string buffer, as `snprintf` does:
 * the text is cut to fit, and the buffer is always zero-terminated.
 * @pre `buffer` is not empty
 */
template<typename... Args>
inline void FormatTo(const std::span<char> buffer, const std::format_string<Args...> format, Args&&... args)
{
	assert(!buffer.empty());

	// Leave room for the terminating zero.
	const auto maximumLength = static_cast<std::ptrdiff_t>(buffer.size() - 1);
	const auto result = std::format_to_n(buffer.data(), maximumLength, format, std::forward<Args>(args)...);

	*result.out = '\0';
}

#define NOT_IMPLEMENTED { throw std::runtime_error("Not Implemented"); }

#define DOOM_NO_COPY(a_className) \
	a_className(const a_className& other) = delete;\
	a_className& operator=(const a_className& other) = delete;

#define DOOM_DEFAULT_COPY(a_className) \
	a_className(const a_className& other) = default;\
	a_className& operator=(const a_className& other) = default;

#define DOOM_NO_MOVE(a_className) \
	a_className(a_className&& other) noexcept = delete;\
	a_className& operator=(a_className&& other) noexcept = delete;

#define DOOM_DEFAULT_MOVE(a_className) \
	a_className(a_className&& other) noexcept = default;\
	a_className& operator=(a_className&& other) noexcept = default;

#define ENUM_FLAGS_FUNC(enum_name)\
	static_assert(\
		std::is_unsigned_v<std::underlying_type_t<enum_name>>,\
		"Underlying type of the enum must be an unsigned number. Specify uint8_t (or similar), don't leave it empty."\
	);\
	[[nodiscard]] inline constexpr enum_name operator |(const enum_name a, const enum_name b) noexcept\
	{\
		return static_cast<enum_name>(std::to_underlying(a) | std::to_underlying(b));\
	}\
	[[nodiscard]] inline constexpr enum_name operator -(const enum_name a, const enum_name b) noexcept\
	{\
		return static_cast<enum_name>(std::to_underlying(a) & ~std::to_underlying(b));\
	}\
	[[nodiscard]] inline constexpr enum_name operator &(const enum_name a, const enum_name b) noexcept\
	{\
		return static_cast<enum_name>(std::to_underlying(a) & std::to_underlying(b));\
	}\
	[[nodiscard]] inline constexpr enum_name operator ^(const enum_name a, const enum_name b) noexcept\
	{\
		return static_cast<enum_name>(std::to_underlying(a) ^ std::to_underlying(b));\
	}\
	inline constexpr enum_name& operator |=(enum_name& a, const enum_name b) noexcept\
	{\
		return a = a | b;\
	}\
	inline constexpr enum_name& operator -=(enum_name& a, const enum_name b) noexcept\
	{\
		return a = a - b;\
	}
#define ENUM_FLAGS_FUNC_OTHER(enum_name, other_name)\
	static_assert(\
		std::is_unsigned_v<std::underlying_type_t<enum_name>>,\
		"Underlying type of the enum must be an unsigned number. Specify uint8_t (or similar), don't leave it empty."\
	);\
	static_assert(\
		std::is_unsigned_v<std::underlying_type_t<other_name>>,\
		"Underlying type of the other enum must be an unsigned number. Specify uint8_t (or similar), don't leave it empty."\
	);\
	[[nodiscard]] inline constexpr enum_name operator |(const enum_name a, const other_name b) noexcept\
	{\
		return a | static_cast<enum_name>(::Bit(std::to_underlying(b)));\
	}\
	[[nodiscard]] inline constexpr enum_name operator -(const enum_name a, const other_name b) noexcept\
	{\
		return a - static_cast<enum_name>(::Bit(std::to_underlying(b)));\
	}\
	[[nodiscard]] inline constexpr bool operator &(const enum_name a, const other_name b) noexcept\
	{\
		return (a & static_cast<enum_name>(::Bit(std::to_underlying(b)))) != enum_name{};\
	}\
	inline constexpr enum_name& operator |=(enum_name& a, const other_name b) noexcept\
	{\
		return a = a | b;\
	}\
	inline constexpr enum_name& operator -=(enum_name& a, const other_name b) noexcept\
	{\
		return a = a - b;\
	}

#define DOOM_STD_HASH(a_type, a_implementation) \
template<>\
struct std::hash<a_type>\
{\
	[[nodiscard]] inline std::size_t operator()(const a_type& value) const noexcept\
	{\
		a_implementation\
	}\
};

#define DOOM_STD_LESS(a_type, a_implementation) \
template<>\
struct std::less<a_type>\
{\
	[[nodiscard]] inline bool operator()(const a_type& left, const a_type& right) const noexcept\
	{\
		a_implementation\
	}\
};

#define DOOM_STD_FORMATTER(a_type, a_func_value) \
template<>\
struct std::formatter<a_type, char>\
{\
	template<class ParseContext>\
	constexpr typename ParseContext::iterator parse(ParseContext& ctx)\
	{\
		auto it = ctx.begin();\
		if(it != ctx.end() && *it != '}')\
		{\
			throw std::format_error("No formating supported");\
		}\
		return it;\
	}\
	template<typename FormatContext>\
	typename FormatContext::iterator format(const a_type& value, FormatContext& ctx) const\
	{\
		std::ostringstream out;\
		out << a_func_value;\
		\
		return std::ranges::copy(out.str(), ctx.out()).out;\
	}\
};
#define DOOM_STD_FORMATTER_TOSTRING(a_type) DOOM_STD_FORMATTER(a_type, to_string(value))
