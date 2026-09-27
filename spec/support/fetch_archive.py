#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later

"""
Back up the Compet-N and Speed Demos Archive dumps from archive.org.

Everything https://soulsphere.org/random/competn-index/ links to lives inside
two zips of one archive.org item ("competn"): `competn.zip` (pub/compet-n) and
`sda.zip` (pub/sda), about 350 MB together. Downloading those two files is far
gentler than fetching the ~11,500 demos one by one through archive.org's
zip-browsing links.

To stay polite the script:
- asks the metadata API once for the file list, sizes and MD5s,
- downloads one file at a time over a single connection,
- resumes a partial download instead of starting over,
- skips a file that is already there with the right MD5,
- backs off on errors (honouring `Retry-After`) and gives up after a few tries,
- can cap its bandwidth with `--limit-rate`.

Usage:
    python3 spec/support/fetch_archive.py                  # both zips
    python3 spec/support/fetch_archive.py --files competn.zip
    python3 spec/support/fetch_archive.py --limit-rate 2M --extract
"""

import argparse
import hashlib
import json
import pathlib
import sys
import time
import urllib.error
import urllib.request
import zipfile

ITEM = "competn"
METADATA_URL = f"https://archive.org/metadata/{ITEM}"
DOWNLOAD_URL = f"https://archive.org/download/{ITEM}/{{name}}"
DEFAULT_FILES = ["competn.zip", "sda.zip"]
USER_AGENT = "dsda-doom-spec-archiver/1.0 (one-off personal backup; sequential, resumable)"

CHUNK_SIZE = 64 * 1024
MAX_ATTEMPTS = 5
FIRST_BACKOFF_SECONDS = 10
PAUSE_BETWEEN_FILES_SECONDS = 5


def parse_rate(text):
	"""'500K' / '2M' / '1048576' -> bytes per second."""
	units = {"K": 1024, "M": 1024 * 1024}
	suffix = text[-1].upper()

	if suffix in units:
		return int(float(text[:-1]) * units[suffix])

	return int(text)


def request(url, headers=None):
	return urllib.request.Request(url, headers={"User-Agent": USER_AGENT, **(headers or {})})


def fetch_metadata():
	with urllib.request.urlopen(request(METADATA_URL), timeout=60) as response:
		metadata = json.load(response)

	return {entry["name"]: entry for entry in metadata.get("files", [])}


def md5_of(path):
	digest = hashlib.md5()

	with path.open("rb") as file:
		for chunk in iter(lambda: file.read(1024 * 1024), b""):
			digest.update(chunk)

	return digest.hexdigest()


def download(name, expected_size, destination, limit_rate):
	"""Download into `<name>.part`, resuming it if present, then rename. Raises on failure."""
	partial = destination.with_name(destination.name + ".part")
	offset = partial.stat().st_size if partial.exists() else 0

	if expected_size is not None and offset > expected_size:
		# Longer than the real file, so it is not a prefix of it.
		partial.unlink()
		offset = 0

	if expected_size is not None and offset == expected_size:
		# Complete already; asking for the bytes after the end would get a 416.
		partial.replace(destination)
		return

	headers ={"Range": f"bytes={offset}-"} if offset else {}

	with urllib.request.urlopen(request(DOWNLOAD_URL.format(name=name), headers), timeout=60) as response:
		if offset and response.status != 206:
			# The server ignored the range, so the body is the whole file again.
			offset = 0

		mode = "ab" if offset else "wb"
		received = offset
		started = time.monotonic()
		sent_this_session = 0

		with partial.open(mode) as file:
			for chunk in iter(lambda: response.read(CHUNK_SIZE), b""):
				file.write(chunk)
				received += len(chunk)
				sent_this_session += len(chunk)

				if limit_rate:
					# Sleep off whatever we are ahead of the allowed average.
					ahead = sent_this_session / limit_rate - (time.monotonic() - started)
					if ahead > 0:
						time.sleep(ahead)

				if expected_size:
					print(f"\r  {received / 1e6:8.1f} / {expected_size / 1e6:.1f} MB", end="", flush=True)

	print()

	if expected_size is not None and partial.stat().st_size != expected_size:
		raise IOError(f"got {partial.stat().st_size} bytes, expected {expected_size}")

	partial.replace(destination)


def retry_delay(error, attempt):
	"""Seconds to wait before the next attempt: the server's `Retry-After`, else exponential backoff."""
	if isinstance(error, urllib.error.HTTPError):
		retry_after = error.headers.get("Retry-After")

		if retry_after and retry_after.isdigit():
			return int(retry_after)

	return FIRST_BACKOFF_SECONDS * 2 ** (attempt - 1)


def fetch(name, entry, directory, limit_rate):
	destination = directory / name
	expected_md5 = entry.get("md5")
	expected_size = int(entry["size"]) if entry.get("size") else None

	if destination.exists() and expected_md5 and md5_of(destination) == expected_md5:
		print(f"{name}: already downloaded and verified, skipping")
		return True

	for attempt in range(1, MAX_ATTEMPTS + 1):
		print(f"{name}: downloading (attempt {attempt}/{MAX_ATTEMPTS})")

		try:
			download(name, expected_size, destination, limit_rate)
		except (urllib.error.URLError, IOError, TimeoutError) as error:
			if isinstance(error, urllib.error.HTTPError) and error.code in (403, 404):
				print(f"{name}: {error} - not retrying")
				return False

			if attempt == MAX_ATTEMPTS:
				print(f"{name}: {error} - giving up")
				return False

			delay = retry_delay(error, attempt)
			print(f"{name}: {error} - retrying in {delay} s")
			time.sleep(delay)
			continue

		if expected_md5 and md5_of(destination) != expected_md5:
			# The resumed part was not a prefix of the real file; start clean.
			print(f"{name}: MD5 mismatch, discarding and starting over")
			destination.unlink()
			continue

		print(f"{name}: done, MD5 verified")
		return True

	return False


def extract(archive, directory):
	target = directory / archive.stem
	print(f"{archive.name}: extracting into {target}")

	with zipfile.ZipFile(archive) as zip_file:
		zip_file.extractall(target)


def main():
	parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
	parser.add_argument("--files", nargs="+", default=DEFAULT_FILES,
		help=f"files of the archive.org item to fetch (default: {' '.join(DEFAULT_FILES)})")
	parser.add_argument("--output", type=pathlib.Path, default=pathlib.Path(__file__).parent / "archive",
		help="where to put them (default: spec/support/archive)")
	parser.add_argument("--limit-rate", type=parse_rate, default=0,
		help="bandwidth cap such as 500K or 2M bytes per second (default: none)")
	parser.add_argument("--extract", action="store_true", help="unpack each zip after it is verified")
	arguments = parser.parse_args()

	arguments.output.mkdir(parents=True, exist_ok=True)
	files = fetch_metadata()

	unknown = [name for name in arguments.files if name not in files]
	if unknown:
		print(f"not in the '{ITEM}' item: {', '.join(unknown)}; it has: {', '.join(sorted(files))}")
		return 1

	failed = []

	for index, name in enumerate(arguments.files):
		if index:
			time.sleep(PAUSE_BETWEEN_FILES_SECONDS)

		if not fetch(name, files[name], arguments.output, arguments.limit_rate):
			failed.append(name)
		elif arguments.extract and name.endswith(".zip"):
			extract(arguments.output / name, arguments.output)

	if failed:
		print(f"failed: {', '.join(failed)} - run again to resume")
		return 1

	return 0


if __name__ == "__main__":
	sys.exit(main())
