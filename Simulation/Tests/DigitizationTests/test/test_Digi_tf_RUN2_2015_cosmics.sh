#!/bin/bash
#
# art-description: Run digitization of a cosmics sample produced with MC15 using 2015 geometry and conditions
# art-include: main/Athena
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-output: mc15_2015_cosmics.RDO.pool.root

CosmicsHITSFile="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DigitizationTests/testCosmics.ATLAS-R2-2015-03-01-00_VALIDATION.HITS.pool.root"
DigiOutFileName="mc15_2015_cosmics.RDO.pool.root"

Digi_tf.py \
    --CA \
    --inputHITSFile ${CosmicsHITSFile}  \
    --outputRDOFile ${DigiOutFileName}  \
    --maxEvents 100  \
    --skipEvents 0  \
    --digiSeedOffset1 11 \
    --digiSeedOffset2 22  \
    --geometryVersion ATLAS-R2-2015-03-01-00  \
    --conditionsTag default:OFLCOND-RUN12-SDR-31-01  \
    --DataRunNumber 222500  \
    --preExec 'HITtoRDO:flags.Sim.TRTRangeCut=0.05' \
    --preInclude 'HITtoRDO:LArConfiguration.LArConfigRun2.LArConfigRun2NoPileUp' \
    --postInclude 'default:PyJobTransforms.UseFrontier'

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
    art.py compare grid --entries 10 "$1" "$2" --mode=semi-detailed
    rc5=$?
    status=$rc5
fi
echo "art-result: $rc5 regression"

exit $status
