// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
// Containers that take C-style designated initializers.
// C's `{ 3, 1, [5] = 0, 2 }` is written `{ 3, 1, {At(5), 0}, 2 }`: the value goes to index 5, and the next plain value continues at index 6.
// Indices that are not given are value-initialized.
// A later entry overwrites an earlier one with the same index, as in C.

#pragma once

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

/// Index of a designated entry: `{At(index), value}`.
/// The index is an integer, or an enum for containers indexed by that enum.
template<typename K>
requires std::integral<K> || std::is_enum_v<K>
struct At
{
	K key;

	explicit constexpr At(const K a_key) noexcept
		: key(a_key)
	{
	}
};

/// Position of `key` in a container.
/// @throws std::out_of_range if `key` is negative
template<typename K>
requires std::integral<K> || std::is_enum_v<K>
[[nodiscard]] constexpr std::size_t DesignatedIndex(const K key)
{
	if constexpr(std::is_enum_v<K>)
	{
		return DesignatedIndex(std::to_underlying(key));
	}
	else
	{
		if(std::cmp_less(key, 0))
			throw std::out_of_range("Negative designated index");
		return static_cast<std::size_t>(key);
	}
}

/// One entry of a designated initializer list: a plain `value`, or `{At(key), value}`.
/// `Key` is the index type the container takes; any integer is accepted for an integer `Key`.
template<typename T, typename Key>
struct DesignatedEntry
{
	std::optional<std::size_t> index;
	T value;

	constexpr DesignatedEntry(T a_value)
		: value(std::move(a_value))
	{
	}

	/// A plain value written as its own braced list: `{190, 213, 226}` instead of `{{190, 213, 226}}`.
	/// The elements go straight to `T{...}`, so a braced list nested inside it (`{"a", {1, 2}}`) cannot be deduced and needs `At`.
	template<typename... Args>
	requires (sizeof...(Args) >= 1)
		&& (!std::same_as<std::remove_cvref_t<Args>, DesignatedEntry> && ...)
		&& requires(Args&&... a_args) { T{std::forward<Args>(a_args)...}; }
	constexpr DesignatedEntry(Args&&... a_args)
		: value{std::forward<Args>(a_args)...}
	{
	}

	template<typename K>
	requires std::same_as<K, Key> || (std::integral<Key> && std::integral<K>)
	constexpr DesignatedEntry(
		const At<K> a_at,
		T a_value
	)
		: index(DesignatedIndex(a_at.key)), value(std::move(a_value))
	{
	}
};

/// Hand each entry to `place(index, value)` in list order, with C's rule for indices: an entry without `At` goes right after the previous entry.
template<typename T, typename Key, typename Place>
constexpr void PlaceDesignated(
	const std::initializer_list<DesignatedEntry<T, Key>> entries,
	Place place
)
{
	std::size_t next = 0;
	for(const DesignatedEntry<T, Key>& entry : entries)
	{
		const std::size_t index = entry.index.value_or(next);
		place(index, entry.value);
		next = index + 1;
	}
}

/// `std::array` that takes a designated initializer list.
/// @throws std::out_of_range from the constructor if an entry lands at or past `N`
template<typename T, std::size_t N>
struct DesignatedArray : std::array<T, N>
{
	constexpr DesignatedArray() = default;

	constexpr DesignatedArray(const std::initializer_list<DesignatedEntry<T, std::size_t>> entries)
		: std::array<T, N>{}
	{
		PlaceDesignated(entries, [this](const std::size_t index, const T& value)
		{
			this->at(index) = value;
		});
	}
};

/// `std::vector` that takes a designated initializer list.
/// The size is one past the highest index an entry lands on.
template<typename T>
struct DesignatedVector : std::vector<T>
{
	using std::vector<T>::vector;

	constexpr DesignatedVector(const std::initializer_list<DesignatedEntry<T, std::size_t>> entries)
	{
		PlaceDesignated(entries, [this](const std::size_t index, const T& value)
		{
			if(index >= this->size())
				this->resize(index + 1);
			(*this)[index] = value;
		});
	}
};

static_assert([]
{
	const DesignatedArray<int32_t, 6> a = {3, 1, {At(4), 7}, 2};
	return a[0] == 3 && a[1] == 1 && a[2] == 0 && a[3] == 0 && a[4] == 7 && a[5] == 2;
}());
static_assert([]
{
	const DesignatedArray<int32_t, 3> a = {{At(2), 5}, {At(0), 4}, 6};
	return a[0] == 4 && a[1] == 6 && a[2] == 5;
}());
static_assert([]
{
	const DesignatedVector<int32_t> v = {3, 1, {At(5), 0}, 2};
	return v.size() == 7 && v[0] == 3 && v[1] == 1 && v[4] == 0 && v[5] == 0 && v[6] == 2;
}());
static_assert([]
{
	const DesignatedArray<std::array<int32_t, 3>, 3> a = {{At(1), {1, 2, 3}}, {4, 5, 6}};
	return a[0][0] == 0 && a[1][2] == 3 && a[2][0] == 4 && a[2][2] == 6;
}());
static_assert([]
{
	struct Pair
	{
		const char* name;
		int32_t value;
	};
	const DesignatedArray<Pair, 2> a = {{"one", 1}, {"two", 2}};
	return a[0].value == 1 && a[1].value == 2 && a[1].name[0] == 't';
}());
