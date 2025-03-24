#!/bin/bash
#
# art-description: Reco_tf.py data18 RAWtoALL in MT mode with RUCIO input dataset processing 50 files with 3 files/job, excuted only on x86_64-centos7-gcc8-opt and once per week on Saturday
# art-type: grid
# art-input: data18_13TeV:data18_13TeV.00357750.physics_Main.daq.RAW
# art-input-nfiles: 300
# art-input-nfilesperjob: 3
# art-include: main/Athena/x86_64-el9-gcc13-opt
# art-athena-mt: 8
# art-runon: Saturday

conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN2_DATA)")
timeout 64800 Reco_tf.py \
  --inputBSFile=${ArtInFile} \
  --outputAODFile=myAOD.pool.root \
  --outputHISTFile=myHIST.root \
  --outputDESDM_MCPFile=myDESDM_MCP.pool.root \
  --outputDRAW_ZMUMUFile=myDRAW_ZMUMU.data \
  --outputDAOD_IDTIDEFile=myIDTIDE.pool.root \
  --multithreaded='True' \
  --preExec 'all:flags.DQ.Steering.doHLTMon=False; flags.Exec.FPE=10;' \
  --autoConfiguration='everything' \
  --conditionsTag "${conditions}" --geometryVersion='default:ATLAS-R2-2016-01-00-01' \
  --runNumber='357750' --maxEvents='-1'

rc1=$?
echo "art-result: ${rc1} Reco_tf_data18_rucio_weekly_mt"

# Check for FPEs in the logiles
test_trf_check_fpe.sh
fpeStat=$?

echo "art-result: ${fpeStat} FPEs in logfiles"
