#!/bin/sh
#
# art-description: CA-based config ATLFAST3F_G4MS with MC-overlay for MC23e ttbar running serial
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena
# art-output: log.*
# art-output: *.pkl
# art-output: RDO.pool.root
# art-output: AOD.pool.root
# art-architecture: '#x86_64-intel'
# art-memory: 5999

events=50

EVNT_File=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.EVNT_RUN3_50[0])")
RDO_BKG_File="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/FastChainPileup/TrackOverlay/RDO_TrackOverlay_Run3_MC23e.pool.root"
RDO_File='RDO.pool.root'
AOD_File='AOD.pool.root'

geometry=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN3)")
conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")

FastChain_tf.py \
   --CA \
   --simulator ATLFAST3F_G4MS \
   --physicsList FTFP_BERT_ATL \
   --useISF True \
   --randomSeed 123 \
   --inputEVNTFile ${EVNT_File} \
   --inputRDO_BKGFile ${RDO_BKG_File} \
   --outputRDOFile ${RDO_File} \
   --maxEvents ${events} \
   --skipEvents 0 \
   --digiSeedOffset1 511 \
   --digiSeedOffset2 727 \
   --preInclude 'EVNTtoRDO:Campaigns.MC23eSimulationMultipleIoV' 'EVNTtoRDO:Campaigns.MC23e' \
   --postInclude 'PyJobTransforms.UseFrontier' \
   --conditionsTag "default:${conditions}" \
   --geometryVersion "default:${geometry}" \
   --postExec 'with open("Config.pkl", "wb") as f: cfg.store(f)' \
   --imf False

fastchain=$?
echo  "art-result: $fastchain EVNTtoRDO"

rec=-9999
reg=-9999

# Reconstruction
if [ ${fastchain} -eq 0 ]
then
   Reco_tf.py \
      --CA \
      --inputRDOFile ${RDO_File} \
      --outputAODFile ${AOD_File} \
      --steering 'doRDO_TRIG' 'doTRIGtoALL' \
      --maxEvents '-1' \
      --autoConfiguration=everything \
      --conditionsTag "default:${conditions}" \
      --geometryVersion "default:${geometry}" \
      --postExec 'RAWtoALL:from AthenaCommon.ConfigurationShelve import saveToAscii;saveToAscii("RAWtoALL_config.txt")' \
      --athenaopts "all:--threads=1" \
      --imf False
     rec=$?
fi

echo  "art-result: $rec reconstruction"

# Regression test
if [ ${fastchain} -eq 0 ]
then
   ArtPackage=$1
   ArtJobName=$2
   art.py compare grid --entries 4 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --order-trees --diff-root --file=${RDO_File}
   reg=$?
fi

echo  "art-result: $reg regression"

# Set status to the first failure encountered
if [ ${fastchain} -ne 0 ]; then
    status=$fastchain
elif [ ${rec} -ne 0 ]; then
    status=$rec
elif [ ${reg} -ne 0 ]; then
    status=$reg
else
    status=0
fi

exit $status
