# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def NRPCCablingConfigCfg(flags, name = "MuonNRPC_CablingAlg", **kwargs):
    result = ComponentAccumulator()

    ### Add DB folder
    kwargs.setdefault("JSONFile", "")
    if len(kwargs["JSONFile"]) == 0:
        from IOVDbSvc.IOVDbSvcConfig import addFolders
        dbName = 'RPC_OFL' if flags.Input.isMC else 'RPC'
        cablingFolder = "/RPC/NCABLING/JSON" if flags.Input.isMC else "/RPC/Onl/NCABLING/JSON"
        cablingTag = "RpcNcablingJson-RUN3-08"
        from AthenaConfiguration.Enums import LHCPeriod
        if flags.Muon.usePhaseIIGeoSetup and flags.Input.isMC:  
            if flags.GeoModel.Run <= LHCPeriod.Run3:   
                cablingTag = "RpcNcablingJson-RUN3-FantasyCabling-5"
            else:
                cablingTag = "RpcNcablingJson-RUN4-FantasyCabling-6"

        result.merge(addFolders(flags, [cablingFolder], detDb=dbName, className='CondAttrListCollection', tag=cablingTag))
        kwargs.setdefault("MapFolders",  cablingFolder)
    
    ### Cabling algorithm setup
    NRPCCablingAlg = CompFactory.Muon.NRpcCablingAlg(name, **kwargs)

    result.addCondAlgo( NRPCCablingAlg, primary= True)
    return result


def RPCCablingConfigCfg(flags):
    acc = ComponentAccumulator()
    if not flags.Detector.GeometryRPC: return acc
    if flags.Muon.enableNRPC:
        acc.merge(NRPCCablingConfigCfg(flags))
    acc.merge(RPCLegacyCablingConfigCfg(flags))
    return acc
    
def RPCLegacyCablingConfigCfg(flags):
    acc = ComponentAccumulator()
    dbName = 'RPC_OFL' if flags.Input.isMC else 'RPC'
    dbRepo="MuonRPC_Cabling/ATLAS.data"
    rpcCabMap="/RPC/CABLING/MAP_SCHEMA"
    rpcCabMapCorr="/RPC/CABLING/MAP_SCHEMA_CORR"
    rpcTrigEta="/RPC/TRIGGER/CM_THR_ETA"
    rpcTrigPhi="/RPC/TRIGGER/CM_THR_PHI"

    from IOVDbSvc.IOVDbSvcConfig import addFolders
    acc.merge(addFolders(flags, [rpcCabMap,rpcCabMapCorr], dbName, className='CondAttrListCollection' ))

    acc.merge(addFolders(flags, [rpcTrigEta,rpcTrigPhi], dbName, className='CondAttrListCollection'))

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

    acc.merge(MuonTGC_CablingSvcCfg(flags))

    from IOVDbSvc.IOVDbSvcConfig import addFolders
    dbName = 'TGC_OFL' if flags.Input.isMC else 'TGC'
    acc.merge(addFolders(flags, '/TGC/CABLING/MAP_SCHEMA', dbName))

    return acc

# This should be checked by experts since I just wrote it based on 
# athena/MuonSpectrometer/MuonCnv/MuonCnvExample/python/MuonCablingConfig.py
def MDTCablingConfigCfg(flags, name = "MuonMDT_CablingAlg", **kwargs):
    acc = ComponentAccumulator()
    if not flags.Detector.GeometryMDT: return acc
    from AthenaConfiguration.Enums import LHCPeriod
    
    kwargs.setdefault("UseJSONFormat", flags.Muon.usePhaseIIGeoSetup and \
                                       flags.GeoModel.Run >= LHCPeriod.Run4)

    kwargs.setdefault("MezzanineJSON", "")
    kwargs.setdefault("CablingJSON", "")

    kwargs.setdefault("isRun3", flags.GeoModel.Run >= LHCPeriod.Run3 )
    from IOVDbSvc.IOVDbSvcConfig import addFolders
    if len(kwargs["MezzanineJSON"]) == 0 and len(kwargs["CablingJSON"]) == 0:
        if flags.Input.isMC is True:
            dbTagMezz = None 
            dbTagSchema = None
            if flags.Muon.usePhaseIIGeoSetup and \
               flags.GeoModel.Run >= LHCPeriod.Run4: 
                dbTagMezz = "MDTMezMapSchemaJSON_RUN4_FantasyCabling_1"
                dbTagSchema = "MDTCablingMapSchemaJSON_RUN4_FantasyCabling_1"
            elif flags.GeoModel.Run >= LHCPeriod.Run4:
                dbTagSchema = "MDTOflCablingMapSchema_RUN124_MC15_02"
                dbTagMezz = "MDTOflCablingMezzanineSchema_RUN124_MC15_02"
            if kwargs["UseJSONFormat"]:
                kwargs.setdefault("MapFolders", "/MDT/CABLING/MAP_SCHEMA_JSON")
                kwargs.setdefault("MezzanineFolders", "/MDT/CABLING/MEZZANINE_SCHEMA_JSON")
            else:
                kwargs.setdefault("MapFolders", "/MDT/Ofl/CABLING/MAP_SCHEMA")
                kwargs.setdefault("MezzanineFolders", "/MDT/Ofl/CABLING/MEZZANINE_SCHEMA")
            acc.merge( addFolders( flags, [kwargs["MapFolders"]], 'MDT_OFL',  
                                className="CondAttrListCollection", tag = dbTagSchema))
            acc.merge( addFolders( flags, [kwargs["MezzanineFolders"]], 'MDT_OFL',  
                                className="CondAttrListCollection", tag = dbTagMezz) )
        else:
            if kwargs["UseJSONFormat"]:
                kwargs.setdefault("MapFolders", "/MDT/CABLING/MAP_SCHEMA_JSON")
                kwargs.setdefault("MezzanineFolders", "/MDT/CABLING/MEZZANINE_SCHEMA_JSON")
            else:
                kwargs.setdefault("MapFolders", "/MDT/CABLING/MAP_SCHEMA")
                kwargs.setdefault("MezzanineFolders", "/MDT/CABLING/MEZZANINE_SCHEMA")
            acc.merge( addFolders( flags, [kwargs["MapFolders"], kwargs["MezzanineFolders"]], 'MDT', 
                                    className="CondAttrListCollection") )

    
    MDTCablingAlg = CompFactory.MuonMDT_CablingAlg(name, **kwargs)
    acc.addCondAlgo( MDTCablingAlg, primary = True )
   
    return acc

def MdtTwinTubeMapCondAlgCfg(flags, name="MdtTwinTubeCondAlg", **kwargs):
    result = ComponentAccumulator()
    if not flags.Detector.GeometryMDT: return result
    kwargs.setdefault("JSONFile","")
    if(not kwargs["JSONFile"]):
        kwargs.setdefault("FolderName","/MDT/TWINMAPPING")
        from IOVDbSvc.IOVDbSvcConfig import addFolders
        result.merge(addFolders(flags,[kwargs["FolderName"]], ("MDT_OFL" if flags.Input.isMC else "MDT"),className="CondAttrListCollection", tag="MDTTwinMapping_compactFormat_Run123"))
    else:
        kwargs["FolderName"] = ""

    the_alg = CompFactory.Muon.TwinTubeMappingCondAlg(name, **kwargs)
    result.addCondAlgo(the_alg, primary = True)    
    return result

# This should be checked by experts 
def CSCCablingConfigCfg(flags):
    acc = ComponentAccumulator()
    if not flags.Detector.GeometryCSC: return acc
    CSCcablingSvc=CompFactory.CSCcablingSvc
    cscCablingSvc = CSCcablingSvc()

    acc.addService( cscCablingSvc, primary=True )

    return acc

def MmCablingCfg(flags, name = "MuonMm_CablingAlg", **kwargs):
    result = ComponentAccumulator()
    #### Only setup the MM Cabling algorithm for data
    if flags.Input.isMC or (not flags.Detector.GeometryMM): 
        return result

    from IOVDbSvc.IOVDbSvcConfig import addFolders
    cablingFolder = ["/MDT/MM/CABLING" if not flags.Common.isOnline else "/MDT/Onl/MM/CABLING"]
    kwargs.setdefault("CablingFolder",cablingFolder)
    result.merge(addFolders(flags, kwargs["CablingFolder"], detDb=("MDT_OFL" if not flags.Common.isOnline else "MDT_ONL"), className="CondAttrListCollection"))

    the_alg = CompFactory.MuonNSW_CablingAlg(name, **kwargs)
    result.addCondAlgo(the_alg, primary = True)
    return result

def sTgcCablingCfg(flags, name = "MuonsTgc_CablingAlg", **kwargs):
    result = ComponentAccumulator()
    #### Only setup the MM Cabling algorithm for data
    if flags.Input.isMC or (not flags.Detector.GeometrysTGC): 
        return result

    from IOVDbSvc.IOVDbSvcConfig import addFolders
    cablingFolder = ["/TGC/NSW/CABLING" if not flags.Common.isOnline else "/TGC/Onl/NSW/CABLING"]
    kwargs.setdefault("CablingFolder",cablingFolder)
    kwargs.setdefault("WriteKey",'stgcCablingMap')
    result.merge(addFolders(flags, kwargs["CablingFolder"], detDb=("TGC_OFL" if not flags.Common.isOnline else "TGC_ONL"), className="CondAttrListCollection"))

    the_alg = CompFactory.MuonNSW_CablingAlg(name, **kwargs)
    result.addCondAlgo(the_alg, primary = True)
    return result



#All the cabling configs together (convenience function)
def MuonCablingConfigCfg(flags):
    acc = ComponentAccumulator()
    acc.merge( RPCCablingConfigCfg(flags) )
    acc.merge( TGCCablingConfigCfg(flags) )

    acc.merge( MDTCablingConfigCfg(flags) )

    acc.merge( CSCCablingConfigCfg(flags) )

    acc.merge(MmCablingCfg(flags))
    acc.merge(sTgcCablingCfg(flags))

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
