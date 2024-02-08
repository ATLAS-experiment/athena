#!/bin/bash
#
# art-description: Run digitization of an MC16a qball sample with  MC20a geometry and conditions, 25ns pile-up
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-include: 24.0/Athena
# art-include: main/Athena
# art-output: mc20a_qball.*.RDO.pool.root
# art-output: log.*
# art-output: legacy.*
# art-output: DigiPUConfig*

Events=10
DigiOutFileName="mc20a_qball.CG.RDO.pool.root"
HSHitsFile=" /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DigitizationTests/QBALLS.HITS.17587614._000019.pool.root.1"
HighPtMinbiasHitsFiles="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/mc16_13TeV.800831.Py8EG_minbias_inelastic_highjetphotonlepton.simul.HITS_FILT.e8341_s3687_s3704/*"
LowPtMinbiasHitsFiles="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/mc16_13TeV.900311.Epos_minbias_inelastic_lowjetphoton.simul.HITS_FILT.e8341_s3687_s3704/*"

Digi_tf.py \
    --CA \
    --conditionsTag default:OFLCOND-MC16-SDR-RUN2-09 \
    --digiSeedOffset1 170 --digiSeedOffset2 170 \
    --digiSteeringConf 'StandardSignalOnlyTruth' \
    --geometryVersion default:ATLAS-R2-2016-01-00-01 \
    --inputHITSFile ${HSHitsFile} \
    --inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles} \
    --inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles} \
    --jobNumber 1 \
    --maxEvents ${Events} \
    --outputRDOFile ${DigiOutFileName} \
    --postInclude 'PyJobTransforms.UseFrontier' 'HITtoRDO:Digitization.DigitizationSteering.DigitizationTestingPostInclude' \
    --preInclude 'HITtoRDO:Campaigns.MC20a' \
    --skipEvents 0

rc=$?
status=$rc
echo "art-result: $rc digiCA"

# get reference directory
source DigitizationCheckReferenceLocation.sh
echo "Reference set being used: ${DigitizationTestsVersion}"

if [[ $rc -eq 0 ]]
then
    # Do reference comparisons
    art.py compare ref --mode=semi-detailed --no-diff-meta "$DigiOutFileName" "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DigitizationTests/ReferenceFiles/$DigitizationTestsVersion/$CMTCONFIG/$DigiOutFileName"
    rc4=$?
    if [[ $status -eq 0 ]]
    then
        status=$rc4
    fi
fi
echo "art-result: $rc4 OLDvsFixedRef"

if [[ $rc -eq 0 ]]
then
    art.py compare grid --entries 10 "$1" "$2" --mode=semi-detailed --file="$DigiOutFileName"
    rc5=$?
    if [[ $status -eq 0 ]]
    then
        status=$rc5
    fi
fi
echo "art-result: $rc5 regression"

exit $status
