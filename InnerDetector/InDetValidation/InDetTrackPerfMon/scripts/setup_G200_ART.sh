#!/bin/bash
## For the moment, this script checks out the G-200 branch and builds.
## Later it should be replaced with any special setup to run from Athena - or maybe just a noop.
## This script can be used to for G-0xx, G-1xx, and G-2xx pipelines.
## Normally, this script should be `source`d so environment variables are inherited by the caller. Error `exit`s will exit the caller.

cmd() {
  echo ++ "$@"
  "$@"
}

# don't rebuild if already built
if [ ! -s "G-200/build/$CMTCONFIG/lib/libEFTrackingCUDA.so" ]; then

#git clone https://:@gitlab.cern.ch:8443/atlas-tdaq-ph2upgrades/atlas-tdaq-eftracking/traccc-integration/G-200.git
## FIXME - temporary, until above repo is public
cmd git clone -b ART https://:@gitlab.cern.ch:8443/maparo/G-200.git
if [ ! -d G-200 ]; then
  echo "Could not clone G-200 repository. Exiting."
  exit 1
fi
if [ -z "$( ls -A G-200 )" ]; then
  echo "Cloned an empty repository. Exiting."
  exit 1
fi

cmd cd G-200
cmd mkdir build
cmd cd build

# nvcc ($CUDACXX) not needed since we have -DATLAS_USE_SYSTEM_TRACCC=ON (Athena,main,r2026-05-29T2100)
if [ -n "$CUDACXX" ]; then
  cmd "$CUDACXX" --version
  # hack for when we have a local CUDA installation
  cmd export CMAKE_PREFIX_PATH="$(dirname "$(dirname "$CUDACXX")")/targets/x86_64-linux:${CMAKE_PREFIX_PATH}"
  # hack to work on CERN-GPU Grid jobs
  cmd unset CUDAToolkit_ROOT
fi
cmd nvidia-smi -L
env > envlog.log
echo "Environment variables:"
grep -e ^CUDA -e ^PANDA_RESOURCE= -e ^ALRB_CONT_PARENTHOSTNAME= -e ^CMAKE_PREFIX_PATH envlog.log

cmd cmake ../traccc-athena -DATLAS_USE_SYSTEM_TRACCC=ON
rc=$?
echo "G-200 cmake result: $rc"
if [ $rc != 0 ]; then exit $rc; fi

cmd make -j4
rc=$?
echo "G-200 make result: $rc"
if [ $rc != 0 ]; then exit $rc; fi

## get the input files and update the config
# don't try to make a variable for the data path unless you really like playing with sed
# the data tarfile contains a whole bunch of directories we don't want, so transform to flat
cmd cd ../..
cmd mkdir ITk_data
cmd cd ITk_data
cmd tar jxvf /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetTrackPerfMon/GPU_EFTracking/GPU.tar.bz2 --transform='s/.*\///'
cmd cd ..

fi   # end of skipped build

cmd source "G-200/build/$CMTCONFIG/setup.sh"
# weird environment fix
cmd export $(grep ^CMAKE_PREFIX_PATH= G-200/build/envlog.log)
