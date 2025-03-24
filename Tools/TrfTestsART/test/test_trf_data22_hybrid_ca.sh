#!/bin/bash
#
# art-description: Reco_tf.py data22 RAWtoALL in hybrid MT/MP mode: nprocs=2, threads=4 and ComponentAccumulator
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena
# art-athena-mt: 8

conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_DATA22")
timeout 64800 Reco_tf.py \
  --inputBSFile=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/TCT_Run3/data22_13p6TeV.00431493.physics_Main.daq.RAW._lb0525._SFO-16._0001.data \
  --outputAODFile=myAOD.pool.root \
  --outputHISTFile=myHIST.root \
  --outputDESDM_MCPFile=myDESDM_MCP.pool.root \
  --outputDRAW_ZMUMUFile=myDRAW_ZMUMU.data \
  --outputDRAW_EGZFile=myDRAW_EGZ.data \
  --outputDAOD_IDTIDEFile=myDAOD_IDTIDE.pool.root \
  --athenaopts="--nprocs=2 --threads=4" \
  --preExec 'flags.Exec.FPE=10' \
  --autoConfiguration="everything" \
  --conditionsTag "${conditions}" \
  --geometryVersion="all:ATLAS-R3S-2021-03-00-00" \
  --runNumber="431493" \
  --maxEvents='100'

rc1=$?
echo "art-result: ${rc1} Reco_tf_data22_hybrid"

# Check for FPEs in the logiles
test_trf_check_fpe.sh
fpeStat=$?

echo "art-result: ${fpeStat} FPEs in logfiles"
