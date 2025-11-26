#!/usr/bin/bash
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

NTHREADS=${1}
NEVENTS=${2}

DATAFILE="/eos/atlas/atlascerngroupdisk/proj-spot/spot-job-inputs/HI/data23_hi.00463427.physics_HardProbes.daq.RAW._lb0585._SFO-11._0001.data"
CONDTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_DATA23)")
GEOTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN3)")

# Run the job
export TRF_ECHO=1;
ATHENA_CORE_NUMBER=${NTHREADS} \
Reco_tf.py \
          --CA 'True' \
          --maxEvents ${NEVENTS} \
          --perfmon 'fullmonmt' \
          --multithreaded 'True' \
          --conditionsTag  ${CONDTAG} \
          --geometryVersion ${GEOTAG} \
          --preExec 'flags.Reco.HIMode=HIMode.HI' \
          --postExec 'all:cfg.getService("AlgResourcePool").CountAlgorithmInstanceMisses = True' \
          --inputBSFile ${DATAFILE} \
          --outputAODFile 'myAOD.pool.root' >  __log.txt 2>&1;
      
echo $? > __exitcode;
