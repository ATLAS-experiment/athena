#!/bin/bash
#
# art-description: Reco_tf.py data23 RAWtoALL in MT mode and ComponentAccumulator
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena
# art-athena-mt: 8

timeout 64800 Reco_tf.py --CA \
  --inputBSFile=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/data23/RAW/data23_13p6TeV.00452463.physics_Main.daq.RAW/540events.data23_13p6TeV.00452463.physics_Main.daq.RAW._lb0514._SFO-16._0004.data \
  --outputAODFile="myAOD.pool.root" \
  --outputHISTFile="myHIST.root" \
  --outputDAOD_IDTIDEFile="myDAOD_IDTIDE.pool.root" \
  --outputDRAW_ZMUMUFile="myDRAW_ZMUMU.data" \
  --outputDESDM_MCPFile="myDESDM_MCP.pool.root" \
  --outputDRAW_EGZFile="myDRAW_EGZ.data" \
  --outputDESDM_ALLCELLSFile="myDESDM_ALLCELLS.pool.root" \
  --outputDAOD_L1CALO1File="myDAOD_L1CALO1.pool.root" \
  --outputDESDM_PHOJETFile="myDESDM_PHOJET.pool.root" \
  --multithreaded='True' \
  --preExec 'flags.Exec.FPE=10' \
  --autoConfiguration="everything" \
  --conditionsTag "all:CONDBR2-BLKPA-2023-01" \
  --geometryVersion="all:ATLAS-R3S-2021-03-02-00" \
  --maxEvents='-1'

rc1=$?
echo "art-result: ${rc1} Reco_tf_data23_mt_ca"

# Check for FPEs in the logiles
test_trf_check_fpe.sh
fpeStat=$?

echo "art-result: ${fpeStat} FPEs in logfiles"

files=( myAOD.pool.root myHIST.root myDESDM_MCP.pool.root myDRAW_EGZ.data myDRAW_ZMUMU.data myDAOD_IDTIDE.pool.root myDESDM_ALLCELLS.pool.root myDAOD_L1CALO1.pool.root myDESDM_PHOJET.pool.root )
for i in "${files[@]}"
do
    if [ -f "$i" ]; then
        rcFile=0
    else 
        rcFile=1
    fi
    echo "art-result: ${rcFile} $i exists"
done
