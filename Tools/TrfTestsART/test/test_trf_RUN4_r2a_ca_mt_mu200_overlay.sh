#!/bin/bash
#
# art-description: Reco_tf.py Run 4, mu=200, Overlay,RAWtoALL in MT mode
# art-type: grid
# art-include: main/Athena
# art-athena-mt: 8

if [ -z ${ATLAS_REFERENCE_DATA+x} ]; then
  ATLAS_REFERENCE_DATA="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art"
fi

HSHitsFile=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.HITS_RUN4[0])")
RDOFile=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RDO_BKG_RUN4[0])")

conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")

Reco_tf.py \
  --conditionsTag "${conditions}" \
  --steering "doOverlay" "doRAWtoALL" \
  --preInclude "all:Campaigns.MC23PhaseIIPileUp200" \
  --postInclude "all:PyJobTransforms.UseFrontier.py" \
  --inputHITSFile "${HSHitsFile}" \
  --inputRDO_BKGFile "$RDOFile" \
  --outputAODFile RUN4.AOD.pool.root \
  --imf="False" \
  --maxEvents 25

rc1=$?
echo "art-result: ${rc1} Reco_tf_RUN4_r2a_mt"

# Check for FPEs in the logiles
test_trf_check_fpe.sh
fpeStat=$?

echo "art-result: ${fpeStat} FPEs in logfiles"
