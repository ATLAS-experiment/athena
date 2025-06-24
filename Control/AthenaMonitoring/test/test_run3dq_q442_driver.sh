#!/bin/bash
# art-description: AOD->HIST, R22 Run 2 data AOD/ESD
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena
# art-output: ExampleMonitorOutput_AOD.root
# art-output: ExampleMonitorOutput_ESD.root
# art-output: log*

art.py download Tier0ChainTests test_q442.sh
AODFILE=(./ref-*/AOD.pool.root)
Run3DQTestingDriver.py --inputFiles=${AODFILE} DQ.Environment=AOD DQ.Steering.doHLTMon=False > log.HIST_Creation_AOD 2>&1
echo "art-result: $? HIST_Creation_AOD"
mv ExampleMonitorOutput.root ExampleMonitorOutput_AOD.root

ESDFILE=(./ref-*/ESD.pool.root)
Run3DQTestingDriver.py --inputFiles=${ESDFILE} DQ.Steering.doHLTMon=False > log.HIST_Creation_ESD 2>&1
echo "art-result: $? HIST_Creation_ESD"
mv ExampleMonitorOutput.root ExampleMonitorOutput_ESD.root

rm -rf ref-*

ArtPackage=$1
ArtJobName=$2
art.py download ${ArtPackage} ${ArtJobName}
REFFILE=(./ref-*/ExampleMonitorOutput_AOD.root)
hist_diff.sh ExampleMonitorOutput_AOD.root $REFFILE -x 'TIME_(execute|convert|prepareROBs)' -i > log.HIST_Diff_AOD 2>&1
echo "art-result: $? HIST_Diff_AOD"

REFFILE=(./ref-*/ExampleMonitorOutput_ESD.root)
hist_diff.sh ExampleMonitorOutput_ESD.root $REFFILE -x "(TIME_(execute|convert|prepareROBs)|TotalEnergyVsEtaPhi.*_CSCveto|CandidateMNBFebs)" -i > log.HIST_Diff_ESD 2>&1
echo "art-result: $? HIST_Diff_ESD"

rm -rf ref-*