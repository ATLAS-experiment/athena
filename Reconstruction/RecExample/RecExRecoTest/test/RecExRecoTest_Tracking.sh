#!/bin/sh
#
# art-description: Run latest Track reconstruction in Athena. Report issues to https://its.cern.ch/jira/projects/ATLASRECTS/
# art-type: grid
# art-athena-mt: 8
# art-include: main/Athena
# art-include: 24.0/Athena
# art-output: *.log   

python -m RecExRecoTest.Tracking --threads=8 --evtMax=40 RecExRecoTest.doMC=True RecExRecoTest.doESD=True | tee temp.log
python -m RecExRecoTest.Tracking --threads=8 --evtMax=40 RecExRecoTest.doMC=False | tee -a temp.log
echo "art-result: ${PIPESTATUS[0]}"
RecExRecoTest_postProcessing_Errors.sh temp.log