#!/bin/sh
#
# art-description: Athena runs MET reconstruction from an ESD file using CA
# art-type: grid
# art-include: main/Athena
# art-include: 23.0/Athena
# art-athena-mt: 8
# art-output: *.log   

python -m RecExRecoTest.METReco_ESDMC21 | tee temp.log
echo "art-result: ${PIPESTATUS[0]}"
test_postProcessing_Errors.sh temp.log
