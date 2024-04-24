#!/usr/bin/bash
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# HI configuration with ITk
input_rdo=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-01/RDO_HIJING_ITk_lowstat.pool.root
n_events=5

ignore_pattern="ActsTrackFindingAlg.+ERROR.+Propagation.+reached.+the.+step.+count.+limit,ActsTrackFindingAlg.+ERROR.+Propagation.+failed:.+PropagatorError:3.+Propagation.+reached.+the.+configured.+maximum.+number.+of.+steps.+with.+the.+initial.+parameters,ActsTrackFindingAlg.Acts.+ERROR.+CombinatorialKalmanFilter.+failed:.+CombinatorialKalmanFilterError:5.+Propagation.+reaches.+max.+steps.+before.+track.+finding.+is.+finished.+with.+the.+initial.+parameters"

export ATHENA_CORE_NUMBER=1
Reco_tf.py --CA \
  --preExec "flags.Exec.FPE=500;" "from Campaigns import PhaseIINoPileUp; PhaseIINoPileUp(flags);" \
  --preInclude "all:InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude" "all:ActsConfig.ActsCIFlags.actsHeavyIonFlags" \
  --postInclude "all:PyJobTransforms.UseFrontier" \
  --autoConfiguration everything \
  --ignorePatterns "${ignore_pattern}" \
  --inputRDOFile ${input_rdo} \
  --outputAODFile AOD.pool.root \
  --maxEvents ${n_events} \
  --multithreaded
