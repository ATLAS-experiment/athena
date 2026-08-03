#!/usr/bin/bash
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

NTHREADS=${1}
NEVENTS=${2}
input_rdo=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RDO_RUN4[0])")

conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")

# Run the job
export TRF_ECHO=1;
ATHENA_CORE_NUMBER=${NTHREADS} Reco_tf.py \
    --maxEvents  ${NEVENTS} \
    --perfmon 'fullmonmt' \
    --multithreaded 'True' \
    --conditionsTag "all:${conditions}" \
    --postInclude 'all:PyJobTransforms.UseFrontier' \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsProductionFlags" \
    --steering 'doRAWtoALL' \
    --preExec 'from ActsConfig.ActsConfigFlags import SeedingStrategy;\
               flags.Acts.SeedingStrategy=SeedingStrategy.Gbts;' \
    --postExec 'all:cfg.getService("AlgResourcePool").CountAlgorithmInstanceMisses = True;' \
    --inputRDOFile ${input_rdo} \
    --outputAODFile 'myAOD.pool.root' \
    --jobNumber '1'
