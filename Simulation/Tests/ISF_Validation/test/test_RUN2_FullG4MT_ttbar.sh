#!/bin/sh
#
# art-description: MC23-style RUN2 simulation using best knowledge geometry and FullG4MT in serial Athena
# art-include: 24.0/Athena
# art-include: 24.0/AthSimulation
# art-include: main/Athena
# art-include: main/AthSimulation
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-output: test.CA.HITS.pool.root
# art-output: Config*

# RUN3 setup
# ATLAS-R2-2016-01-02-01 and OFLCOND-MC23-SDR-RUN3-01
  Sim_tf.py \
      --CA \
      --conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-01' \
      --simulator 'FullG4MT' \
      --postExec 'with open("ConfigCA.pkl", "wb") as f: cfg.store(f)' \
      --postInclude 'default:PyJobTransforms.UseFrontier' \
      --preInclude 'EVNTtoHITS:Campaigns.MC23SimulationNoIoV' \
      --DataRunNumber 284500 \
      --geometryVersion 'default:ATLAS-R2-2016-01-02-01' \
      --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.evgen.EVNT.e4993.EVNT.08166201._000012.pool.root.1" \
      --outputHITSFile "test.CA.HITS.pool.root" \
      --maxEvents 3 \
      --imf False

rc=$?
mv log.EVNTtoHITS log.EVNTtoHITS_CA
echo  "art-result: $rc simCA"
status=$rc

rc4=-9999
if [ $rc -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 3 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --file=test.CA.HITS.pool.root
    rc4=$?
    if [ $status -eq 0 ]
    then
        status=$rc4
    fi
fi
echo  "art-result: $rc4 regression"

exit $status
