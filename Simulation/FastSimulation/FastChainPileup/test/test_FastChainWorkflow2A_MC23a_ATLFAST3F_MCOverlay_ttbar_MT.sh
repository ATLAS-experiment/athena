#!/bin/sh
#
# art-description: FastChain workflow 2A running ATLFAST3F_G4MS and MC-overlay in MP/MT for MC23a ttbar with HITS, AOD and DAOD output formats
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena
# art-output: log.*
# art-output: *.pkl
# art-output: *.txt
# art-output: HITS.pool.root
# art-output: AOD.pool.root
# art-output: DAOD_PHYS.OUT.pool.root
# art-output: DAOD_PHYSVAL.OUT.pool.root
# art-architecture: '#x86_64-intel'
# art-athena-mt: 8

events=50

export ATHENA_CORE_NUMBER=8

EVNT_File='/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc23/EVNT/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8514/EVNT.32288062._002040.pool.root.1'
RDO_BKG_File='/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/FastChainPileup/TrackOverlay/RDO_TrackOverlay_Run3_MC23a.pool.root'
HITS_File='HITS.pool.root'
AOD_File='AOD.pool.root'
DAOD_File="OUT.pool.root"

FastChain_tf.py \
   --CA \
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
   --maxEvents ${events} \
   --skipEvents 0 \
   --digiSeedOffset1 511 \
   --digiSeedOffset2 727 \
   --preInclude 'EVNTtoRDO:Campaigns.MC23aSimulationMultipleIoV' 'Campaigns.MC23a' \
   --postInclude 'PyJobTransforms.UseFrontier' 'OverlayConfiguration.OverlayTestHelpers.OverlayJobOptsDumperCfg' \
   --conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-07' \
   --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
   --postExec 'with open("Config.pkl", "wb") as f: cfg.store(f)' \
   --sharedWriter True \
   --parallelCompression False \
   --formats PHYS PHYSVAL \
   --athenaopts "EVNTtoRDO:--threads=0 --nprocs=${ATHENA_CORE_NUMBER}" "RDOtoRDOTrigger:--threads=${ATHENA_CORE_NUMBER} --nprocs=0" "RAWtoALL:--threads=${ATHENA_CORE_NUMBER} --nprocs=0" \
   --imf False

fastchain=$?
echo  "art-result: $fastchain EVNTtoDAOD"
status=$fastchain

reg=-9999
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
