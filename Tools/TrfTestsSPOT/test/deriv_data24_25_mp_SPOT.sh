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

# Run the jobexport TRF_ECHO=1;
export ATHENA_CORE_NUMBER=${NTHREADS}
Derivation_tf.py \
      --maxEvents ${NEVENTS} \
      --perfmon 'fullmonmt' \
      --inputAODFile ${DATAFILE} \
      --outputDAODFile 'pool.root' \
      --multiprocess 'True' \
      --multithreadedFileValidation 'False' \
      --postInclude "default:AthenaServices.TransformUtils.ExecCondAlgsAtPreFork" \
      --athenaopts "Trigger.triggerConfig='DB'" \
      --athenaMPMergeTargetSize 'DAOD_*:0' \
      --sharedWriter 'true' \
      --parallelCompression 'true'\
      --formats ${FORMAT} > __log.txt 2>&1;

echo $? > __exitcode;
