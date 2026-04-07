if [[ "$1" == "DEBUG" ]]; then
    DBGFLAG=-DDEBUG
else
    DBGFLAG=
fi

cd "$(dirname "$0")"

echo ASAN TEST
echo ========================================
gcc -I.. -I../src -I../include ./all_tests.c $DBGFLAG -fsanitize=address -g -o ./all_tests
./all_tests
rm -f ./all_tests

echo
echo \*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*
echo

echo VALGRIND TEST
echo ========================================
gcc -I.. -I../src -I../include ./all_tests.c $DBGFLAG -g -o ./all_tests
valgrind ./all_tests
rm -f ./all_tests
