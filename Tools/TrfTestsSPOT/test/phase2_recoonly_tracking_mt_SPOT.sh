#!/usr/bin/bash
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

NTHREADS=${1}
NEVENTS=${2}
RDOFile=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RDO_RUN4[0])")

conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")

# Run the job
export TRF_ECHO=1;
ATHENA_CORE_NUMBER=${NTHREADS} \
Reco_tf.py \
      --maxEvents ${NEVENTS} \
      --perfmon 'fullmonmt' \
      --CA 'all:True' \
      --multithreaded 'True' \
      --autoConfiguration 'everything' \
      --conditionsTag "default:${conditions}" \
      --postInclude 'all:PyJobTransforms.UseFrontier' \
      --preInclude 'all:Campaigns.MC23PhaseIIPileUp200' \
      --steering 'doRDO_TRIG' \
      --preExec 'all:flags.Tracking.doITkFastTracking=False' \
      --postExec 'all:cfg.getService("AlgResourcePool").CountAlgorithmInstanceMisses = True' \
      --inputRDOFile ${RDOFile} \
      --outputAODFile 'myAOD.pool.root' \
      --jobNumber '1' >  __log.txt 2>&1;

echo $? > __exitcode;
