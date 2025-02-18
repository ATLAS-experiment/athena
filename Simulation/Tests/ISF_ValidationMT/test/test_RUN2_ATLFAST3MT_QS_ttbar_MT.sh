#!/bin/sh
#
# art-description: MC23-style RUN2 simulation using best knowledge geometry and ATLFAST3MT_QS in AthenaMT
# art-include: 24.0/Athena
# art-include: main/Athena
# art-type: grid
# art-athena-mt: 8
# art-architecture:  '#x86_64-intel'
# art-output: test.*.HITS.pool.root
# art-output: log.*
# art-output: Config*.pkl

export ATHENA_CORE_NUMBER=8

# RUN3 setup
# ATLAS-R2-2016-01-02-01 and OFLCOND-MC23-SDR-RUN3-08
Sim_tf.py \
    --CA \
    --multithreaded \
    --conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-08' \
    --simulator 'ATLFAST3MT_QS' \
    --postInclude 'PyJobTransforms.UseFrontier' \
    --preInclude 'EVNTtoHITS:Campaigns.MC23SimulationNoIoV' \
    --DataRunNumber 284500 \
    --geometryVersion 'default:ATLAS-R2-2016-01-02-01' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc20/EVNT/mc15_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.evgen.EVNT.e6337/EVNT.12458444._006026.pool.root.1" \
    --outputHITSFile "test.CA.HITS.pool.root" \
    --maxEvents 50 \
    --postExec 'with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
    --imf False

rc=$?
mv log.EVNTtoHITS log.EVNTtoHITS.CA
echo  "art-result: $rc simCA"
status=$rc

rc2=-9999
if [ $rc -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 10 ${ArtPackage} ${ArtJobName} --order-trees --mode=semi-detailed --diff-root --file=test.CA.HITS.pool.root
    rc2=$?
    if [ $status -eq 0 ]
    then
        status=$rc2
    fi
fi
echo  "art-result: $rc2 regression"

exit $status
