#!/bin/sh
#
# art-description: MC23-style simulation using FullG4 (7 TeV minbias input - needs updating)
# art-include: 24.0/Athena
# art-include: main/Athena
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-memory: 2999
# art-output: test.HITS.pool.root

# RUN3 setup - Frozen Showers currently off by default
# ATLAS-R3S-2021-03-02-00 and OFLCOND-MC23-SDR-RUN3-08
Sim_tf.py \
    --CA \
    --conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-08' \
    --simulator 'FullG4MT' \
    --postInclude 'default:PyJobTransforms.UseFrontier' \
    --preInclude 'EVNTtoHITS:Campaigns.MC23SimulationSingleIoV' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ISF_Validation/mc12_valid.119994.Pythia8_A2MSTW2008LO_minbias_inelastic.evgen.EVNT.e3099.01517253._000001.pool.root.1" \
    --outputHITSFile "test.HITS.pool.root" \
    --maxEvents 50 \
    --imf False

rc=$?
status=$rc
echo  "art-result: $rc simCA"

rc2=-9999
if [ $rc -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 10 ${ArtPackage} ${ArtJobName} --mode=semi-detailed
    rc2=$?
    status=$rc2
fi

echo  "art-result: $rc2 regression"
exit $status
