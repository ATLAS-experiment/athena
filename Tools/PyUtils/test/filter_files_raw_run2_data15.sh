
#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
set -eux
[ $# -eq 1 ] || { echo "$0: The only argument must be path to '${ATLAS_CTEST_PACKAGE}' package directory" >&2; exit 2; }

inputFiles=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RAW_RUN2_DATA15[0])")
outputFile=${ATLAS_CTEST_TESTNAME}.RAW.data
rm -f $outputFile

acmd filter-files --selection ${1}/share/${ATLAS_CTEST_TESTNAME}.ref --output "$outputFile" $inputFiles

eventList=${ATLAS_CTEST_TESTNAME}.txt

acmd list-events --output "$eventList" "$outputFile"
cat "$eventList"
