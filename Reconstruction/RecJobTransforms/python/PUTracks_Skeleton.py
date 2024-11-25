# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude

# force no legacy job properties
from AthenaCommon import JobProperties
JobProperties.jobPropertiesDisallowed = True


def fromRunArgs(runArgs):
    from AthenaCommon.Logging import logging
    log = logging.getLogger('PUTracks')
    log.info('****************** STARTING Reconstruction (PUTracks) *****************')
    import time
    timeStart = time.time()
    log.info('**** Setting-up configuration flags')
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    commonRunArgsToFlags(runArgs, flags)

    # Autoconfigure enabled subdetectors
    if hasattr(runArgs, 'detectors'):
        detectors = runArgs.detectors
    else:
        detectors = None

    ## Inputs
    # RDO
    if hasattr(runArgs, 'inputRDOFile'):
        flags.Input.Files = runArgs.inputRDOFile
    
    ## Outputs

    flags.Output.RDOFileName = runArgs.outputRDO_PUFile
    log.info("---------- Configured BKG_RDO output")

    # Reconstruction flags should be parsed after inputs are set
    from RecJobTransforms.RecoConfigFlags import recoRunArgsToFlags
    recoRunArgsToFlags(runArgs, flags)

    from AthenaConfiguration.Enums import ProductionStep
    flags.Common.ProductionStep = ProductionStep.PileUpPretracking
    flags.Input.isMC = True # should be set from the input file?
    flags.Tracking.doVertexFinding = False
    flags.Overlay.SigPrefix = flags.Overlay.BkgPrefix  # TODO: processing pileup background file, but perhapse we can just use BkgPrefix?

    # Setup detector flags
    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
    setupDetectorFlags(flags, detectors, use_metadata=True, toggle_geometry=True, keep_beampipe=True)
    # Print reco domain status
    from RecJobTransforms.RecoConfigFlags import printRecoFlags
    printRecoFlags(flags)

    # Setup perfmon flags from runargs
    from PerfMonComps.PerfMonConfigHelpers import setPerfmonFlagsFromRunArgs
    setPerfmonFlagsFromRunArgs(flags, runArgs)
     
    # Pre-include
    processPreInclude(runArgs, flags)

    # Pre-exec
    processPreExec(runArgs, flags)

    # To respect --athenaopts 
    flags.fillFromArgs()

    # Lock flags
    flags.lock()
    
    from FastChainPileup.PileUpPreTrackingConfig import PreTrackingCfg
    cfg = PreTrackingCfg(flags) #it includes a MainServicesCfg
    log.info("---------- Configured pileup tracks")

    # Post-include
    processPostInclude(runArgs, flags, cfg)

    # Post-exec
    processPostExec(runArgs, flags, cfg)

    timeConfig = time.time()
    log.info("configured in %d seconds", timeConfig - timeStart)

    log.info("Configured according to flag values listed below")
    flags.dump()

    # Print sum information about AccumulatorCache performance
    from AthenaConfiguration.AccumulatorCache import AccumulatorDecorator
    AccumulatorDecorator.printStats() 

    # Run the final accumulator
    sc = cfg.run()
    timeFinal = time.time()
    log.info("Run PUTracks_skeleton in %d seconds (running %d seconds)", timeFinal - timeStart, timeFinal - timeConfig)

    import sys
    sys.exit(not sc.isSuccess())
