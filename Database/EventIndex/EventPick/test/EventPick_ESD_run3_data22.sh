#!/usr/bin/env bash
#
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
set -ex
[ $# -eq 1 ] || { echo "$0: The only argument must be path to '${ATLAS_CTEST_PACKAGE}' package directory" >&2; exit 2; }

inputFiles=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.ESD_RUN3_DATA22[0])")
outputFile=${ATLAS_CTEST_TESTNAME}.pool.root

EventPick_tf.py --inputESDFiles "$inputFiles" --outputESD_PICKEDFile "$outputFile" --eventList ${1}/share/${ATLAS_CTEST_TESTNAME}.ref

pickedEventList=${ATLAS_CTEST_TESTNAME}.txt

EventInfo.py --inputFiles "$outputFile" --outputFile "$pickedEventList"
cat "$pickedEventList"
