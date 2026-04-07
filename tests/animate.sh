#!/bin/bash
set -o pipefail

if [[ -z "$1" ]]; then
    FRAMES="$(dirname "$0")/frames/*.raw"
else
    FRAMES="$1/*.raw"
fi
WIDTH="${2:-800}"
HEIGHT="${3:-600}"
FRATE="${4:-60}"

if [[ -z "$5" ]]; then
    OUTPUT_FILE="$(dirname "$0")/out/anim.mp4"
else
    OUTPUT_FILE="$5"
fi

mkdir -p "$(dirname "$OUTPUT_FILE")"
if cat $FRAMES | ffmpeg -f rawvideo -pix_fmt bgra -s "$WIDTH"x"$HEIGHT" -r "$FRATE" -i - -c:v libx264 -y "$OUTPUT_FILE"; then
    echo Done \("$OUTPUT_FILE"\)
else
    echo "ImageMagick failed to animate the frames."
    exit 1
fi