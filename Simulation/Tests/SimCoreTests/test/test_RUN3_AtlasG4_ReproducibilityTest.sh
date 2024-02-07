#!/bin/sh
#
# art-description: Reading single particle gen events, checking that the SkipEvents argument works, using RUN3 geometry and conditions
# art-include: 24.0/Athena
# art-include: 24.0/AthSimulation
# art-include: main/Athena
# art-include: main/AthSimulation
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-output: hitsFull.ttbar.pool.root
# art-output: hitsHalf1.ttbar.pool.root
# art-output: hitsHalf2.ttbar.pool.root
# art-output: hitsMerged.ttbar.pool.root
# art-output: hitsFullMerged.ttbar.pool.root

# Run 10 events normally
AtlasG4_tf.py \
    --CA \
    --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/ttbar_muplusjets-pythia6-7000.evgen.pool.root' \
    --preInclude 'AtlasG4Tf:Campaigns.MC23SimulationSingleIoV' \
    --geometryVersion 'ATLAS-R3S-2021-03-02-00' \
    --conditionsTag 'OFLCOND-MC23-SDR-RUN3-01' \
    --outputHITSFile 'hitsFull.ttbar.pool.root' \
    --maxEvents '10' \
    --skipEvents '0'

echo "art-result: $? unsplit-sim"

# Run first 5 events
AtlasG4_tf.py \
    --CA \
    --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/ttbar_muplusjets-pythia6-7000.evgen.pool.root' \
    --preInclude 'AtlasG4Tf:Campaigns.MC23SimulationSingleIoV' \
    --geometryVersion 'ATLAS-R3S-2021-03-02-00' \
    --conditionsTag 'OFLCOND-MC23-SDR-RUN3-01' \
    --outputHITSFile 'hitsHalf1.ttbar.pool.root' \
    --maxEvents '5' \
    --skipEvents '0'

echo "art-result: $? split-sim1"

# Run next 5 events
AtlasG4_tf.py \
    --CA \
    --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/ttbar_muplusjets-pythia6-7000.evgen.pool.root' \
    --preInclude 'AtlasG4Tf:Campaigns.MC23SimulationSingleIoV' \
    --geometryVersion 'ATLAS-R3S-2021-03-02-00' \
    --conditionsTag 'OFLCOND-MC23-SDR-RUN3-01' \
    --outputHITSFile 'hitsHalf2.ttbar.pool.root' \
    --maxEvents '5' \
    --skipEvents '5'

echo "art-result: $? split-sim2"

# Merge the partial files
HITSMerge_tf.py \
    --CA \
    --inputHitsFile hitsHalf1.ttbar.pool.root hitsHalf2.ttbar.pool.root \
    --outputHitsFile 'hitsMerged.ttbar.pool.root'

echo "art-result: $? split-merge"

# Run a dummy merge on the full hits file to deal with lossy compression:
HITSMerge_tf.py \
    --CA \
    --inputHitsFile 'hitsFull.ttbar.pool.root' \
    --outputHitsFile 'hitsFullMerged.ttbar.pool.root'

echo "art-result: $? dummy-merge"

# Compare the merged outputs
acmd.py diff-root hitsFullMerged.ttbar.pool.root hitsMerged.ttbar.pool.root --order-trees --error-mode resilient
rc=$?
echo "art-result: $rc comparison"

exit $rc
