#!/bin/sh
#
# art-description: TRTCalibration R-t chain - CA based
# art-type: local
# art-include: main/Athena
# art-include: 24.0/Athena

python -m TRT_CalibAlgs.TRTCalibrationMgrConfig
result=$?

# Sending notifications if test fails
if [ ${result} -ne 0 ]; then
    python -m TRT_CalibAlgs.TRTCalibrationMessage --jobStatus ${result}
fi
echo "art-result: ${result} TRT CalibWorkflow"
