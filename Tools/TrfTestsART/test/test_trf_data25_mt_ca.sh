#!/bin/bash
#
# art-description: Reco_tf.py data25 RAWtoALL in MT mode and ComponentAccumulator
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena
# art-athena-mt: 8

conditionsTag=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_DATA)")
timeout 64800 Reco_tf.py \
  --inputBSFile=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigP1Test/data25_13p6TeV.00500306.physics_Main.daq.RAW._lb0434._SFO-11._0006.data_150evt \
  --outputAODFile="myAOD.pool.root" \
  --outputHISTFile="myHIST.root" \
  --outputDAOD_IDTIDEFile="myDAOD_IDTIDE.pool.root" \
  --outputDAOD_L1CALO1File="myDAOD_L1CALO1.pool.root" \
  --outputDESDM_MCPFile="myDESDM_MCP.pool.root" \
  --outputDESDM_EXOTHIPFile="myDESDM_EXOTHIP.pool.root" \
  --outputDRAW_EGZFile="myDRAW_EGZ.data" \
  --outputDRAW_TAULHFile="myDRAW_TAULH.data" \
  --outputDRAW_ZMUMUFile="myDRAW_ZMUMU.data" \
  --multithreaded='True' \
  --preExec 'flags.Exec.FPE=10' \
  --autoConfiguration="everything" \
  --conditionsTag "all:${conditionsTag}" \
  --geometryVersion="all:ATLAS-R3S-2021-03-02-00" \
  --maxEvents='-1'

rc1=$?
echo "art-result: ${rc1} Reco_tf_data25_mt_ca"

# Check for FPEs in the logiles
test_trf_check_fpe.sh
fpeStat=$?

echo "art-result: ${fpeStat} FPEs in logfiles"

files=( myAOD.pool.root myHIST.root myDAOD_IDTIDE.pool.root myDAOD_L1CALO1.pool.root myDESDM_MCP.pool.root myDESDM_EXOTHIP.pool.root myDRAW_EGZ.data myDRAW_TAULH.data myDRAW_ZMUMU.data )
for i in "${files[@]}"
do
    if [ -f "$i" ]; then
        rcFile=0
    else 
        rcFile=1
    fi
    echo "art-result: ${rcFile} $i exists"
done
