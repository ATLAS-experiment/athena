#!/bin/sh
# 
# art-description: MC23-style simulation of decaying Rhadrons using FullG4MT_QS (tests RHadrons package)
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-include: 24.0/Athena
# art-include: 24.0/AthSimulation
# art-include: main/Athena
# art-include: main/AthSimulation
# art-output: *.root
# art-output: PDGTABLE.MeV.*
# art-output: *.HITS.pool.root
# art-output: log.*
# art-output: Config*.pkl

mkdir -p CA && cd CA
Sim_tf.py \
    --CA True \
    --conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-01' \
    --simulator 'FullG4MT_QS' \
    --postInclude 'PyJobTransforms.UseFrontier' \
    --preInclude 'EVNTtoHITS:Campaigns.MC23cSimulationMultipleIoV' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/Rhadrons.EVNT.34227998._000001.pool.root.1" \
    --outputHITSFile "test.CA.HITS.pool.root" \
    --maxEvents '50' \
    --jobNumber 1 \
    --postExec 'with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
    --imf False

rc=$?
mv PDGTABLE.MeV ../PDGTABLE.MeV.CA
mv log.EVNTtoHITS ../log.EVNTtoHITS.CA
mv ConfigSimCA.pkl ../ConfigSimCA.pkl
mv test.CA.HITS.pool.root ../test.CA.HITS.pool.root
cd ../
echo  "art-result: $rc simCA"
status=$rc

rc2=-9999
if [ $rc -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 50 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --file=test.CA.HITS.pool.root
    rc2=$?
    status=$rc2
fi
echo  "art-result: $rc2 regression"

exit $status
