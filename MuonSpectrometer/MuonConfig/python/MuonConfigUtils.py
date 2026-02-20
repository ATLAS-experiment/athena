# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration


### Configuration snippet to setup the THistSvc
def setupHistSvcCfg(flags, outFile: str, outStream: str):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()
    if len(outFile) == 0: 
        raise ValueError("The output file must not be empty")
    if len(outStream) == 0: 
        raise ValueError("The outstream must not be empty")

    from AthenaConfiguration.ComponentFactory import CompFactory
    histSvc = CompFactory.THistSvc(Output=[f"{outStream} DATAFILE='{outFile}', OPT='RECREATE'"])
    print(f"Register new stream {outStream} piped to {outFile}")
    result.addService(histSvc, primary=True)
    return result

def executeTest(cfg):
    cfg.printConfig(withDetails=True, summariseProps=True)
    if not cfg.run().isSuccess(): exit(1)

def configureCondTag(flags):
    if not flags.GeoModel.AtlasVersion:
        raise ValueError("No ATLAS version is configured")

    from AthenaConfiguration.Enums import LHCPeriod
    from AthenaConfiguration.TestDefaults import defaultConditionsTags
    if flags.GeoModel.Run == LHCPeriod.Run2:   
        flags.IOVDb.GlobalTag = defaultConditionsTags.RUN2_MC if flags.Input.isMC else defaultConditionsTags.RUN2_DATA
    elif flags.GeoModel.Run == LHCPeriod.Run3:   
        flags.IOVDb.GlobalTag = defaultConditionsTags.RUN3_MC if flags.Input.isMC else defaultConditionsTags.RUN3_DATA
    elif flags.GeoModel.Run == LHCPeriod.Run4:
          flags.IOVDb.GlobalTag = defaultConditionsTags.RUN4_MC
    else:
        raise ValueError(f"Invalid run period {flags.GeoModel.Run}")

def SetupMuonStandaloneConfigFlags():
    """
    Setup flags necessary for Muon standalone.
    """
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Detector.GeometryMDT   = True 
    flags.Detector.GeometryTGC   = True
    
    flags.Detector.GeometryRPC   = True
    # TODO: disable these for now, to be determined if needed
    flags.Detector.GeometryCalo  = False
    flags.Detector.GeometryID    = False

    # FIXME This is temporary. I think it can be removed with some other refactoring
    flags.Muon.makePRDs          = False

    flags.Exec.MaxEvents = 20 # Set default to 20 if not overridden
    flags.Scheduler.ShowDataDeps = True
    flags.Scheduler.CheckDependencies = True
    flags.Scheduler.ShowDataFlow = True
    flags.Scheduler.ShowControlFlow = True
    flags.Concurrency.NumThreads  = 1
    flags.Concurrency.NumConcurrentEvents = 1
    flags.Exec.FPE= 500

    flags.fillFromArgs()

    if flags.Input.Files == ['_ATHENA_GENERIC_INPUTFILE_NAME_'] or len(flags.Input.Files) == 0:
        # If something is set from an arg (i.e. the command line), this takes priority
        from AthenaConfiguration.TestDefaults import defaultTestFiles
        flags.Input.Files = defaultTestFiles.ESD_RUN3_MC 

    configureCondTag(flags)
   
    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
    setupDetectorFlags(flags)

    if flags.Output.ESDFileName == '':
        flags.Output.ESDFileName='newESD.pool.root'
  
    flags.lock()
    flags.dump(evaluate = True)
    return flags
    
def SetupMuonStandaloneCA(flags):
    # When running from a pickled file, athena inserts some services automatically. So only use this if running now.
 
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)
    msgService = cfg.getService('MessageSvc')
    msgService.Format = "S:%s E:%e % F%128W%S%7W%R%T  %0W%M"
    msgService.debugLimit = 2147483647
    msgService.verboseLimit = 2147483647
    msgService.infoLimit = 2147483647
   
 
    from AthenaConfiguration.Enums import Format
    if not flags.Input.Files:
        # No input file --- skip setting up event reading.
        pass
    elif flags.Input.Format is Format.POOL:
        from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
        cfg.merge(PoolReadCfg(flags))
    elif flags.Input.Format == Format.BS:
        from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
        cfg.merge(ByteStreamReadCfg(flags)) 

    if flags.Input.isMC:
        if ("TruthEvents" in flags.Input.Collections):
            from xAODTruthCnv.RedoTruthLinksConfig import RedoTruthLinksAlgCfg
            cfg.merge( RedoTruthLinksAlgCfg(flags) )
        elif ("TruthEvent" in flags.Input.Collections):
            from xAODTruthCnv.xAODTruthCnvConfig import GEN_AOD2xAODCfg
            cfg.merge(GEN_AOD2xAODCfg(flags))

    ## Standard setup of the geometry
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    cfg.merge(MuonGeoModelCfg(flags))
    from MuonConfig.MuonGeometryConfig import MuonIdHelperSvcCfg
    cfg.merge(MuonIdHelperSvcCfg(flags))    

    return cfg
    
def SetupMuonStandaloneOutput(cfg, flags, itemsToRecord):
    # Set up output
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg, outputStreamName

    cfg.merge( OutputStreamCfg( flags, 'ESD', ItemList=itemsToRecord) )
    outstream = cfg.getEventAlgo(outputStreamName("ESD"))
    outstream.ForceRead = True

    # Fix for ATLASRECTS-5151
    from TrkEventCnvTools.TrkEventCnvToolsConfig import (
        TrkEventCnvSuperToolCfg)
    cfg.merge(TrkEventCnvSuperToolCfg(flags))

