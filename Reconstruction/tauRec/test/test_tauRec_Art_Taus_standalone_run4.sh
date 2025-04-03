#!/bin/sh
#
# art-description: Athena runs tau reconstruction, using the new job configuration, Run4 input and LCTopo jets as tau seed
# art-type: grid
# art-athena-mt: 8
# art-include: main/Athena
# art-output: *.log   

python -m tauRec.runTauOnly_run4 | tee temp.log
echo "art-result: ${PIPESTATUS[0]}"
RecExRecoTest_postProcessing_Errors.sh temp.log

