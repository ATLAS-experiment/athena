#!/usr/bin/bash
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration


FORMAT=${1}
NEVENTS=${2}
YEAR=${3}

if [ "${YEAR}" == "mc23a" ]; then
	DATAFILE=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.AOD_RUN3_MC[0])")
elif [ "${YEAR}" == "mc23d" ]; then
	DATAFILE="root://eosatlas.cern.ch//eos/atlas/atlascerngroupdisk/data-art/large-input/CampaignInputs/mc23/AOD/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.AOD.e8514_s4159_r15530/AOD.38803011._001713.pool.root.1"
elif [ "${YEAR}" == "mc23e" ]; then
    DATAFILE="root://eosatlas.cern.ch//eos/atlas/atlascerngroupdisk/data-art/large-input/CampaignInputs/mc23/AOD/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.AOD.e8514_s4159_r15530/AOD.38803011._001713.pool.root.1"
elif [ "${YEAR}" == "mc23g" ]; then
    DATAFILE="root://eosatlas.cern.ch//eos/atlas/atlascerngroupdisk/data-art/large-input/CampaignInputs/mc23/AOD/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.AOD.e8514_s4369_r16083/AOD.41608496._001231.pool.root.1"
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
