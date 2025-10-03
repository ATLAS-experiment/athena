#!/bin/bash
set -e

source FPGATrackSim_CommonEnv.sh

echo "... RDO to AOD with sim"
Reco_tf.py \
    --steering doRAWtoALL \
    --preExec "flags.Trigger.FPGATrackSim.wrapperFileName='wrapper.root'" \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsValidateTracksFlags" \
    --postInclude "FPGATrackSimSGInput.FPGATrackSimSGInputConfig.FPGATrackSimSGInputCfg" \
    --inputRDOFile ${RDO_SINGLE_MUON} \
    --outputAODFile AOD.pool.root \
    --maxEvents ${RDO_EVT}
ls -l
echo "... RDO to AOD with sim, this part is done ..."
