#!/bin/sh
#
# art-description: Run simulation outside ISF, reading Z->tau tau events, modifying the Neutron timing cut, writing HITS, using 2015 geometry and conditions
# art-include: 24.0/Athena
# art-include: main/Athena
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-output: *.HITS.pool.root
# art-output: log.*
# art-output: Config*.pkl

AtlasG4_tf.py \
    --CA \
    --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/A3_Ztautau_filter-pythia6-7000.evgen.pool.root' \
    --outputHITSFile 'test.HITS.pool.root' \
    --maxEvents '5' \
    --skipEvents '0' \
    --randomSeed '10' \
    --geometryVersion 'ATLAS-R2-2015-03-01-00' \
    --conditionsTag 'OFLCOND-RUN12-SDR-19' \
    --DataRunNumber '222525' \
    --physicsList 'FTFP_BERT' \
    --truthStrategy 'MC15aPlus' \
    --postInclude 'PyJobTransforms.TransformUtils.UseFrontier' \
    --preInclude 'SimuJobTransforms.G4SignalCavern' \
    --imf False

rc=$?
status=$rc
mv log.AtlasG4Tf log.AtlasG4Tf_CA
echo  "art-result: $rc simCA"

rc2=-9999
if [ $rc -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 5 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --file=test.HITS.pool.root
    rc2=$?
    status=$rc2
fi
echo  "art-result: $rc2 regression"

exit $status
