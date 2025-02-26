#!/bin/sh
#
# art-description: Modified/Simplified MC20e digitization job
# art-type: build
# art-include: 23.0/Athena
# art-include: main/Athena

Events=10
HSHitsFile="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/OverlayTests/mc16_13TeV.900149.PG_single_nu_Pt50.simul.HITS.e8307_s3482/HITS.24078104._234467.pool.root.1"
HighPtMinbiasHitsFiles="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/mc16_13TeV.800831.Py8EG_minbias_inelastic_highjetphotonlepton.simul.HITS_FILT.e8341_s3687_s3704/*"
LowPtMinbiasHitsFiles="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/mc16_13TeV.900311.Epos_minbias_inelastic_lowjetphoton.simul.HITS_FILT.e8341_s3687_s3704/*"

geometry=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN2)")
conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN2_MC)")

Digi_tf.py \
    --CA \
    --PileUpPresampling True \
    --conditionsTag "default:${conditions}" \
    --geometryVersion "default:${geometry}" \
    --digiSeedOffset1 170 --digiSeedOffset2 170 \
    --digiSteeringConf 'StandardSignalOnlyTruth' \
    --inputHITSFile ${HSHitsFile} \
    --inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles} \
    --inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles} \
    --jobNumber 568 \
    --maxEvents ${Events} \
    --outputRDOFile 'test.RDO.pool.root' \
    --postInclude 'PyJobTransforms.UseFrontier' \
    --preExec 'HITtoRDO:ConfigFlags.Digitization.PU.CustomProfile={"run":310000, "startmu":0.0, "endmu":1.0, "stepmu":1.0, "startlb":1, "timestamp": 1550000000};' \
    --preInclude 'HITtoRDO:Campaigns.MC20e' \
    --skipEvents 0

echo  "art-result: $? digitization"
