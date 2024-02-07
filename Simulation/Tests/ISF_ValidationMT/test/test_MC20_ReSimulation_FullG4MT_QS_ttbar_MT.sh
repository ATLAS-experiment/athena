#!/bin/sh
#
# art-description: ReSimulation Workflow running with MC16 conditions/geometry
# art-include: main/Athena
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-output: log.*
# art-output: Config*.pkl
# art-output: original.HITS.pool.root
# art-output: resim.*.HITS.pool.root

INPUTEVNTFILE="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.evgen.EVNT.e4993.EVNT.08166201._000012.pool.root.1"
#INPUTEVNTFILE='/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ISF_Validation/pi_E50_eta0-60.evgen.pool.root'
MAXEVENTS=10

rc=-9999
if [ -f "original.HITS.pool.root" ]
then
    rc=0
    echo "skipping initial-simOLD step"
else
    Sim_tf.py \
        --CA \
        --conditionsTag 'default:OFLCOND-MC16-SDR-14' \
        --simulator 'FullG4MT' \
        --postInclude 'default:PyJobTransforms.UseFrontier' \
        --preInclude 'EVNTtoHITS:Campaigns.MC16SimulationSingleIoV' \
        --geometryVersion 'default:ATLAS-R2-2016-01-00-01' \
        --inputEVNTFile $INPUTEVNTFILE \
        --outputHITSFile "original.HITS.pool.root" \
        --maxEvents $MAXEVENTS \
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
        --conditionsTag 'ReSim:OFLCOND-MC16-SDR-14' \
        --simulator 'FullG4MT_QS' \
        --postInclude 'ReSim:PyJobTransforms.UseFrontier' \
        --preInclude 'ReSim:Campaigns.MC16SimulationNoIoV' \
        --geometryVersion 'ReSim:ATLAS-R2-2016-01-00-01' \
        --inputHITSFile "original.HITS.pool.root" \
        --outputHITS_RSMFile "resim.CA.HITS.pool.root" \
        --maxEvents $MAXEVENTS \
        --postExec 'ReSim:with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
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
