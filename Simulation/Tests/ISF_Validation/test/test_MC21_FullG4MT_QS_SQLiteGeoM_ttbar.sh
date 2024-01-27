#!/bin/sh
#
# art-description: Run simulation using ISF with the FullG4MT_QS simulator, reading ttbar events, building ATLAS-R3S-2021-03-00-00 geometry from SQLite database
# art-include: 23.0/Athena
# art-include: main/Athena
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-output: *.pool.root
# art-output: log.*

mkdir Geometry
wget -P Geometry https://cernbox.cern.ch/remote.php/dav/public-files/aVAzaQgbvcZkHua/ATLAS-R3S-2021-03-00-00.db
rc=$?
echo "art-result: $rc wget"

rc1=-9999
if [ $rc -eq 0 ]
then
    DATAPATH=.:$DATAPATH Sim_tf.py \
	--CA True \
	--geometrySQLite True \
	--conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-05' \
	--simulator 'FullG4MT_QS' \
	--postInclude 'PyJobTransforms.TransformUtils.UseFrontier' \
	--preInclude 'EVNTtoHITS:Campaigns.MC21Simulation' \
	--geometryVersion 'default:ATLAS-R3S-2021-03-00-00' \
	--inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/EVNT/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8453/EVNT.29328277._003902.pool.root.1' \
	--outputHITSFile 'test.SQLiteGeoM.HITS.pool.root' \
	--maxEvents '10' \
	--imf False \
	--detectors Bpipe ID Calo MDT RPC TGC MM sTGC
    rc1=$?
fi
echo  "art-result: $rc1 Sim_tf"
