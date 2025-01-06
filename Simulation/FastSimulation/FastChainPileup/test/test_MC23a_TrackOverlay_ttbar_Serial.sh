#!/bin/sh
#
# art-description: CA-based config Track-overlay for MC23a ttbar running serial
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena
# art-output: log.*
# art-output: *.pkl
# art-output: *.txt
# art-output: RDO.pool.root
# art-output: AOD.pool.root
# art-architecture: '#x86_64-intel'


events=25

HITS_File='/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc23/HITS/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.simul.HITS.e8514_s4162/100events.HITS.pool.root'
RDO_BKG_File='/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/FastChainPileup/TrackOverlay/RDO_TrackOverlay_Run3_MC23a.pool.root'
RDO_File='RDO.pool.root'
AOD_File='AOD.pool.root'

Overlay_tf.py \
   --CA \
   --inputHITSFile ${HITS_File} \
   --inputRDO_BKGFile ${RDO_BKG_File} \
   --outputRDOFile ${RDO_File} \
   --maxEvents ${events} \
   --skipEvents 0 \
   --digiSeedOffset1 511 \
   --digiSeedOffset2 727 \
   --preInclude 'Campaigns.MC23a' \
   --postInclude 'PyJobTransforms.UseFrontier' 'OverlayConfiguration.OverlayTestHelpers.OverlayJobOptsDumperCfg' \
   --conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-07' \
   --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
   --preExec 'ConfigFlags.Overlay.doTrackOverlay=True;' \
   --postExec 'with open("Config.pkl", "wb") as f: cfg.store(f)' \
   --imf False
overlay=$?
echo  "art-result: $overlay Overlay"
status=$overlay

reg=-9999
if [ $overlay -eq 0 ]
then
   art.py compare --file ${RDO_File} --mode=semi-detailed --entries 10
   reg=$?
   status=$reg
fi
echo  "art-result: $reg regression"

rec=-9999
if [ ${overlay} -eq 0 ]
then
   # Reconstruction
   Reco_tf.py \
      --CA \
      --inputRDOFile ${RDO_File} \
      --outputAODFile ${AOD_File} \
      --steering 'doRDO_TRIG' 'doTRIGtoALL' \
      --maxEvents '-1' \
      --autoConfiguration=everything \
      --conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-07' \
      --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
      --preExec 'RAWtoALL:flags.Reco.EnableTrackOverlay=True; flags.TrackOverlay.MLThreshold=0.95;' 'RDOtoRDOTrigger:flags.Overlay.doTrackOverlay=True;'\
      --postExec 'RAWtoALL:from AthenaCommon.ConfigurationShelve import saveToAscii;saveToAscii("RAWtoALL_config.txt")' \
      --athenaopts "all:--threads=1" \
      --imf False
     rec=$?
     status=$rec
fi

echo  "art-result: $rec reconstruction"

exit $status
