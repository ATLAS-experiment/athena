#!/bin/sh

# art-description: RDOtoBStoAOD for Run 4
# art-type: grid
# art-include: main/Athena
# art-athena-mt: 8

: ${events:=100} #Allow overwriting from command line
RDOFile=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RDO_RUN4[0])")
conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")
geometry="ATLAS-P2-RUN4-04-00-00" # Should match RDO input file, which might not use the default geotag
Reco_tf.py \
--inputRDOFile ${RDOFile} \
--outputBSFile created.BS \
--preExec "flags.Detector.EnableITkStrip=False" \
--maxEvents=${events}

Reco_tf.py \
--inputBSFile created.BS \
--outputAODFile AOD.ttbar.fromBS.pool.root \
--conditionsTag ${conditions} \
--geometryVersion ${geometry} \
--preExec "flags.Tracking.doTruth=False;flags.Tracking.doITkFastTracking=True;flags.Reco.PostProcessing.GeantTruthThinning=False;flags.Reco.EnableHGTDExtension=False"
rc1=$?
echo "art-result: ${rc1} Reco_tf_rdo_to_bs_to_aod"

# Check for FPEs in the logiles
test_trf_check_fpe.sh
fpeStat=$?

echo "art-result: ${fpeStat} FPEs in logfiles"
exit $rc1
