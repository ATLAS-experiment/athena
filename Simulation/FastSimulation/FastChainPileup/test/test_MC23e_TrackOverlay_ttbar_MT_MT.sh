#!/bin/sh
#
# art-description: CA-based config  Track-overlay for MC23e ttbar
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena
# art-output: log.*
# art-output: *.pkl
# art-output: RDO.pool.root
# art-output: AOD.pool.root
# art-architecture: '#x86_64-intel'

events=20

export ATHENA_CORE_NUMBER=8

HITS_File='/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc23/HITS/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.merge.HITS.e8514_e8528_s4369/100events.HITS.pool.root'
RDO_BKG_File="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/FastChainPileup/TrackOverlay/RDO_TrackOverlay_Run3_MC23e.pool.root"
RDO_File='RDO.pool.root'
AOD_File='AOD.pool.root'

geometry=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN3)")
conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")

Overlay_tf.py \
   --CA \
   --multithreaded True \
   --inputHITSFile ${HITS_File} \
   --inputRDO_BKGFile ${RDO_BKG_File} \
   --outputRDOFile ${RDO_File} \
   --maxEvents ${events} \
   --skipEvents 0 \
   --digiSeedOffset1 511 \
   --digiSeedOffset2 727 \
   --preInclude 'Campaigns.MC23e' \
   --postInclude 'PyJobTransforms.UseFrontier' \
   --conditionsTag "default:${conditions}" \
   --geometryVersion "default:${geometry}" \
   --preExec 'flags.Overlay.doTrackOverlay=True;' \
   --postExec 'with open("Config.pkl", "wb") as f: cfg.store(f)' \
   --imf False

overlay=$?
echo  "art-result: $overlay Overlay"
status=$overlay

rec=-9999
reg=-9999

# Reconstruction
if [ ${overlay} -eq 0 ]
then
   Reco_tf.py \
      --CA \
      --multithreaded True \
      --inputRDOFile ${RDO_File} \
      --outputAODFile ${AOD_File} \
      --steering 'doRDO_TRIG' 'doTRIGtoALL' \
      --maxEvents '-1' \
      --autoConfiguration=everything \
      --conditionsTag "default:${conditions}" \
      --geometryVersion "default:${geometry}" \
      --preExec 'RAWtoALL:flags.Reco.EnableTrackOverlay=True; flags.TrackOverlay.MLThreshold=0.95;' 'RDOtoRDOTrigger:flags.Overlay.doTrackOverlay=True;'\
      --postExec 'RAWtoALL:from AthenaCommon.ConfigurationShelve import saveToAscii;saveToAscii("RAWtoALL_config.txt")' \
      --imf False
     rec=$?
     status=$rec
fi

echo  "art-result: $rec reconstruction"

# Regression test
if [ ${rec} -eq 0 ]
then
   ArtPackage=$1
   ArtJobName=$2
   art.py compare grid --entries 4 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --order-trees --diff-root --file=${AOD_File}
   reg=$?
   status=$reg
fi

echo  "art-result: $reg regression"

exit $status
