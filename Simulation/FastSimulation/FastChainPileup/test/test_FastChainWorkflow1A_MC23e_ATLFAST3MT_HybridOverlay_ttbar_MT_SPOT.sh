#!/bin/sh

NTHREADS=${1}
NEVENTS=${2}


EVNT_File=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.EVNT_RUN3_1K[0])")
RDO_BKG_File='/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/FastChainPileup/TrackOverlay/RDO_TrackOverlay_Run3_MC23e.pool.root'
HITS_File='myHITS.pool.root'
AOD_File='myAOD.pool.root'
DAOD_File='pool.root'

geometry=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN3)")
conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")

export TRF_ECHO=1; 
export ATHENA_CORE_NUMBER=${NTHREADS}
FastChain_tf.py \
   --CA \
   --perfmon 'fullmonmt' \
   --steering 'doFCtoDAOD' 'doRDO_TRIG' 'doTRIGtoALL' \
   --simulator ATLFAST3MT \
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
   --preExec 'EVNTtoRDO:flags.Overlay.doTrackOverlay=True;' 'RDOtoRDOTrigger:flags.Reco.EnableTrackOverlay=True; flags.Overlay.doTrackOverlay=True;' 'RAWtoALL:flags.Reco.EnableTrackOverlay=True; flags.Overlay.doTrackOverlay=True;' \
   --postExec 'with open("Config.pkl", "wb") as f: cfg.store(f)' \
   --sharedWriter True \
   --parallelCompression False \
   --formats PHYS PHYSVAL \
   --athenaopts "EVNTtoRDO:--threads=${NTHREADS} --nprocs=0" "RDOtoRDOTrigger:--threads=${NTHREADS} --nprocs=0" "RAWtoALL:--threads=${NTHREADS} --nprocs=0" \
   --imf False > __log.txt 2>&1;

echo $? > __exitcode;
