#!/bin/bash
#
# art-description: Run a digitization example to compare configuration between ConfGetter and the new ComponentAccumulator approach.
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-memory: 3999
# art-include: 24.0/Athena
# art-include: main/Athena
# art-output: mc20e_presampling_custom.RDO.pool.root
# art-output: log.*
# art-output: legacy.*
# art-output: DigiPUConfig*

Events=20
DigiOutFileName="mc20e_presampling_custom.RDO.pool.root"
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
    --outputRDOFile ${DigiOutFileName} \
    --postInclude 'PyJobTransforms.UseFrontier' 'HITtoRDO:DigitizationConfig.DigitizationSteering.DigitizationTestingPostInclude' \
    --preExec 'HITtoRDO:ConfigFlags.Digitization.PU.CustomProfile={"run":310000, "startmu":0.0, "endmu":10.0, "stepmu":1.0, "startlb":1, "timestamp": 1550000000};' \
    --preInclude 'HITtoRDO:Campaigns.MC20e' \
    --skipEvents 0

rc=$?
status=$rc
echo "art-result: $rc digiCA"

# get reference directory
source DigitizationCheckReferenceLocation.sh
echo "Reference set being used: ${DigitizationTestsVersion}"

rc4=-9999
if [[ $rc -eq 0 ]]
then
    # Do reference comparisons
    art.py compare ref --mode=semi-detailed --no-diff-meta "$DigiOutFileName" "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DigitizationTests/ReferenceFiles/$DigitizationTestsVersion/$CMTCONFIG/$DigiOutFileName"
    rc4=$?
    status=$rc4
fi
echo "art-result: $rc4 OLDvsFixedRef"

rc5=-9999
if [[ $rc -eq 0 ]]
then
    art.py compare grid --entries "$Events" "$1" "$2" --mode=semi-detailed
    rc5=$?
    status=$rc5
fi
echo "art-result: $rc5 regression"

exit $status
