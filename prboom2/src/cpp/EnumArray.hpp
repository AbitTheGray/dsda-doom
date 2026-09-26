// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
// `std::array` indexed by an enum instead of a number, so a table indexed by an enum does not need `std::to_underlying` at every access.

#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <type_traits>

#include "cpp/Designated.hpp"

/// The value an enum counts its values with: its `Count`, `COUNT` or `_COUNT`.
template<typename E>
requires std::is_enum_v<E>
inline constexpr E EnumCount = []
{
	if constexpr(requires { E::Count; })
		return E::Count;
	else if constexpr(requires { E::COUNT; })
		return E::COUNT;
	else if constexpr(requires { E::_COUNT; })
		return E::_COUNT;
	else
		static_assert(false, "The enum has no Count, COUNT or _COUNT; give EnumArray the count explicitly.");
}();

/// `std::array` of `T` with one value per enum value below `Count`, indexed by that enum.
/// `EnumArray<state_t, StateId::DoomCount>`, or `EnumArray<config_t, EnumCount<ConfigId>>`.
/// Takes a designated initializer list: `{ a, b, {At(E::X), x} }`.
template<typename T, auto Count>
requires std::is_enum_v<decltype(Count)>
class EnumArray
{
public:
	using Enum = decltype(Count);

	static constexpr std::size_t Size = DesignatedIndex(Count);

	constexpr EnumArray() = default;

	/// @throws std::out_of_range if an entry lands at or past `Count`
	constexpr EnumArray(const std::initializer_list<DesignatedEntry<T, Enum>> entries)
	{
		PlaceDesignated(entries, [this](const std::size_t index, const T& value)
		{
			values.at(index) = value;
		});
	}

	/// @pre `key` is below `Count`
	[[nodiscard]] constexpr T& operator[](const Enum key) noexcept
	{
		assert(DesignatedIndex(key) < Size);
		return values[DesignatedIndex(key)];
	}

	/// @pre `key` is below `Count`
	[[nodiscard]] constexpr const T& operator[](const Enum key) const noexcept
	{
		assert(DesignatedIndex(key) < Size);
		return values[DesignatedIndex(key)];
	}

	[[nodiscard]] static constexpr std::size_t size() noexcept { return Size; }

	[[nodiscard]] constexpr T* data() noexcept { return values.data(); }
	[[nodiscard]] constexpr const T* data() const noexcept { return values.data(); }

	[[nodiscard]] constexpr auto begin() noexcept { return values.begin(); }
	[[nodiscard]] constexpr auto begin() const noexcept { return values.begin(); }
	[[nodiscard]] constexpr auto end() noexcept { return values.end(); }
	[[nodiscard]] constexpr auto end() const noexcept { return values.end(); }

private:
	std::array<T, Size> values{};
};

static_assert([]
{
	enum struct Fruit : uint8_t
	{
		Apple,
		Pear,
		Plum,
		Count,
	};
	const EnumArray<int32_t, EnumCount<Fruit>> a = {{At(Fruit::Plum), 3}, {At(Fruit::Apple), 1}, 2};
	return a.size() == 3 && a[Fruit::Apple] == 1 && a[Fruit::Pear] == 2 && a[Fruit::Plum] == 3;
}());
static_assert([]
{
	enum struct Shape : uint8_t
	{
		Circle,
		Square,
		Triangle,
	};
	EnumArray<int32_t, Shape::Triangle> a;
	a[Shape::Square] = 4;
	return a.size() == 2 && a[Shape::Circle] == 0 && a[Shape::Square] == 4;
}());
