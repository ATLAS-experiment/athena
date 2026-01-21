#!/usr/bin/bash
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

NTHREADS=${1}
NEVENTS=${2}

DATAFILE="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc23/EVNT/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8514/EVNT.32288062._002040.pool.root.1"
CONDTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")
GEOTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN3)")


# Run the job
export TRF_ECHO=1;
ATHENA_CORE_NUMBER=${NTHREADS} \
Sim_tf.py \
      --perfmon 'fullmonmt' \
      --CA 'True'\
      --multithreaded 'True' \
      --inputEVNTFile ${DATAFILE} \
      --conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-07' \
      --geometryVersion 'default:ATLAS-R3S-2021-03-02-00'   \
      --postExec 'all:cfg.getService("AlgResourcePool").CountAlgorithmInstanceMisses = True' \
      --AMIConfig 's4006' \
      --outputHITSFile 'myHITS.pool.root' \
      --maxEvents ${NEVENTS} \
      --jobNumber '1'  > __log.txt 2>&1;

# Get the exit code
echo $? > __exitcode;
