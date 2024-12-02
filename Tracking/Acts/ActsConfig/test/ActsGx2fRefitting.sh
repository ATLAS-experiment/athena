#!/usr/bin/bash
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# ttbar mu=200 input
input_rdo=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/RDO.33629020._000047.pool.root.1
n_events=5

# Note: To fit from PrepRawData instead of RIO_OnTrack:
#  1) use the --preExec option: flags.Acts.fitFromPRD=True (in addition to all the other ones needed)
#  2) in addition to only use the --postInclude option:  ActsConfig.ActsTrackFittingConfig.forceITkActsReFitterAlgCfg

export ATHENA_CORE_NUMBER=1
Reco_tf.py \
   --preExec "flags.Exec.FPE=-1;" \
   --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsValidateGX2FFlags" \
   --postInclude "ActsConfig.ActsTrackFittingConfig.ActsReFitterAlgCfg" \
   --inputRDOFile ${input_rdo} \
   --outputESDFile ESD.pool.root \
   --outputAODFile AOD.pool.root \
   --maxEvents ${n_events} \
