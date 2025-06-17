#!/bin/sh

NTHREADS=${1}
NEVENTS=${2}

EVNT_File='/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc23/EVNT/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8514/EVNT.32288062._002040.pool.root.1'
RDO_BKG_File='/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/FastChainPileup/TrackOverlay/RDO_TrackOverlay_Run3_MC23e.pool.root'
HITS_File='myHITS.pool.root'
AOD_File='myAOD.pool.root'
DAOD_File='pool.root'

geometry=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN3)")
conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")

export TRF_ECHO=1;
ATHENA_CORE_NUMBER=${NTHREADS} FastChain_tf.py \
   --CA \
   --perfmon 'fullmonmt' \
   --steering 'doFCtoDAOD' 'doRDO_TRIG' 'doTRIGtoALL' \
   --simulator ATLFAST3F_G4MS \
   --physicsList FTFP_BERT_ATL \
   --useISF True \
   --randomSeed 123 \
   --inputEVNTFile ${EVNT_File} \
   --inputRDO_BKGFile ${RDO_BKG_File} \
   --outputHITSFile ${HITS_File} \
   --outputAODFile ${AOD_File} \
   --outputDAODFile ${DAOD_File} \
   --maxEvents ${NEVENTS} \
   --skipEvents 0 \
   --digiSeedOffset1 511 \
   --digiSeedOffset2 727 \
   --preInclude 'EVNTtoRDO:Campaigns.MC23eSimulationMultipleIoV' 'Campaigns.MC23e' \
   --postInclude 'PyJobTransforms.UseFrontier' \
   --conditionsTag "default:${conditions}" \
   --geometryVersion "default:${geometry}" \
   --postExec 'with open("Config.pkl", "wb") as f: cfg.store(f)' \
   --sharedWriter True \
   --parallelCompression False \
   --formats PHYS PHYSVAL \
   --athenaopts "EVNTtoRDO:--threads=0 --nprocs=${ATHENA_CORE_NUMBER}" "RDOtoRDOTrigger:--threads=${ATHENA_CORE_NUMBER} --nprocs=0" "RAWtoALL:--threads=${ATHENA_CORE_NUMBER} --nprocs=0" \
   --imf False  > __log.txt 2>&1;

echo $? > __exitcode;
