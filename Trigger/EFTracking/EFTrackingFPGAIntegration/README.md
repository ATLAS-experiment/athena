# EFTracking Athena Integration for FPGA-based Accelerator Card
This repository hosts codes for the EFTracking 2nd demonstrator (and beyond) Athena integration targeting FPGA-based accelerator card. Each kernel has its own class (algorithm) and python steering script. Please check `src` and `python` for details.

Users are encouraged to use the main branch from upstream Athena. Developers are encouraged to do MR as frequently as possible. In case of questions or specific needs, please contact @zhcui, @yuchou, @sabidi

## Getting Codes
There are two ways to get the codes
1. Sparse checkout
2. Full checkout

Considering that the integration might modify more than one package, a full checkout is recommended to ease the setup. If you prefer to perform a sparse checkout, you can follow the ATLAS git tutorial [here](https://atlassoftwaredocs.web.cern.ch/gittutorial/git-clone/). Only the full checkout instructions are provided here.

### Full checkout
If you are familiar with `Athena` and `git`, you don't have to follow this checkout instructions. Otherwise, you can follow the instructions below

**Case 1: You don't have a local athena copy**

Pick one of the options below
```bash
# Option 1: use krb5 authentication
# Need to do kinit for each new terminal shell for git remote operation
kinit yourUsername@CERN.CH 
git clone https://:@gitlab.cern.ch:8443/atlas/athena.git
git remote rename origin upstream
git pull upstream main

# Options 2: use your ssh key
git clone ssh://git@gitlab.cern.ch:7999/atlas/athena.git
git remote rename origin upstream
git pull upstream main
```

**Case 2: You have a local athena copy**
```bash
cd athena
# Option 1: use krb5 authentication
# Need to do kinit for each new terminal shell for git remote operation
kinit yourUsername@CERN.CH
git remote add upstream https://:@gitlab.cern.ch:8443/atlas/athena.git
git switch main
git pull upstream main


# Option 2: use your ssh key
git remote add upstream ssh://git@gitlab.cern.ch:7999/atlas/athena.git
git switch main
git pull upstream main
```

### Updating codes
Once the repository is setup, you can use git pull (fetch and merge) to update the codes
```bash
git pull upstream main
```

## Compiling the codes
### Getting Docker image
To compile and run this package, you must have AlmaLinux9 + XRT 2022.2. If you don't have this environment ready by default, you can use the docker image `atlas-alma9-xrt` from [GitLab registry](https://gitlab.cern.ch/atlas-tdaq-ph2upgrades/atlas-tdaq-eftracking/eftracking_fpga_dev/fpga/xilinx/container_registry/18381) or from @zhcui's [DockerHub](https://hub.docker.com/r/maxwellcui/athenaxrt/tags)

```bash
# Image from gitlab registry
docker login gitlab-registry.cern.ch/atlas-tdaq-ph2upgrades/atlas-tdaq-eftracking/eftracking_fpga_dev

docker pull docker pull gitlab-registry.cern.ch/atlas-tdaq-ph2upgrades/atlas-tdaq-eftracking/eftracking_fpga_dev/fpga/xilinx:atlas-alma9-xrt

# Image from @zhcui's DockerHub
docker pull maxwellcui/athenaxrt:2022.2
```

### Launching container enviroment
Use the following script to launch the image with Docker
```bash
xclmgmt_driver="$(find /dev -name xclmgmt\*)"
docker_devices=""
for i in ${xclmgmt_driver} ;
do
  docker_devices+="--device=$i "
done

render_driver="$(find /dev/dri -name renderD\*)"
for i in ${render_driver} ;
do
  docker_devices+="--device=$i "
done

docker run \
        -it \
        $docker_devices \
        --rm \
        --net host \
        -v /cvmfs:/cvmfs:shared \
        -v /dev/shm:/dev/shm \
        -v /opt/xilinx/platforms:/opt/xilinx/platforms \
        -v /opt/xilinx/firmware:/opt/xilinx/firmware \
        -v /lib/firmware/xilinx:/lib/firmware/xilinx \
        -v /tools/Xilinx/:/tools/Xilinx/ \
        maxwellcui/athenaxrt:2022.2
```
For singularity or apptainer:
```bash
# /dev/xclmgmt49408 should be replaced by the device id on your machine or can be ignored if you are running in a software only mode, i.e. not running on the accelerator card
# Use find /dev -name xclmgmt\* to find the id
singularity run --bind /dev/xclmgmt49408,/cvmfs,$PWD docker://maxwellcui/athenaxrt:2022.2
# Or
apptainer run --bind /dev/xclmgmt49408,/cvmfs,$PWD docker://maxwellcui/athenaxrt:2022.2
```

### Structure setup and compilation
Once the enviroment is ready, go to the directory that includes the `athena`
```bash
mkdir build run
# Now, ls should show athena, build, run
echo $'+ Trigger/EFTracking/EFTrackingFPGAIntegration\n- .*' > package_filter_EFT.txt
cd build
setupATLAS
asetup Athena,main,latest
cmake -DATLAS_PACKAGE_FILTER_FILE=../package_filter_EFT.txt ../athena/Projects/WorkDir/
make -j20
source x86_64-el9-gcc13-opt/setup.sh 
```

### Running algorithm
Modify the python script in `python/` directory and run from run directory. DataPrep as an example
```bash
cd ../run
python -m EFTrackingFPGAIntegration.DataPrepConfig
```

### TL;DR Run the full ITk Pass-though Chain 
```bash 
# Spin up container
singularity run --bind /cvmfs,$PWD docker://maxwellcui/athenaxrt:2022.2
# Move to build directory
cd build
# Setup Athena
asetup Athena,main,latest
# CMake 
cmake -DATLAS_PACKAGE_FILTER_FILE=../package_filter_EFT.txt ../athena/Projects/WorkDir/
# Build
make -j20
source x*/setup.sh

cd ../run 
# Run Reco_tf with fpgaPassThroughValidation
Reco_tf.py \
  --CA 'all:True' \
  --maxEvents '100' \
  --perfmon 'fullmonmt' \
  --multithreaded 'True' \
  --autoConfiguration 'everything' \
  --conditionsTag 'all:OFLCOND-MC21-SDR-RUN4-02' \
  --geometryVersion 'all:ATLAS-P2-RUN4-03-00-00' \
  --postInclude 'all:PyJobTransforms.UseFrontier' \
  --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,TrkConfig.InDetFPGATrackingFlags.fpgaPassThroughValidation" \
  --steering 'doRAWtoALL' \
  --preExec 'flags.Acts.doMonitoring=True;' \
  --inputRDOFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/*' \
  --outputAODFile 'myAOD.pool.root' \
  --jobNumber '1' \
  --ignorePatterns 'ActsTrackFindingAlg.+ERROR.+Propagation.+reached.+the.+step.+count.+limit,ActsTrackFindingAlg.+ERROR.+Propagation.+failed:.+PropagatorError:..+Propagation.+reached.+the.+configured.+maximum.+number.+of.+steps.+with.+the.+initial.+parameters,ActsTrackFindingAlg.Acts.+ERROR.+CombinatorialKalmanFilter.+failed:.+CombinatorialKalmanFilterError:5.+Propagation.+reaches.+max.+steps.+before.+track.+finding.+is.+finished.+with.+the.+initial.+parameters,ActsTrackFindingAlg.Acts.+ERROR.+SurfaceError:1'