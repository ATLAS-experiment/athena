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
