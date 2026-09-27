#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later

"""
Measure archive demos with upstream dsda-doom and record the result.

Replays demos from `spec/support/archive/<archive>.json` through a reference
binary (upstream, which we trust) and writes the total time it reached into
each demo's `measured` field, the way the spec suite reads it: the cumulative
time of the last finished level, e.g. "17:55", or "00:00" when the game
finished no level. Our build is then tested against these times.

A demo that needs PWADs is replayed with every file of that name we have -
loose in `spec/support/wads`, or inside any zip of the idgames mirror in
`spec/support/wads/idgames` - since a name is often shared by unrelated WADs
or by several versions of one. What it records in `pwad_files`:
- the first (largest first) whose time equals a time the demo claims;
- else the largest one with which the game finishes at least one level;
- else nothing, and `measured` stays null.
Mirror files are extracted to `spec/support/wads/pwads/<zip path>/<file>`,
where the suite finds them again.

A demo is skipped when its IWAD, or any file for one of its PWADs, is missing,
or when it is already measured. A run that fails or times out counts as not
finishing.

Usage:
    python3 spec/support/measure_archive.py --reference /path/to/upstream/dsda-doom \\
        --archive competn-2005 --first 1000
"""

import argparse
import concurrent.futures
import io
import itertools
import json
import os
import pathlib
import subprocess
import sys
import tempfile
import time
import zipfile

# Importing the indexer would otherwise leave a `__pycache__` in spec/support.
sys.dont_write_bytecode = True

from pkzip import read_member
from index_archive import SUPPORT, pwad_candidates, write_index

# Demos naming several PWADs multiply their candidates; beyond this many
# combinations only the largest are tried.
MAXIMUM_COMBINATIONS = 8


def read_lump(archive, demo):
	with zipfile.ZipFile(archive / demo["zip"]) as outer:
		if "nested_zip" not in demo:
			return read_member(outer, outer.getinfo(demo["lump"]))

		with zipfile.ZipFile(io.BytesIO(read_member(outer, outer.getinfo(demo["nested_zip"])))) as inner:
			return read_member(inner, inner.getinfo(demo["lump"]))


def combinations(demo, candidates):
	"""The sets of files to try for the demo's PWADs, largest first; None when one is missing."""
	per_pwad = [candidates.get(name.lower()) for name in demo["pwads"]]

	if None in per_pwad:
		return None

	found = sorted(itertools.product(*per_pwad), key=lambda files: -sum(file["size"] for file in files))
	return found[:MAXIMUM_COMBINATIONS]


def extract(wads, candidate):
	"""Make sure a mirror candidate is extracted where its `path` says; loose files already are."""
	if "zip" not in candidate:
		return

	target = wads / candidate["path"]

	if target.is_file() and target.stat().st_size == candidate["size"]:
		return

	target.parent.mkdir(parents=True, exist_ok=True)

	with zipfile.ZipFile(candidate["zip"]) as archive:
		data = read_member(archive, archive.getinfo(candidate["member"]))

	# Written aside and renamed, so a reader never sees half a file.
	partial = target.with_name(target.name + ".part")
	partial.write_bytes(data)
	partial.replace(target)


def total_time(levelstat):
	"""The cumulative time of the last level, as `Levelstat::TotalTime` in the spec reads it."""
	lines = [line for line in levelstat.splitlines() if line.strip()]

	if not lines:
		return "00:00"

	fields = lines[-1].split()
	return fields[3].strip("()") if len(fields) > 3 else None


def measure(reference, archive, demo, wads, files, timeout):
	"""The time upstream reaches with these PWAD files, or None when the run fails."""
	with tempfile.TemporaryDirectory(prefix="dsda-measure-") as directory:
		directory = pathlib.Path(directory)
		lmp = directory / pathlib.PurePosixPath(demo["lump"]).name
		lmp.write_bytes(read_lump(archive, demo))

		command = [str(reference), "-iwad", str(wads / demo["iwad"])]

		# DeHackEd patches load with `-deh`, everything else with `-file` - as in the spec suite.
		is_patch = lambda file: pathlib.PurePosixPath(file["path"]).suffix.lower() in (".deh", ".bex")
		wad_files = [str(wads / file["path"]) for file in files if not is_patch(file)]
		deh_files = [str(wads / file["path"]) for file in files if is_patch(file)]

		if wad_files:
			command += ["-file", *wad_files]

		if deh_files:
			command += ["-deh", *deh_files]

		# The same flags as `DemoRun` in the spec suite, including its own config
		# and data directory, so no run touches `~/.dsda-doom` or another run.
		command += [
			"-fastdemo", str(lmp), "-nosound", "-nomusic", "-nodraw", "-levelstat", "-analysis", "-noautoload",
			"-config", str(directory / "dsda-doom.cfg"), "-data", str(directory),
		]

		try:
			completed = subprocess.run(command, cwd=directory, capture_output=True, timeout=timeout)
		except subprocess.TimeoutExpired:
			return None

		if completed.returncode != 0:
			return None

		levelstat_file = directory / "levelstat.txt"
		levelstat = levelstat_file.read_text(errors="replace") if levelstat_file.exists() else ""

	return total_time(levelstat)


def choose(demo, results):
	"""(pwad files, measured) by the rule in the module description; `results` is in size order."""
	claims = {time for time in demo["time"].values() if time}

	for files, measured in results:
		if measured in claims:
			return files, measured

	for files, measured in results:
		if measured not in (None, "00:00"):
			return files, measured

	return None, None


def is_measured(demo):
	# A demo measured before PWADs were chosen per file still needs choosing.
	return demo["measured"] is not None and (not demo["pwads"] or "pwad_files" in demo)


def main():
	parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
	parser.add_argument("--reference", type=pathlib.Path, required=True, help="the upstream dsda-doom binary")
	parser.add_argument("--archive", required=True, choices=["competn-2005", "sda-2005"])
	parser.add_argument("--first", type=int, default=0, help="only the first N demos of the index (default: all)")
	parser.add_argument("--jobs", type=int, default=os.cpu_count())
	parser.add_argument("--timeout", type=int, default=120, help="seconds one run may take")
	parser.add_argument("--wads", type=pathlib.Path, default=SUPPORT / "wads")
	arguments = parser.parse_args()

	wads = arguments.wads.absolute()
	index = SUPPORT / "archive" / f"{arguments.archive}.json"
	archive = SUPPORT / "archive" / arguments.archive
	demos = json.loads(index.read_text(encoding="utf-8"))
	iwads = {path.name for path in wads.iterdir() if path.is_file()}
	candidates = pwad_candidates(wads)

	selected = demos[:arguments.first] if arguments.first else demos
	counts = {"already measured": 0, "missing WADs": 0}
	plans = {}

	for demo in selected:
		if is_measured(demo):
			counts["already measured"] += 1
			continue

		files = combinations(demo, candidates) if demo["pwads"] else [()]

		if demo["iwad"] not in iwads or files is None:
			counts["missing WADs"] += 1
			continue

		plans[id(demo)] = (demo, files)

	runs = sum(len(files) for _, files in plans.values())
	print(f"{len(selected)} demos selected, {len(plans)} to measure in {runs} runs on {arguments.jobs} threads", flush=True)

	for _, files in plans.values():
		for combination in files:
			for candidate in combination:
				extract(wads, candidate)

	started = time.monotonic()
	jobs = {}

	with concurrent.futures.ThreadPoolExecutor(max_workers=arguments.jobs) as pool:
		for key, (demo, files) in plans.items():
			for position, combination in enumerate(files):
				future = pool.submit(measure, arguments.reference.absolute(), archive, demo, wads, combination, arguments.timeout)
				jobs[future] = (key, position)

		results = {key: [None] * len(files) for key, (_, files) in plans.items()}

		for done, future in enumerate(concurrent.futures.as_completed(jobs), 1):
			key, position = jobs[future]

			try:
				results[key][position] = future.result()
			except Exception as error:
				print(f"  {plans[key][0]['zip']}: {error}", flush=True)

			if done % 500 == 0 or done == len(jobs):
				print(f"  {done}/{len(jobs)} runs after {time.monotonic() - started:.0f} s", flush=True)

	elapsed = time.monotonic() - started

	for key, (demo, files) in plans.items():
		chosen, measured = choose(demo, list(zip(files, results[key])))
		demo["measured"] = measured

		if demo["pwads"]:
			if chosen is None:
				demo.pop("pwad_files", None)
			else:
				demo["pwad_files"] = [file["path"] for file in chosen]

		if measured is not None:
			status = "measured"
		else:
			status = "no candidate finishes" if demo["pwads"] else "failed"

		counts[status] = counts.get(status, 0) + 1

	write_index(index, demos)

	for status, count in sorted(counts.items()):
		print(f"{count:6} {status}")

	if jobs:
		print(f"{elapsed:.0f} s for {len(jobs)} runs ({elapsed / len(jobs) * 1000:.0f} ms each on {arguments.jobs} threads)")

	return 0


if __name__ == "__main__":
	sys.exit(main())
