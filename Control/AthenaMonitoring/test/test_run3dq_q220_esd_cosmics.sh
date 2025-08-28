#!/bin/bash
# art-description: ESD->HIST, R22 Run 2 cosmics data ESD
# art-type: grid
# art-memory: 3072
# art-include: main/Athena
# art-include: 24.0/Athena
# art-output: ExampleMonitorOutput.root
# art-output: myHIST.root
# art-output: log*
# art-athena-mt: 8

Reco_tf.py \
--CA \
--AMI=q220 \
--conditionsTag 'all:CONDBR2-BLKPA-RUN2-11' \
--athenaopts='--threads=8' \
--preExec='pass' \
--maxEvents=100 \
--outputESDFile=myESD.pool.root \
--outputHISTFile=myHIST.root --imf False
echo "art-result: $? Reco_tf"

Run3DQTestingDriver.py --inputFiles=myESD.pool.root DQ.Steering.doHLTMon=False > log.HIST_Creation 2>&1

echo "art-result: $? HIST_Creation"

ArtPackage=$1
ArtJobName=$2
art.py download ${ArtPackage} ${ArtJobName}

REFFILE=(./ref-*/myHIST.root)
hist_diff.sh myHIST.root $REFFILE -x "(TIME_(execute|convert|prepareROBs)|TotalEnergyVsEtaPhi.*_CSCveto)" -i > log.HIST_Diff_direct 2>&1
echo "art-result: $? HIST_Diff_direct"

REFFILE=(./ref-*/ExampleMonitorOutput.root)
hist_diff.sh ExampleMonitorOutput.root $REFFILE -x "(TIME_(execute|convert|prepareROBs)|TotalEnergyVsEtaPhi.*_CSCveto)" -i > log.HIST_Diff 2>&1
echo "art-result: $? HIST_Diff"
