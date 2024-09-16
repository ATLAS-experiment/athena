#!/bin/sh

# art-description: SUSYTools ART test - TestSUSYToolsAlg.py
# art-type: grid
# art-include: main/AnalysisBase
# art-output: hist-MC23dDAOD_PHYSLITE.root
# art-output: dcube

# Create empty pool file
art.py createpoolfile

echo "Running SUSYTools test: \'TestSUSYToolsAlg.py -f PHYSLITE -t mc23d  -m 2000 --dosyst'"
TestSUSYToolsAlg.py -f PHYSLITE -t mc23d  -m 2000 --dosyst 
echo  "art-result: $? TEST"

mv submitDir/hist-*.root ./hist-mc23d_DAOD_PHYSLITE.root

echo "Running DCube post-processing"

tName="mc23d"
dcubeRef=/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev/SUSYTools/ART/References/hist-mc23d_DAOD_PHYSLITE-rel22.root
dcubeXml=/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev/SUSYTools/ART/xml/dcube_config_master_rel22_Run3.xml

/cvmfs/atlas.cern.ch/repo/sw/art/dcube/bin/art-dcube ${tName} hist-mc23d_DAOD_PHYSLITE.root ${dcubeXml} ${dcubeRef}

echo "art-result: $? DCUBE"

echo "Done."
