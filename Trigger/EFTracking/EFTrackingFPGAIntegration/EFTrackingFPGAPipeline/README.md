# EFTracking Athena Integration for FPGA-based Accelerator Card
This repository hosts codes for the EFTracking Athena integration targeting FPGA-based accelerator card.

Users are encouraged to use the main branch from upstream Athena. Developers are encouraged to do MR as frequently as possible. In case of questions or specific needs, please contact @zhcui, @yuchou, @sabidi, @llewitt

## Getting Codes
There are two ways to get the codes
1. Sparse checkout
2. Full checkout

Considering that the integration might modify more than one package, a full checkout is recommended to ease the setup. If you prefer to perform a sparse checkout, you can follow the ATLAS git tutorial [here](https://atlas-software.docs.cern.ch/athena/git/). Only the full checkout instructions are provided here.

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
For users, once the repository is setup, you can use git pull (fetch and merge) to update the codes to the latest in upstream main
```bash
git pull upstream main
```

## Compiling the codes
To compile and run this package, you must have AlmaLinux9 + XRT 2024.1 (or above). 

> [!important]
> If you are running on the EFTracking Testbed, please apply a git patch before compiling
```bash
# This needs to be done on EFTracking FPGA testbed to use the right OpenCL library
cd athena
git apply /scratch/medium/sabidi/testbed-cmake.patch 
```

### Structure setup and compilation
Once the enviroment is ready, go to the directory that includes the `athena`
```bash
mkdir build run
# Now, ls should show athena, build, run
echo $'+ Trigger/EFTracking/EFTrackingFPGAIntegration/.*\n+ Control/AthXRT/AthXRTInterfaces\n+ Control/AthXRT/AthXRTServices\n- .*' > package_filter_EFT.txt
cd build
export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase
alias setupATLAS='source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh'
setupATLAS
source /opt/xilinx/xrt/setup.sh ; source /opt/xilinx/Vitis/2024.1/settings64.sh # this is on the testbed, otherwise setup your own xrt and xilinx
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

### Running the benchmark algorithm
Once the pakcage is built successfully, modify the file `python/BenchmarkConfig.py` to include the correct device BDF id, xclbin path, kernel names, and the input RDO file you want to use.
> [!important]
> When running on the testbed, please book a slot on the confluence calendar. Specify the device BDF id on both the calendar and in the python script so that people don't try to use the same accelerator at the same time. Use `xrt-smi examine` to get a list of available devices and their BDF ID.

To run
```
python -m EFTrackingFPGAPipeline.BenchmarkConfig FPGADataPrep.DoActs=True FPGADataPrep.RunPassThrough=False FPGADataPrep.DoEmulation=False
```
If `FPGADataPrep.DoActs` is set to False, the algorithm will stop at the cluster level, otherwise it runs the ACTS spacepoint formation, seeding, and tracking. If `RunPassThrough` is set to True, only the EDMPrep kernel will be executed on the FPGA. If `DoEmulation` is set to True, the code will assume you are trying to run software or hardware emulation kernels.

The benchmark algorithm produces two files `FPGA.Benchmark.AOD.pool.root` and `FPGAOutputValidation.root`. The first one include clusters and track particles (if ACTS enabled). This file can then be used to create IDTPM plots. The second one includes cluster-level monitoring histograms.

The benchmark algorithm prints a decent amount of timing information on the screen. The timing strategy is preliminary and under discussion. Please be extra careful on quoting any of them. If in doubt please reach out to @zhcui.

To get a vitis run summary, please make sure you have a copy of the file `script/xrt.ini` locates at the directory where you launch the job or export the environmental variable `export XRT_INI_PATH=/path-to/xrt.ini`. After the algorithm is done, you will see a file named `xrt.run_summary` in the directory.
The run summary file can then be opened by `vitis_analyzer`. You may see that the device compute unit trace has no information. This means that the xclbin file were linked without profiling enabled. Add the following lines in the config file for linkage stage
```
# u250.cfg
[profile]
data=all:all:all
```

### Running IDTPM
Use the script `script/run_FPGADataPrep_IDTPM.sh`. You can adjust the path to the IDTPM json file and the output prefix inside of the script. To run
```
run_FPGADataPrep_IDTPM.sh FPGA.Benchmark.AOD.pool.root
```
The output of this is a file named `IDTPM.DataPrep.HIST.root`, which includes tracks from FPGA+ACTS and offline.

## For Running F6X0 integration 
Follow the same instruction to checkout and compile the code. The instruction to run are
```
MAPS_5L=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/ATLAS-P2-RUN4-03-00-00/maps_5L/InsideOut/v0.22/
BANKS_5L=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/ATLAS-P2-RUN4-03-00-00/banks_5L/v0.22/     

python -m EFTrackingFPGAPipeline.F600IntegrationConfig \
        Trigger.FPGATrackSim.mapsDir=${MAPS_5L} \
        Trigger.FPGATrackSim.bankDir=${BANKS_5L} \
        Trigger.FPGATrackSim.region=34 \
        Trigger.FPGATrackSim.pipeline='F-600' \
        Trigger.FPGATrackSim.tracking=True \
        Trigger.FPGATrackSim.doEDMConversion=False \
        Trigger.FPGATrackSim.doOverlapRemoval=False \
        Trigger.FPGATrackSim.Hough.secondStage=False \
        Trigger.FPGATrackSim.writeToAOD=False \
        Trigger.FPGATrackSim.writeAdditionalOutputData=False \
        Trigger.FPGATrackSim.outputMonitorFile="monitoring_F600.root" \
        Output.AODFileName=aod.root \
        --evtMax 20
```

### TL;DR Run the full ITk Pass-though Chain
<details>
<summary>click to show legacy instructions</summary>

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
  --ignorePatterns ''
```
</details>

## Old container instruction
You should avoid using this because the XRT in this image is out-dated!
<details>
  <summary>click to show</summary>

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
</details>
