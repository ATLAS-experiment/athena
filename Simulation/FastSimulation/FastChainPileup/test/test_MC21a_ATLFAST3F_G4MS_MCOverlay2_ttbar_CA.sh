#!/bin/sh
#
# art-description: CA-based config ATLFAST3F_G4MS with Hybrid-overlay with ML-threshold set to 0.1 (which should be equivalent to MC-overlay in terms of physics performance) for MC21a ttbar
# art-type: grid
# art-include: main/Athena
# art-include: 23.0/Athena
# art-output: log.*
# art-output: *.pkl
# art-output: *.txt
# art-output: RDO.pool.root
# art-architecture: '#x86_64-intel'

events=50
EVNT_File='/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/EVNT/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8453/EVNT.29328277._003902.pool.root.1'
RDO_BKG_File="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/FastChainPileup/TrackOverlay/RDO_TrackOverlay_Run3.pool.root"
RDO_File="RDO.pool.root"
AOD_File="AOD.pool.root"
NTUP_File="NTUP_PHYSVAL.pool.root"

FastChain_tf.py \
  --CA \
  --runNumber 601229 \
  --simulator ATLFAST3F_G4MS \
  --physicsList FTFP_BERT_ATL \
  --useISF True \
  --jobNumber 1 \
  --randomSeed 123 \
  --inputEVNTFile ${EVNT_File} \
  --inputRDO_BKGFile ${RDO_BKG_File} \
  --outputRDOFile ${RDO_File} \
  --maxEvents ${events} \
  --skipEvents 0 \
  --digiSeedOffset1 511 \
  --digiSeedOffset2 727 \
  --preInclude 'Campaigns.MC21a' 'Campaigns.MC21SimulationNoIoV' \
  --postInclude 'PyJobTransforms.UseFrontier' 'OverlayConfiguration.OverlayTestHelpers.OverlayJobOptsDumperCfg' \
  --conditionsTag 'OFLCOND-MC21-SDR-RUN3-07'  \
  --geometryVersion 'ATLAS-R3S-2021-03-00-00' \
  --preExec 'all:ConfigFlags.Overlay.doTrackOverlay=True;' \
  --postExec 'with open("ConfigCA.pkl", "wb") as f: cfg.store(f)' \
  --imf False
ca=$?
echo  "art-result: $ca EVNTtoRDO_CA"
status=$ca

reg=-9999
if [ $ca -eq 0 ]
then
   art.py compare --file ${RDO_File} --mode=semi-detailed --entries 10
   reg=$?
   status=$reg
fi
echo  "art-result: $reg regression"

rec=-9999
ntup=-9999
if [ ${ca} -eq 0 ]
then
    # Reconstruction
    Reco_tf.py \
               --CA \
               --inputRDOFile ${RDO_File} \
               --outputAODFile ${AOD_File} \
               --steering 'doRDO_TRIG' 'doTRIGtoALL' \
               --maxEvents '-1' \
               --autoConfiguration=everything \
               --conditionsTag 'OFLCOND-MC21-SDR-RUN3-07'  \
               --geometryVersion 'ATLAS-R3S-2021-03-00-00' \
               --athenaopts "all:--threads=1" \
               --postExec 'RAWtoALL:from AthenaCommon.ConfigurationShelve import saveToAscii;saveToAscii("RAWtoALL_config.txt")' \
               --preExec 'RAWtoALL:flags.Reco.EnableTrackOverlay=True; flags.TrackOverlay.MLThreshold=0.1;' 'RDOtoRDOTrigger:flags.Overlay.doTrackOverlay=True;'\
               --imf False

     rec=$?
     if [ ${rec} -eq 0 ]
     then
         # NTUP prod. (old-style - will be updated after FastChain metadata is fixed)
         Reco_tf.py --inputAODFile ${AOD_File} \
                    --outputNTUP_PHYSVALFile ${NTUP_File} \
                    --maxEvents '-1' \
                    --conditionsTag 'OFLCOND-MC21-SDR-RUN3-07' \
                    --geometryVersion 'ATLAS-R3S-2021-03-00-00' \
		                --asetup 'Athena,23.0.53' \
                    --ignoreErrors True \
                    --validationFlags 'doInDet' \
                    --valid 'True'
         ntup=$?
         status=$ntup
     fi
fi

echo  "art-result: $rec reconstruction"
echo  "art-result: $ntup physics validation"

exit $status
