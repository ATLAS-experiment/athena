#!/bin/sh
#
# art-description: Testing LAr ElecCalib Chain I
# art-type: build
# art-include: 23.0/Athena
# art-include: 24.0/Athena
# art-include: main/Athena
 
athena --CA LArCalibProcessing/LArCalib_PedestalAutoCorrConfig.py 
echo  "art-result: $? pedestal"

athena --CA LArCalibProcessing/LArCalib_Delay_OFCCaliConfig.py
echo  "art-result: $? delay"

athena --CA LArCalibProcessing/LArCalib_RampConfig.py 

echo  "art-result: $? ramp"
