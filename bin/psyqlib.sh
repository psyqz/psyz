#!/bin/sh

set -e

usage() {
	printf 'usage: %s <Psy-Q lib dir> <output lib dir>\n' "$0" 1>&2
}

tolower() {
	tr '[:upper:]' '[:lower:]'
}

PSYQ_DIR="$1"
OUT_DIR="$2"

[ ! -d "$PSYQ_DIR" ] && { usage; exit 1; }
[ -z "$OUT_DIR" ] && { usage; exit 1; }

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

mkdir -p "$OUT_DIR"

mkdir -p "$TMP/psyq"
mkdir -p "$TMP/elf"

for psyq_lib in "$PSYQ_DIR"/*.LIB; do
	[ ! -f "$psyq_lib" ] && continue

	elf_lib="$OUT_DIR/$(basename "${psyq_lib%.*}" | tolower).a"

	printf '%s -> %s\n' "$psyq_lib" "$elf_lib"
	./psyqdump "$psyq_lib" "$TMP/psyq"

	for psyq_obj in "$TMP/psyq"/*.OBJ; do
		elf_obj="$TMP/elf/$(basename "${psyq_obj%.*}" | tolower).o"
		../psyq-obj-parser "$psyq_obj" -o "$elf_obj"
	done

	ar rcs "$elf_lib" "$TMP/elf"/*.o

	rm "$TMP/psyq"/*.OBJ
	rm "$TMP/elf"/*.o
done
