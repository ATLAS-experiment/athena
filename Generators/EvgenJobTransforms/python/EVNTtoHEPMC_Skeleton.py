# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
# Job transform version of converting an EVNT file into a HEPMC file

# For the exit code at the end
import sys

# For translating the run arguments into flags
from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags

# For using pre include, pre-exec, etc; should only rarely be needed
from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude

# Force no legacy job properties
from AthenaCommon import JobProperties
JobProperties.jobPropertiesDisallowed = True

def fromRunArgs(runArgs):
    # Start the logger and identify ourselves
    from AthenaCommon.Logging import logging
    log = logging.getLogger('EVNTtoHEPMC')
    log.info('*** Starting EVNTtoHEPMC translation ***')

    # Print some job information
    log.info('*** Transformation run arguments ***')
    log.info(str(runArgs))

    # Set up the flags we need
    log.info('*** Setting-up configuration flags ***')
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    commonRunArgsToFlags(runArgs, flags)

    # Set ProductionStep
    from AthenaConfiguration.Enums import ProductionStep
    flags.Common.ProductionStep = ProductionStep.Derivation

    # Set the input file
    if hasattr(runArgs, 'inputEVNTFile'):
        flags.Input.Files = runArgs.inputEVNTFile
    else:
        log.error('Input EVNT file required for EVNTtoHEPMC')

    # Set the output file
    if hasattr(runArgs, 'outputEvtFile'):
        my_output_HepMCFile = runArgs.outputEvtFile
    else:
        log.error('Output Evt file (HepMC) required for EVNTtoHEPMC')

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

    # Do the configuration of the main services
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    # Set us up for reading a POOL file
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))

    # Use the WriteHepMC AlgTool from TruthIO to do the conversion
    from AthenaConfiguration.ComponentFactory import CompFactory
    cfg.addEventAlgo( CompFactory.WriteHepMC( 'WriteHepMC',
                      OutputFile = my_output_HepMCFile ) )

    # Post-include
    processPostInclude(runArgs, flags, cfg)

    # Post-exec
    processPostExec(runArgs, flags, cfg)

    import time
    tic = time.time()

    # Run the final accumulator
    sc = cfg.run()
    log.info("Ran EVNTtoHEPMC in " + str(time.time()-tic) + " seconds")

    sys.exit(not sc.isSuccess())

