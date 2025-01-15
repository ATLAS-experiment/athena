#!/bin/bash
#
# art-description: Athena runs standard pflow phys val
# art-type: grid
# art-include: main/Athena

python -m PFODQA.PFPhysValGlobalContainersCfg | tee temp.log
echo "art-result: ${PIPESTATUS[0]}"
RecExRecoTest_postProcessing_Errors.sh temp.log