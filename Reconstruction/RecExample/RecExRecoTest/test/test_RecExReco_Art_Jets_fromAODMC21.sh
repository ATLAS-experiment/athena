#!/bin/sh
#
# art-description: Athena runs jet reconstruction from an ESD file
# art-type: grid
# art-athena-mt: 8
# art-include: main/Athena
# art-include: 24.0/Athena
# art-output: *.log   

python -m RecExRecoTest.Jets_AODMC21 --threads=8 | tee temp.log
echo "art-result: ${PIPESTATUS[0]}"
test_postProcessing_Errors.sh temp.log

