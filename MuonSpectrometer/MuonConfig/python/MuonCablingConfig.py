# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def NRPCCablingConfigCfg(flags, name = "MuonNRPC_CablingAlg", **kwargs):
    result = ComponentAccumulator()
    if not flags.Muon.enableNRPC:
        return result

    from IOVDbSvc.IOVDbSvcConfig import addFolders
    dbName = 'RPC_OFL' if flags.Input.isMC else 'RPC'
    cablingFolder = " /RPC/NCABLING/JSON"
    cablingTag = "RpcNcablingJson-RUN3-01"
    result.merge(addFolders(flags, [cablingFolder], detDb=dbName, className='CondAttrListCollection', tag=cablingTag))
    ### Add the database configuration here
    NRPCCablingAlg = CompFactory.MuonNRPC_CablingAlg(name, **kwargs)
    result.addCondAlgo( NRPCCablingAlg, primary= True)
    return result


def RPCCablingConfigCfg(flags):
    acc = ComponentAccumulator()
    if not flags.Detector.GeometryRPC: return acc
    acc.merge(NRPCCablingConfigCfg(flags))

    dbName = 'RPC_OFL' if flags.Input.isMC else 'RPC'
    dbRepo="MuonRPC_Cabling/ATLAS.data"
    rpcCabMap="/RPC/CABLING/MAP_SCHEMA"
    rpcCabMapCorr="/RPC/CABLING/MAP_SCHEMA_CORR"
    rpcTrigEta="/RPC/TRIGGER/CM_THR_ETA"
    rpcTrigPhi="/RPC/TRIGGER/CM_THR_PHI"

    # This block with conditions override is only used in Trigger and Reco, and only needed until mid-May 2022.
    # See ATR-25059 for discussion. To avoid this block being executed in simulation/digitization,
    # skip this if ProductionStep is not Reconstruction or Default (i.e. unset)
    from AthenaConfiguration.Enums import ProductionStep
    if flags.Common.ProductionStep in [ProductionStep.Reconstruction, ProductionStep.Default] and \
            flags.Trigger.doLVL1 and flags.Trigger.enableL1MuonPhase1:
        # Run3 trigger roads are not avaialble in the global tag yet (OFLCOND-MC16-SDR-RUN3-01)
        # Relevant folder tags are set for now, until new global tag (RUN3-02) becomes avaialble
        rpcTrigEta="/RPC/TRIGGER/CM_THR_ETA <tag>RPCTriggerCMThrEta_RUN12_MC16_04</tag> <forceRunNumber>330000</forceRunNumber>"
        rpcTrigPhi="/RPC/TRIGGER/CM_THR_PHI <tag>RPCTriggerCMThrPhi_RUN12_MC16_04</tag> <forceRunNumber>330000</forceRunNumber>"
        from AthenaConfiguration.Enums  import LHCPeriod
        if flags.Input.isMC and flags.GeoModel.Run >= LHCPeriod.Run3:        # from Run3 on geometry
           rpcCabMap="/RPC/CABLING/MAP_SCHEMA <tag>RPCCablingMapSchema_2015-2018Run3-4</tag> <forceRunNumber>330000</forceRunNumber>"
           rpcCabMapCorr="/RPC/CABLING/MAP_SCHEMA_CORR <tag>RPCCablingMapSchemaCorr_2015-2018Run3-4</tag> <forceRunNumber>330000</forceRunNumber>"


    from IOVDbSvc.IOVDbSvcConfig import addFolders
    acc.merge(addFolders(flags, [rpcCabMap,rpcCabMapCorr], dbName, className='CondAttrListCollection' ))
    # Same protection of ProductionStep as above, ATR-25059
    if flags.Common.ProductionStep in [ProductionStep.Reconstruction, ProductionStep.Default] and \
            flags.Trigger.doLVL1 and not flags.Input.isMC:
        # RPC trigger roads in the online database are not up-to-dated
        # Use offline database for now
        # Will switch to online database once online database has been updated (ATR-23465)
        if flags.Trigger.enableL1MuonPhase1:
            acc.merge(addFolders(flags, [rpcTrigEta,rpcTrigPhi], detDb='RPC_OFL', className='CondAttrListCollection', db='OFLP200'))
        else:
            conddbNameOffline = flags.Trigger.L1MuonSim.CondDBOffline if flags.Trigger.L1MuonSim.CondDBOffline != '' else "OFLCOND-MC16-SDR-RUN2-04"
            acc.merge(addFolders(flags, [rpcTrigEta,rpcTrigPhi], detDb='RPC_OFL', className='CondAttrListCollection', tag=conddbNameOffline, db='OFLP200' ))
    else:
        acc.merge(addFolders(flags, [rpcTrigEta,rpcTrigPhi], dbName, className='CondAttrListCollection' ))

    RpcCablingCondAlg=CompFactory.RpcCablingCondAlg
    RpcCablingAlg = RpcCablingCondAlg("RpcCablingCondAlg",DatabaseRepository=dbRepo)
    acc.addCondAlgo( RpcCablingAlg )

    return acc


def TGCCablingDbToolCfg(flags):
    acc = ComponentAccumulator()

    filename = 'ASD2PP_diff_12_OFL.db' if flags.Input.isMC else 'ASD2PP_diff_12_ONL.db'
    acc.setPrivateTools(CompFactory.TGCCablingDbTool(name = "TGCCablingDbTool",
                                                     filename_ASD2PP_DIFF_12 = filename))

    return acc


def MuonTGC_CablingSvcCfg(flags):
    acc = ComponentAccumulator()

    svc = CompFactory.MuonTGC_CablingSvc()
    tool = acc.popToolsAndMerge(TGCCablingDbToolCfg(flags))
    # The same tool is used as a public tool by TGCCableASDToPP and a
    # private tool by MuonTGC_CablingSvc - not great...
    acc.addPublicTool(tool)
    svc.TGCCablingDbTool = tool
    acc.addService(svc, primary = True)

    return acc


def TGCCablingConfigCfg(flags):
    acc = ComponentAccumulator()
    if not flags.Detector.GeometryTGC: return acc
    # No ServiceHandle in TGCcablingServerSvc
    acc.merge(MuonTGC_CablingSvcCfg(flags))

    TGCcablingServerSvc=CompFactory.TGCcablingServerSvc
    TGCCablingSvc = TGCcablingServerSvc()
    acc.addService( TGCCablingSvc, primary=True )

    from IOVDbSvc.IOVDbSvcConfig import addFolders
    dbName = 'TGC_OFL' if flags.Input.isMC else 'TGC'
    acc.merge(addFolders(flags, '/TGC/CABLING/MAP_SCHEMA', dbName))

    return acc

# This should be checked by experts since I just wrote it based on 
# athena/MuonSpectrometer/MuonCnv/MuonCnvExample/python/MuonCablingConfig.py
def MDTCablingConfigCfg(flags, name = "MuonMDT_CablingAlg", **kwargs):
    acc = ComponentAccumulator()
    from AthenaConfiguration.Enums import LHCPeriod
    kwargs.setdefault("isRun3", flags.GeoModel.Run >= LHCPeriod.Run3 )
    MDTCablingAlg = CompFactory.MuonMDT_CablingAlg(name, **kwargs)
   
    from IOVDbSvc.IOVDbSvcConfig import addFolders
    if flags.Input.isMC is True:
        MDTCablingAlg.MapFolders = "/MDT/Ofl/CABLING/MAP_SCHEMA" 
        MDTCablingAlg.MezzanineFolders    = "/MDT/Ofl/CABLING/MEZZANINE_SCHEMA" 
        acc.merge( addFolders( flags, ["/MDT/Ofl/CABLING/MAP_SCHEMA",
                                       "/MDT/Ofl/CABLING/MEZZANINE_SCHEMA"], 'MDT_OFL', className="CondAttrListCollection") )
    else:
        MDTCablingAlg.MapFolders = "/MDT/CABLING/MAP_SCHEMA" 
        MDTCablingAlg.MezzanineFolders    = "/MDT/CABLING/MEZZANINE_SCHEMA" 
        acc.merge( addFolders( flags, ["/MDT/CABLING/MAP_SCHEMA",
                                       "/MDT/CABLING/MEZZANINE_SCHEMA"], 'MDT', className="CondAttrListCollection") )

    acc.addCondAlgo( MDTCablingAlg, primary = True )
   
    return acc


# This should be checked by experts 
def CSCCablingConfigCfg(flags):
    acc = ComponentAccumulator()
    if not flags.Detector.GeometryCSC: return acc
    CSCcablingSvc=CompFactory.CSCcablingSvc
    cscCablingSvc = CSCcablingSvc()

    acc.addService( cscCablingSvc, primary=True )

    return acc

def MicroMegaCablingCfg(flags, name = "MuonMM_CablingAlg", **kwargs):
    result = ComponentAccumulator()
    #### Only setup the MM Cabling algorithm for data
    if flags.Input.isMC or not flags.Detector.GeometryMM: 
        return result

    from IOVDbSvc.IOVDbSvcConfig import addFolders
    cablingFolder = "/MDT/MM/CABLING" if not flags.Common.isOnline else  "/MDT/Onl/MM/CABLING"
    kwargs.setdefault("CablingFolder",cablingFolder)
    result.merge(addFolders(flags,[kwargs["CablingFolder"]], detDb=("MDT_OFL" if not  flags.Common.isOnline else "MDT_ONL"), className="CondAttrListCollection", tag="MmCabling-FrontEndShifts-v1"))

    the_alg = CompFactory.MuonMM_CablingAlg(name, **kwargs)
    result.addCondAlgo(the_alg, primary = True)
    return result
#All the cabling configs together (convenience function)
def MuonCablingConfigCfg(flags):
    acc = ComponentAccumulator()

    result = RPCCablingConfigCfg(flags)
    acc.merge( result )

    result = TGCCablingConfigCfg(flags)
    acc.merge( result )

    result = MDTCablingConfigCfg(flags)
    acc.merge( result )

    result = CSCCablingConfigCfg(flags)
    acc.merge( result )

    acc.merge(MicroMegaCablingCfg(flags))

    return acc

if __name__ == '__main__':
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles, defaultGeometryTags
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags.lock()

    acc = ComponentAccumulator()

    result = MuonCablingConfigCfg(flags)
    acc.merge( result )

    f=open('MuonCabling.pkl','wb')
    acc.store(f)
    f.close()
