#!/bin/sh
#
# art-description: TRTCalibration R-t chain - CA based
# art-type: local
# art-include: main/Athena
# art-include: 24.0/Athena

TRTCalib_accu_tf.py --outputTARFile="TRTCalibAccu_ART_Output" --geometryVersion=ATLAS-R3S-2021-03-02-00 --conditionsTag=CONDBR2-BLKPA-2024-03 --maxEvents=1 --outputFileValidation=False --ignoreErrors=True
ACCU=$?

TRTCalib_merge_tf.py --inputTARFile="TRTCalibAccu_ART_Output" --outputTAR_MERGEDFile="TRTCalibMerge_ART_Output" --outputFileValidation=False --ignoreErrors=True
MERGE=$?

# Sending notifications if test fails
if [ ${ACCU} -ne 0 ] || [ ${MERGE} -ne 0 ]; then
    python -m TRT_CalibAlgs.TRTCalibrationMessage --ACCU ${ACCU} --MERGE ${MERGE}
fi
echo "art-result: ${ACCU} TRT_ACCU"
echo "art-result: ${MERGE} TRT_MERGE"
