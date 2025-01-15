#!/bin/sh
#
# art-description: Athena runs Standard ESD pflow,jet,tau and met reconstruction with pflow truth cheating enabled
# art-type: grid
# art-include: main/Athena
# art-athena-mt: 8

export ATHENA_CORE_NUMBER=8 # set number of cores used in multithread to 8.

python -m eflowRec.PFRunESDtoAOD_WithJetsTausMET_TruthCheating_mc21_13p6TeV | tee temp.log
echo "art-result: ${PIPESTATUS[0]}"
RecExRecoTest_postProcessing_Errors.sh temp.log
