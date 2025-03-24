#!/bin/bash
#
# art-description: Reco_tf.py data15 RAWtoALL in MT mode
# art-type: grid
# art-include: main/Athena

# art-include: 24.0/Athena
# art-athena-mt: 8

conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN2_DATA)")
timeout 64800 Reco_tf.py \
  --inputBSFile=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/data15_13TeV.00276689.physics_Main.daq.RAW._lb0220._SFO-1._0001.data \
  --outputAODFile=myAOD.pool.root \
  --outputHISTFile=myHIST.root \
  --outputDESDM_MCPFile=myDESDM_MCP.pool.root \
  --outputDRAW_ZMUMUFile=myDRAW_ZMUMU.data \
  --outputDAOD_IDTIDEFile=myIDTIDE.pool.root \
  --multithreaded='True' \
  --preExec 'all:flags.DQ.Steering.doHLTMon=False; flags.Exec.FPE=10;' \
  --autoConfiguration='everything' \
  --conditionsTag "${conditions}" --geometryVersion='default:ATLAS-R2-2016-01-00-01' \
  --runNumber='276689' --maxEvents='-1'

rc1=$?
echo "art-result: ${rc1} Reco_tf_data15_mt"

# Check for FPEs in the logiles
test_trf_check_fpe.sh
fpeStat=$?

echo "art-result: ${fpeStat} FPEs in logfiles"
