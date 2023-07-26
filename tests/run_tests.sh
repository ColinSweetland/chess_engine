#!/bin/sh
set -u

# This runs all test scripts we currently have

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

# make sure current directory is PROJECTROOT/tests -- instead of directory script was ran from (tests are stored there)
cd -P -- "$(dirname -- "${0}")" &&
./perft_compare_test.sh "${1}" &&
./fen_serialization_test.sh "${1}" &&
./best_move_tests.sh "${1}"

exit 0

