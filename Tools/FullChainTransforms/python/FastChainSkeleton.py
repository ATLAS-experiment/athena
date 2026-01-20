# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

import sys
from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
from SimuJobTransforms.CommonSimulationSteering import specialConfigPreInclude, specialConfigPostInclude

# force no legacy job properties
from AthenaCommon import JobProperties
JobProperties.jobPropertiesDisallowed = True


def fromRunArgs(runArgs):
    from AthenaCommon.Logging import logging
    logFastChain = logging.getLogger('FastChainSkeleton')
    logFastChain.info('****************** STARTING FastChain Simulation *****************')

    logFastChain.info('**** Transformation run arguments')
    logFastChain.info(str(runArgs))

    logFastChain.info('**** Setting-up configuration flags')
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    from SimulationConfig.SimEnums import SimulationFlavour
    commonRunArgsToFlags(runArgs, flags)

    # Set ProductionStep
    from AthenaConfiguration.Enums import ProductionStep
    flags.Common.ProductionStep = ProductionStep.FastChain

    # Set simulator
    if hasattr(runArgs, 'simulator'):
        flags.Sim.ISF.Simulator = SimulationFlavour(runArgs.simulator)

    # This is ISF
    flags.Sim.ISFRun = True

    # Set input files
    if hasattr(runArgs, 'inputRDO_BKGFile'):
        # Set inputs for Overlay
        from OverlayConfiguration.OverlaySkeleton import setOverlayInputFiles
        setOverlayInputFiles(runArgs, flags, logFastChain)
        flags.Common.isOverlay = True
        flags.Digitization.PileUp = False

        if flags.Overlay.DataOverlay:
            from SimulationConfig.SimEnums import VertexSource
            flags.Sim.VertexSource = VertexSource.MatchingBkg
    else:
        # Setting input files for FastChain without overlay
        if hasattr(runArgs, 'inputEVNTFile'):
            flags.Input.Files = runArgs.inputEVNTFile
        else:
            raise RuntimeError('No input EVNT file defined')

    # Setting output files (including for Overlay) for FastChain
    if hasattr(runArgs, 'outputHITSFile'):
        flags.Output.HITSFileName = runArgs.outputHITSFile

    if hasattr(runArgs, 'outputRDOFile'):
        if runArgs.outputRDOFile == 'None':
            flags.Output.RDOFileName = ''
        else:
            flags.Output.RDOFileName = runArgs.outputRDOFile
    else:
        raise RuntimeError('No outputRDOFile defined')

    if flags.Common.isOverlay:
        if hasattr(runArgs, 'outputRDO_SGNLFile'):
            flags.Output.RDO_SGNLFileName = runArgs.outputRDO_SGNLFile

    if hasattr(runArgs, 'conditionsTag'):
        flags.IOVDb.GlobalTag = runArgs.conditionsTag

    # Generate detector list (must be after input setting)
    from SimuJobTransforms.SimulationHelpers import getDetectorsFromRunArgs
    detectors = getDetectorsFromRunArgs(flags, runArgs)

    # Setup detector flags
    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
    setupDetectorFlags(flags, detectors, toggle_geometry=True)

    # Common simulation runtime arguments
    from SimulationConfig.SimConfigFlags import simulationRunArgsToFlags
    simulationRunArgsToFlags(runArgs, flags)

    # Setup digitization flags
    from DigitizationConfig.DigitizationConfigFlags import digitizationRunArgsToFlags
    digitizationRunArgsToFlags(runArgs, flags)

    # Setup flags for pile-up
    if not flags.Common.isOverlay:
        # Setup common digitization flags
        from DigitizationConfig.DigitizationConfigFlags import setupDigitizationFlags
        setupDigitizationFlags(runArgs, flags)
        logFastChain.info('Running with pile-up: %s', flags.Digitization.PileUp)

    # Disable LVL1 trigger if triggerConfig explicitly set to 'NONE'
    if hasattr(runArgs, 'triggerConfig') and runArgs.triggerConfig == 'NONE':
        flags.Detector.EnableL1Calo = False

    # Setup perfmon flags from runargs
    from PerfMonComps.PerfMonConfigHelpers import setPerfmonFlagsFromRunArgs
    setPerfmonFlagsFromRunArgs(flags, runArgs)

    # Pre-include
    processPreInclude(runArgs, flags)

    # Special Configuration preInclude
    specialConfigPreInclude(flags)

    # Pre-exec
    processPreExec(runArgs, flags)

    if not flags.Common.isOverlay:
        # Load pile-up stuff after pre-include/exec to ensure everything is up-to-date
        from DigitizationConfig.DigitizationConfigFlags import pileupRunArgsToFlags
        pileupRunArgsToFlags(runArgs, flags)

        # Setup pile-up profile
        if flags.Digitization.PileUp:
            from RunDependentSimComps.PileUpUtils import setupPileUpProfile
            setupPileUpProfile(flags)

    flags.Sim.DoFullChain = True
    # For jobs running Overlay we take the run number from the
    # background RDOs, so we don't actually need to override the run
    # number.
    flags.Input.OverrideRunNumber = not flags.Common.isOverlay
    
    # To respect --athenaopts 
    flags.fillFromArgs()

    # Moving here so that it is ahead of flags being locked. Need to
    # iterate on exact best position w.r.t. above calls
    # Handle metadata correctly
    if flags.Common.isOverlay:
        from OverlayConfiguration.OverlayMetadata import fastChainOverlayMetadataCheck
        fastChainOverlayMetadataCheck(flags)

    # Lock flags
    flags.lock()

    if flags.Digitization.PileUp:
        from DigitizationConfig.PileUpConfig import PileUpEventLoopMgrCfg
        cfg = MainServicesCfg(flags, LoopMgr="PileUpEventLoopMgr")
        cfg.merge(PileUpEventLoopMgrCfg(flags))
    else:
        cfg = MainServicesCfg(flags)

    cfg.merge(PoolReadCfg(flags))

    from BeamEffects.BeamEffectsAlgConfig import BeamEffectsAlgCfg
    cfg.merge(BeamEffectsAlgCfg(flags))

    if not flags.Digitization.PileUp and not flags.Common.isOverlay:
        # Make sure signal EventInfo is rebuilt from event context
        # TODO: this is probably not needed, but keeping it to be in sync with standard simulation
        from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoUpdateFromContextAlgCfg
        cfg.merge(EventInfoUpdateFromContextAlgCfg(flags))

    if flags.Common.isOverlay:
        from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoOverlayCfg
        cfg.merge(EventInfoOverlayCfg(flags))
        # CopyMcEventCollection should be before Kernel
        if not flags.Overlay.DataOverlay:
            from OverlayCopyAlgs.OverlayCopyAlgsConfig import CopyMcEventCollectionCfg
            cfg.merge(CopyMcEventCollectionCfg(flags))

    from ISF_Config.ISF_MainConfig import ISF_KernelCfg
    cfg.merge(ISF_KernelCfg(flags))

    # Main Overlay Steering
    if flags.Common.isOverlay:
        from OverlayConfiguration.OverlaySteering import OverlayMainContentCfg
        cfg.merge(OverlayMainContentCfg(flags))
    else:
        from DigitizationConfig.DigitizationSteering import DigitizationMainContentCfg
        cfg.merge(DigitizationMainContentCfg(flags))

    # Allow writing hits to output if requested
    if flags.Output.HITSFileName:
        from AthenaConfiguration.Enums import MetadataCategory
        from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
        from SimuJobTransforms.SimOutputConfig import getStreamHITS_ItemList
        from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
        cfg.merge(OutputStreamCfg(flags, "HITS", ItemList=getStreamHITS_ItemList(flags)))
        cfg.merge(SetupMetaDataForStreamCfg(flags, "HITS", createMetadata=[MetadataCategory.IOVMetaData]))

    # Special message service configuration
    from DigitizationConfig.DigitizationSteering import DigitizationMessageSvcCfg
    cfg.merge(DigitizationMessageSvcCfg(flags))

    # Special Configuration postInclude
    specialConfigPostInclude(flags, cfg)

    # Post-include
    processPostInclude(runArgs, flags, cfg)

    # Post-exec
    processPostExec(runArgs, flags, cfg)

    from AthenaConfiguration.Utils import setupLoggingLevels
    setupLoggingLevels(flags, cfg)

    # Run the final accumulator
    sc = cfg.run()
    sys.exit(not sc.isSuccess())
