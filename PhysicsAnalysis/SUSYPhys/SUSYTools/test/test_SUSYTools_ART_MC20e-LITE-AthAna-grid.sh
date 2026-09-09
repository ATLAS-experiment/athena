#!/bin/sh

# art-description: SUSYTools ART test - share/minimalExampleJobOptions.py
# art-type: grid
# art-include: main/AthAnalysis
# art-output: hist-Ath_mc20e_DAOD_PHYSLITE.root
# art-output: dcube

# Create empty pool file
art.py createpoolfile

echo "Running SUSYTools test: 'python -m SUSYTools.SUSYToolsConfig --evtMax 2000 --testCampaign mc20e --testFormat PHYSLITE'"
python -m SUSYTools.SUSYToolsConfig --evtMax 2000 --testCampaign mc20e --testFormat PHYSLITE
echo  "art-result: $? TEST"

echo "Running DCube post-processing"

tName="mc20e"
dcubeRef=/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev/SUSYTools/ART/References/hist-mc20e_DAOD_PHYSLITE-rel22.root
dcubeXml=/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev/SUSYTools/ART/xml/dcube_config_master_rel22.xml

/cvmfs/atlas.cern.ch/repo/sw/art/dcube/bin/art-dcube ${tName} hist-Ath_${tName}_DAOD_PHYSLITE.root ${dcubeXml} ${dcubeRef}

echo "art-result: $? DCUBE"

echo "Done."
