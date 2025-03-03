#!/bin/sh
#
# art-description: Tests inner detector response to minbias events, using Run3 geometry and conditions
# art-include: 24.0/Athena
# art-include: main/Athena
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-output: test.HITS.pool.root

geometry=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN3)")
conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")

AtlasG4_tf.py \
    --CA \
    --conditionsTag "default:${conditions}" \
    --geometryVersion "default:${geometry}" \
    --detectors Bpipe ID Truth \
    --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/minbias_Inelastic-pythia8-7000.evgen.pool.root' \
    --outputHITSFile 'test.HITS.pool.root' \
    --maxEvents '50' \
    --skipEvents '0' \
    --randomSeed '10' \
    --preInclude 'AtlasG4Tf:Campaigns.MC23SimulationSingleIoV' \
    --postInclude 'PyJobTransforms.UseFrontier' \
    --imf False
# TODO would be good to test applying beam rotations in this job
rc=$?
status=$rc
echo  "art-result: $rc simulation"

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
