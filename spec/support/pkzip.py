# SPDX-License-Identifier: GPL-2.0-or-later

"""
Old PKZIP compression methods that Python's `zipfile` cannot read.

1990s zips - Compet-N demos and idgames WADs alike - still use two of them,
both described in PKWARE's APPNOTE.TXT:
- "shrink" (method 1): LZW with codes growing from 9 to 13 bits, where a
  "partial clear" frees the dictionary entries nothing else builds on;
- "implode" (method 6): a sliding window of 4 or 8 KB, with literals, match
  lengths and match distances coded by Shannon-Fano trees stored at the start
  of the data.

`read_member` reads any zip member - whatever `zipfile` supports, plus these -
and checks the result against the CRC-32 the zip stores.
"""

import struct
import zipfile
import zlib

SHRUNK = 1
IMPLODED = 6


class _Bits:
	"""Reads the compressed data least significant bit first, as both methods store it."""

	def __init__(self, data, position=0):
		self.data = data
		self.position = position
		self.buffer = 0
		self.count = 0

	def _fill(self, needed):
		while self.count < needed:
			# Past the end reads zeros: decoding may look ahead further than the last code.
			byte = self.data[self.position] if self.position < len(self.data) else 0
			self.position += 1
			self.buffer |= byte << self.count
			self.count += 8

	def read(self, bits):
		self._fill(bits)
		value = self.buffer & ((1 << bits) - 1)
		self.buffer >>= bits
		self.count -= bits
		return value

	def decode(self, tree):
		"""One symbol of a Shannon-Fano tree built by `_tree`."""
		table, longest = tree
		self._fill(longest)
		symbol, length = table[self.buffer & ((1 << longest) - 1)]
		self.buffer >>= length
		self.count -= length
		return symbol


# Shrink

_SHRINK_CONTROL = 256
_SHRINK_FIRST_FREE = 257
_SHRINK_CODES = 1 << 13


def unshrink(data, size):
	"""Decompress `size` bytes of shrunk `data`."""
	bits = _Bits(data)
	code_size = 9

	# An entry is only (the code it extends, one more byte), and its string is
	# rebuilt from that chain whenever it is used - as PKZIP does. It matters:
	# a partial clear can free a code that later entries still extend, and once
	# the code is reused, those entries mean something new.
	free = -1
	parents = [free] * _SHRINK_CODES
	suffixes = list(range(256)) + [0] * (_SHRINK_CODES - 256)

	def string_of(code):
		reversed_string = bytearray()

		while code >= _SHRINK_FIRST_FREE:
			if parents[code] == free or len(reversed_string) >= _SHRINK_CODES:
				raise zipfile.BadZipFile("shrunk data refers to a code it never defined")

			reversed_string.append(suffixes[code])
			code = parents[code]

		reversed_string.append(code)
		reversed_string.reverse()
		return reversed_string

	def lowest_free(start):
		return next((code for code in range(start, _SHRINK_CODES) if parents[code] == free), _SHRINK_CODES)

	output = bytearray()
	next_free = _SHRINK_FIRST_FREE
	previous_code = None
	previous_first = 0

	while len(output) < size:
		code = bits.read(code_size)

		if code == _SHRINK_CONTROL:
			command = bits.read(code_size)

			if command == 1 and code_size < 13:
				code_size += 1
			elif command == 2:
				# Free every entry no other entry extends.
				extended = {parents[entry] for entry in range(_SHRINK_FIRST_FREE, _SHRINK_CODES) if parents[entry] != free}

				for entry in range(_SHRINK_FIRST_FREE, _SHRINK_CODES):
					if entry not in extended:
						parents[entry] = free

				next_free = lowest_free(_SHRINK_FIRST_FREE)
			else:
				raise zipfile.BadZipFile(f"unknown shrink control code {command}")

			continue

		if previous_code is None:
			if code >= 256:
				raise zipfile.BadZipFile("shrunk data does not start with a literal")

			current = string_of(code)
		elif code >= _SHRINK_FIRST_FREE and parents[code] == free:
			# The code being defined right now: the previous string plus the first byte it had.
			current = string_of(previous_code) + bytes([previous_first])
		else:
			current = string_of(code)

		output += current

		if previous_code is not None and next_free < _SHRINK_CODES:
			parents[next_free] = previous_code
			suffixes[next_free] = current[0]
			next_free = lowest_free(next_free + 1)

		previous_code = code
		previous_first = current[0]

	return bytes(output[:size])


# Implode

def _read_lengths(data, position, expected):
	"""A tree's code lengths, stored run-length encoded; (lengths, position after them)."""
	stored = data[position] + 1
	lengths = []

	for byte in data[position + 1:position + 1 + stored]:
		# Low nibble: the code length - 1; high nibble: how many codes have it - 1.
		lengths += [(byte & 0x0F) + 1] * ((byte >> 4) + 1)

	if len(lengths) != expected:
		raise zipfile.BadZipFile(f"imploded tree has {len(lengths)} codes, expected {expected}")

	return lengths, position + 1 + stored


def _tree(lengths):
	"""A lookup table for decoding: every `longest`-bit value -> (symbol, its code length)."""
	# APPNOTE: sort by length (keeping the stored order among equal lengths),
	# then hand out 16-bit codes from the longest end, and reverse their bits.
	order = sorted(range(len(lengths)), key=lambda symbol: lengths[symbol])
	codes = [0] * len(lengths)
	code = increment = last_length = 0

	for symbol in reversed(order):
		code += increment

		if lengths[symbol] != last_length:
			last_length = lengths[symbol]
			increment = 1 << (16 - last_length)

		codes[symbol] = code

	longest = max(lengths)
	table = [None] * (1 << longest)

	for symbol, (code, length) in enumerate(zip(codes, lengths)):
		# Reversed, the code's top `length` bits are the low bits read first.
		reversed_code = int(f"{code & 0xFFFF:016b}"[::-1], 2)

		for high in range(1 << (longest - length)):
			table[reversed_code | (high << length)] = (symbol, length)

	if None in table:
		raise zipfile.BadZipFile("imploded tree does not cover every code")

	return table, longest


def explode(data, size, large_window, literal_tree):
	"""Decompress `size` bytes of imploded `data`."""
	position = 0
	literals = None

	if literal_tree:
		lengths, position = _read_lengths(data, position, 256)
		literals = _tree(lengths)

	lengths, position = _read_lengths(data, position, 64)
	match_lengths = _tree(lengths)
	lengths, position = _read_lengths(data, position, 64)
	distances = _tree(lengths)

	bits = _Bits(data, position)
	distance_low_bits = 7 if large_window else 6
	minimum_length = 3 if literal_tree else 2
	output = bytearray()

	while len(output) < size:
		if bits.read(1):
			output.append(bits.decode(literals) if literals else bits.read(8))
			continue

		distance = bits.read(distance_low_bits)
		distance |= bits.decode(distances) << distance_low_bits
		length = bits.decode(match_lengths)

		if length == 63:
			length += bits.read(8)

		length += minimum_length
		start = len(output) - distance - 1

		if start >= 0 and distance + 1 >= length:
			output += output[start:start + length]
		else:
			# Overlapping the output, or reaching before its start, where
			# APPNOTE says to read zeros: byte by byte.
			for index in range(start, start + length):
				output.append(output[index] if index >= 0 else 0)

	return bytes(output[:size])


def read_member(archive, info):
	"""The contents of `info` in `archive` (a `zipfile.ZipFile`), in any method we can read."""
	if info.compress_type not in (SHRUNK, IMPLODED):
		return archive.read(info)

	if info.flag_bits & 0x1:
		raise zipfile.BadZipFile(f"{info.filename} is encrypted")

	# `zipfile` will not open it, so read the raw data after its local header.
	file = archive.fp
	file.seek(info.header_offset)
	header = file.read(30)

	if header[:4] != b"PK\x03\x04":
		raise zipfile.BadZipFile(f"no local header for {info.filename}")

	name_length, extra_length = struct.unpack("<HH", header[26:30])
	file.seek(info.header_offset + 30 + name_length + extra_length)
	raw = file.read(info.compress_size)

	if info.compress_type == SHRUNK:
		data = unshrink(raw, info.file_size)
	else:
		data = explode(raw, info.file_size, bool(info.flag_bits & 0x2), bool(info.flag_bits & 0x4))

	if zlib.crc32(data) != info.CRC:
		raise zipfile.BadZipFile(f"{info.filename}: CRC-32 does not match after decompressing")

	return data
