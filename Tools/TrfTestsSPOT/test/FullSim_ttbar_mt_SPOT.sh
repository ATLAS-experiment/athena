#!/usr/bin/bash
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

NTHREADS=${1}
NEVENTS=${2}

DATAFILE="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc23/EVNT/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8514/EVNT.32288062._002040.pool.root.1"
CONDTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")
GEOTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN3)")


# Run the job
source "$(dirname "${BASH_SOURCE[0]}")/spot_numa.sh"
spot_numa_setup "${NTHREADS}"

export TRF_ECHO=1;
echo "${SPOT_NUMA_INFO}" > __log.txt
ATHENA_CORE_NUMBER=${NTHREADS} \
${SPOT_NUMA_PREFIX} \
Sim_tf.py \
      --perfmon 'fullmonmt' \
      --multithreaded 'True' \
      --inputEVNTFile ${DATAFILE} \
      --conditionsTag 'default:${CONDTAG}' \
      --geometryVersion 'default:${GEOTAG}' \
      --postExec 'all:cfg.getService("AlgResourcePool").CountAlgorithmInstanceMisses = True' \
      --AMIConfig 's4369' \
      --outputHITSFile 'myHITS.pool.root' \
      --maxEvents ${NEVENTS} \
      --jobNumber '1'  >> __log.txt 2>&1;

# Get the exit code
echo $? > __exitcode;
