#!/bin/bash
set -e

GEO_TAG="ATLAS-P2-RUN4-03-00-00"
RDO="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/${GEO_TAG}/RDO/reg0_singlemu.root"
RDO_EVT=200

echo "... RDO to AOD with sim"
Reco_tf.py \
    --steering doRAWtoALL \
    --preExec "flags.Trigger.FPGATrackSim.wrapperFileName='wrapper.root'" \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsValidateTracksFlags" \
    --postInclude "FPGATrackSimSGInput.FPGATrackSimSGInputConfig.FPGATrackSimSGInputCfg" \
    --inputRDOFile ${RDO} \
    --outputAODFile AOD.pool.root \
    --maxEvents ${RDO_EVT}
ls -l
echo "... RDO to AOD with sim, this part is done ..."
