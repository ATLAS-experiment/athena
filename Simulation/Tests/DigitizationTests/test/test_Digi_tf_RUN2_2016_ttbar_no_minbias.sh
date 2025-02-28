#!/bin/bash
#
# art-description: Run a digitization example to compare configuration between ConfGetter and the new ComponentAccumulator approach.
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-include: 24.0/Athena
# art-include: main/Athena
# art-output: mc20a_ttbar_no_minbias.RDO.pool.root
# art-output: log.*
# art-output: legacy.*
# art-output: DigiPUConfig*

Events=25
DigiOutFileName="mc20a_ttbar_no_minbias.RDO.pool.root"
HSHitsFile="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.simul.HITS.e4993_s3091/HITS.10504490._000425.pool.root.1"

geometry=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN2)")
conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN2_MC)")

Digi_tf.py \
    --CA \
    --conditionsTag "default:${conditions}" \
    --geometryVersion "default:${geometry}" \
    --digiSeedOffset1 170 --digiSeedOffset2 170 \
    --digiSteeringConf 'StandardSignalOnlyTruth' \
    --inputHITSFile ${HSHitsFile} \
    --jobNumber 1 \
    --maxEvents ${Events} \
    --outputRDOFile ${DigiOutFileName} \
    --postInclude 'PyJobTransforms.UseFrontier' 'HITtoRDO:DigitizationConfig.DigitizationSteering.DigitizationTestingPostInclude' \
    --preInclude 'HITtoRDO:Campaigns.MC20a' \
    --skipEvents 0

rc2=$?
status=$rc2
echo "art-result: $rc2 digiCA"

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
