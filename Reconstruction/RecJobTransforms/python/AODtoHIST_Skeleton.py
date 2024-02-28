# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import sys

from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude

# force no legacy job properties
from AthenaCommon import JobProperties
JobProperties.jobPropertiesDisallowed = True


def fromRunArgs(runArgs):
    
    from AthenaCommon.Logging import logging
    log = logging.getLogger('AODtoHIST')
    log.info('****************** STARTING AOD->HIST MAKING *****************')

    log.info('**** Transformation run arguments')
    log.info(str(runArgs))

    import time
    timeStart = time.time()

    # some basic settings here...
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
    commonRunArgsToFlags(runArgs, flags)
    from RecJobTransforms.RecoConfigFlags import recoRunArgsToFlags
    recoRunArgsToFlags(runArgs, flags)

    # Input
    if hasattr(runArgs, 'inputAODFile'):
        flags.Input.Files = runArgs.inputAODFile

    # Output 
    if hasattr(runArgs, 'outputHIST_AODFile'):
        flags.Output.HISTFileName = runArgs.outputHIST_AODFile
        log.info("---------- Configured HIST_AOD output")
    if hasattr(runArgs, 'outputHISTFile'):
        flags.Output.HISTFileName = runArgs.outputHISTFile
        log.info("---------- Configured HIST output")
    
    

    # Autoconfigure enabled subdetectors
    if hasattr(runArgs, 'detectors'):
        detectors = runArgs.detectors
    else:
        detectors = None
     # Setup detector flags
    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
    setupDetectorFlags(flags, detectors, use_metadata=True, toggle_geometry=True, keep_beampipe=True)

    processPreInclude(runArgs, flags)
    processPreExec(runArgs, flags)

    # To respect --athenaopts 
    flags.fillFromArgs()
    flags.lock()
    
    # Main DQ steering
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)
    # setup input
    cfg.flagPerfmonDomain('IO')
    from AthenaConfiguration.Enums import Format
    if flags.Input.Format is Format.BS:
        from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
        cfg.merge(ByteStreamReadCfg(flags))
        log.info("---------- Configured BS reading")
    else:
        from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
        cfg.merge(PoolReadCfg(flags))
        # Check if running on legacy inputs
        if "EventInfo" not in flags.Input.Collections:
            from xAODEventInfoCnv.xAODEventInfoCnvConfig import (
                EventInfoCnvAlgCfg)
            cfg.merge(EventInfoCnvAlgCfg(flags))
        log.info("---------- Configured POOL reading")

    from AthenaMonitoring.AthenaMonitoringCfg import AthenaMonitoringCfg, AthenaMonitoringPostprocessingCfg
    cfg.merge(AthenaMonitoringCfg(flags))
    cfg.merge(AthenaMonitoringPostprocessingCfg(flags))

    processPostInclude(runArgs, flags, cfg)
    processPostExec(runArgs, flags, cfg)

    # Run the final accumulator
    sc = cfg.run()
    timeFinal = time.time()
    log.info("Run AODtoHIST_Skeleton in %d seconds", timeFinal - timeStart)

    sys.exit(not sc.isSuccess())
