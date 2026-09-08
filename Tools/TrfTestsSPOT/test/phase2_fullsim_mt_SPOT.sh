#!/usr/bin/bash
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

NTHREADS=${1}
NEVENTS=${2}
DATAFILE='/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EVNT/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8481/EVNT.33964680._002197.pool.root.1'

geometry=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN4)")
conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")


# Run the job
source "$(dirname "${BASH_SOURCE[0]}")/spot_numa.sh"
spot_numa_setup "${NTHREADS}"

export TRF_ECHO=1;
echo "${SPOT_NUMA_INFO}" > __log.txt
ATHENA_CORE_NUMBER=${NTHREADS} \
${SPOT_NUMA_PREFIX} \
Sim_tf.py \
      --multithreaded 'True' \
      --maxEvents ${NEVENTS} \
      --perfmon 'fullmonmt' \
      --geometryVersion "default:${geometry}" \
      --conditionsTag "default:${conditions}" \
      --postInclude 'default:PyJobTransforms.UseFrontier' \
      --preInclude 'EVNTtoHITS:Campaigns.MC23PhaseIISimulation' \
      --postExec 'all:cfg.getService("AlgResourcePool").CountAlgorithmInstanceMisses = True' \
      --simulator 'FullG4MT' \
      --inputEVNTFile ${DATAFILE} \
      --outputHITSFile 'myHITS.pool.root' >> __log.txt 2>&1;

echo $? > __exitcode;
