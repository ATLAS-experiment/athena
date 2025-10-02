#!/bin/sh
#
# art-description: FastChain workflow 1A running ATLFAST3MT and Hybrid-overlay in MT for MC23e ttbar with HITS, AOD and DAOD output formats
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena
# art-output: log.*
# art-output: *.pkl
# art-output: HITS.pool.root
# art-output: AOD.pool.root
# art-output: DAOD_PHYS.OUT.pool.root
# art-output: DAOD_PHYSVAL.OUT.pool.root
# art-architecture: '#x86_64-intel'
# art-athena-mt: 8

events=50

export ATHENA_CORE_NUMBER=8

EVNT_File=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.EVNT_RUN3_50[0])")
RDO_BKG_File="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/FastChainPileup/TrackOverlay/RDO_TrackOverlay_Run3_MC23e.pool.root"
HITS_File='HITS.pool.root'
AOD_File='AOD.pool.root'
DAOD_File="OUT.pool.root"

geometry=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN3)")
conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")

FastChain_tf.py \
   --CA \
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
   --maxEvents ${events} \
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
   --athenaopts "EVNTtoRDO:--threads=${ATHENA_CORE_NUMBER} --nprocs=0" "RDOtoRDOTrigger:--threads=${ATHENA_CORE_NUMBER} --nprocs=0" "RAWtoALL:--threads=${ATHENA_CORE_NUMBER} --nprocs=0" \
   --imf False

fastchain=$?
echo  "art-result: $fastchain EVNTtoDAOD"
status=$fastchain

reg=-9999

# Regression test
if [ $fastchain -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 4 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --order-trees --diff-root --file=${AOD_File}
    reg=$?
    status=$reg
fi
echo  "art-result: $reg regression"

exit $status
