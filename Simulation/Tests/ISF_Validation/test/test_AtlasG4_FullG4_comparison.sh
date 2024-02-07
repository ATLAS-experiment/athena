#!/bin/sh
#
# art-description: MC16-style simulation comparison of FullG4 and AtlasG4
# art-type: build
# art-include: 21.0/Athena
# art-include: 21.0/AthSimulation
# art-include: 21.3/Athena
# art-include: 21.9/Athena
# art-include: main/Athena
# art-include: main/AthSimulation

# MC16 setup
# ATLAS-R2-2016-01-00-01 and OFLCOND-MC16-SDR-14

export TRF_ECHO=1
Sim_tf.py \
    --CA \
    --conditionsTag 'default:OFLCOND-MC16-SDR-14' \
    --simulator 'FullG4MT_QS' \
    --postInclude 'default:PyJobTransforms.UseFrontier' \
    --preInclude 'EVNTtoHITS:Campaigns.MC23SimulationSingleIoV' \
    --geometryVersion 'default:ATLAS-R2-2016-01-00-01' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.evgen.EVNT.e4993.EVNT.08166201._000012.pool.root.1" \
    --outputHITSFile "HITS.FullG4.pool.root" \
    --maxEvents 2 \
    --imf False

rc1=$?
echo  "art-result: $rc1 simulation FullG4"

AtlasG4_tf.py \
    --CA \
    --conditionsTag 'default:OFLCOND-MC16-SDR-14' \
    --postInclude 'default:PyJobTransforms.UseFrontier' \
    --preInclude 'sim:Campaigns.MC23SimulationSingleIoV' \
    --geometryVersion 'default:ATLAS-R2-2016-01-00-01' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.evgen.EVNT.e4993.EVNT.08166201._000012.pool.root.1" \
    --outputHITSFile "HITS.AtlasG4.pool.root" \
    --maxEvents 2 \
    --imf False

rc2=$?
echo  "art-result: $rc2 simulation AtlasG4"

rc3=-999
if [ $rc1 -eq 0 ]
then
  if [ $rc2 -eq 0 ]
  then
    # Compare the merged outputs
    acmd.py diff-root HITS.FullG4.pool.root HITS.AtlasG4.pool.root --error-mode resilient --ignore-leaves TrackRecordCollection_p2_CaloEntryLayer TrackRecordCollection_p2_MuonEntryLayer TrackRecordCollection_p2_MuonExitLayer index_ref
    rc3=$?
  fi
fi
echo "art-result: $rc3 comparison"

