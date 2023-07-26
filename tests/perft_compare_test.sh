#!/bin/sh
set -u

#       This test compares the output of our 'perftdiv' command to stockfish's 
#       'go perft' command, to ensure our movegenerator is working

if [ -z "${1-}" ]
then
    echo "usage: ${0} [engine executable to test]"
    exit 2
fi

engine_exe=${1}

engine_output_tf=$(mktemp /tmp/perft_compare_XXXXXXX)

stockf_exe=$(readlink -f ~/Misc/chess_engines/stockfish)
stockf_output_tf=$(mktemp /tmp/perft_compare_XXXXXXX)

diff_output_tf=$(mktemp /tmp/perft_compare_XXXXXXX)

# remove temp files at end of program
trap 'rm -f -- ${engine_output_tf} ${stockf_output_tf} ${diff_output_tf}' 0 2 3 15

# check executables exist and are executable
if [ ! -x "${engine_exe}" ] || [ ! -x "${stockf_exe}" ]
then
    echo "ERROR: can't find or/ execute engine exe (expected at ${engine_exe}) or stockfish exe (expected at ${stockf_exe})"
    echo "exiting..."
    exit 2
fi


# pattern for isolating the lines with chessmoves in engine output 
move_pat="[a-h][1-8][a-h][1-8][bnrqk]*:"

# depth to search
depth=4

echo "=== COMPARING PERFT RESULTS TO STOCKFISH (DEPTH ${depth}) ==="

while read -r fen; do

    printf 'position fen %s\nperftdiv %s\nquit' "${fen}" "${depth}" | "${engine_exe}" | grep -P "${move_pat}" | sort > "${engine_output_tf}" &&
    printf 'position fen %s\ngo perft %s\nquit' "${fen}" "${depth}" | "${stockf_exe}" | grep -P "${move_pat}" | sort > "${stockf_output_tf}"
    
    diff --color=always -u0 --label="ENGINE" --label="Stockfish" "${engine_output_tf}" "${stockf_output_tf}" > "${diff_output_tf}"

    # no difference
    if [ "$?" -eq 0 ]
    then
        echo "***PASSED TEST*** FEN: ${fen} "

    #difference -> test failed
    else
        echo "!!!FAILED TEST!!! FEN: ${fen}"
        cat "${diff_output_tf}"
        exit 1
    fi

done < fens.txt

echo "================= ALL TESTS PASSED ===================="
echo

exit 0

