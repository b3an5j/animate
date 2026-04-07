#!/bin/bash

BREAK="--------------------------------------------------------------------------------"

make example

cd "$(dirname "$0")"
mkdir -p ./tests/frames
mkdir -p ./tests/out
echo Done \(./tests/example\)
echo $BREAK

echo Generating frames ...
./tests/example
frames=(tests/frames/*.raw)
echo Done \(${#frames[@]} frames\)
echo $BREAK

echo Generating example single frame ...
if [[ -f "./tests/out/simple.dat" ]]; then
    ./tests/raw_to_img.sh
else
    echo No simple.dat file generated.
fi
echo $BREAK

echo Generating example video ...
if [[ -f "./tests/frames/frame000000.raw" ]]; then
    ./tests/animate.sh
else
    echo No frames file generated.
fi