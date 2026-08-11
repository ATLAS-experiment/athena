#!/usr/bin/bash
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# NN pixel cluster calibration applied to the measurements selected by the CKF
# (NNClustering == calibrate after measurement selection)

# ttbar mu=200 input
input_rdo=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RDO_RUN4[0])")
conditions_tag=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")
n_events=1

export ATHENA_CORE_NUMBER=1
Reco_tf.py \
  --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsProductionFlags" \
  --preExec 'flags.Exec.FPE=-1; \
             from ActsConfig.ActsConfigFlags import PixelCalibrationStrategy; \
             flags.Acts.PixelCalibrationStrategy=PixelCalibrationStrategy.NNClustering; \
             flags.Acts.Clusters.RetrieveChargeInformation=True' \
  --conditionsTag ${conditions_tag} \
  --inputRDOFile ${input_rdo} \
  --outputAODFile test.AOD.pool.root  \
  --maxEvents ${n_events} \
  --multithreaded
