#!/usr/bin/bash
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

NTHREADS=${1}
NEVENTS=${2}
DATADIR="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO"

# Run the job
export TRF_ECHO=1;
ATHENA_CORE_NUMBER=${NTHREADS} Reco_tf.py \
  --maxEvents  ${NEVENTS} \
  --perfmon 'fullmonmt' \
  --preExec "from Campaigns import PhaseIINoPileUp; \
             PhaseIINoPileUp(flags); \
	     flags.Acts.doAnalysis=False; \
	     flags.Detector.EnableHGTD=False;" \
  --postExec "cfg.getService(\"AlgResourcePool\").CountAlgorithmInstanceMisses=True;" \
  --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsHeavyIonFlags" \
  --autoConfiguration 'everything' \
  --conditionsTag 'default:OFLCOND-MC21-SDR-RUN4-01' \
  --geometryVersion 'all:ATLAS-P2-RUN4-03-00-01' \
  --postInclude 'all:PyJobTransforms.UseFrontier' \
  --steering 'doRAWtoALL' \
  --inputRDOFile ${DATADIR}"/ATLAS-P2-RUN4-03-00-01/mc23_5p36TeV.860167.Hijing_PbPb_MinBias_Flow_JJFV6.evgen.RDO.e8548_s4345/*" \
  --outputAODFile 'myAOD.pool.root' \
  --jobNumber '1' \
  --multithreaded
