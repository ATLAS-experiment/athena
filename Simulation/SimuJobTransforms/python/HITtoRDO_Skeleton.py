# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import sys

from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude
from SimuJobTransforms.CommonSimulationSteering import specialConfigPreInclude, specialConfigPostInclude

# force no legacy job properties
from AthenaCommon import JobProperties
JobProperties.jobPropertiesDisallowed = True


def fromRunArgs(runArgs):
    from AthenaCommon.Logging import logging
    log = logging.getLogger('HITtoRDO')
    log.info('****************** STARTING HITtoRDO *****************')

    log.info('**** Transformation run arguments')
    log.info(str(runArgs))

    log.info('**** Setting-up configuration flags')
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    commonRunArgsToFlags(runArgs, flags)

    if not hasattr(runArgs, 'inputHITSFile'):
        raise RuntimeError('No input HITS file defined')

    if not hasattr(runArgs, 'outputRDOFile'):
        raise RuntimeError('No output RDO file defined')

    flags.Input.isMC = True
    flags.Input.Files = runArgs.inputHITSFile
    flags.Output.RDOFileName = runArgs.outputRDOFile

    # runNumber is MC channel number in reco
    if hasattr(runArgs, 'runNumber'):
        if flags.Input.MCChannelNumber != runArgs.runNumber:
            log.warning('Got different MC channel number (%d) from runNumber than from metadata (%d)', runArgs.runNumber, flags.Input.MCChannelNumber)
            flags.Input.MCChannelNumber = runArgs.runNumber
        else:
            log.info('MC channel number: %d', flags.Input.MCChannelNumber)

    # Autoconfigure enabled subdetectors
    if hasattr(runArgs, 'detectors'):
        detectors = runArgs.detectors
    else:
        detectors = None

    # Setup digitization flags
    from DigitizationConfig.DigitizationConfigFlags import digitizationRunArgsToFlags
    digitizationRunArgsToFlags(runArgs, flags)

    # Setup common digitization flags
    from DigitizationConfig.DigitizationConfigFlags import setupDigitizationFlags
    setupDigitizationFlags(runArgs, flags)
    log.info('Running with pile-up: %s', flags.Digitization.PileUp)

    # Setup detector flags
    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
    setupDetectorFlags(flags, detectors, use_metadata=True, toggle_geometry=True)

    # Setup perfmon flags from runargs
    from PerfMonComps.PerfMonConfigHelpers import setPerfmonFlagsFromRunArgs
    setPerfmonFlagsFromRunArgs(flags, runArgs)

    # Special Configuration preInclude
    specialConfigPreInclude(flags)

    # Pre-include
    processPreInclude(runArgs, flags)

    # Pre-exec
    processPreExec(runArgs, flags)

    # Load pile-up stuff after pre-include/exec to ensure everything is up-to-date
    from DigitizationConfig.DigitizationConfigFlags import pileupRunArgsToFlags
    pileupRunArgsToFlags(runArgs, flags)

    # Setup pile-up profile
    if flags.Digitization.PileUp:
        from RunDependentSimComps.PileUpUtils import setupPileUpProfile
        setupPileUpProfile(flags)

    # TODO not parsed yet:
    # '--outputRDO_FILTFile'

    # To respect --athenaopts 
    flags.fillFromArgs()

    # Lock flags
    flags.lock()

    # Main overlay steering
    from DigitizationConfig.DigitizationSteering import DigitizationMainCfg
    cfg = DigitizationMainCfg(flags)

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
