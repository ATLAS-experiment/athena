#!/bin/sh
#
# art-description: Athena runs Standard ESD pflow with topotowers input,jet,tau and met reconstruction
# art-type: grid
# art-include: main/Athena
# art-athena-mt: 8

export ATHENA_CORE_NUMBER=8 # set number of cores used in multithread to 8.

python -m eflowRec.PFRunESDtoAOD_TopoTowers_mc21_14TeV | tee temp.log
echo "art-result: ${PIPESTATUS[0]}"
test_postProcessing_Errors.sh temp.log
