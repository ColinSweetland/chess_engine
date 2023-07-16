#!/bin/sh
set -u

# This runs all test scripts we currently have

# make sure current directory is PROJECTROOT/tests -- instead of directory script was ran from
cd -P -- "$(dirname -- "$0")" || exit 2

if [ -z "${1-}" ]
then
    echo "usage: ${0} [engine executable to test]"
    exit 2
fi

if [ ! -x "${1}" ]
then
    echo "engine exe ${1} couldn't be found or not executable"
    exit 2
fi

./perft_compare_test.sh "${1}" &&
./fen_serialization_test.sh "${1}"

exit 0

