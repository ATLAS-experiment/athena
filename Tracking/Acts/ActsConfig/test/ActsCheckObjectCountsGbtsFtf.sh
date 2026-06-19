#!/usr/bin/bash
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# ttbar mu=200 input
input_rdo=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RDO_RUN4[0])")
conditions_tag=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")
n_events=5
log_file="reco.log"

ignore_pattern=""

export ATHENA_CORE_NUMBER=1
Reco_tf.py \
  --preExec "flags.Exec.FPE=-1; \
             from ActsConfig.ActsConfigFlags import SeedingStrategy; \
             flags.Acts.SeedingStrategy=SeedingStrategy.GbtsFtf; \
             flags.Tracking.doTruth=False; \
             flags.Tracking.doPixelDigitalClustering=True; \
             from ActsConfig.ActsConfigFlags import PixelCalibrationStrategy; \
             flags.Acts.PixelCalibrationStrategy=PixelCalibrationStrategy.Uncalibrated;" \
  --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsWorkflowFlags" \
  --conditionsTag ${conditions_tag} \
  --ignorePatterns "${ignore_pattern}" \
  --inputRDOFile ${input_rdo} \
  --outputAODFile AOD.pool.root \
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
