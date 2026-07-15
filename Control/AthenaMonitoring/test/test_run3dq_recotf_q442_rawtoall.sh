#!/bin/bash
# art-description: new DQ in Reco_tf, Run 2 data q442
# art-type: grid
# art-memory: 6144
# art-include: main/Athena
# art-include: 24.0/Athena
# art-output: myHIST.root
# art-output: log*
# art-athena-mt: 3

conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN2_DATA)")

Reco_tf.py --athenaopts='--threads=1' \
--AMI=q442 \
--conditionsTag "$conditions" \
--preExec "all:flags.DQ.Steering.doHLTMon=False" \
--imf False

echo "art-result: $? HIST_Creation"
rm -rf ref-*

ArtPackage=$1
ArtJobName=$2
art.py download ${ArtPackage} ${ArtJobName}
REFFILE=(./ref-*/myHIST.root)
hist_diff.sh myHIST.root $REFFILE -x 'TIME_(execute|convert|prepareROBs)' -i > log.HIST_Diff 2>&1
echo "art-result: $? HIST_Diff"
