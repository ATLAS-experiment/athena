#!/usr/bin/bash
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

NTHREADS=${1}
NEVENTS=${2}
DATAFILE="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc23/EVNT/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8514/EVNT.32288062._002040.pool.root.1"
# RUN3 setup
GEOTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN3)")
CONDTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")



# Run the job
export TRF_ECHO=1;
export ATHENA_CORE_NUMBER=1

Sim_tf.py \
      --CA 'True' \
      --maxEvents ${NEVENTS} \
      --perfmon 'fullmonmt' \
      --conditionsTag "default:${CONDTAG}" \
      --geometryVersion "default:${GEOTAG}" \
      --postInclude 'default:PyJobTransforms.UseFrontier' \
      --preInclude 'EVNTtoHITS:Campaigns.MC23eSimulationMultipleIoV' \
      --preExec "EVNTtoHITS:flags.Sim.FastCalo.ParamsInputFilename=\"FastCaloSim/MC23/TFCSparam_dev_Hybrid_D_v1_all_baryons_0_500.root\";" \
      --simulator 'ATLFAST3F_G4MS' \
      --DataRunNumber 284500 \
      --imf False \
      --inputEVNTFile ${DATAFILE} \
      --outputHITSFile 'myHITS.pool.root' > __log.txt 2>&1;

echo $? > __exitcode;
