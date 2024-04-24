#!/bin/bash
#
# art-description: athena AthExCUDA/TrackParticleCalibratorExampleConfig.py 
# art-type: grid
# art-include: main/Athena
# art-architecture: '#&nvidia'

# Print out some environment
echo "----- nvidia-smi -----"
nvidia-smi
echo "----- lscpu -----"
lscpu
echo "----- PATH -----"
echo $PATH
echo "----- LD_LIBRARY_PATH -----"
echo $LD_LIBRARY_PATH

# Choose GPU with lowest utilization
export CUDA_VISIBLE_DEVICES=$(nvidia-smi --query-gpu=memory.free,index --format=csv,nounits,noheader | sort -nr | head -1 | awk '{ print $NF }')
echo "GPU with lowest utilization: $CUDA_VISIBLE_DEVICES"

# Run athena
echo "----- athena -----"
athena --CA AthExCUDA/TrackParticleCalibratorExampleConfig.py
rc1=$?
echo "art-result: ${rc1} athena AthExCUDA/TrackParticleCalibratorExampleConfig.py"

# Check for FPEs in the logiles
test_trf_check_fpe.sh
fpeStat=$?

echo "art-result: ${fpeStat} FPEs in logfiles"
