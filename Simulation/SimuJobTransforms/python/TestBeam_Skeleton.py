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
    log = logging.getLogger('TeamBeam')
    log.info('****************** STARTING ATLASG4 Test Beam *****************')

    log.info('**** Transformation run arguments')
    log.info(str(runArgs))

    log.info('**** Setting-up configuration flags')
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.Enums import BeamType
    from SimulationConfig.SimEnums import SimulationFlavour
    flags = initConfigFlags()
    commonRunArgsToFlags(runArgs, flags)

    if not hasattr(runArgs, 'geometryVersion'):
        log.info('no geometryVersion specified, so assuming tb_Tile2000_2003_5B')
        flags.GeoModel.AtlasVersion = "ATLAS-CTB-01" # 'tb_Tile2000_2003_5B' # "ctbh8"

    # Set ProductionStep
    from AthenaConfiguration.Enums import ProductionStep
    flags.Common.ProductionStep = ProductionStep.Simulation
    # Set BeamType
    flags.Beam.Type = BeamType.TestBeam
    # Set Default ParticleGun configuration
    flags.Sim.GenerationConfiguration="ParticleGun.ParticleGunConfig.ParticleGun_TestBeam_SingleParticleCfg"
    flags.IOVDb.DatabaseInstance = "TMCP200"
    # Set the simulator
    flags.Sim.ISF.Simulator = SimulationFlavour.AtlasG4

    # This is not ISF
    flags.Sim.ISFRun = False

    # Generate detector list
    detectors = set()
    detectors.add('Tile')

    # Setup input: Three possible cases:
    # 1) inputEVNTFile (normal)
    # 2) inputEVNT_TRFile (TrackRecords from pre-simulated events,
    # used with TrackRecordGenerator)
    # 3) no input file (on-the-fly generation - typically ParticleGun
    # or CosmicGenerator)
    if hasattr(runArgs, 'inputEVNTFile'):
        flags.Input.Files = runArgs.inputEVNTFile
    else:
        # Common cases
        # 3a) ParticleGun
        flags.Input.Files = []
        flags.Input.isMC = True
        log.info('No inputEVNTFile provided. Assuming that you are running a generator on the fly.')
    if hasattr(runArgs, 'outputHITSFile'):
        if runArgs.outputHITSFile == 'None':
            flags.Output.HITSFileName = ''
        else:
            flags.Output.HITSFileName  = runArgs.outputHITSFile
    else:
        raise RuntimeError('No outputHITSFile defined')

    if hasattr(runArgs, 'conditionsTag'):
        flags.IOVDb.GlobalTag = runArgs.conditionsTag

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

    # Common simulation runtime arguments
    from SimulationConfig.TestBeamConfigFlags import testBeamRunArgsToFlags
    testBeamRunArgsToFlags(runArgs, flags)
    
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

    # Write AMI tag into in-file metadata
    from PyUtils.AMITagHelperConfig import AMITagCfg
    cfg.merge(AMITagCfg(flags, runArgs))

    import time
    tic = time.time()
    # Run the final accumulator
    sc = cfg.run()
    log.info("Run G4AtlasAlg in " + str(time.time()-tic) + " seconds")

    sys.exit(not sc.isSuccess())
