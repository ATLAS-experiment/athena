#!/bin/sh
#
# art-description: MC23-style simulation using FullG4, checking that the SkipEvents argument works (7 TeV ttbar input - needs updating)
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
Sim_tf.py \
    --CA \
    --conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-03' \
    --simulator 'FullG4MT_QS' \
    --postInclude 'default:PyJobTransforms.UseFrontier' \
    --preInclude 'EVNTtoHITS:Campaigns.MC23SimulationSingleIoV' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
    --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ISF_Validation/mc12_valid.110401.PowhegPythia_P2012_ttbar_nonallhad.evgen.EVNT.e3099.01517252._000001.pool.root.1' \
    --outputHITSFile 'hitsFull.ttbar.pool.root' \
    --maxEvents '10' \
    --skipEvents '0'

rc=$?
status=$rc
echo "art-result: $rc unsplit-sim"

# Run first 5 events
Sim_tf.py \
    --CA \
    --conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-03' \
    --simulator 'FullG4MT_QS' \
    --postInclude 'default:PyJobTransforms.UseFrontier' \
    --preInclude 'EVNTtoHITS:Campaigns.MC23SimulationSingleIoV' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
    --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ISF_Validation/mc12_valid.110401.PowhegPythia_P2012_ttbar_nonallhad.evgen.EVNT.e3099.01517252._000001.pool.root.1' \
    --outputHITSFile 'hitsHalf1.ttbar.pool.root' \
    --maxEvents '5' \
    --skipEvents '0'

rc2=$?
if [ $status -eq 0 ]
then
    status=$rc2
fi
echo "art-result: $rc2 split-sim1"

# Run next 5 events
Sim_tf.py \
    --CA \
    --conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-03' \
    --simulator 'FullG4MT_QS' \
    --postInclude 'default:PyJobTransforms.UseFrontier' \
    --preInclude 'EVNTtoHITS:Campaigns.MC23SimulationSingleIoV' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
    --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ISF_Validation/mc12_valid.110401.PowhegPythia_P2012_ttbar_nonallhad.evgen.EVNT.e3099.01517252._000001.pool.root.1' \
    --outputHITSFile 'hitsHalf2.ttbar.pool.root' \
    --maxEvents '5' \
    --skipEvents '5'

rc3=$?
if [ $status -eq 0 ]
then
    status=$rc3
fi
echo "art-result: $rc3 split-sim2"

rc4=-9999
if [ $rc2 -eq 0 ] && [ $rc3 -eq 0 ]
then
    # Merge the partial files
    HITSMerge_tf.py \
        --CA \
        --inputHitsFile hitsHalf1.ttbar.pool.root hitsHalf2.ttbar.pool.root \
        --outputHitsFile 'hitsMerged.ttbar.pool.root'
    rc4=$?
    if [ $status -eq 0 ]
    then
        status=$rc4
    fi
fi
echo "art-result: $rc4 split-merge"

rc5=-9999
if [ $rc4 -eq 0 ]
then
    # Run a dummy merge on the full hits file to deal with lossy compression:
    HITSMerge_tf.py \
        --CA \
        --inputHitsFile 'hitsFull.ttbar.pool.root' \
        --outputHitsFile 'hitsFullMerged.ttbar.pool.root'
    rc5=$?
    if [ $status -eq 0 ]
    then
        status=$rc5
    fi
fi
echo "art-result: $rc5 dummy-merge"

rc6=-9999
if [ $rc -eq 0 ] && [ $rc5 -eq 0 ]
then
    # Compare the merged outputs
    acmd.py diff-root hitsFullMerged.ttbar.pool.root hitsMerged.ttbar.pool.root
    rc6=?$
    if [ $status -eq 0 ]
    then
        status=$rc6
    fi
fi

echo "art-result: $rc6 comparison"
exit $status
