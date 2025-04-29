#!/bin/bash

echo "Setting up the TrigMenuRulebook"

export ATLAS_LOCAL_ROOT_BASE="/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase"
source $ATLAS_LOCAL_ROOT_BASE/packageSetups/localSetup.sh git

klist -l | grep 'CERN\.CH' > /dev/null
if [ $? -ne 0 ]; then
  echo "ERROR no Kerberos token found, cannot check out rulebook"
  exit 1
fi

git clone https://:@gitlab.cern.ch:8443/atlas-trigger-menu/TrigMenuRulebook.git -o upstream

if [ $? -ne 0 ]; then
  echo "ERROR git clone failed"
  exit 1
fi

TEST_DIR=${PWD}
BUILD_DIR=${TEST_DIR}/build_rb_test/

mkdir ${BUILD_DIR}
cd ${BUILD_DIR}
cmake ../TrigMenuRulebook
make
source */setup.sh
# Return to test working dir
cd ${TEST_DIR}

# input file path, menu, release
update_inputs.sh ${TEST_DIR} ${AtlasVersion} PhysicsP1_pp_run3_v1

totalL1PhysP1json=`ls L1*_PhysicsP1_pp_run3_v1_*.json | wc -l`
totalHLTPhysP1json=`ls HLT*_PhysicsP1_pp_run3_v1_*.json | wc -l`

if [ $totalL1PhysP1json -gt 2 ] || [ $totalHLTPhysP1json -gt 3 ]; then
    echo "ERROR More JSON files than needed"
    exit 1
elif [ $totalL1PhysP1json -lt 2 ] || [ $totalHLTPhysP1json -lt 3 ]; then
    echo "ERROR Less JSON files than needed"
    exit 1
fi

exit 0
