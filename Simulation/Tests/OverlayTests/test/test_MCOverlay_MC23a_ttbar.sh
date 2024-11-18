#!/bin/sh

# art-description: MC+MC Overlay without reco for MC23a, ttbar
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-include: main/Athena

# art-memory: 5999
# art-output: *.root
# art-output: log.*
# art-output: mem.summary.*
# art-output: mem.full.*
# art-output: runargs.*

events=20
HITS_File="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc23/HITS/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.simul.HITS.e8514_s4162/100events.HITS.pool.root"
RDO_BKG_File="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc23/RDO_BKG/mc23_13p6TeV.900149.PG_single_nu_Pt50.merge.RDO.e8514_e8528_s4112_d1865_d1858/100events.RDO.pool.root"


Overlay_tf.py \
--CA \
--runNumber 601229 \
--inputHITSFile ${HITS_File} \
--inputRDO_BKGFile ${RDO_BKG_File} \
--outputRDOFile MC_plus_MC.RDO.pool.root \
--maxEvents ${events} \
--skipEvents 10 --digiSeedOffset1 511 --digiSeedOffset2 727 \
--conditionsTag OFLCOND-MC23-SDR-RUN3-01  \
--geometryVersion ATLAS-R3S-2021-03-02-00 \
--preInclude 'all:Campaigns.MC23a' \
--imf False

rc=$?
status=$rc
echo "art-result: $rc overlay"

if command -v art.py >/dev/null 2>&1; then
    rc2=-9999
    if [ $rc -eq 0 ]
    then
        ArtPackage=$1
        ArtJobName=$2
        art.py compare grid --entries 10 "${ArtPackage}" "${ArtJobName}" --mode=semi-detailed --file MC_plus_MC.RDO.pool.root --diff-root
        rc2=$?
        status=$rc2
    fi
    echo "art-result: $rc2 regression"
fi

exit $status
