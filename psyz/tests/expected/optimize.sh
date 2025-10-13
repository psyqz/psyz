#!/bin/sh
find . -type f -iname '*.png' -print0 | while IFS= read -r -d '' f; do
    echo "Optimizing: $f"
    optipng -o7 -strip all "$f"
done
