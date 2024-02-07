#!/bin/sh
#
# art-description: ReSimulation Workflow running with MC21 conditions/geometry and variable beamspot
# art-include: main/Athena
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-output: log.*
# art-output: Config*.pkl
# art-output: original.HITS.pool.root
# art-output: resim.*.HITS.pool.root

INPUTEVNTFILE="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/EVNT/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8453/EVNT.29328277._003902.pool.root.1"
MAXEVENTS=50

rc=-9999
if [ -f "original.HITS.pool.root" ]
then
    rc=0
    echo "skipping initial-simOLD step"
else
    Sim_tf.py \
        --CA \
        --conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-05' \
        --simulator 'FullG4MT' \
        --postInclude 'default:PyJobTransforms.UseFrontier' \
        --preInclude 'EVNTtoHITS:Campaigns.MC21SimulationMultipleIoV' \
        --geometryVersion 'default:ATLAS-R3S-2021-03-00-00' \
        --inputEVNTFile $INPUTEVNTFILE \
        --outputHITSFile "original.HITS.pool.root" \
        --maxEvents $MAXEVENTS \
        --jobNumber 29 \
        --imf False

    rc=$?
    mv log.EVNTtoHITS log.EVNTtoHITS.initial
fi
status=$rc
echo "art-result: $rc initial-simOLD"

rc1=-9999
if [ $rc -eq 0 ]
then
    ReSim_tf.py \
        --CA \
        --conditionsTag 'ReSim:OFLCOND-MC21-SDR-RUN3-05' \
        --simulator 'FullG4MT_QS' \
        --postInclude 'ReSim:PyJobTransforms.UseFrontier' \
        --preInclude 'ReSim:Campaigns.MC21SimulationNoIoV' \
        --geometryVersion 'ReSim:ATLAS-R3S-2021-03-00-00' \
        --inputHITSFile "original.HITS.pool.root" \
        --outputHITS_RSMFile "resim.CA.HITS.pool.root" \
        --maxEvents $MAXEVENTS \
        --postExec 'ReSim:with open("ConfigReSimCA.pkl", "wb") as f: cfg.store(f)' \
        --imf False

    rc1=$?
    status=$rc1
    mv log.ReSim log.ReSim.CA
fi
echo "art-result: $rc1 resimCA"

rc2=-9999
if [ $rc1 -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 10 ${ArtPackage} ${ArtJobName} --order-trees --mode=semi-detailed --diff-root --file=resim.CA.HITS.pool.root
    rc2=$?
    if [ $status -eq 0 ]
    then
        status=$rc2
    fi
fi
echo  "art-result: $rc2 regression"

exit $status
