# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

def fromRunArgs(runArgs):

    """ This is the main skeleton for creating FCS_NTUP from ESD files.
    """

    # Setup logging
    from AthenaCommon.Logging import logging
    log = logging.getLogger('FCS_Ntup_tf')
    log.info( '****************** STARTING Ntuple Production *****************' )

    # Print arguments
    log.info('**** Transformation run arguments')
    log.info(str(runArgs))

    # Setup configuration flags
    log.info('**** Setting up configuration flags')

    from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Exec.EventPrintoutInterval = 100
    commonRunArgsToFlags(runArgs, flags)

    # First let's find the input/output files
    if hasattr(runArgs,"inputESDFile"):
        flags.Input.Files = runArgs.inputESDFile
    else:
        raise RuntimeError('Could NOT determine the input files!')

    # Now set the input files before we attempt to read the processing tags
    if hasattr(runArgs,"outputNTUP_FCSFile"):
        log.info("Output is")
        flags.Output.HISTFileName = runArgs.outputNTUP_FCSFile
    else:
        log.warning('No output file set! Using output.NTUP_FCS.root')
        flags.Output.HISTFileName = 'output.NTUP_FCS.root'

    ## Optional output Geometry File
    outputGeoFileName = None
    if hasattr(runArgs,"outputGeoFileName"):
        outputGeoFileName = runArgs.outputGeoFileName

    # Autoconfigure enabled subdetectors
    if hasattr(runArgs, 'detectors'):
        detectors = runArgs.detectors
    else:
        detectors = None
     # Setup detector flags
    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
    setupDetectorFlags(flags, detectors, use_metadata=True, toggle_geometry=True, keep_beampipe=True)

    ## Flag for doG4Hits
    doG4HitsArg = False
    if hasattr(runArgs,"doG4Hits"):
        doG4HitsArg = runArgs.doG4Hits

    ## Flag for doClusterInfo
    doClusterInfoArg = False
    if hasattr(runArgs,"doClusterInfo"):
        doClusterInfoArg = runArgs.doClusterInfo

    ## Flag for saveAllBranches
    saveAllBranchesArg = False
    if hasattr(runArgs, "saveAllBranches"):
        saveAllBranchesArg = runArgs.saveAllBranches

    ## Sets the number of truth particles copied to ntuples
    NTruthParticlesArg = 1
    if hasattr(runArgs, "NTruthParticles"):
        NTruthParticlesArg = runArgs.NTruthParticles

    # Setup perfmon flags from runargs
    from PerfMonComps.PerfMonConfigHelpers import setPerfmonFlagsFromRunArgs
    setPerfmonFlagsFromRunArgs(flags, runArgs)

    # Pre-include
    from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude
    log.info('**** Processing preInclude')
    processPreInclude(runArgs, flags)

    # Pre-exec
    log.info('**** Processing preExec')
    processPreExec(runArgs, flags)

    # To respect --athenaopts
    log.info('**** Processing athenaopts')
    flags.fillFromArgs()

    # Lock configuration flags
    log.info('**** Locking configuration flags')
    flags.lock()

    # Set up necessary job components
    log.info('**** Setting up job components')

    # Main services
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    # Input reading
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))

    from ISF_FastCaloSimParametrization.ISF_FastCaloSimParametrizationConfig import ISF_HitAnalysisCfg
    cfg.merge(ISF_HitAnalysisCfg(flags,
                                 NTruthParticles=NTruthParticlesArg, saveAllBranches=saveAllBranchesArg,
                                 doG4Hits=doG4HitsArg, doClusterInfo=doClusterInfoArg, outputGeoFileName=outputGeoFileName))
    # TODO! FCS config here

    # Post-include
    log.info('**** Processing postInclude')
    processPostInclude(runArgs, flags, cfg)

    # Post-exec
    log.info('**** Processing postExec')
    processPostExec(runArgs, flags, cfg)

    # Now run the job and exit accordingly
    sc = cfg.run()
    import sys
    sys.exit(not sc.isSuccess())
