#!/bin/sh
#
# art-description: Athena runs calo reconstruction from a mc20e ESD file
# art-type: grid
# art-athena-mt: 8
# art-include: main/Athena
# art-include: 23.0/Athena
# art-output: *.log   

python -m RecExRecoTest.CaloTopoClusterReco_ESDMC21 | tee temp.log
echo "art-result: ${PIPESTATUS[0]}"
test_postProcessing_Errors.sh temp.log

