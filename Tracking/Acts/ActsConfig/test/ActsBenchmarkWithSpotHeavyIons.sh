#!/usr/bin/bash
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

NTHREADS=${1}
NEVENTS=${2}
DATADIR="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO"
conditions_tag=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")

# Run the job
export TRF_ECHO=1;
ATHENA_CORE_NUMBER=${NTHREADS} Reco_tf.py \
  --maxEvents  ${NEVENTS} \
  --perfmon 'fullmonmt' \
  --preExec "flags.Acts.doAnalysis=False; \
	     flags.Detector.EnableHGTD=False;" \
  --postExec "cfg.getService(\"AlgResourcePool\").CountAlgorithmInstanceMisses=True;" \
  --preInclude "Campaigns.MC23PhaseIINoPileUp,InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsHeavyIonFlags" \
  --conditionsTag ${conditions_tag} \
  --geometryVersion 'all:ATLAS-P2-RUN4-03-00-01' \
  --postInclude 'all:PyJobTransforms.UseFrontier' \
  --steering 'doRAWtoALL' \
  --inputRDOFile ${DATADIR}"/ATLAS-P2-RUN4-03-00-01/mc23_5p36TeV.860167.Hijing_PbPb_MinBias_Flow_JJFV6.evgen.RDO.e8548_s4345/*" \
  --outputAODFile 'myAOD.pool.root' \
  --jobNumber '1' \
  --multithreaded
