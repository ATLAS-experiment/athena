#!/usr/bin/bash
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# ttbar mu=200 input
input_rdo=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/RDO.33629020._000047.pool.root.1
n_events=2

# Run reconstruction and produce AOD with persistified Acts EDM
export ATHENA_CORE_NUMBER=1
Reco_tf.py --CA \
  --preExec "flags.Exec.FPE=500;" "flags.Acts.EDM.PersistifyClusters=True;flags.Acts.EDM.PersistifySpacePoints=True;flags.Acts.EDM.PersistifyTracks=True;" \
  --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsWorkflowFlags" \
  --postInclude "InDetConfig.InDetPrepRawDataFormationConfig.HGTDInDetToXAODClusterConversionCfg,ActsConfig.ActsPostIncludes.PersistifyActsEDMCfg" \
  --inputRDOFile ${input_rdo} \
  --outputAODFile AOD.pool.root \
  --maxEvents ${n_events} \
  --multithreaded

rc=$?
if [ $rc != 0 ]; then
    exit $rc
fi

checkxAOD.py AOD.pool.root

# Check we can retrieve the EDM, and related quantities, with our analysis algorithms
ActsReadEDM.py \
   --filesInput AOD.pool.root -- \
   readClusters=True \
   readSpacePoints=True \
   readTracks=True \
   tracks="ActsTracks"

rc=$?
if [ $rc != 0 ]; then
    exit $rc
fi

# Check we can run IDPVM
runIDPVM.py \
   --filesInput AOD.pool.root \
   --outputFile idpvm.root \
   --doActs
