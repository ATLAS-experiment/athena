#!/usr/bin/env python

# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.ComponentAccumulator import printProperties
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaCommon.Logging import logging

    flags = initConfigFlags()
    flags.Detector.GeometryCalo = False
    flags.Detector.GeometryMuon = False
    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents = 1
    flags.DQ.useTrigger = False
    flags.ITk.doTruth = False
    flags.Exec.MaxEvents = 10
    flags.Output.HISTFileName = "ActsMonitoringOutput.root"
    flags.Acts.doAnalysisNtuples = True
    flags.Tracking.doITkFastTracking = True

    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags.Input.Files = defaultTestFiles.RDO_RUN4

    from ActsConfig.ActsCIFlags import actsWorkflowFlags
    actsWorkflowFlags(flags)

    # Set the Main Pass
    flags = flags.cloneAndReplace(
        "Tracking.ActiveConfig",
        "Tracking.ITkActsPass")
 
    flags.lock()
    flags.dump()
    
    acc = MainServicesCfg(flags)
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))
    
    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
    acc.merge(BeamSpotCondAlgCfg(flags))

    # Schedule Athena clustering
    from InDetConfig.InDetPrepRawDataFormationConfig import AthenaTrkClusterizationCfg
    acc.merge(AthenaTrkClusterizationCfg(flags))

    # Schedule Cluster conversion
    from InDetConfig.InDetPrepRawDataFormationConfig import ITkInDetToXAODClusterConversionCfg
    acc.merge(ITkInDetToXAODClusterConversionCfg(flags))

    # Schedule Athena Space Point Formation
    from InDetConfig.SiSpacePointFormationConfig import ITkSiTrackerSpacePointFinderCfg
    acc.merge(ITkSiTrackerSpacePointFinderCfg(flags))

    # Schedule Space Point conversion
    from InDetConfig.SiSpacePointFormationConfig import InDetToXAODSpacePointConversionCfg
    acc.merge(InDetToXAODSpacePointConversionCfg(flags))
    
    # Schedule Seeding Analysis
    from ActsConfig.ActsAnalysisConfig import ActsSeedingAlgorithmAnalysisAlgCfg
    acc.merge(ActsSeedingAlgorithmAnalysisAlgCfg(flags))

    mlog = logging.getLogger("SeedingAlgorithmAnalysis")
    mlog.info("Configuring  SeedingAlgorithmAnalysis: ")
    printProperties(
      mlog,
      acc.getEventAlgo("ActsSeedingAlgorithmAnalysis"),
      nestLevel=2,
      printDefaults=True,
    )

    flags.dump()

    # debug printout
    acc.printConfig(withDetails=True, summariseProps=True)

    # run the job
    status = acc.run()

    # report the execution status (0 ok, else error)
    import sys
    sys.exit(not status.isSuccess())
