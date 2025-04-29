#!/bin/bash
#
# art-description: Athena runs standard pflow phys val
# art-type: grid
# art-include: main/Athena

Derivation_tf.py --CA 'all:True' --sharedWriter 'True' --validationFlags 'doPFlow_FlowElements' --maxEvents=100 --inputDAOD_PHYSVALFile=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PFlowTests/valid1/valid1.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.deriv.DAOD_PHYSVAL.e8514_a945_s4374_r16157_p6557/DAOD_PHYSVAL.42163833._000004.pool.root.1 --outputNTUP_PHYSVALFile=physval.root | tee temp.log
echo "art-result: ${PIPESTATUS[0]}"
RecExRecoTest_postProcessing_Errors.sh temp.log