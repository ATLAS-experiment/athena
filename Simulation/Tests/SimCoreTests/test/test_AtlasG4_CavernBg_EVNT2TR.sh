#!/bin/sh
#
# art-description: Run simulation outside ISF, reading min bias events, write cavern background track records, using 2015 geometry and conditions
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-include: 24.0/Athena
# art-include: main/Athena
# art-output: *.EVNT_TR.pool.root
# art-output: log.*
# art-output: Config*.pkl

AtlasG4_tf.py \
    --CA \
    --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/minbias_Inelastic_low-pythia8-7000.evgen.pool.root' \
    --outputHITSFile 'discard.HITS.pool.root' \
    --outputEVNT_TRFile 'test.EVNT_TR.pool.root' \
    --maxEvents '2' \
    --skipEvents '0' \
    --randomSeed '5678' \
    --geometryVersion 'ATLAS-R2-2015-03-01-00' \
    --conditionsTag 'OFLCOND-RUN12-SDR-19' \
    --DataRunNumber '222525' \
    --physicsList 'QGSP_BERT_HP' \
    --postInclude 'PyJobTransforms.TransformUtils.UseFrontier' \
    --truthStrategy 'MC12' \
    --postExec 'with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
    --imf False

rc=$?
status=$rc
mv log.AtlasG4Tf log.AtlasG4Tf_CA
echo  "art-result: $rc simCA"

# remove HITS file
rm discard.HITS.pool.root

rc2=-9999
if [ $rc -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 2 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --file=test.EVNT_TR.pool.root
    rc2=$?
    status=$rc2
fi
echo  "art-result: $rc2 regression"

exit $status
