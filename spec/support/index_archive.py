#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later

"""
Index the Compet-N and SDA demo archives.

For every demo (`.lmp`) inside the zips under `spec/support/archive/competn-2005/`
and `spec/support/archive/sda-2005/` - including zips nested one level deep -
this works out:
- the time the demo claims, from its file name (`e1m4-238` -> 2:38) and from
  the `.txt` beside it (`Total time: 36:55`), kept separately;
- which IWAD and PWADs it needs, from the directory, the demo header and the
  `.txt`.

The results go next to the archives, to `spec/support/archive/competn-2005.json`
and `spec/support/archive/sda-2005.json`, which are committed; the archives themselves
are not. Each demo also has a
`measured` time, left `null` here, for the time the game actually reaches.
WADs that demos need but neither `spec/support/wads/` nor the idgames mirror in
`spec/support/wads/idgames/` has are listed in `spec/support/archive-missing-wads.txt`.

The heuristics are deliberately simple and their result is recorded per demo,
so a wrong guess can be seen in the index and fixed here.

Usage:
    python3 spec/support/index_archive.py
"""

import argparse
import io
import json
import pathlib
import re
import sys
import zipfile

# Importing a sibling module would otherwise leave a `__pycache__` in spec/support.
sys.dont_write_bytecode = True

from pkzip import IMPLODED, SHRUNK, read_member

SUPPORT = pathlib.Path(__file__).resolve().parent
ARCHIVES = ["competn-2005", "sda-2005"]

# The upper-case names are the original releases; the lower-case ones in
# `spec/support/wads` may be the 2024 re-release, which demos do not sync on.
IWAD_BY_DIRECTORY = {
	"doom": "DOOM.WAD",
	"doom2": "DOOM2.WAD",
	"tnt": "TNT.WAD",
	"plutonia": "PLUTONIA.WAD",
}
IWAD_NAMES = {
	"doom.wad", "doom1.wad", "doomu.wad", "doom2.wad", "doom2f.wad", "tnt.wad", "plutonia.wad",
	"heretic.wad", "heretic1.wad", "hexen.wad", "freedoom.wad", "freedoom1.wad", "freedoom2.wad",
}
# Checked in order; the first hint found in the text wins.
IWAD_TEXT_HINTS = [
	("HERETIC.WAD", ("heretic",)),
	("HEXEN.WAD", ("hexen",)),
	("TNT.WAD", ("tnt.wad", "evilution")),
	("PLUTONIA.WAD", ("plutonia",)),
	("DOOM2.WAD", ("doom2", "doom ii", "doom 2")),
	("DOOM.WAD", ("ultimate doom", "doom.exe", "doom.wad", "episode")),
]

# Doom demo header versions: 104-111 vanilla 1.4 to 1.11, 200-214 Boom, MBF,
# PrBoom and dsda. Anything below 5 is a v1.0-1.2 demo, whose first byte is
# the skill.
BOOM_VERSIONS = range(200, 215)
VANILLA_VERSIONS = range(104, 112)

NAME_TIME = re.compile(r"-?(\d{3,4})$")
TEXT_TIME = re.compile(r"(?<![\d:])(\d{1,3}):([0-5]\d)(?![\d:])")
WAD_MENTION = re.compile(r"[a-z0-9_\-]+\.(?:wad|deh|bex)\b")
PWAD_SUFFIXES = (".wad", ".deh", ".bex")
EXTRACTABLE = {zipfile.ZIP_STORED, zipfile.ZIP_DEFLATED, zipfile.ZIP_BZIP2, zipfile.ZIP_LZMA, SHRUNK, IMPLODED}


def time_text(minutes, seconds):
	return f"{int(minutes)}:{int(seconds):02d}"


def claimed_time_from_name(stem):
	match = NAME_TIME.search(stem)

	if not match:
		return None

	digits = match.group(1)
	return time_text(digits[:-2], digits[-2:]) if int(digits[-2:]) < 60 else None


def claimed_time_from_text(text):
	"""The time on a 'total' line, else the first time on a 'time' line."""
	fallback = None

	for line in text.lower().splitlines():
		if "time" not in line:
			continue

		times = TEXT_TIME.findall(line)

		if not times:
			continue

		if "total" in line:
			return time_text(*times[-1])

		if fallback is None:
			fallback = time_text(*times[0])

	return fallback


def parse_header(data):
	"""(version, episode, map) as far as the header tells; None where it does not."""
	if not data:
		return None, None, None

	version = data[0]

	if version < 5 and len(data) >= 3:
		return version, data[1], data[2]

	if version in VANILLA_VERSIONS and len(data) >= 4:
		return version, data[2], data[3]

	if version in BOOM_VERSIONS and len(data) >= 11:
		return version, data[9], data[10]

	return version, None, None


def guess_iwad(parts, version, episode, map_, text):
	"""The IWAD a demo needs, or None when nothing tells."""
	for part in parts:
		if part in IWAD_BY_DIRECTORY:
			return IWAD_BY_DIRECTORY[part]

	lowered = text.lower()

	# Heretic and Hexen demos have no version byte, so they look like old Doom ones.
	if version is not None and version < 5:
		for iwad, hints in IWAD_TEXT_HINTS[:2]:
			if any(hint in lowered for hint in hints):
				return iwad

		return "DOOM.WAD"

	if episode is not None and map_ is not None:
		if episode > 1:
			return "DOOM.WAD"

		if map_ > 9:
			for iwad, hints in IWAD_TEXT_HINTS[2:4]:
				if any(hint in lowered for hint in hints):
					return iwad

			return "DOOM2.WAD"

	for iwad, hints in IWAD_TEXT_HINTS:
		if any(hint in lowered for hint in hints):
			return iwad

	return None


def guess_pwads(parts, text):
	"""The PWAD/DEH files a demo needs: those its text names, else its Compet-N directory's."""
	mentioned = []

	for name in WAD_MENTION.findall(text.lower()):
		if name not in IWAD_NAMES and name not in mentioned:
			mentioned.append(name)

	if mentioned:
		return mentioned

	if "pwads" in parts:
		index = parts.index("pwads")

		if index + 1 < len(parts) - 1:
			return [parts[index + 1] + ".wad"]

	return []


def containers(zip_path):
	"""Yield (nested zip name or None, ZipFile) for the zip and each zip inside it."""
	with zipfile.ZipFile(zip_path) as outer:
		yield None, outer

		for name in outer.namelist():
			if name.lower().endswith(".zip"):
				try:
					with zipfile.ZipFile(io.BytesIO(read_member(outer, outer.getinfo(name)))) as inner:
						yield name, inner
				except zipfile.BadZipFile:
					continue


def demos_in(archive, zip_path):
	relative = zip_path.relative_to(archive).as_posix()
	parts = [part.lower() for part in zip_path.relative_to(archive).parts]

	for nested, container in containers(zip_path):
		names = container.namelist()
		texts = {pathlib.PurePosixPath(name).stem.lower(): name for name in names if name.lower().endswith(".txt")}

		for name in names:
			if not name.lower().endswith(".lmp"):
				continue

			stem = pathlib.PurePosixPath(name).stem
			data = read_member(container, container.getinfo(name))

			# A few zips list a demo whose stored data is empty
			# (competn plutonia/max/pl09-530.zip); there is nothing to replay.
			if not data:
				continue

			# The demo's own description, else every description in its zip.
			if stem.lower() in texts:
				text = read_member(container, container.getinfo(texts[stem.lower()])).decode("latin-1")
			else:
				text = "\n".join(read_member(container, container.getinfo(t)).decode("latin-1") for t in texts.values())

			version, episode, map_ = parse_header(data)

			demo = {"zip": relative}
			if nested is not None:
				demo["nested_zip"] = nested
			demo |= {
				"lump": name,
				"iwad": guess_iwad(parts, version, episode, map_, text),
				"pwads": guess_pwads(parts, text),
				"time": {
					"name": claimed_time_from_name(stem),
					"text": claimed_time_from_text(text),
				},
				"measured": None,
			}

			yield demo


def pwad_candidates(wads):
	"""
	PWAD file name (lower case) -> every file by that name we have, largest first.

	Each candidate is a dict with `path` - relative to `spec/support/wads`, the
	form demos record in `pwad_files` - and `size`. Besides loose files in
	`spec/support/wads`, every WAD/DEH inside a zip of the idgames mirror in
	`spec/support/wads/idgames` is one; those also carry the `zip` and `member`
	to extract them from, into `spec/support/wads/pwads/<zip path>/<member>`.
	"""
	candidates = {}

	for path in wads.iterdir():
		if path.is_file() and path.suffix.lower() in PWAD_SUFFIXES:
			candidates.setdefault(path.name.lower(), []).append({"path": path.name, "size": path.stat().st_size})

	mirror = wads / "idgames"

	if mirror.is_dir():
		for zip_path in mirror.rglob("*"):
			if zip_path.suffix.lower() != ".zip" or not zip_path.is_file():
				continue

			try:
				with zipfile.ZipFile(zip_path) as archive:
					infos = archive.infolist()
			except Exception:
				continue

			relative = zip_path.relative_to(mirror).as_posix()

			for info in infos:
				name = pathlib.PurePosixPath(info.filename).name

				if info.is_dir() or pathlib.PurePosixPath(name).suffix.lower() not in PWAD_SUFFIXES:
					continue

				# A file we cannot extract (an unusual compression method) is no candidate.
				if info.compress_type not in EXTRACTABLE:
					continue

				candidates.setdefault(name.lower(), []).append({
					"path": f"pwads/{relative}/{info.filename}",
					"size": info.file_size,
					"zip": zip_path,
					"member": info.filename,
				})

	for found in candidates.values():
		found.sort(key=lambda candidate: -candidate["size"])

	return candidates


def missing_wads(demo, wads, candidates):
	"""The WADs `demo` needs that we have no file for at all."""
	missing = []

	# IWADs by exact name, as the lower-case files may be the re-release.
	if demo["iwad"] is not None and demo["iwad"] not in wads:
		missing.append(demo["iwad"])

	missing += [name for name in demo["pwads"] if name.lower() not in candidates]
	return missing


def write_index(path, demos):
	# One demo per line keeps the diffs of this large file readable.
	with path.open("w", encoding="utf-8", newline="\n") as file:
		file.write("[\n")
		file.write(",\n".join(json.dumps(demo, separators=(",", ":")) for demo in demos))
		file.write("\n]\n")


def write_missing(path, counts):
	with path.open("w", encoding="utf-8", newline="\n") as file:
		file.write("# WADs that archive demos need but neither spec/support/wads nor its idgames mirror has.\n")
		file.write("# Generated by spec/support/index_archive.py: demos needing it, the WAD, one zip using it.\n")

		for name, zips in sorted(counts.items(), key=lambda item: (-len(item[1]), item[0])):
			file.write(f"{len(zips):5} {name} {zips[0]}\n")


def main():
	parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
	parser.add_argument("--archive", type=pathlib.Path, default=SUPPORT / "archive")
	parser.add_argument("--wads", type=pathlib.Path, default=SUPPORT / "wads")
	parser.add_argument("--output", type=pathlib.Path, default=SUPPORT / "archive", help="where the JSON indexes go")
	parser.add_argument("--missing", type=pathlib.Path, default=SUPPORT / "archive-missing-wads.txt")
	arguments = parser.parse_args()

	wads = {path.name for path in arguments.wads.iterdir() if path.is_file()}
	candidates = pwad_candidates(arguments.wads)
	missing = {}

	for name in ARCHIVES:
		archive = (arguments.archive / name).resolve()

		if not archive.is_dir():
			print(f"{name}: not found at {archive}, skipping")
			continue

		demos = []
		unreadable = 0

		for zip_path in sorted(path for path in archive.rglob("*") if path.suffix.lower() == ".zip"):
			try:
				found = list(demos_in(archive, zip_path))
			# Old archives fail in many ways (bad CRCs, truncated entries, odd
			# compression); whatever it is, the zip is left out of the index.
			except Exception as error:
				print(f"{name}: unreadable {zip_path.relative_to(archive)}: {error}")
				unreadable += 1
				continue

			for demo in found:
				demos.append(demo)

				for wad in missing_wads(demo, wads, candidates):
					missing.setdefault(wad, []).append(f"{name}/{demo['zip']}")

		# Keep what `measure_archive.py` already measured and chose; re-indexing must not lose it.
		output = arguments.output / f"{name}.json"

		if output.exists():
			key = lambda demo: (demo["zip"], demo.get("nested_zip"), demo["lump"])
			previous = {key(demo): demo for demo in json.loads(output.read_text(encoding="utf-8"))}

			for demo in demos:
				old = previous.get(key(demo), {})
				demo["measured"] = old.get("measured")

				if "pwad_files" in old:
					demo["pwad_files"] = old["pwad_files"]

		write_index(output, demos)

		unknown = sum(1 for demo in demos if demo["iwad"] is None)
		print(f"{name}: {len(demos)} demos, {unknown} with an unknown IWAD, {unreadable} unreadable zips")

	write_missing(arguments.missing, missing)
	return 0


if __name__ == "__main__":
	sys.exit(main())
