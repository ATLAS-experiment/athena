#!/bin/sh
#
# art-description: Athena runs Standard ESD pflow,jet,tau and met reconstruction with pflow CPData added to output
# art-type: grid
# art-include: main/Athena
# art-athena-mt: 8

export ATHENA_CORE_NUMBER=8 # set number of cores used in multithread to 8.

python -m eflowRec.PFRunESDtoAOD_WithJetsTausMET_CPData_mc21_13p6TeV | tee temp.log
echo "art-result: ${PIPESTATUS[0]}"
RecExRecoTest_postProcessing_Errors.sh temp.log
