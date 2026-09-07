#!/bin/sh
#
# art-description: Run MT and ST simulation with concurrent Geant4-Gaudi event loop. Experimental development for Run 4
# art-include: main/Athena

# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-athena-mt: 8
# art-output: log.*
# art-output: test.MT.HITS.pool.root
# art-output: test.ST.HITS.pool.root

export ATHENA_CORE_NUMBER=8

geometry=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN3)")
conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")

AtlasG4_tf.py \
    --multithreaded \
    --useG4Workers True \
    --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/ttbar_muplusjets-pythia6-7000.evgen.pool.root' \
    --outputHITSFile 'test.MT.HITS.pool.root' \
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
echo  "art-result: $rc MTsim"
status=$rc

unset ATHENA_CORE_NUMBER

AtlasG4_tf.py \
    --useG4Workers True \
    --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/ttbar_muplusjets-pythia6-7000.evgen.pool.root' \
    --outputHITSFile 'test.ST.HITS.pool.root' \
    --maxEvents '20' \
    --skipEvents '0' \
    --randomSeed '10' \
    --geometryVersion "default:${geometry}" \
    --conditionsTag "default:${conditions}" \
    --DataRunNumber '284500' \
    --physicsList 'FTFP_BERT_ATL' \
    --postInclude 'PyJobTransforms.UseFrontier' \
    --imf False
rc2=$?
echo  "art-result: $rc2 STsim"
if [ $status -eq 0 ]
then
    status=$rc2
fi

rc3=-9999
if [ $status -eq 0 ]
then
    acmd.py diff-root test.MT.HITS.pool.root test.ST.HITS.pool.root --error-mode resilient --mode=semi-detailed --order-trees
    rc3=$?
    status=$rc3
fi
echo  "art-result: $rc3 comparision"
exit $status
