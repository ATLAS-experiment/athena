#!/bin/bash
#
# art-description: Run digitization of an MC20 ttbar sample with 2018 geometry and conditions, without pile-up using AthenaMT
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-athena-mt: 8
# art-include: 22.0-mc20/Athena
# art-include: 23.0/Athena
# art-include: main/Athena
# the name below is needed because of the environment variable (marks storing in tar file).
# art-output: mc20_nopileup_ttbar.MT.RDO.pool.root
# art-output: mc20_nopileup_ttbar.ST.RDO.pool.root
# art-output: log.*

export ATHENA_CORE_NUMBER=8

HSHITSFILE="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.simul.HITS.e4993_s3091/HITS.10504490._000425.pool.root.1"
MTDigiOutputFile="mc20_nopileup_ttbar.MT.RDO.pool.root"
STDigiOutputFile="mc20_nopileup_ttbar.ST.RDO.pool.root"

Digi_tf.py \
    --multithreaded \
    --inputHITSFile ${HSHITSFILE} \
    --conditionsTag default:OFLCOND-MC16-SDR-RUN2-09 \
    --digiSeedOffset1 170 \
    --digiSeedOffset2 170 \
    --geometryVersion default:ATLAS-R2-2016-01-00-01 \
    --DataRunNumber 310000 \
    --outputRDOFile ${MTDigiOutputFile} \
    --preInclude 'HITtoRDO:Campaigns/MC20NoPileUp.py,Digitization/ForceUseOfAlgorithms.py' \
    --postInclude 'PyJobTransforms/UseFrontier.py' \
    --skipEvents 0  \
    --maxEvents 100 \
    --imf False

rc=$?
status=$rc
echo "art-result: $rc MTdigi"
mv log.HITtoRDO log.HITtoRDO_MT

Digi_tf.py \
    --inputHITSFile ${HSHITSFILE} \
    --conditionsTag default:OFLCOND-MC16-SDR-RUN2-09 \
    --digiSeedOffset1 170 \
    --digiSeedOffset2 170 \
    --geometryVersion default:ATLAS-R2-2016-01-00-01 \
    --DataRunNumber 310000 \
    --outputRDOFile ${STDigiOutputFile} \
    --preInclude 'HITtoRDO:Campaigns/MC20NoPileUp.py,Digitization/ForceUseOfAlgorithms.py' \
    --postInclude 'PyJobTransforms/UseFrontier.py' \
    --skipEvents 0  \
    --maxEvents 100 \
    --imf False

rc2=$?
status=$rc2
echo "art-result: $rc2 STdigi"

rc3=-9999
if [ $status -eq 0 ]
then
    acmd.py diff-root mc20_nopileup_ttbar.ST.RDO.pool.root  mc20_nopileup_ttbar.MT.RDO.pool.root --error-mode resilient --mode semi-detailed --order-trees --ignore-leaves index_ref
    rc3=$?
    status=$rc3
fi
echo "art-result: $rc3 comparison"

rc4=-9999
if [ $status -eq 0 ]
then
    art.py compare grid --entries 10 "$1" "$2" --mode=semi-detailed --order-trees --diff-root --file=mc20_nopileup_ttbar.MT.RDO.pool.root
    rc4=$?
    status=$rc4
fi
echo "art-result: $rc4 regression"

exit $status
