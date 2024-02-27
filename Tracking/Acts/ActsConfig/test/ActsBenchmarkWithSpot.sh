#!/usr/bin/bash
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

NTHREADS=${1}
NEVENTS=${2}
DATADIR="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO"

# Ignore specific error messages from Acts CKF
ignore_pattern="ActsTrackFindingAlg.+ERROR.+Propagation.+reached.+the.+step.+count.+limit,ActsTrackFindingAlg.+ERROR.+Propagation.+failed:.+PropagatorError:3.+Propagation.+reached.+the.+configured.+maximum.+number.+of.+steps.+with.+the.+initial.+parameters,ActsTrackFindingAlg.Acts.+ERROR.+CombinatorialKalmanFilter.+failed:.+CombinatorialKalmanFilterError:5.+Propagation.+reaches.+max.+steps.+before.+track.+finding.+is.+finished.+with.+the.+initial.+parameters,ActsTrackFindingAlg.Acts.+ERROR.+SurfaceError:1"

# Run the job
export TRF_ECHO=1;
ATHENA_CORE_NUMBER=${NTHREADS} Reco_tf.py \
    --CA 'all:True' \
    --maxEvents  ${NEVENTS} \
    --perfmon 'fullmonmt' \
    --multithreaded 'True' \
    --autoConfiguration 'everything' \
    --conditionsTag 'all:OFLCOND-MC15c-SDR-14-05' \
    --geometryVersion 'all:ATLAS-P2-RUN4-03-00-00' \
    --postInclude 'all:PyJobTransforms.UseFrontier' \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsBenchmarkSpotFlags" \
    --steering 'doRAWtoALL' \
    --preExec 'all:ConfigFlags.Tracking.doITkFastTracking=False' \
    --postExec 'all:cfg.getService("AlgResourcePool").CountAlgorithmInstanceMisses = True;cfg.getEventAlgo("ActsPixelClusterizationAlg").ClustersKey="xAODpixelClusters";cfg.getEventAlgo("ActsStripClusterizationAlg").ClustersKey="xAODstripClusters";' \
    --inputRDOFile ${DATADIR}"/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/*" \
    --outputAODFile 'myAOD.pool.root' \
    --jobNumber '1' \
    --ignorePatterns "${ignore_pattern}"

