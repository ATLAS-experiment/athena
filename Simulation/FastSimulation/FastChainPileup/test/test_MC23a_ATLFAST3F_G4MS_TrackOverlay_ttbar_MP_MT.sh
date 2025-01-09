#!/bin/sh
#
# art-description: CA-based config ATLFAST3F_G4MS with Track-overlay for MC23a ttbar
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena
# art-output: log.*
# art-output: *.pkl
# art-output: *.txt
# art-output: RDO.pool.root
# art-output: AOD.pool.root
# art-architecture: '#x86_64-intel'


events=50

export ATHENA_CORE_NUMBER=8

EVNT_File='/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc23/EVNT/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8514/EVNT.32288062._002040.pool.root.1'
RDO_BKG_File='/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/FastChainPileup/TrackOverlay/RDO_TrackOverlay_Run3_MC23a.pool.root'
RDO_File='RDO.pool.root'
AOD_File='AOD.pool.root'

FastChain_tf.py \
   --CA \
   --multiprocess True \
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
   --preInclude 'EVNTtoRDO:Campaigns.MC23aSimulationMultipleIoV' 'EVNTtoRDO:Campaigns.MC23a' \
   --postInclude 'PyJobTransforms.UseFrontier' 'OverlayConfiguration.OverlayTestHelpers.OverlayJobOptsDumperCfg' \
   --conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-07' \
   --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
   --preExec 'EVNTtoRDO:ConfigFlags.Overlay.doTrackOverlay=True;' \
   --postExec 'with open("Config.pkl", "wb") as f: cfg.store(f)' \
   --sharedWriter True \
   --parallelCompression False \
   --imf False
fastchain=$?
echo  "art-result: $fastchain EVNTtoRDO"
status=$fastchain

reg=-9999
if [ $fastchain -eq 0 ]
then
   art.py compare --file ${RDO_File} --mode=semi-detailed --entries 10
   reg=$?
   status=$reg
fi
echo  "art-result: $reg regression"

rec=-9999
if [ ${fastchain} -eq 0 ]
then
   # Reconstruction
   Reco_tf.py \
      --CA \
      --multithreaded True \
      --inputRDOFile ${RDO_File} \
      --outputAODFile ${AOD_File} \
      --steering 'doRDO_TRIG' 'doTRIGtoALL' \
      --maxEvents '-1' \
      --autoConfiguration=everything \
      --conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-07' \
      --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
      --preExec 'RAWtoALL:flags.Reco.EnableTrackOverlay=True; flags.TrackOverlay.MLThreshold=0.95;' 'RDOtoRDOTrigger:flags.Overlay.doTrackOverlay=True;'\
      --postExec 'RAWtoALL:from AthenaCommon.ConfigurationShelve import saveToAscii;saveToAscii("RAWtoALL_config.txt")' \
      --imf False
     rec=$?
     status=$rec
fi

echo  "art-result: $rec reconstruction"

exit $status
