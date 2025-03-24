#!/bin/bash
#
# art-description: ESDMerge_tf.py mt and CA
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena
# art-athena-mt: 8

conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN2_DATA)")
ESDMerge_tf.py \
    --multithreaded="True" \
    --inputESDFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/DESDM_MCP.26614755._001203.pool.root.1,/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/DESDM_MCP.26614755._001208.pool.root.1 \
    --preExec 'flags.Exec.FPE=10' \
    --autoConfiguration="everything" \
    --conditionsTag="${conditions}" \
    --geometryVersion="all:ATLAS-R2-2016-01-00-01" \
    --runNumber="358031" \
    --outputESD_MRGFile="DESDM_MCP.pool.root" \
    --AMITag="p4756" 

rc1=$?
echo "art-result: ${rc1} ESDMerge_tf_mt_ca"

# Check for FPEs in the logiles
test_trf_check_fpe.sh
fpeStat=$?

echo "art-result: ${fpeStat} FPEs in logfiles"
