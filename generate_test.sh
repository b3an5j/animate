BREAK="--------------------------------------------------------------------------------"

make test
mkdir -p $(dirname $0)/tests/frames
mkdir -p $(dirname $0)/tests/out
echo Done \(./tests/test\)
echo $BREAK

echo Generating frames ...
$(dirname $0)/tests/test
frames=(tests/frames/*.raw)
echo Done \(${#frames[@]} frames\)
echo $BREAK

echo Generating test single frame ...
if [[ -f "$(dirname $0)/tests/out/simple.dat" ]] then
    $(dirname $0)/tests/raw_to_nmp.sh 800 600
else
    echo No simple.dat file generated.
fi
echo $BREAK

echo Generating test video ...
if [[ -f "$(dirname $0)/tests/frames/frame000000.raw" ]] then
    $(dirname $0)/tests/animate.sh 800 600
else
    echo No frames file generated.
fi