#!/bin/sh
#
# art-description: MC23-style RUN2 simulation using 21.0-compatible geometry and ATLFAST3F
# art-include: 24.0/Athena
# art-include: main/Athena
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-output: test.HITS.pool.root

# MC16 setup
# ATLAS-R2-2016-01-00-01 and OFLCOND-MC23-SDR-RUN3-01

Sim_tf.py \
    --CA \
    --conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-01' \
    --simulator 'ATLFAST3F_G4MS' \
    --DataRunNumber 284500 \
    --preInclude 'EVNTtoHITS:Campaigns.MC23SimulationNoIoV' \
    --geometryVersion 'default:ATLAS-R2-2016-01-00-01' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.evgen.EVNT.e4993.EVNT.08166201._000012.pool.root.1" \
    --outputHITSFile "test.HITS.pool.root" \
    --maxEvents 2000 \
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
    if [ $status -eq 0 ]
    then
        status=$rc2
    fi
fi

echo  "art-result: $rc2 regression"

exit $status
