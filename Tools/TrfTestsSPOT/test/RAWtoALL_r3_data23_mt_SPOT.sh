#!/usr/bin/bash
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

NTHREADS=${1}
NEVENTS=${2}
MINFIT=${3}

DATAFILE="/eos/atlas/atlascerngroupdisk/proj-spot/spot-job-inputs/data23_13p6TeV/data23_13p6TeV.00454083.physics_Main.daq.RAW._lb1100._SFO-11._0001.data"
CONDTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_DATA23)")
GEOTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN3)")

# Run the job
source "$(dirname "${BASH_SOURCE[0]}")/spot_numa.sh"
spot_numa_setup "${NTHREADS}"

export TRF_ECHO=1;
echo "${SPOT_NUMA_INFO}" > __log.txt
ATHENA_CORE_NUMBER=${NTHREADS} \
${SPOT_NUMA_PREFIX} \
Reco_tf.py \
      --CA  'True' \
      --perfmon 'fullmonmt' \
      --inputBSFile ${DATAFILE} \
      --maxEvents ${NEVENTS} \
      --outputAODFile 'myAOD.pool.root' \
      --outputHISTFile 'myHIST.root' \
      --multithreaded 'True' \
      --postExec "all:cfg.getService('PerfMonMTSvc').memFitLowerLimit = ${MINFIT};cfg.getService('AlgResourcePool').CountAlgorithmInstanceMisses = True;" \
      --autoConfiguration 'everything' \
      --conditionsTag ${CONDTAG} \
      --geometryVersion ${GEOTAG} \
      --runNumber '454083' \
      --steering 'doRAWtoALL' >> __log.txt 2>&1;
      
echo $? > __exitcode;
