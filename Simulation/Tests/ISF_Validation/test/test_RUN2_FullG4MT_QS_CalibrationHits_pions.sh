#!/bin/sh
#
# art-description: MC23-style RUN2 simulation using best knowledge geometry and FullG4MT_QS simulator, reading single pion events, writing HITS including full CaloCalibrationHit information
# art-include: 24.0/Athena
# art-include: main/Athena
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-memory: 2999
# art-output: *.HITS.pool.root
# art-output: log.*
# art-output: Config*.pkl

# RUN2 setup
# ATLAS-R2-2016-01-02-01 and OFLCOND-MC23-SDR-RUN3-08
Sim_tf.py \
    --CA \
    --conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-08' \
    --simulator 'FullG4MT_QS' \
    --postInclude 'PyJobTransforms.UseFrontier' \
    --preInclude 'EVNTtoHITS:Campaigns.MC23SimulationNoIoV' \
    --DataRunNumber 284500 \
    --geometryVersion 'default:ATLAS-R2-2016-01-02-01' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc20/EVNT/mc15_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.evgen.EVNT.e6337/EVNT.12458444._006026.pool.root.1" \
    --outputHITSFile "test.HITS.pool.root" \
    --maxEvents 10 \
    --postExec 'with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
    --imf False

rc=$?
mv log.EVNTtoHITS log.EVNTtoHITS_CA
echo  "art-result: $rc simCA"
status=$rc

rc2=-9999
if [ $rc -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 10 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --file=test.HITS.pool.root
    rc2=$?
    if [ $status -eq 0 ]
    then
        status=$rc2
    fi
fi
echo  "art-result: $rc2 regression"

exit $status
