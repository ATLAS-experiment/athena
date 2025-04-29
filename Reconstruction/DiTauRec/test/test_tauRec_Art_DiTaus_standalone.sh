#!/bin/sh
#
# art-description: Athena runs ditau reconstruction
# art-type: grid
# art-athena-mt: 8
# art-include: main/Athena
# art-output: *.log   

python -m DiTauRec.StandAloneDiTauBuilder | tee temp.log
echo "art-result: ${PIPESTATUS[0]}"
RecExRecoTest_postProcessing_Errors.sh temp.log



