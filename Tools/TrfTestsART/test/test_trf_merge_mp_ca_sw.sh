#!/bin/bash
#
# art-description: Merge_tf.py MP, CA and SharedWriter
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena
# art-athena-mt: 8

Merge_tf.py --CA \
  --multiprocess="True" \
  --sharedWriter 'True' \
  --inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/mc16_13TeV.361107.PowhegPythia8EvtGen_AZNLOCTEQ6L1_Zmumu.recon.AOD.e3601_s3126_r12305/AOD.23662571._000001.pool.root.1,/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/mc16_13TeV.361107.PowhegPythia8EvtGen_AZNLOCTEQ6L1_Zmumu.recon.AOD.e3601_s3126_r12305/AOD.23662571._000001.pool.root.1 \
  --preExec 'flags.Exec.FPE=10' \
  --maxEvents 1000 \
  --outputAOD_MRGFile aod.pool.root

rc1=$?
echo "art-result: ${rc1} Merge_tf_mt_ca_sw"

# Check for FPEs in the logiles
test_trf_check_fpe.sh
fpeStat=$?

echo "art-result: ${fpeStat} FPEs in logfiles"
