#!/usr/bin/bash
NTHREADS=${1}
FORMAT=${2}
NEVENTS=${3}
YEAR=${4}


if [ "${YEAR}" == "data24" ]; then
    DATAFILE="/eos/atlas/atlascerngroupdisk/proj-spot/spot-job-inputs/AODtoDAOD/data24/data24_13p6TeV.00485051.physics_Main.merge.AOD.f1518_m2248._lb0092._0002.1"
elif [ "${YEAR}" == "data25" ]; then
    DATAFILE="/eos/atlas/atlascerngroupdisk/data-art/large-input/CampaignInputs/data25/AOD/data25_13p6TeV.00498515.physics_Main.merge.AOD.r17521_p7232/AOD.49752827._000024.pool.root.1"
else
    echo "Unknown year: ${YEAR}"
    exit 1
fi

# Run the job
source "$(dirname "${BASH_SOURCE[0]}")/spot_numa.sh"
spot_numa_setup "${NTHREADS}"

export TRF_ECHO=1;
echo "${SPOT_NUMA_INFO}" > __log.txt
ATHENA_CORE_NUMBER=${NTHREADS} \
${SPOT_NUMA_PREFIX} \
Derivation_tf.py \
      --multithreaded True \
      --maxEvents ${NEVENTS} \
      --perfmon 'fullmonmt' \
      --inputAODFile ${DATAFILE} \
      --outputDAODFile 'pool.root' \
      --multithreadedFileValidation 'False' \
      --formats ${FORMAT} > __log.txt 2>&1;

echo $? > __exitcode;
