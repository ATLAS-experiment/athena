#!/bin/sh
#
# art-description: CA-based config ATLFAST3F_G4MS with standard pile-up digitization for MC23e ttbar
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena
# art-output: log.*
# art-output: *.pkl
# art-output: *.txt
# art-output: RDO.pool.root
# art-output: AOD.pool.root
# art-architecture: '#x86_64-intel'
# art-athena-mt: 8

events=50

export ATHENA_CORE_NUMBER=8

EVNT_File='/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc23/EVNT/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8514/EVNT.32288062._002040.pool.root.1'
HighPtMinbiasHitsFiles="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc23/HITS/mc23_13p6TeV.800831.Py8EG_minbias_inelastic_highjetphotonlepton.merge.HITS.e8514_e8528_s4334_s4371/500events.HITS.pool.root"
LowPtMinbiasHitsFiles="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc23/HITS/mc23_13p6TeV.900311.Epos_minbias_inelastic_lowjetphoton.merge.HITS.e8514_e8528_s4334_s4371/5000events.HITS.pool.root"
RDO_File='RDO.pool.root'
AOD_File='AOD.pool.root'

geometry=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN3)")
conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")

FastChain_tf.py \
   --CA \
   --multiprocess True \
   --simulator ATLFAST3F_G4MS \
   --physicsList FTFP_BERT_ATL \
   --useISF True \
   --randomSeed 123 \
   --inputEVNTFile ${EVNT_File} \
   --digiSteeringConf "StandardSignalOnlyTruth" \
   --inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles} \
   --inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles} \
   --outputRDOFile ${RDO_File} \
   --maxEvents ${events} \
   --skipEvents 0 \
   --digiSeedOffset1 511 \
   --digiSeedOffset2 727 \
   --preInclude 'EVNTtoRDO:Campaigns.MC23eSimulationMultipleIoV' 'EVNTtoRDO:Campaigns.MC23e' \
   --postInclude 'PyJobTransforms.UseFrontier' 'DigitizationConfig.DigitizationSteering.DigitizationTestingPostInclude' \
   --conditionsTag "default:${conditions}" \
   --geometryVersion "default:${geometry}" \
   --postExec 'with open("Config.pkl", "wb") as f: cfg.store(f)' \
   --sharedWriter True \
   --parallelCompression False \
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
      --multithreaded True \
      --inputRDOFile ${RDO_File} \
      --outputAODFile ${AOD_File} \
      --steering 'doRDO_TRIG' 'doTRIGtoALL' \
      --maxEvents '-1' \
      --autoConfiguration=everything \
      --conditionsTag "default:${conditions}" \
      --geometryVersion "default:${geometry}" \
      --postExec 'RAWtoALL:from AthenaCommon.ConfigurationShelve import saveToAscii;saveToAscii("RAWtoALL_config.txt")' \
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
