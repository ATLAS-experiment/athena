#!/usr/bin/bash
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

TESTNAME=${1}
REFFILENAME=${TESTNAME}".ref"

if [[ -z "${TESTNAME}" ]]; then
    echo "Empty test name"
    exit 1
fi

echo "Running ${TESTNAME} test ..."
echo "Outcome will be compared against reference file: ${REFFILENAME}"

# search in $DATAPATH for matching file
refFileAbsPath=$(find -H ${DATAPATH//:/ } -mindepth 1 -maxdepth 2 -name ${REFFILENAME} -print -quit 2>/dev/null)
if [ -z "$refFileAbsPath" ]; then
    echo "Could not identify ref file for this test with name ${REFFILENAME}"
    exit 1
fi

echo "Reference file has been resolved to: ${refFileAbsPath}"

echo "Running reconstruction job ..."
source ${TESTNAME}.sh >& countsObjects.txt

rc=$?
if [ $rc != 0 ]; then
    echo ">>>>>>>>>>>>>>>> here is the full log (log.RAWtoALL):"
    cat log.RAWtoALL
    exit $rc
fi

echo "Comparing against reference"
diff ${refFileAbsPath} countsObjects.txt
ret=$?
if [[ $ret -eq 0 ]]; then
    echo "  -- SUCCESS"
    exit 0
else
    echo "  -- FAILURE"
    exit 1
fi
