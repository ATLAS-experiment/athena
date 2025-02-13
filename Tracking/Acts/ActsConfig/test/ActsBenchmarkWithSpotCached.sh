#!/usr/bin/bash
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

NTHREADS=${1}
NEVENTS=${2}
DATADIR="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO"

# Ignore specific error messages from Acts CKF
ignore_pattern="Acts.+FindingAlg.+ERROR.+Propagation.+reached.+the.+step.+count.+limit,Acts.+FindingAlg.+ERROR.+Propagation.+failed:.+PropagatorError:..+Propagation.+reached.+the.+configured.+maximum.+number.+of.+steps.+with.+the.+initial.+parameters,Acts.+FindingAlg.Acts.+ERROR.+CombinatorialKalmanFilter.+failed:.+CombinatorialKalmanFilterError:5.+Propagation.+reaches.+max.+steps.+before.+track.+finding.+is.+finished.+with.+the.+initial.+parameters,Acts.+FindingAlg.Acts.+ERROR.+SurfaceError:1,Acts.+FindingAlg.Acts.+ERROR.+failed.+to.+extrapolate.+track"

conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")

# Run the job
export TRF_ECHO=1;
ATHENA_CORE_NUMBER=${NTHREADS} Reco_tf.py \
    --maxEvents  ${NEVENTS} \
    --perfmon 'fullmonmt' \
    --multithreaded 'True' \
    --autoConfiguration 'everything' \
    --conditionsTag "all:${conditions}" \
    --geometryVersion 'all:ATLAS-P2-RUN4-03-00-00' \
    --postInclude 'all:PyJobTransforms.UseFrontier' \
    --preInclude 'InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsAloneWorkflowFlags' \
    --steering 'doRAWtoALL' \
    --preExec 'all:flags.Exec.FPE=-1;ConfigFlags.Tracking.doITkFastTracking=False;flags.Acts.useCache=True;' \
    --postExec 'all:cfg.getService("AlgResourcePool").CountAlgorithmInstanceMisses = True;' \
    --inputRDOFile ${DATADIR}"/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/*" \
    --outputAODFile 'myAOD.pool.root' \
    --jobNumber '1' \
    --ignorePatterns "${ignore_pattern}"

