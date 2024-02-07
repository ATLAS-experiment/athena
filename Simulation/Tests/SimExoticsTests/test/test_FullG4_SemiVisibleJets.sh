#!/bin/sh
#
# art-description: MC16-style simulation of decaying staus using FullG4 (tests the Sleptons + Gauginos packages)
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

# MC16 setup
# ATLAS-R2-2016-01-00-01 and OFLCOND-MC16-SDR-14
Sim_tf.py \
    --CA \
    --conditionsTag 'default:OFLCOND-MC16-SDR-14' \
    --truthStrategy 'MC15aPlusLLP' \
    --simulator 'FullG4MT' \
    --postInclude 'PyJobTransforms.UseFrontier' \
    --preInclude 'EVNTtoHITS:Campaigns.MC16Simulation' \
    --preExec 'EVNTtoHITS:ConfigFlags.Input.SpecialConfiguration={"NonInteractingPDGCodes":"[51,52,-53,53]"}' \
    --geometryVersion 'default:ATLAS-R2-2016-01-00-01' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/mc16_13TeV.999999.DMsimp_s_spin1_SVJ.test.EVNT.pool.root" \
    --outputHITSFile "CA.HITS.pool.root" \
    --maxEvents 10 \
    --postExec 'with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
    --imf False

rc=$?
mv PDGTABLE.MeV PDGTABLE.MeV.CA
mv log.EVNTtoHITS log.EVNTtoHITS.CA
echo  "art-result: $rc simCA"
status=$rc

rc2=-9999
if [ $rc -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 10 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --file=CA.HITS.pool.root
    rc2=$?
    status=$rc2
fi
echo  "art-result: $rc2 regression"

exit $status
