#!/bin/sh
set -u

#
#   This test verifies the sanity of our best move search
#   We want to make sure the engine doesn't make obviously bad moves
#   and makes obviously good ones. (e.g. plays the correct move when checkmate in 1)
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

depth=4


echo "================= TESTING BEST MOVE =================="

while read -r csv_line; do

    fen=$(echo "${csv_line}" | awk -F ',' '{print $1} ')
    best_move=$(echo "${csv_line}" | awk -F ',' '{print $2}' | grep -Po "[a-h][1-8][a-h][1-8]")

    engine_output=$(printf 'position fen %s\ngo depth %s\nquit' "${fen}" "${depth}" | ${engine_exe} | grep -Po "[a-h][1-8][a-h][1-8]")

    if [ "${engine_output}" = "${best_move}" ]
    then
        echo "***PASSED TEST*** FEN: ${fen}"
    else
        echo "!!!FAILED TEST!!! FEN: ${fen}"
        echo "Expected best move: ${best_move}"
        echo "Received best move: ${engine_output}"
        exit 1;
    fi

done < best_move_fens.csv

echo "================= ALL TESTS PASSED ===================="
echo

exit 0
