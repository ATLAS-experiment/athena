#!/usr/bin/bash
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

FORMAT=${1}
NEVENTS=${2}
YEAR=${3}

if [ "${YEAR}" == "data22" ]; then
    DATAFILE="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/data22/AOD/data22_13p6TeV.00431906.physics_Main.merge.AOD.r13928_p5279/1000events.AOD.30220215._001367.pool.root.1"
elif [ "${YEAR}" == "data23" ]; then
    DATAFILE="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/data23/AOD/data23_13p6TeV.00453713.physics_Main.recon.AOD.f1357/2012events.data23_13p6TeV.00453713.physics_Main.recon.AOD.f1357._lb1416._0006.1"
elif [ "${YEAR}" == "data24" ]; then
    DATAFILE="/eos/atlas/atlascerngroupdisk/proj-spot/spot-job-inputs/AODtoDAOD/data24/data24_13p6TeV.00485051.physics_Main.merge.AOD.f1518_m2248._lb0092._0002.1"
elif [ "${YEAR}" == "data25" ]; then
    DATAFILE="/eos/atlas/atlascerngroupdisk/data-art/large-input/CampaignInputs/data25/AOD/data25_13p6TeV.00498515.physics_Main.merge.AOD.r17521_p7232/AOD.49752827._000024.pool.root.1"
else
    echo "Unknown year: ${YEAR}"
    exit 1
fi
# Run the job
export TRF_ECHO=1;
Derivation_tf.py \
      --CA 'True' \
      --maxEvents ${NEVENTS} \
      --perfmon 'fullmonmt' \
      --inputAODFile ${DATAFILE} \
      --outputDAODFile 'pool.root' \
      --formats ${FORMAT} > __log.txt 2>&1;

 echo $? > __exitcode;
