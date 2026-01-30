#!/usr/bin/bash
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

NTHREADS=${1}
NEVENTS=${2}

DATADIR="/eos/atlas/atlascerngroupdisk/proj-spot/spot-job-inputs/mc23_13p6TeV"
CONDTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")
GEOTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN3)")


# Run the job
export TRF_ECHO=1;
ATHENA_CORE_NUMBER=${NTHREADS} \
      Reco_tf.py \
      --CA 'default:True' \
      --inputHITSFile "${DATADIR}/HITS/mc23_13p6TeV.601237.PhPy8EG_A14_ttbar_hdamp258p75_allhad.merge.HITS.e8514_e8528_s4159_s4114/HITS.34124871._003416.pool.root.1" \
      --inputRDO_BKGFile "${DATADIR}/RDO/mc23_13p6TeV.900149.PG_single_nu_Pt50.merge.RDO.e8514_e8528_s4153_d1879_d1880/RDO.33837536._002942.pool.root.1" \
      --outputAODFile 'myAOD.pool.root' \
      --perfmon 'fullmonmt' \
      --maxEvents ${NEVENTS} \
      --multithreaded='True' \
      --preInclude 'all:Campaigns.MC23c' \
      --postInclude 'default:PyJobTransforms.UseFrontier' \
      --skipEvents '0' \
      --autoConfiguration 'everything' \
      --conditionsTag ${CONDTAG} \
      --geometryVersion ${GEOTAG} \
      --runNumber '601237' \
      --digiSeedOffset1 '232' \
      --digiSeedOffset2 '232' \
      --AMITag 'r14799' \
      --steering 'doOverlay' 'doRDO_TRIG' 'doTRIGtoALL' > __log.txt 2>&1;
      
echo $? > __exitcode;
