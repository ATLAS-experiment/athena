#!/bin/sh
#
# art-description: Run MT and ST simulation, reading ttbar events, writing HITS, using MC23e geometry and conditions
# art-include: 23.0/Athena
# art-include: 23.0/AthSimulation
# art-include: 24.0/Athena
# art-include: 24.0/AthSimulation
# art-include: main/Athena
# art-include: main/AthSimulation
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-athena-mt: 8
# art-output: log.*
# art-output: Config*.pkl
# art-output: test.MT.HITS.pool.root
# art-output: test.ST.HITS.pool.root

export ATHENA_CORE_NUMBER=8

geometry=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN3)")
conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")

Sim_tf.py \
    --CA \
    --multithreaded \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/EVNT/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8453/EVNT.29328277._003902.pool.root.1" \
    --outputHITSFile "test.MT.HITS.pool.root" \
    --maxEvents 50 \
    --conditionsTag "default:${conditions}" \
    --geometryVersion "default:${geometry}" \
    --simulator 'FullG4MT_QS' \
    --postInclude 'default:PyJobTransforms.UseFrontier' \
    --preInclude 'EVNTtoHITS:Campaigns.MC23eSimulationMultipleIoV' \
    --jobNumber 1 \
    --postExec 'with open("ConfigSimMT.pkl", "wb") as f: cfg.store(f)' \
    --imf False

rc=$?
mv log.EVNTtoHITS log.EVNTtoHITS.MT
echo  "art-result: $rc AthenaMT"
status=$rc

rc2=-9999
unset ATHENA_CORE_NUMBER
Sim_tf.py \
    --CA \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/EVNT/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8453/EVNT.29328277._003902.pool.root.1" \
    --outputHITSFile "test.ST.HITS.pool.root" \
    --maxEvents 50 \
    --conditionsTag "default:${conditions}" \
    --geometryVersion "default:${geometry}" \
    --simulator 'FullG4MT_QS' \
    --postInclude 'default:PyJobTransforms.UseFrontier' \
    --preInclude 'EVNTtoHITS:Campaigns.MC23eSimulationMultipleIoV' \
    --jobNumber 1 \
    --postExec 'with open("ConfigSimST.pkl", "wb") as f: cfg.store(f)' \
    --imf False

mv log.EVNTtoHITS log.EVNTtoHITS.ST
rc2=$?
if [ $status -eq 0 ]
then
    status=$rc2
fi
echo  "art-result: $rc2 serial Athena"

rc3=-9999
if [ $status -eq 0 ]
then
    acmd.py diff-root test.MT.HITS.pool.root test.ST.HITS.pool.root \
        --error-mode resilient \
        --mode=semi-detailed \
        --order-trees
    rc3=$?
    status=$rc3
fi
echo  "art-result: $rc3 FullG4MT_STvsMT"

rc4=-9999
if [ $rc2 -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 50 ${ArtPackage} ${ArtJobName} --order-trees --diff-root --mode=semi-detailed --file=test.MT.HITS.pool.root
    rc4=$?
    if [ $status -eq 0 ]
    then
        status=$rc4
    fi
fi
echo  "art-result: $rc4 regression"

exit $status
