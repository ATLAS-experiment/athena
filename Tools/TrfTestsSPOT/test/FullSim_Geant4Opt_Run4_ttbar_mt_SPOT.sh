#!/usr/bin/bash
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

NTHREADS=${1}
NEVENTS=${2}
DATAFILE='/eos/atlas/atlascerngroupdisk/data-art/grid-input/PhaseIIUpgrade/EVNT/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8481/EVNT.33964680._002197.pool.root.1'
CONDTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")
GEOTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN4)")


# Run the job
export TRF_ECHO=1;
ATHENA_CORE_NUMBER=${NTHREADS} \
Sim_tf.py \
      --CA 'True' \
      --multithreaded 'True' \
      --maxEvents ${NEVENTS} \
      --perfmon 'fullmonmt' \
      --conditionsTag 'default:${CONDTAG}' \
      --geometryVersion 'default:${GEOTAG}' \
      --postInclude 'default:PyJobTransforms.UseFrontier' \
      --preInclude 'EVNTtoHITS:Campaigns.MC23PhaseIISimulation' \
      --postExec 'all:cfg.getService("AlgResourcePool").CountAlgorithmInstanceMisses = True' \
      --simulator 'FullG4MT_QS' \
      --inputEVNTFile ${DATAFILE} \
      --outputHITSFile 'myHITS.pool.root' > __log.txt 2>&1;

echo $? > __exitcode;
