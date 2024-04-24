# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

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
    from AthenaConfiguration.Enums import BeamType
    from SimulationConfig.SimEnums import CalibrationRun, CavernBackground, SimulationFlavour
    flags = initConfigFlags()

    commonRunArgsToFlags(runArgs, flags)

    # Set ProductionStep
    from AthenaConfiguration.Enums import ProductionStep
    flags.Common.ProductionStep = ProductionStep.Simulation

    # Set the simulator
    if hasattr(runArgs, 'simulator'):
       flags.Sim.ISF.Simulator = SimulationFlavour(runArgs.simulator)

    # This is ISF
    flags.Sim.ISFRun = True

    # Generate detector list
    from SimuJobTransforms.SimulationHelpers import getDetectorsFromRunArgs
    detectors = getDetectorsFromRunArgs(flags, runArgs)

    # Beam Type
    if hasattr(runArgs,'beamType'):
        if runArgs.beamType == 'cosmics':
            flags.Beam.Type = BeamType.Cosmics
            flags.Sim.CavernBackground = CavernBackground.Off

    # Setup input: Three possible cases:
    # 1) inputEVNTFile (normal)
    # 2) inputEVNT_TRFile (TrackRecords from pre-simulated events,
    # used with TrackRecordGenerator)
    # 3) no input file (on-the-fly generation - typically ParticleGun
    # or CosmicGenerator)
    if hasattr(runArgs, 'inputEVNTFile'):
        flags.Input.Files = runArgs.inputEVNTFile
    elif hasattr(runArgs, 'inputEVNT_TRFile'):
        flags.Input.Files = runArgs.inputEVNT_TRFile
        # Three common cases here:
        # 2a) Cosmics simulation
        # 2b) Stopped particle simulation
        # 2c) Cavern background simulation
        if flags.Beam.Type is BeamType.Cosmics:
            flags.Sim.ReadTR = True
            flags.Sim.CosmicFilterVolumeNames = ['Muon']
            detectors.add('Cavern')  # simulate the cavern with a cosmic TR file
        elif hasattr(runArgs,"trackRecordType") and runArgs.trackRecordType=="stopped":
            flags.Sim.ReadTR = True
            log.error('Stopped Particle simulation is not supported yet')
        else:
            detectors.add('Cavern')  # simulate the cavern
            flags.Sim.CavernBackground = CavernBackground.Read
    else:
        # Common cases
        # 3a) ParticleGun
        # 3b) CosmicGenerator
        flags.Input.Files = []
        flags.Input.isMC = True
        log.info('No inputEVNTFile provided. Assuming that you are running a generator on the fly.')
        if flags.Beam.Type is BeamType.Cosmics:
            flags.Sim.CosmicFilterVolumeNames = [getattr(runArgs, "CosmicFilterVolume", "InnerDetector")]
            flags.Sim.CosmicFilterVolumeNames += [getattr(runArgs, "CosmicFilterVolume2", "NONE")]
            flags.Sim.CosmicPtSlice = getattr(runArgs, "CosmicPtSlice", 'NONE')
            detectors.add('Cavern')  # simulate the cavern when generating cosmics on-the-fly
            log.debug('No inputEVNTFile provided. OK, as performing cosmics simulation.')

    if hasattr(runArgs, 'outputHITSFile'):
        if runArgs.outputHITSFile == 'None':
            flags.Output.HITSFileName = ''
        else:
            flags.Output.HITSFileName  = runArgs.outputHITSFile
    if hasattr(runArgs, "outputEVNT_TRFile"):
        # Three common cases
        # 1b) Write TrackRecords for Cavern background
        # 1c) Write TrackRecords for Stopped particles
        # 3b) CosmicGenerator
        flags.Output.EVNT_TRFileName = runArgs.outputEVNT_TRFile
        if hasattr(runArgs,"trackRecordType") and runArgs.trackRecordType=="stopped":
            # Case 1c)
            log.error('Stopped Particle simulation not supported yet!')
        elif flags.Beam.Type is BeamType.Cosmics:
            # Case 3b)
            pass
        else:
            #Case 1b) Cavern Background
            detectors.add('Cavern')  # simulate the cavern
            flags.Sim.CalibrationRun = CalibrationRun.Off
            flags.Sim.CavernBackground = CavernBackground.Write
    if not (hasattr(runArgs, 'outputHITSFile') or hasattr(runArgs, "outputEVNT_TRFile")):
        log.warning('No outputHITSFile or outputEVNT_TRFile defined')

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

    # Special Configuration postInclude
    specialConfigPostInclude(flags, cfg)

    # Post-include
    processPostInclude(runArgs, flags, cfg)

    # Post-exec
    processPostExec(runArgs, flags, cfg)

    from AthenaConfiguration.Utils import setupLoggingLevels
    setupLoggingLevels(flags, cfg)

    # Write AMI tag into in-file metadata
    from PyUtils.AMITagHelperConfig import AMITagCfg
    cfg.merge(AMITagCfg(flags, runArgs))

    import time
    tic = time.time()
    # Run the final accumulator
    sc = cfg.run()
    log.info("Run ISF simulation in " + str(time.time()-tic) + " seconds")

    sys.exit(not sc.isSuccess())
