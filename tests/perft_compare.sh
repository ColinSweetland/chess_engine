#!/usr/bin/env bash

# make sure current directory is PROJECTROOT/tests -- instead of directory script was ran from
cd -P -- "$(dirname -- "$0")"

# make sure our release executable is up to date
make -C .. -f Makefile all RELEASE=1 && echo

engine_exe=../output/chessengine-release
stockf_exe=stockfish

# pattern for isolating the lines with chessmoves in engine output 
move_pat="[a-h][1-8][a-h][1-8]:"

# depth to search
depth=5

error=0

while read fen; do

    diff --color=always -u0 --label="ENGINE" --label="Stockfish"\
    <(printf "position fen ${fen}\nperftdiv ${depth}\nquit" | ./${engine_exe} | grep -P ${move_pat} | sort) \
    <(printf "position fen ${fen}\ngo perft ${depth}\nquit" | ./${stockf_exe} | grep -P ${move_pat} | sort) > tmp.txt

    if [ "$?" -eq 1 ]
    then
        error=1
        echo "!!!FAILED TEST!!! FEN: ${fen}"
        cat tmp.txt

        break;
    else
        echo "***PASSED TEST*** FEN: ${fen} "
    fi

done < fens.txt

rm tmp.txt

if [[ "$error" -eq 1 ]]
then
    exit 1
else
    echo
    echo "test successful!!! :)"
    exit 0
fi
