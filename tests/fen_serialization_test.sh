#!/bin/sh
set -u

#
#   this test causes our program to serialize and 
#   deserialize fens to it's internal representation 
#   of position, testing these two capabilities.
#

if [ -z "${1-}" ]
then
    echo "usage: ${0} [engine executable to test]"
    exit 2
fi

engine_exe="${1}"

# check executable exists and is executable
if [ ! -x "${engine_exe}" ]
then
    echo "ERROR: can't find or execute engine exe (expected at ${engine_exe})"
    echo "exiting..."
    exit 2
fi

echo "============= TESTING FEN SERIALIZATION =============="

while read -r fen; do

    engine_output=$(printf 'position fen %s\nprintfen\nquit' "${fen}" | "${engine_exe}" | head -n 1)

    if [ "${engine_output}" != "${fen}" ]
    then
        echo "!!!FAILED TEST!!!"
        echo "Expected: ${fen}"
        echo "Received: ${engine_output}"
        exit 1
    else
        echo "***PASSED TEST*** FEN: ${fen} "
    fi
    
done < fens.txt

echo "================= ALL TESTS PASSED ===================="
echo

exit 0
