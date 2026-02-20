
#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
set -ex
[ $# -eq 1 ] || { echo "$0: The only argument must be path to '${ATLAS_CTEST_PACKAGE}' package directory" >&2; exit 2; }

inputFiles=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.AOD_RUN2_DATA[0])")
outputFile=${ATLAS_CTEST_TESTNAME}.pool.root

EventPicking.py --eventList ${1}/share/${ATLAS_CTEST_TESTNAME}.ref --inputFiles "$inputFiles" --outputFile "$outputFile"

eventList=${ATLAS_CTEST_TESTNAME}.txt

EventInfo.py --inputFiles "$outputFile" --outputFile "$eventList"
cat "$eventList"
