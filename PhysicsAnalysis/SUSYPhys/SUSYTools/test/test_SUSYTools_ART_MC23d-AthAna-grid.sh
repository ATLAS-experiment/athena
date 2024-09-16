#!/bin/sh

# art-description: SUSYTools ART test - share/minimalExampleJobOptions.py
# art-type: grid
# art-include: main/AthAnalysis
# art-output: hist-Ath_mc23d_DAOD_PHYS.root
# art-output: dcube

# Create empty pool file
art.py createpoolfile

echo "Running SUSYTools test: 'share/athena SUSYTools/jobOptions.py - --testCampaign mc23d'"
athena SUSYTools/jobOptions.py --evtMax 2000 - --testCampaign mc23d
echo  "art-result: $? TEST"

echo "Running DCube post-processing"

tName="mc23d"
dcubeRef=/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev/SUSYTools/ART/References/hist-mc23d_DAOD_PHYS-rel22.root
dcubeXml=/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev/SUSYTools/ART/xml/dcube_config_master_rel22_Run3.xml

/cvmfs/atlas.cern.ch/repo/sw/art/dcube/bin/art-dcube ${tName} hist-Ath_${tName}_DAOD_PHYS.root ${dcubeXml} ${dcubeRef}

echo "art-result: $? DCUBE"

echo "Done."
