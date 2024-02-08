#!/bin/bash
#
# art-description: Run digitization of an MC15 ttbar sample with 2011 geometry and conditions, without pile-up
# art-include: 24.0/Athena
# art-include: main/Athena
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-output: mc15_2011_ttbar_no_pileup.RDO.pool.root
# art-output: ConfigDigi*.pkl

DigiOutFileName="mc15_2011_ttbar_no_pileup.RDO.pool.root"

Digi_tf.py \
    --CA True \
    --inputHITSFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DigitizationTests/ttbar.ATLAS-R1-2011-02-00-00.HITS.pool.root \
    --outputRDOFile  ${DigiOutFileName} \
    --maxEvents 25 \
    --skipEvents 0  \
    --digiSeedOffset1 11 \
    --digiSeedOffset2 22 \
    --geometryVersion ATLAS-R1-2011-02-00-00 \
    --conditionsTag OFLCOND-RUN12-SDR-31-01  \
    --DataRunNumber 180164 \
    --preExec 'HITtoRDO:flags.Beam.NumberOfCollisions=0;flags.Sim.TRTRangeCut=0.05' \
    --preInclude 'default:LArConfiguration.LArConfigRun1.LArConfigRun1NoPileUp' \
    --postInclude 'default:PyJobTransforms.UseFrontier' \
    --postExec 'with open("ConfigDigiCA.pkl", "wb") as f: cfg.store(f)'

rc=$?
status=$rc
echo "art-result: $rc digiCA"

# get reference directory
source DigitizationCheckReferenceLocation.sh
echo "Reference set being used: ${DigitizationTestsVersion}"

rc3=-9999
if [ $rc -eq 0 ]
then
    # Do reference comparisons
    art.py compare ref --mode=semi-detailed --no-diff-meta "$DigiOutFileName" "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DigitizationTests/ReferenceFiles/$DigitizationTestsVersion/$CMTCONFIG/$DigiOutFileNameCG"
    rc3=$?
    if [ $status -eq 0 ]
    then
       status=$rc3
    fi
fi
echo "art-result: $rc3 OLDvsFixedRef"

rc5=-9999
if [ $rc -eq 0 ]
then
    art.py compare grid --entries 10 "$1" "$2" --mode=semi-detailed --file="$DigiOutFileName"
    rc5=$?
    if [ $status -eq 0 ]
    then
       status=$rc5
    fi
fi
echo "art-result: $rc5 regression"

exit $status
