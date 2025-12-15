#!/usr/bin/bash
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration


FORMAT=${1}
NEVENTS=${2}
DATAFILE='/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/data22/AOD/data22_13p6TeV.00431906.physics_Main.merge.AOD.r13928_p5279/1000events.AOD.30220215._001367.pool.root.1'

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
