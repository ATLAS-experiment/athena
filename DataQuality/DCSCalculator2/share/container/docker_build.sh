#!/bin/bash

echo $0
SCRIPTDIR=$(dirname $(realpath $0))
mkdir docker_build
cd docker_build
SHAREDIR=$SCRIPTDIR/..
cp -dpr $SHAREDIR/run .
cp -dpr $SHAREDIR/container .
mkdir build
if [ -n "$WorkDir_DIR" ] ; then
  cp -dpLr $WorkDir_DIR build
fi

docker build --push -t registry.cern.ch/atlas-dqm-core/dcscalculator:latest -f container/Dockerfile .

cd ..
rm -rf docker_build