# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# This file is just for shared functions etc used by this package.
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    
def SetupMuonStandaloneConfigFlags( default_input = ['/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecExRecoTest/ESD.16747874._000011_100events.pool.root']):
    """
    Setup flags necessary for Muon standalone.
    """
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Detector.GeometryMDT   = True 
    flags.Detector.GeometryTGC   = True
    flags.Detector.GeometryCSC   = True     
    flags.Detector.GeometryRPC   = True
    # TODO: disable these for now, to be determined if needed
    flags.Detector.GeometryCalo  = False
    flags.Detector.GeometryID    = False

    # FIXME This is temporary. I think it can be removed with some other refactoring
    flags.Muon.makePRDs          = False
    
    args = flags.fillFromArgs()

    if flags.Input.Files == ['_ATHENA_GENERIC_INPUTFILE_NAME_'] :
        # If something is set from an arg (i.e. the command line), this takes priority
        flags.Input.Files = default_input

    if flags.Output.ESDFileName == '':
        flags.Output.ESDFileName='newESD.pool.root'
    else:
        print('ESD = ', flags.Output.ESDFileName )
    flags.lock()
    flags.dump()
    return args, flags
    
def SetupMuonStandaloneCA(args,flags):
    # When running from a pickled file, athena inserts some services automatically. So only use this if running now.
    if not args.config_only:
        from AthenaConfiguration.MainServicesConfig import MainServicesCfg
        cfg = MainServicesCfg(flags)
        msgService = cfg.getService('MessageSvc')
        msgService.Format = "S:%s E:%e % F%128W%S%7W%R%T  %0W%M"
    else:
        cfg=ComponentAccumulator()

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))

    if flags.Input.isMC:
        from xAODTruthCnv.xAODTruthCnvConfig import GEN_AOD2xAODCfg
        cfg.merge(GEN_AOD2xAODCfg(flags))

    return cfg
    
def SetupMuonStandaloneOutput(cfg, flags, itemsToRecord):
    # Set up output
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg

    cfg.merge( OutputStreamCfg( flags, 'ESD', ItemList=itemsToRecord) )
    outstream = cfg.getEventAlgo("OutputStreamESD")
    outstream.ForceRead = True

    # Fix for ATLASRECTS-5151
    from TrkEventCnvTools.TrkEventCnvToolsConfig import (
        TrkEventCnvSuperToolCfg)
    cfg.merge(TrkEventCnvSuperToolCfg(flags))

