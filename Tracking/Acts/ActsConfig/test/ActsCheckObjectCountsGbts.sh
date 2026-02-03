#!/usr/bin/bash
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# ttbar mu=200 input
input_rdo=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/RDO.33629020._000047.pool.root.1
n_events=5
log_file="reco.log"

ignore_pattern=""

export ATHENA_CORE_NUMBER=1
Reco_tf.py \
  --preExec "flags.Exec.FPE=-1; \
             from ActsConfig.ActsConfigFlags import SeedingStrategy; \
             flags.Acts.SeedingStrategy=SeedingStrategy.Gbts2; \
             flags.Tracking.doTruth=False; \
             flags.Tracking.doPixelDigitalClustering=True; \
             from ActsConfig.ActsConfigFlags import PixelCalibrationStrategy; \
             flags.Acts.PixelCalibrationStrategy=PixelCalibrationStrategy.Uncalibrated;" \
  --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsWorkflowFlags" \
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
