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
    OUTPUT_FILE="$(dirname "$0")/out/out.png"
else
    OUTPUT_FILE="$(dirname "$0")/out/$3"
fi

mkdir -p ""$(dirname "$0")"/out/"
convert -size "$WIDTH"x"$HEIGHT" -depth 8 \
BGRA:""$(dirname "$0")"/out/simple.dat" "$OUTPUT_FILE"
echo Done \("$OUTPUT_FILE"\)