if [[ -z "$1" ]]; then
    WIDTH="800"
else
    WIDTH="$1"
fi
if [[ -z "$2" ]]; then
    HEIGHT="600"
else
    HEIGHT="$2"
fi
if [[ -z "$3" ]]; then
    FRATE="60"
else
    FRATE="$3"
fi
if [[ -z "$4" ]]; then
    OUTPUT_FILE="$(dirname "$0")/out/anim.mp4"
else
    OUTPUT_FILE="$(dirname "$0")/out/$4"
fi

mkdir -p ""$(dirname "$0")"/out/"
cat $(dirname "$0")/frames/*.raw | ffmpeg -f rawvideo -pix_fmt bgra -s "$WIDTH"x"$HEIGHT" -r "$FRATE" -i - -c:v libx264 -y "$OUTPUT_FILE"
echo Done \("$OUTPUT_FILE"\)