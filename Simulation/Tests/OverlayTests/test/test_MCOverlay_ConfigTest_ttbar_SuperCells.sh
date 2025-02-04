#!/bin/bash

# art-description: MC+MC Overlay with MT support, config test
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-include: main/Athena

# art-output: mcOverlayRDO.pool.root
# art-output: log.*
# art-output: mem.summary.*
# art-output: mem.full.*
# art-output: runargs.*
# art-output: *.pkl

set -o pipefail

events=2
HITS_File="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/HITS/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.simul.HITS.e8453_s3873/50events.HITS.pool.root"
RDO_BKG_File="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/RDO_BKG/mc21_13p6TeV.900149.PG_single_nu_Pt50.digit.RDO.e8453_e8455_s3864_d1761/50events.RDO.pool.root"

Overlay_tf.py \
--CA \
--detectors Calo L1Calo \
--runNumber 601229 \
--inputHITSFile ${HITS_File} \
--inputRDO_BKGFile ${RDO_BKG_File} \
--outputRDOFile mcOverlayRDO.pool.root \
--maxEvents $events \
--conditionsTag OFLCOND-MC21-SDR-RUN3-10  \
--geometryVersion ATLAS-R3S-2021-03-00-00 \
--preInclude 'all:Campaigns.MC21a' \
--postExec 'with open("ConfigOverlay.pkl", "wb") as f: cfg.store(f)' \
--imf False \
--athenaopts="--threads=1"

rc=$?
status=$rc
echo "art-result: $rc overlay"

if command -v art.py >/dev/null 2>&1; then
    rc2=-9999
    if [ $rc -eq 0 ]
    then
        ArtPackage=$1
        ArtJobName=$2
        art.py compare grid --entries 10 "${ArtPackage}" "${ArtJobName}" --mode=semi-detailed --order-trees
        rc2=$?
        status=$rc2
    fi
    echo  "art-result: $rc2 regression"
fi

exit $status
