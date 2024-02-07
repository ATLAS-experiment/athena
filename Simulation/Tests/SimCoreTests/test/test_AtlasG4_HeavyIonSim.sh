#!/bin/sh
#
# art-description: Run simulation outside ISF, reading lead ions peripheral simulation (low multiplicity) events, writing HITS, using 2015 geometry and conditions
# art-include: 24.0/Athena
# art-include: main/Athena
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-output: *.HITS.pool.root
# art-output: log.*
# art-output: Config*.pkl

AtlasG4_tf.py \
    --CA \
    --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/pbpb_Peripheral-hijing-5500.evgen.pool.root' \
    --outputHITSFile 'test.HITS.pool.root' \
    --maxEvents '1' \
    --skipEvents '4' \
    --randomSeed '10' \
    --geometryVersion 'ATLAS-R1-2012-03-02-00' \
    --conditionsTag 'OFLCOND-RUN12-SDR-19' \
    --DataRunNumber '210184' \
    --physicsList 'FTFP_BERT' \
    --postInclude 'PyJobTransforms.TransformUtils.UseFrontier' \
    --truthStrategy 'MC12' \
    --postExec 'with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
    --imf False

#todo add postInclude SimuJobTransforms.HijingPars
rc=$?
status=$rc
mv log.AtlasG4Tf log.AtlasG4Tf_CA
echo  "art-result: $rc simCA"

rc2=-9999
if [ $rc -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 1 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --file=test.HITS.pool.root
    rc2=$?
    status=$rc2
fi
echo  "art-result: $rc2 regression"

exit $status
