# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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
    log = logging.getLogger('POOLtoHEPMC')
    log.info('*** Starting POOLtoHEPMC translation ***')

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

    # Set the input file and configure the input collection key
    if hasattr(runArgs, 'inputEVNTFile'):
        flags.Input.Files = runArgs.inputEVNTFile
        McEventKey = 'GEN_EVENT'
    elif hasattr(runArgs, 'inputHITSFile'):
        flags.Input.Files = runArgs.inputHITSFile
        McEventKey = 'TruthEvent'
    elif hasattr(runArgs, 'inputRDOFile'):
        flags.Input.Files = runArgs.inputRDOFile
        McEventKey = 'TruthEvent'
    else:
        log.error('Input EVNT, HITS, or RDO file required for POOLtoHEPMC')

    # Set the output file
    if hasattr(runArgs, 'outputHEPMCFile'):
        if '.tar' in runArgs.outputHEPMCFile or '.tgz' in runArgs.outputHEPMCFile or '.gz' in runArgs.outputHEPMCFile:
            log.info('Output will be compressed')
            if '.tar' in runArgs.outputHEPMCFile:
                index = runArgs.outputHEPMCFile.find('.tar')
            elif '.tgz' in runArgs.outputHEPMCFile:
                index = runArgs.outputHEPMCFile.find('.tgz')
            elif '.gz' in runArgs.outputHEPMCFile:
                index = runArgs.outputHEPMCFile.find('.gz')
            if hasattr(runArgs, 'extension') and 'events' in runArgs.extension:
                my_output_HepMCFile = runArgs.outputHEPMCFile[:index]+'.events'
            else:
                my_output_HepMCFile = runArgs.outputHEPMCFile[:index]+'.hepmc'
        else:
            log.info('Output will not be compressed')
            my_output_HepMCFile = runArgs.outputHEPMCFile
    else:
        log.error('OutputHEPMCFile required for POOLtoHEPMC')
        raise RuntimeError('OutputHEPMCFile required for POOLtoHEPMC')

    hepMCFormat = 'hepmc2'
    if hasattr(runArgs, 'hepmcFormat'):
        hepMCFormat = runArgs.hepmcFormat

    hepMCUnits = 'GEVMM'
    if hasattr(runArgs, 'hepmcUnits'):
        hepMCUnits = runArgs.hepmcUnits

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

    # We need the component factory to build the job up
    from AthenaConfiguration.ComponentFactory import CompFactory

    # Add FixHepMC to remove loops here
    # This is a work-around for AGENE-2342, which needs a HepMC patch to fix
    cfg.addEventAlgo(CompFactory.FixHepMC("FixHepMC"))

    # Use the WriteHepMC AlgTool from TruthIO to do the conversion
    cfg.addEventAlgo( CompFactory.WriteHepMC( 'WriteHepMC',
                      OutputFile = my_output_HepMCFile,
                      Format = hepMCFormat,
                      Units = hepMCUnits,
                      McEventKey = McEventKey ) )
    # Here one should set the output format
    # Post-include
    processPostInclude(runArgs, flags, cfg)

    # Post-exec
    processPostExec(runArgs, flags, cfg)

    import time
    tic = time.time()

    # Run the final accumulator
    sc = cfg.run()

    # Check based on the file name if we need to compress the output
    if '.tgz' in runArgs.outputHEPMCFile or '.tar.gz' in runArgs.outputHEPMCFile:
        log.info('Compressing output into tar+gz format (this may take a moment)')
        import tarfile
        with tarfile.open(runArgs.outputHEPMCFile,'w:gz') as out_tar:
            out_tar.add(my_output_HepMCFile)
        # Remove the original uncompressed file
        log.debug(f'Deleting original (uncompressed) file {my_output_HepMCFile}')
        import os
        os.remove(my_output_HepMCFile)
    elif '.gz' in runArgs.outputHEPMCFile:
        log.info('Compressing output into gz format (this may take a moment)')
        import gzip
        import shutil
        with open(my_output_HepMCFile,'rb') as in_file, gzip.open(runArgs.outputHEPMCFile,'wb') as out_file:
            shutil.copyfileobj(in_file,out_file)
        # Remove the original uncompressed file
        log.debug(f'Deleting original (uncompressed) file {my_output_HepMCFile}')
        import os
        os.remove(my_output_HepMCFile)

    # All done, now just report back
    log.info("Ran POOLtoHEPMC in " + str(time.time()-tic) + " seconds")

    sys.exit(not sc.isSuccess())

