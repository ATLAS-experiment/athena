#!/usr/bin/env bash
#
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
set -ex
[ $# -eq 1 ] || { echo "$0: The only argument must be path to '${ATLAS_CTEST_PACKAGE}' package directory" >&2; exit 2; }

inputFiles=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.AOD_RUN3_MC[0])")
outputFile=${ATLAS_CTEST_TESTNAME}.txt

EventGUIDLookup_tf.py --inputFile "$inputFiles" --inputDataType AOD --eventList ${1}/share/${ATLAS_CTEST_TESTNAME}-in.txt --dataType HITS --outputTXTFile "$outputFile"

cat "$outputFile"
