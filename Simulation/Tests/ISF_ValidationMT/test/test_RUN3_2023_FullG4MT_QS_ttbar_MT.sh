#!/bin/sh
#
# art-description: Run simulation using ISF with the FullG4MT_QS simulator, reading 13.6 TeV ttbar events, writing HITS, using the best knowledge RUN3 geometry and MC23 conditions
# art-include: 23.0/Athena
# art-include: 23.0/AthSimulation
# art-include: 24.0/Athena
# art-include: 24.0/AthSimulation
# art-include: main/Athena
# art-include: main/AthSimulation
# art-type: grid
# art-athena-mt: 8
# art-architecture:  '#x86_64-intel'
# art-output: *.pool.root
# art-output: log.*
# art-output: Config*.pkl

export ATHENA_CORE_NUMBER=8

InputEVNT='/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/EVNT/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8453/EVNT.29328277._003902.pool.root.1'

Sim_tf.py \
    --CA True \
    --multithreaded \
    --conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-01' \
    --simulator 'FullG4MT_QS' \
    --postInclude 'PyJobTransforms.TransformUtils.UseFrontier' \
    --preInclude 'EVNTtoHITS:Campaigns.MC23cSimulationMultipleIoV' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
    --inputEVNTFile $InputEVNT \
    --outputHITSFile 'test.HITS.pool.root' \
    --maxEvents '50' \
    --jobNumber 1 \
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
    art.py compare grid --entries 50 ${ArtPackage} ${ArtJobName} --order-trees --mode=semi-detailed --diff-root --file=test.HITS.pool.root
    rc2=$?
    if [ $status -eq 0 ]
    then
        status=$rc2
    fi
fi
echo  "art-result: $rc2 regression"

exit $status
