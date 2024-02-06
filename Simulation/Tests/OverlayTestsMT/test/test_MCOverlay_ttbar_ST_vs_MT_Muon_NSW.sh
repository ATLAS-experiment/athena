#!/bin/sh

# art-description: MC+MC Overlay with MT support, running with 8 threads
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-athena-mt: 8
# art-include: 22.0/Athena
# art-include: main/Athena

# art-output: MC_plus_MC.MT.RDO.pool.root
# art-output: MC_plus_MC.ST.RDO.pool.root
# art-output: log.*
# art-output: mem.summary.*
# art-output: mem.full.*
# art-output: runargs.*

export ATHENA_CORE_NUMBER=8

HITS_File="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/HITS/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.simul.HITS.e8453_s3873/50events.HITS.pool.root"
RDO_BKG_File="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/RDO_BKG/mc21_13p6TeV.900149.PG_single_nu_Pt50.digit.RDO.e8453_e8455_s3864_d1761/50events.RDO.pool.root"


Overlay_tf.py \
--CA \
--detector Muon \
--multithreaded \
--runNumber 601229 \
--inputHITSFile ${HITS_File} \
--inputRDO_BKGFile ${RDO_BKG_File} \
--outputRDOFile MC_plus_MC.MT.RDO.pool.root \
--maxEvents 10 --skipEvents 10 --digiSeedOffset1 511 --digiSeedOffset2 727 \
--conditionsTag OFLCOND-MC21-SDR-RUN3-07  \
--geometryVersion ATLAS-R3S-2021-03-00-00 \
--preInclude 'all:Campaigns.MC21a' \
--imf False

rc=$?
status=$rc
echo "art-result: $rc overlayMT"
mv log.Overlay log.OverlayMT

rc2=-9999
if [ $rc -eq 0 ]
then
    Overlay_tf.py \
    --CA \
    --detector Muon \
    --runNumber 601229 \
    --inputHITSFile ${HITS_File} \
    --inputRDO_BKGFile ${RDO_BKG_File} \
    --outputRDOFile MC_plus_MC.ST.RDO.pool.root \
    --maxEvents 10 --skipEvents 10 --digiSeedOffset1 511 --digiSeedOffset2 727 \
    --conditionsTag OFLCOND-MC21-SDR-RUN3-07  \
    --geometryVersion ATLAS-R3S-2021-03-00-00 \
    --preInclude 'all:Campaigns.MC21a' \
    --imf False
    rc2=$?
    status=$rc2
fi
echo  "art-result: $rc2 overlayST"

rc3=-9999
if [ $rc2 -eq 0 ]
then
    acmd.py diff-root MC_plus_MC.ST.RDO.pool.root MC_plus_MC.MT.RDO.pool.root --error-mode resilient --mode=semi-detailed --order-trees
    rc3=$?
    status=$rc3
fi
echo  "art-result: $rc3 comparison"

exit $status
