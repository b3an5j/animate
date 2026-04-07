#!/bin/bash
set -o pipefail

WIDTH="${1:-800}"
HEIGHT="${2:-600}"
INPUT_FILE="${3:-$(dirname "$0")/out/simple.dat}"
OUTPUT_FILE="${4:-$(dirname "$0")/out/out.png}"

# check if input exists
if [[ ! -f "$INPUT_FILE" ]]; then
    echo "Error: Cannot find input file at: $INPUT_FILE"
    exit 1
fi

mkdir -p "$(dirname "$OUTPUT_FILE")"
if convert -size "${WIDTH}x${HEIGHT}" -depth 8 BGRA:"$INPUT_FILE" "$OUTPUT_FILE"; then
    echo Done \("$OUTPUT_FILE"\)
else
    echo "ImageMagick failed to convert the file."
    exit 1
fi