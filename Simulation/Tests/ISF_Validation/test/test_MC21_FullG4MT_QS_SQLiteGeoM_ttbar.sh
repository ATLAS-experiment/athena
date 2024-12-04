#!/bin/sh
#
# art-description: Run simulation using ISF with the FullG4MT_QS simulator, reading ttbar events, building ATLAS-R3S-2021-03-02-00 geometry from SQLite database
# art-include: main/Athena
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-memory: 4999
# art-output: *.pool.root
# art-output: log.*

Sim_tf.py \
    --CA True \
    --multithreaded True \
    --athenaopts "default:--threads=1" \
    --geometrySQLite True \
    --geometrySQLiteFullPath "/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/Geometry/ATLAS-R3S-2021-03-02-00-DEV01.db" \
    --conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-05' \
    --simulator 'FullG4MT_QS' \
    --postInclude 'PyJobTransforms.TransformUtils.UseFrontier' \
    --preInclude 'EVNTtoHITS:Campaigns.MC21Simulation' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
    --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/EVNT/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8453/EVNT.29328277._003902.pool.root.1' \
    --outputHITSFile 'test.SQLiteGeoM.HITS.pool.root' \
    --maxEvents '10' \
    --imf False \
    --detectors Bpipe ID Calo MDT RPC TGC MM sTGC
rc=$?

echo "art-result: $rc Sim_tf"
exit $rc
