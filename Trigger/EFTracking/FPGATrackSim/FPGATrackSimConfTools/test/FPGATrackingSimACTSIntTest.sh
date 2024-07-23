
rdo_23p0=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.900498.PG_single_muonpm_Pt100_etaFlatnp0_43.recon.RDO.e8481_s4149_r14697/RDO.33675668._000016.pool.root.1
Reco_tf.py \
    --steering doRAWtoALL \
    --preExec "flags.Trigger.FPGATrackSim.wrapperFileName=None" \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsValidateTracksFlags" \
    --postInclude "FPGATrackSimConfTools.FPGATrackSimAnalysisConfig.FPGATrackSimLogicalHistProcessAlgCfg" \
    --inputRDOFile ${rdo_23p0} \
    --outputAODFile AOD.root \
    --maxEvents 100
