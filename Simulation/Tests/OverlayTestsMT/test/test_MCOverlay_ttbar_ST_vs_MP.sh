#!/bin/sh

# art-description: MC+MC Overlay with MP support, running with 8 processes
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-athena-mt: 8
# art-include: main/Athena

# art-output: MC_plus_MC.MP.RDO.pool.root
# art-output: MC_plus_MC.SP.RDO.pool.root
# art-output: log.*
# art-output: mem.summary.*
# art-output: mem.full.*
# art-output: runargs.*

export ATHENA_CORE_NUMBER=8

HITS_File=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.HITS_RUN2[0])")
RDO_BKG_File="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/OverlayTests/PresampledPileUp/22.0/Run2/large/mc20_13TeV.900149.PG_single_nu_Pt50.digit.RDO.e8307_s3482_s3136_d1715/RDO.26811908._031801.pool.root.1"


Overlay_tf.py \
--CA \
--multiprocess \
--inputHITSFile ${HITS_File} \
--inputRDO_BKGFile ${RDO_BKG_File} \
--outputRDOFile MC_plus_MC.MP.RDO.pool.root \
--maxEvents 10 --skipEvents 10 --digiSeedOffset1 511 --digiSeedOffset2 727 \
--conditionsTag OFLCOND-MC16-SDR-RUN2-12 \
--geometryVersion ATLAS-R2-2016-01-00-01 \
--preInclude 'all:Campaigns.MC20e' \
--imf False

rc=$?
status=$rc
echo "art-result: $rc overlay MP"
mv log.Overlay log.OverlayMP

rc2=-9999
if [ $rc -eq 0 ]
then
    Overlay_tf.py \
    --CA \
    --inputHITSFile ${HITS_File} \
    --inputRDO_BKGFile ${RDO_BKG_File} \
    --outputRDOFile MC_plus_MC.SP.RDO.pool.root \
    --maxEvents 10 --skipEvents 10 --digiSeedOffset1 511 --digiSeedOffset2 727 \
    --conditionsTag OFLCOND-MC16-SDR-RUN2-12 \
    --geometryVersion ATLAS-R2-2016-01-00-01 \
    --preInclude 'all:Campaigns.MC20e' \
    --imf False
    rc2=$?
    status=$rc2
fi
echo  "art-result: $rc2 overlay SP"

rc3=-9999
if [ $rc2 -eq 0 ]
then
    mv MC_plus_MC.SP.RDO.pool.root backup_MC_plus_MC.SP.RDO.pool.root
    rm PoolFileCatalog.xml
    RDOMerge_tf.py \
        --CA \
        --inputRDOFile backup_MC_plus_MC.SP.RDO.pool.root \
        --outputRDO_MRGFile MC_plus_MC.SP.RDO.pool.root
    rc3=$?
    rm backup_MC_plus_MC.SP.RDO.pool.root
    status=$rc3
fi
echo "art-result: $rc3 RDOMerge_tf.py SP"

rc4=-9999
if [ $rc3 -eq 0 ]
then
    acmd.py diff-root MC_plus_MC.SP.RDO.pool.root MC_plus_MC.MP.RDO.pool.root --error-mode resilient --mode=semi-detailed --order-trees
    rc4=$?
    status=$rc4
fi
echo "art-result: $rc4 comparison"

exit $status
