#!/bin/bash
set -e

RDO=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.900498.PG_single_muonpm_Pt100_etaFlatnp0_43.recon.RDO.e8481_s4149_r14697/RDO.33675668._000016.pool.root.1
RDO_EVT=200

echo "... RDO to AOD with sim"
Reco_tf.py --CA \
    --steering doRAWtoALL \
    --preExec "flags.Trigger.FPGATrackSim.wrapperFileName='wrapper.root'" \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsValidateTracksFlags" \
    --postInclude "FPGATrackSimSGInput.FPGATrackSimSGInputConfig.FPGATrackSimSGInputCfg" \
    --inputRDOFile ${RDO} \
    --outputAODFile AOD.pool.root \
    --maxEvents ${RDO_EVT}
ls -l
echo "... RDO to AOD with sim, this part is done ..."