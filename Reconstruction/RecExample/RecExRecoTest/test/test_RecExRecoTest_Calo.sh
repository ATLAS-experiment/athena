#!/bin/sh
#
# art-description: Run latest Calorimeter and topocluster reconstruction in Athena. Report issues to https://its.cern.ch/jira/projects/ATLASRECTS/
# art-type: grid
# art-athena-mt: 8
# art-include: main/Athena
# art-include: 24.0/Athena
# art-output: *.log   

# ESD input is not a recommended workflow, but keep this technical check
python -m RecExRecoTest.Calo --threads=8 --evtMax=100 RecExRecoTest.doMC=True RecExRecoTest.doESD=True | tee temp.log
python -m RecExRecoTest.Calo --threads=8 --evtMax=100 RecExRecoTest.doMC=False | tee -a temp.log
echo "art-result: ${PIPESTATUS[0]}"
RecExRecoTest_postProcessing_Errors.sh temp.log