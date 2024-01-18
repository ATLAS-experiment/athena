# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude
from TriggerJobOpts import runHLT
from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg

from AthenaCommon.Logging import logging
log = logging.getLogger('RDOtoRDO_TRIG')

# force no legacy job properties
from AthenaCommon import JobProperties
JobProperties.jobPropertiesDisallowed = True


def configureFlags(runArgs):
    # some basic settings here...
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
    commonRunArgsToFlags(runArgs, flags)
    from RecJobTransforms.RecoConfigFlags import recoRunArgsToFlags
    recoRunArgsToFlags(runArgs, flags)

    # Set standard flags for HLT jobs
    runHLT.set_flags(flags)

    # Input
    if hasattr(runArgs, 'inputRDOFile'):
        flags.Input.Files = runArgs.inputRDOFile

    # Output 
    if hasattr(runArgs, 'outputRDO_TRIGFile'):
        flags.Output.RDOFileName = runArgs.outputRDO_TRIGFile
        log.info("---------- Configured RDO_TRIG output")

    from AthenaConfiguration.Enums import ProductionStep
    flags.Common.ProductionStep=ProductionStep.Reconstruction

    # Setup perfmon flags from runargs
    from PerfMonComps.PerfMonConfigHelpers import setPerfmonFlagsFromRunArgs
    setPerfmonFlagsFromRunArgs(flags, runArgs)

    # process pre-include/exec
    processPreInclude(runArgs, flags)
    processPreExec(runArgs, flags)

    # To respect --athenaopts
    flags.fillFromArgs()

    # Lock flags
    flags.lock()

    return flags



def fromRunArgs(runArgs):

    import time
    timeStart = time.time()

    flags = configureFlags(runArgs)

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))
    cfg.merge( runHLT.runHLTCfg(flags) )
    cfg.merge( PerfMonMTSvcCfg(flags) )

    # Post-include
    processPostInclude(runArgs, flags, cfg)

    # Post-exec
    processPostExec(runArgs, flags, cfg)

    from AthenaCommon.Constants import INFO
    if flags.Exec.OutputLevel <= INFO:
        cfg.printConfig()

    # Run the final accumulator
    sc = cfg.run()
    timeFinal = time.time()
    log.info("Run RDOtoRDO_TRIG_skeleton in %d seconds", timeFinal - timeStart)

    import sys
    sys.exit(sc.isFailure())
