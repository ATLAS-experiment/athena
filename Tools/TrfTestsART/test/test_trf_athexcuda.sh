#!/bin/bash
#
# art-description: athena AthExCUDA/TrackParticleCalibratorExampleConfig.py 
# art-type: grid
# art-include: main/Athena
# art-architecture: '#&nvidia'

echo "----- nvidia-smi -----"
nvidia-smi
echo "----- lscpu -----"
lscpu
echo "----- PATH -----"
echo $PATH
echo "----- LD_LIBRARY_PATH -----"
echo $LD_LIBRARY_PATH
echo "----- athena -----"
athena --CA AthExCUDA/TrackParticleCalibratorExampleConfig.py
rc1=$?
echo "art-result: ${rc1} athena AthExCUDA/TrackParticleCalibratorExampleConfig.py"

# Check for FPEs in the logiles
test_trf_check_fpe.sh
fpeStat=$?

echo "art-result: ${fpeStat} FPEs in logfiles"
