#!/usr/bin/env sh
#
# Post-processing script to be used with atlas_add_test if no
# log file post-processing is required (as done in post.sh)

if [ -z "$ATLAS_CTEST_TESTSTATUS" ]; then
   echo "nopost.sh> Error: athena exit status is not available (\$ATLAS_CTEST_TESTSTATUS is not set)"
   exit 1
fi

joblog=${ATLAS_CTEST_TESTNAME}.log
if [ ${ATLAS_CTEST_TESTSTATUS} != 0 ]; then
    echo "$RED nopost.sh> ERROR: Test ${ATLAS_CTEST_TESTNAME} failed with exit code: ${ATLAS_CTEST_TESTSTATUS}$RESET"
    tail $joblog
    echo  " noerror.sh> Please check ${PWD}/${joblog}"
fi

exit $ATLAS_CTEST_TESTSTATUS
