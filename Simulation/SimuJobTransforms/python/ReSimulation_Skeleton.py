# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import sys
from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude
from SimuJobTransforms.CommonSimulationSteering import CommonSimulationCfg, specialConfigPreInclude, specialConfigPostInclude

# force no legacy job properties
from AthenaCommon import JobProperties
JobProperties.jobPropertiesDisallowed = True


def fromRunArgs(runArgs):
    from AthenaCommon.Logging import logging
    log = logging.getLogger('Sim_tf')
    log.info('****************** STARTING Simulation *****************')

    log.info('**** Transformation run arguments')
    log.info(str(runArgs))

    log.info('**** Setting-up configuration flags')
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from SimulationConfig.SimEnums import SimulationFlavour
    flags = initConfigFlags()

    commonRunArgsToFlags(runArgs, flags)

    # Set ProductionStep
    from AthenaConfiguration.Enums import ProductionStep
    flags.Common.ProductionStep = ProductionStep.Simulation

    # Set the simulator
    if hasattr(runArgs, 'simulator'):
       flags.Sim.ISF.Simulator = SimulationFlavour(runArgs.simulator)

    # This is ISF and resimulation
    flags.Sim.ISFRun = True
    flags.Sim.ISF.ReSimulation = True

    # Generate detector list
    from SimuJobTransforms.SimulationHelpers import getDetectorsFromRunArgs
    detectors = getDetectorsFromRunArgs(flags, runArgs)

    if hasattr(runArgs, 'inputHITSFile'):
        flags.Input.Files = runArgs.inputHITSFile
    else:
        log.error('No inputHITSFile provided. Please try using Sim_tf.py instead.')
        raise RuntimeError('No intputHITSFile provided.')

    if hasattr(runArgs, 'outputHITS_RSMFile'):
        if runArgs.outputHITS_RSMFile == 'None':
            flags.Output.HITSFileName = ''
        else:
            flags.Output.HITSFileName  = runArgs.outputHITS_RSMFile

    # Setup detector flags
    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
    setupDetectorFlags(flags, detectors, toggle_geometry=True)

    # Setup perfmon flags from runargs
    from PerfMonComps.PerfMonConfigHelpers import setPerfmonFlagsFromRunArgs
    setPerfmonFlagsFromRunArgs(flags, runArgs)

    # Pre-include
    processPreInclude(runArgs, flags)

    # Special Configuration preInclude
    specialConfigPreInclude(flags)

    # Pre-exec
    processPreExec(runArgs, flags)

    # Common simulation runtime arguments
    from SimulationConfig.SimConfigFlags import simulationRunArgsToFlags
    simulationRunArgsToFlags(runArgs, flags)

    # To respect --athenaopts 
    flags.fillFromArgs()

    # Lock flags
    flags.lock()

    cfg = CommonSimulationCfg(flags, log)

    # Add OLD suffix to the names of collections read in from the input HITS file
    from SimuJobTransforms.ReSimInputConfig import RenameHitCollectionsOnReadCfg
    cfg.merge(RenameHitCollectionsOnReadCfg(flags))

    # Special Configuration postInclude
    specialConfigPostInclude(flags, cfg)

    # Post-include
    processPostInclude(runArgs, flags, cfg)

    # Post-exec
    processPostExec(runArgs, flags, cfg)

    # Write AMI tag into in-file metadata
    from PyUtils.AMITagHelperConfig import AMITagCfg
    cfg.merge(AMITagCfg(flags, runArgs))

    import time
    tic = time.time()
    # Run the final accumulator
    sc = cfg.run()
    log.info("Run resimulation in " + str(time.time()-tic) + " seconds")

    sys.exit(not sc.isSuccess())
