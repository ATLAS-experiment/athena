# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import sys

from AthenaConfiguration.AutoConfigFlags import GetFileMD
from AthenaConfiguration.Enums import ProductionStep
from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude
from SimuJobTransforms.CommonSimulationSteering import specialConfigPreInclude, specialConfigPostInclude

# force no legacy job properties
from AthenaCommon import JobProperties
JobProperties.jobPropertiesDisallowed = True


def setOverlayInputFiles(runArgs, flags, log):
    hasRDO_BKGInput = hasattr(runArgs, 'inputRDO_BKGFile')
    hasBS_SKIMInput = hasattr(runArgs, 'inputBS_SKIMFile')
    hasEVNT_Input   = hasattr(runArgs, 'inputEVNTFile')

    if flags.Common.ProductionStep == ProductionStep.Overlay and not hasattr(runArgs, 'inputHITSFile'):
        raise RuntimeError('No input HITS file defined')

    if hasRDO_BKGInput and hasBS_SKIMInput:
        raise RuntimeError('Both RDO_BKG and BS_SKIM are defined')
    if not hasRDO_BKGInput and not hasBS_SKIMInput:
        raise RuntimeError('Define one of RDO_BKG and BS_SKIM file types')

    if hasattr(runArgs, 'skipSecondaryEvents'):
        flags.Overlay.SkipSecondaryEvents = runArgs.skipSecondaryEvents

    if hasRDO_BKGInput:
        flags.Input.Files = runArgs.inputRDO_BKGFile

        if flags.Input.isMC:
            log.info('Running MC+MC overlay')
            flags.Overlay.DataOverlay = False
        else:
            log.info('Running MC+data overlay')
            flags.Overlay.DataOverlay = True

        if flags.Common.ProductionStep == ProductionStep.Overlay:
            flags.Input.SecondaryFiles = runArgs.inputHITSFile
        elif flags.Common.ProductionStep == ProductionStep.FastChain:
            if not hasEVNT_Input:
                raise RuntimeError('No input EVNT file defined')
            else:
                flags.Input.SecondaryFiles = runArgs.inputEVNTFile
        else:
            raise RuntimeError('No secondaryFiles are defined')

        if flags.Input.isMC:
            # take MCChannelNumber from secondary input:
            flags.Input.MCChannelNumber = GetFileMD(flags.Input.SecondaryFiles, allowEmpty=False).get("mc_channel_number", 0)

            # runNumber is MC channel number in reco
            if hasattr(runArgs, 'runNumber'):
                if flags.Input.MCChannelNumber != runArgs.runNumber:
                    log.warning('Got different MC channel number (%d) from runNumber than from metadata (%d)', runArgs.runNumber, flags.Input.MCChannelNumber)
                    flags.Input.MCChannelNumber = runArgs.runNumber
                else:
                    log.info('MC channel number: %d', flags.Input.MCChannelNumber)
    else:
        log.info('Running MC+data overlay from ByteStream')
        flags.Overlay.DataOverlay = True
        flags.Overlay.ByteStream = True
        flags.Input.isMC = False
        if flags.Common.ProductionStep == ProductionStep.Overlay:
            flags.Input.Files = runArgs.inputHITSFile
        elif flags.Common.ProductionStep == ProductionStep.FastChain:
            if not hasEVNT_Input:
                raise RuntimeError('No input EVNT file defined')
            else:
                flags.Input.Files = runArgs.inputEVNTFile
        else:
            raise RuntimeError('No input files are defined')

        flags.Input.SecondaryFiles = runArgs.inputBS_SKIMFile

        # take secondary metadata for some flags
        # TODO: autoconfigure
        flags.Input.RunNumbers = list(GetFileMD(flags.Input.SecondaryFiles).get("runNumbers", []))
        flags.Input.LumiBlockNumbers = list(GetFileMD(flags.Input.SecondaryFiles).get("lumiBlockNumbers", []))


def fromRunArgs(runArgs):
    from AthenaCommon.Logging import logging
    logOverlay = logging.getLogger('Overlay')
    logOverlay.info('****************** STARTING OVERLAY *****************')

    logOverlay.info('**** Transformation run arguments')
    logOverlay.info(str(runArgs))

    logOverlay.info('**** Setting-up configuration flags')
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    commonRunArgsToFlags(runArgs, flags)

    flags.Common.ProductionStep = ProductionStep.Overlay

    # Setting input files for Overlay
    setOverlayInputFiles(runArgs, flags, logOverlay)

    # Setting output files for Overlay
    if hasattr(runArgs, 'outputRDOFile'):
        if runArgs.outputRDOFile == 'None':
            flags.Output.RDOFileName = ''
        else:
            flags.Output.RDOFileName = runArgs.outputRDOFile
    else:
        raise RuntimeError('No output RDO file defined')

    if hasattr(runArgs, 'outputRDO_SGNLFile'):
        flags.Output.RDO_SGNLFileName = runArgs.outputRDO_SGNLFile

    # Autoconfigure enabled subdetectors
    if hasattr(runArgs, 'detectors'):
        detectors = runArgs.detectors
    else:
        detectors = None

    # Setup digitization flags
    from DigitizationConfig.DigitizationConfigFlags import digitizationRunArgsToFlags
    digitizationRunArgsToFlags(runArgs, flags)

    # Setup detector flags
    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
    setupDetectorFlags(flags, detectors, use_metadata=True, toggle_geometry=True)

    # Disable LVL1 trigger if triggerConfig explicitly set to 'NONE'
    if hasattr(runArgs, 'triggerConfig') and runArgs.triggerConfig == 'NONE':
        flags.Detector.EnableL1Calo = False

    # Setup perfmon flags from runargs
    from PerfMonComps.PerfMonConfigHelpers import setPerfmonFlagsFromRunArgs
    setPerfmonFlagsFromRunArgs(flags, runArgs)

    # Special Configuration preInclude
    specialConfigPreInclude(flags)

    # Pre-include
    processPreInclude(runArgs, flags)

    # Pre-exec
    processPreExec(runArgs, flags)

    # To respect --athenaopts
    flags.fillFromArgs()

    # Moving here so that it is ahead of flags being locked. Need to
    # iterate on exact best position w.r.t. above calls
    from OverlayConfiguration.OverlayMetadata import overlayMetadataCheck
    # Handle metadata correctly
    overlayMetadataCheck(flags)

    # Lock flags
    flags.lock()

    # Main overlay steering
    from OverlayConfiguration.OverlaySteering import OverlayMainCfg
    cfg = OverlayMainCfg(flags)

    # Special message service configuration
    from DigitizationConfig.DigitizationSteering import DigitizationMessageSvcCfg
    cfg.merge(DigitizationMessageSvcCfg(flags))

    # Special Configuration postInclude
    specialConfigPostInclude(flags, cfg)

    # Post-include
    processPostInclude(runArgs, flags, cfg)

    # Post-exec
    processPostExec(runArgs, flags, cfg)

    # Write AMI tag into in-file metadata
    from PyUtils.AMITagHelperConfig import AMITagCfg
    cfg.merge(AMITagCfg(flags, runArgs))

    # Run the final configuration
    sc = cfg.run()
    sys.exit(not sc.isSuccess())
