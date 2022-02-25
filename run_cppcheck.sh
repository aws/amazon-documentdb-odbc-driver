#!/bin/bash

RESULTS_FILE=cppcheck-results.log

# --force: force checks all define combinations (default max is 12)
# -iaws-sdk-cpp: avoid checking AWS C++ SDK source files in our repo
cppcheck --force --library=boost --library=documentdb -I ./src/binary/include/ignite/binary/ -UWIN32 ./src/ 2>  ${RESULTS_FILE}

if [ -s ${RESULTS_FILE} ]; then
    echo "!! Cppcheck errors found! Check ${RESULTS_FILE} for details."
    exit 1
else
    echo "No Cppcheck errors found."
fi