#!/bin/sh
#
# art-description: Athena runs tau reconstruction from a RAW data22 file
# art-type: grid
# art-athena-mt: 8
# art-include: main/Athena
# art-include: 23.0/Athena
# art-output: *.log   

python -m RecExRecoTest.TauReco_RAW_data22_13p6TeV | tee temp.log
echo "art-result: ${PIPESTATUS[0]}"
test_postProcessing_Errors.sh temp.log

