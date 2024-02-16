#!/bin/sh
#
# art-description: CA-based config Track-overlay for MC21a ttbar
# art-type: grid
# art-include: main/Athena
# art-include: 23.0/Athena
# art-output: log.*
# art-output: *.pkl
# art-output: *.txt
# art-output: RDO.pool.root
# art-architecture: '#x86_64-intel'

events=50
HITS_File="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/FastChainPileup/TrackOverlay/HITS.29625925._010619_100evts.pool.root.1"
RDO_BKG_File="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/FastChainPileup/TrackOverlay/RDO_TrackOverlay_Run3.pool.root"
RDO_File="RDO.pool.root"
AOD_File="AOD.pool.root"
NTUP_File="NTUP.pool.root"

Overlay_tf.py \
  --CA \
  --inputHITSFile ${HITS_File} \
  --inputRDO_BKGFile ${RDO_BKG_File} \
  --outputRDOFile ${RDO_File} \
  --maxEvents ${events} \
  --skipEvents 0 \
  --digiSeedOffset1 511 \
  --digiSeedOffset2 727 \
  --preInclude 'Campaigns.MC21a' \
  --postInclude 'PyJobTransforms.UseFrontier' 'OverlayConfiguration.OverlayTestHelpers.OverlayJobOptsDumperCfg' \
  --conditionsTag 'OFLCOND-MC21-SDR-RUN3-07'  \
  --geometryVersion 'ATLAS-R3S-2021-03-00-00' \
  --preExec 'ConfigFlags.Overlay.doTrackOverlay=True;' \
  --postExec 'with open("ConfigCA.pkl", "wb") as f: cfg.store(f)' \
  --imf False
ca=$?
echo  "art-result: $ca HITStoRDO_CA"
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
    	         --conditionsTag 'OFLCOND-MC21-SDR-RUN3-07' \
               --geometryVersion 'ATLAS-R3S-2021-03-00-00' \
               --athenaopts "all:--threads=1" \
               --postExec 'RAWtoALL:from AthenaCommon.ConfigurationShelve import saveToAscii;saveToAscii("RAWtoALL_config.txt")' \
               --preExec 'all:flags.Overlay.doTrackOverlay=True;'\
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
