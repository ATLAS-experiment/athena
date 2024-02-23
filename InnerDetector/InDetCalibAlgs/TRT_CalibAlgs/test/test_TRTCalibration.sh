#!/bin/sh
#
# art-description: TRTCalibration R-t chain - CA based
# art-type: local
# art-include: main/Athena
# art-include: 24.0/Athena

python -m TRT_CalibAlgs.TRTCalibrationMgrConfig

result=$?
echo "art-result: ${result} TRT Calibration ntuple step"
