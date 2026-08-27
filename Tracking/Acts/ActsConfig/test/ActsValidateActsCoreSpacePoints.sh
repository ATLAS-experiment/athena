#!/usr/bin/bash
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Same setup as ActsCheckObjectCounts.sh, but with Acts::StripSpacePointBuilder
# used for strip space point formation, so the two count dumps can be compared.
# Pixels are left on the Athena implementation, see Acts.PixelSpacePointStrategy.

# ttbar mu=200 input
input_rdo=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RDO_RUN4[0])")
conditions_tag=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")
n_events=5
log_file="reco.log"

ignore_pattern=""

export ATHENA_CORE_NUMBER=1
Reco_tf.py \
  --preExec "flags.Exec.FPE=-1; \
       flags.Detector.EnableCalo=True; \
       flags.Detector.EnableHGTD=True; \
       flags.Acts.doLowPt=True;" \
       'from ActsConfig.ActsConfigFlags import SpacePointStrategy; flags.Acts.SpacePointStrategy=SpacePointStrategy.ActsCore' \
  --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsProductionFlags" \
  --ignorePatterns "${ignore_pattern}" \
  --conditionsTag ${conditions_tag} \
  --inputRDOFile ${input_rdo} \
  --outputAODFile AOD.validateCoreSpacePoints.pool.root \
  --maxEvents ${n_events} \
  --multithreaded > ${log_file} 2>&1

rc=$?
if [ $rc != 0 ]; then
    echo ">>>>>>>>>>>>>>>> Reconstruction step just failed:"
    cat ${log_file}
    echo ">>>>>>>>>>>>>>>> here is the full log (log.RAWtoALL):"
    cat log.RAWtoALL
    exit $rc
fi

ParseActsStatDump.py --inputFile log.RAWtoALL
