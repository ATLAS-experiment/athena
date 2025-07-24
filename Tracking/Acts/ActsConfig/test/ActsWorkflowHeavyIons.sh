#!/usr/bin/bash
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# HI configuration with ITk
input_rdo=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-01/RDO_HIJING_ITk_lowstat.pool.root
n_events=5

export ATHENA_CORE_NUMBER=1
Reco_tf.py \
  --preExec "flags.Exec.FPE=-1;" "from Campaigns import PhaseIINoPileUp; PhaseIINoPileUp(flags);" \
  --preInclude "all:InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude" "all:ActsConfig.ActsCIFlags.actsHeavyIonFlags" \
  --postInclude "all:PyJobTransforms.UseFrontier" \
  --autoConfiguration everything \
  --inputRDOFile ${input_rdo} \
  --outputAODFile AOD.pool.root \
  --maxEvents ${n_events} \
  --multithreaded
