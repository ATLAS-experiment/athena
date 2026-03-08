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
          --multithreaded 'True' \
          --autoConfiguration 'everything' \
          --conditionsTag "default:${conditions}" \
          --postInclude 'all:PyJobTransforms.UseFrontier' \
          --preInclude 'all:Campaigns.PhaseIIPileUp200' \
          --steering 'doRAWtoALL' \
          --preExec 'all:flags.Tracking.doITkFastTracking=True' \
          --postExec 'all:cfg.getService("AlgResourcePool").CountAlgorithmInstanceMisses = True' \
          --inputRDOFile ${RDOFile} \
          --outputAODFile 'myAOD.pool.root' >  __log.txt 2>&1;

echo $? > __exitcode;
