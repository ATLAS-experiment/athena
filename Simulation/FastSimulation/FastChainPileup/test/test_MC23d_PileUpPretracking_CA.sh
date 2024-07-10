#!/bin/sh
#
# art-description: CA-based config Pile-up Pretracking for MC23d
# art-type: grid
# art-include: main/Athena
# art-output: log.*
# art-output: *.pkl
# art-output: *.txt
# art-output: mc23d.100events.PU_TRK.RDO.pool.root
# art-architecture: '#x86_64-intel'

events=100
RDO_BKG_File="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc23/RDO_BKG/mc23_13p6TeV.900149.PG_single_nu_Pt50.merge.RDO.e8514_e8528_s4153_d1907_d1908/100events.RDO.pool.root"
RDO_PU_File="mc23d.100events.PU_TRK.RDO.pool.root"

Reco_tf.py \
  --CA \
  --inputRDOFile ${RDO_BKG_File} \
  --outputRDO_PUFile ${RDO_PU_File} \
  --maxEvents ${events} \
  --skipEvents 0 \
  --preInclude 'Campaigns.MC23d' \
  --postInclude 'PyJobTransforms.UseFrontier' \
  --conditionsTag 'OFLCOND-MC23-SDR-RUN3-02'  \
  --geometryVersion 'ATLAS-R3S-2021-03-02-00' \
  --postExec 'with open("ConfigCA.pkl", "wb") as f: cfg.store(f)' \
  --imf False
ca=$?
echo  "art-result: $ca PUTracking"
status=$ca

#reg=-9999
#if [ $ca -eq 0 ]
#then
#   art.py compare --file ${RDO_File} --mode=semi-detailed --entries 10
#   reg=$?
#   status=$reg
#fi
#echo  "art-result: $reg regression"
#
#rec=-9999
#ntup=-9999
#if [ ${ca} -eq 0 ]
#then
#    # Reconstruction
#    Reco_tf.py \
#               --CA \
#               --inputRDOFile ${RDO_File} \
#               --outputAODFile ${AOD_File} \
#               --steering 'doRDO_TRIG' 'doTRIGtoALL' \
#	             --maxEvents '-1' \
#               --autoConfiguration=everything \
#    	         --conditionsTag 'OFLCOND-MC21-SDR-RUN3-07' \
#               --geometryVersion 'ATLAS-R3S-2021-03-00-00' \
#               --athenaopts "all:--threads=1" \
#               --postExec 'RAWtoALL:from AthenaCommon.ConfigurationShelve import saveToAscii;saveToAscii("RAWtoALL_config.txt")' \
#               --preExec 'all:flags.Overlay.doTrackOverlay=True;'\
#               --imf False
#
#     rec=$?
#     if [ ${rec} -eq 0 ]
#     then
#         # NTUP prod. (old-style - will be updated after FastChain metadata is fixed)
#         Reco_tf.py --inputAODFile ${AOD_File} \
#                    --outputNTUP_PHYSVALFile ${NTUP_File} \
#                    --maxEvents '-1' \
#                    --conditionsTag 'OFLCOND-MC21-SDR-RUN3-07' \
#                    --geometryVersion 'ATLAS-R3S-2021-03-00-00' \
#		                --asetup 'Athena,23.0.53' \
#                    --ignoreErrors True \
#                    --validationFlags 'doInDet' \
#                    --valid 'True'
#         ntup=$?
#         status=$ntup
#     fi
#fi
#
#echo  "art-result: $rec reconstruction"
#echo  "art-result: $ntup physics validation"

exit $status
