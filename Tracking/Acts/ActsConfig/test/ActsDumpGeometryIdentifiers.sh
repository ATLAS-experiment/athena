#!/usr/bin/bash
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Informations:
# This script shows how to include the dumper of the tracking geometry detector elements Acts and Athena identifiers
# and their positions. The result will be stored in a csv file (default: transforms.csv)


# ttbar mu=200 input
input_rdo=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/RDO.33629020._000047.pool.root.1
n_events=1

export ATHENA_CORE_NUMBER=1
Reco_tf.py \
  --preExec "flags.Exec.FPE=-1;" \
  --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude" \
  --postInclude "ActsConfig.ActsGeometryConfig.ActsWriteTrackingGeometryTransformsAlgCfg"\
  --inputRDOFile ${input_rdo} \
  --outputAODFile AOD.validateclusters.pool.root \
  --maxEvents ${n_events} \
  --multithreaded
