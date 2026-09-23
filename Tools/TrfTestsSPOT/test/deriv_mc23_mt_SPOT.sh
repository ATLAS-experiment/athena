#!/usr/bin/bash
NTHREADS=${1}
FORMAT=${2}
NEVENTS=${3}
YEAR=${4}


if [ "${YEAR}" == "mc23d" ]; then
    DATAFILE="root://eosatlas.cern.ch//eos/atlas/atlascerngroupdisk/data-art/large-input/CampaignInputs/mc23/AOD/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.AOD.e8514_s4159_r15530/AOD.38803011._001713.pool.root.1"
elif [ "${YEAR}" == "mc23e" ]; then
    DATAFILE="root://eosatlas.cern.ch//eos/atlas/atlascerngroupdisk/data-art/large-input/CampaignInputs/mc23/AOD/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.AOD.e8514_s4369_r16083/AOD.41608496._001231.pool.root.1"
elif [ "${YEAR}" == "mc23g" ]; then
	DATAFILE="root://eosatlas.cern.ch//eos/atlas/atlascerngroupdisk/data-art/large-input/CampaignInputs/mc23/AOD/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.AOD.e8514_s4618_r17610/AOD.50092877._002250.pool.root.1"
else
    echo "Unknown year: ${YEAR}"
    exit 1
fi

# Run the jobexport TRF_ECHO=1;
# Run the job
source "$(dirname "${BASH_SOURCE[0]}")/spot_numa.sh"
spot_numa_setup "${NTHREADS}"

export TRF_ECHO=1;
echo "${SPOT_NUMA_INFO}" > __log.txt
ATHENA_CORE_NUMBER=${NTHREADS} \
${SPOT_NUMA_PREFIX} \
Derivation_tf.py \
      --CA 'True' \
      --maxEvents ${NEVENTS} \
      --perfmon 'fullmonmt' \
      --inputAODFile ${DATAFILE} \
      --outputDAODFile 'pool.root' \
      --multithreaded True \
      --formats ${FORMAT} > __log.txt 2>&1;

echo $? > __exitcode;
