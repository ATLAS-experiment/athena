#!/usr/bin/bash
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# ttbar mu=200 input
input_rdo=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/RDO.33629020._000047.pool.root.1
n_events=5
log_file="reco.log"

ignore_pattern="Acts.+FindingAlg.+ERROR.+Propagation.+reached.+the.+step.+count.+limit,Acts.+FindingAlg.+ERROR.+Propagation.+failed:.+PropagatorError:..+Propagation.+reached.+the.+configured.+maximum.+number.+of.+steps.+with.+the.+initial.+parameters,Acts.+FindingAlg.Acts.+ERROR.+CombinatorialKalmanFilter.+failed:.+CombinatorialKalmanFilterError:5.+Propagation.+reaches.+max.+steps.+before.+track.+finding.+is.+finished.+with.+the.+initial.+parameters,Acts.+FindingAlg.Acts.+ERROR.+failed.+to.+extrapolate.+track"

export ATHENA_CORE_NUMBER=1
Reco_tf.py \
  --preExec "flags.Exec.FPE=-1; \
  	     flags.Acts.doITkConversion=True; \
	     flags.Tracking.doTruth=False; \
	     flags.Tracking.doITkConversion=False; \
	     flags.Acts.useHGTDClusterInTrackFinding=True; \
	     flags.Detector.EnableHGTD=True; \
	     flags.Detector.EnableCalo=True; \
	     flags.Detector.GeometryCalo=True; \
	     flags.Detector.EnableLAr=True; \
	     flags.Detector.EnableTile=True; \
       	     flags.Detector.EnableMuon=False; \
	     flags.Acts.doLargeRadius=True; \
	     flags.Acts.doLowPt=True;" \
  --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsAloneWorkflowFlags" \
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
