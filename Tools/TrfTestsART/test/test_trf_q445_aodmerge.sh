#!/bin/bash
#
# art-description: Reco_tf.py q445, AODMerge and output comparison
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena
# art-athena-mt: 8

export ATHENA_CORE_NUMBER=8
conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")
Reco_tf.py \
    --CA \
    --AMI q445 \
    --maxEvents 100 \
    --multithreaded="True" \
    --conditionsTag "${conditions}" \
    --outputAODFile myAOD.MT.pool.root
rc1=$?
AODMerge_tf.py \
    --autoConfiguration 'everything' \
    --inputAODFile myAOD.MT.pool.root \
    --outputAODFile Merged.MT.AOD.pool.root \
    --multithreaded="True" # Note that this doesn't gain us much...
rc2=$?

# Compare EventData in the merged files
rc3=1
rc4=1
if [[ $rc1 -eq 0 ]] && [[ $rc2 -eq 0 ]]; then
  # EventData
  acmd diff-root --exact-branches --nan-equal --order-trees myAOD.MT.pool.root Merged.MT.AOD.pool.root
  rc3=$?
  meta-diff -x diff -s myAOD.MT.pool.root Merged.MT.AOD.pool.root -d file_guid file_size
  rc4=$?
fi

# Print the results
echo "art-result: ${rc1} q445"
echo "art-result: ${rc2} AODMerge"
echo "art-result: ${rc3} Comparison:EventData"
echo "art-result: ${rc4} Comparison:MetaData"
