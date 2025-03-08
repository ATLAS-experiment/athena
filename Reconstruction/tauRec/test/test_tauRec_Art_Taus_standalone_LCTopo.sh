#!/bin/sh
#
# art-description: Athena runs tau reconstruction, using the new job configuration and LCTopo jets as tau seed
# art-type: grid
# art-athena-mt: 8
# art-include: main/Athena
# art-include: 23.0/Athena
# art-output: *.log   

python -m tauRec.runTauOnly_LCTopo | tee temp.log
echo "art-result: ${PIPESTATUS[0]}"
RecExRecoTest_postProcessing_Errors.sh temp.log


