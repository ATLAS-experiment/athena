#!/usr/bin/bash
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration


FORMAT=${1}
NEVENTS=${2}
DATAFILE='/eos/atlas/atlascerngroupdisk/proj-spot/spot-job-inputs/AODtoDAOD/mc21a/myAOD.pool.root'

# Run the job
export TRF_ECHO=1;
Derivation_tf.py \
      --CA 'True' \
      --maxEvents ${NEVENTS} \
      --perfmon 'fullmonmt' \
      --inputAODFile ${DATAFILE} \
      --outputDAODFile 'pool.root' \
      --preExec="flags.Output.StorageTechnology.EventData={\"*\":\"ROOTRNTUPLE\"};" \
      --formats ${FORMAT} > __log.txt 2>&1;

 echo $? > __exitcode;
