#!/usr/bin/bash
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

NTHREADS=${1}
NEVENTS=${2}

DATAFILE="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/HITS/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.simul.HITS.e8453_s3873/1000events_singleBS.HITS.pool.root"
CONDTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")
GEOTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN3)")


# Run the job
export TRF_ECHO=1;
ATHENA_CORE_NUMBER=${NTHREADS} \
Reco_tf.py \
      --AMI 'q445' \
      --CA 'default:True' \
      --perfmon 'fullmonmt' \
      --maxEvents ${NEVENTS} \
      --outputAODFile 'myAOD.pool.root' \
      --multithreaded 'True' \
      --conditionsTag ${CONDTAG} \
      --inputHITSFile ${DATAFILE} \
      --runNumber=601229 \
      --deleteIntermediateOutputfiles 'True' \
      --steering 'doRDO_TRIG' 'doTRIGtoALL' > __log.txt 2>&1;

echo $? > __exitcode;
