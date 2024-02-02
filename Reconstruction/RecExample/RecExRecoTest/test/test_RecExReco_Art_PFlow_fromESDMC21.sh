#!/bin/sh
#
# art-description: Athena (serial mode only) runs particle flow reconstruction, using the new Run 3 job configuration, from an ESD file
# art-type: grid
# art-athena-mt: 8
# art-include: main/Athena
# art-include: 23.0/Athena
# art-output: *.log   

python -m eflowRec.PFRun3Config | tee temp.log
echo "art-result: ${PIPESTATUS[0]}"
test_postProcessing_Errors.sh temp.log

