#!/bin/sh
#
# art-description: Run MT simulation with concurrent Geant4-Gaudi event loop. Experimental development for Run 4
# art-include: main/Athena

# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-athena-mt: 8
# art-output: log.*
# art-output: test.HITS.pool.root

export ATHENA_CORE_NUMBER=8

geometry=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN3)")
conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")

AtlasG4_tf.py \
    --multithreaded \
    --useG4Workers True \
    --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/ttbar_muplusjets-pythia6-7000.evgen.pool.root' \
    --outputHITSFile 'test.HITS.pool.root' \
    --maxEvents '20' \
    --skipEvents '0' \
    --randomSeed '10' \
    --geometryVersion "default:${geometry}" \
    --conditionsTag "default:${conditions}" \
    --DataRunNumber '284500' \
    --physicsList 'FTFP_BERT_ATL' \
    --postInclude 'PyJobTransforms.UseFrontier' \
    --imf False

rc=$?
echo  "art-result: $rc simulation"
status=$rc

rc2=-9999
if [ $status -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 10 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --order-trees --diff-root
    rc2=$?
    status=$rc2
fi
echo  "art-result: $rc2 regression"
exit $status
