#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import Format
from AthenaCommon.Constants import DEBUG, INFO

## Small class to hold the names for cache containers, should help to avoid copy / paste errors
class MuonPrdCacheNames(object):
    MdtCache       = "MdtPrdCache"
    CscCache       = "CscPrdCache"
    CscStripCache  = "CscStripPrdCache"
    RpcCache       = "RpcPrdCache"
    TgcCache       = "TgcPrdCache"
    sTgcCache      = "sTgcPrdCache"
    MmCache        = "MmPrdCache"
    RpcCoinCache   = "RpcCoinCache"
    TgcCoinCache   = "TgcCoinCache"


## This configuration function creates the IdentifiableCaches for PRD
#
# The function returns a ComponentAccumulator which should be loaded first
# If a configuration wants to use the cache, they need to use the same names as defined here
def MuonPrdCacheCfg(flags):
    # Use MuonGeometryFlags to identify which configuration is being used

    acc = ComponentAccumulator()

    MuonPRDCacheCreator=CompFactory.MuonPRDCacheCreator
    cacheCreator = MuonPRDCacheCreator(CscStripCacheKey  = (MuonPrdCacheNames.CscStripCache if flags.Detector.GeometryCSC else ""),
                                       MdtCacheKey       = MuonPrdCacheNames.MdtCache,
                                       CscCacheKey       = (MuonPrdCacheNames.CscCache if flags.Detector.GeometryCSC else ""),
                                       RpcCacheKey       = MuonPrdCacheNames.RpcCache,
                                       TgcCacheStr       = MuonPrdCacheNames.TgcCache,
                                       sTgcCacheKey      = (MuonPrdCacheNames.sTgcCache if flags.Detector.GeometrysTGC else ""),
                                       MmCacheKey        = (MuonPrdCacheNames.MmCache if flags.Detector.GeometryMM else ""),
                                       TgcCoinCacheStr   = MuonPrdCacheNames.TgcCoinCache,
                                       RpcCoinCacheKey   = MuonPrdCacheNames.RpcCoinCache,
                                       )

    acc.addEventAlgo( cacheCreator, primary=True )
    return acc

def MuonRdoToPrepDataAlgCfg(flags, name="MuonRdoToPrepDataAlg", **kwargs):
    result = ComponentAccumulator()
    # Make sure muon geometry is configured
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    result.merge(MuonGeoModelCfg(flags))
    
    kwargs.setdefault("DoSeededDecoding", flags.Trigger.doHLT )
    the_alg = CompFactory.MuonRdoToPrepDataAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result
## This configuration function sets up everything for decoding RPC RDO to PRD conversion
#
# The function returns a ComponentAccumulator and the data-converting algorithm, which should be added to the right sequence by the user
def RpcRDODecodeCfg(flags, name="RpcRdoToRpcPrepData", RDOContainer = None, **kwargs):
    acc = ComponentAccumulator()
    # We need the RPC cabling to be setup
    from MuonConfig.MuonCablingConfig import RPCCablingConfigCfg
    acc.merge(RPCCablingConfigCfg(flags))

    # Conditions not needed for online

    tool_kwargs={}
    if not flags.Input.isMC:
        tool_kwargs["reduceCablingOverlap"] = True
        tool_kwargs["produceRpcCoinDatafromTriggerWords"] = True
        tool_kwargs["overlap_timeTolerance"] = 1000
        tool_kwargs["solvePhiAmbiguities"] = True
        tool_kwargs["etaphi_coincidenceTime"] = 1000
    if not flags.Trigger.doHLT:
        tool_kwargs["RpcPrdContainerCacheKey"] = ""
        tool_kwargs["RpcCoinDataContainerCacheKey"] = ""

        from MuonConfig.MuonCondAlgConfig import RpcCondDbAlgCfg
        acc.merge(RpcCondDbAlgCfg(flags))
    else:
        tool_kwargs["RPCInfoFromDb"] = False
    
    if RDOContainer:
        tool_kwargs["RDOContainer"] = RDOContainer
    
    if not  flags.Muon.enableNRPC:
        tool_kwargs["NrpcInputCollection"] = ""

    
    #Setup RPC RDO decoder to be consistent with RPC readout settings
    rpcrdo_decode = CompFactory.Muon.RpcRDO_Decoder("RpcRDO_Decoder", BCZERO=flags.Trigger.L1MuonSim.RPCNBCZ)
    if flags.Muon.MuonTrigger:
        tool_kwargs["RdoDecoderTool"]=rpcrdo_decode
    # Get the RDO -> PRD tool
    kwargs.setdefault("DecodingTool", CompFactory.Muon.RpcRdoToPrepDataToolMT(name="RpcPrepDataProviderTool",
                                                                              **tool_kwargs))

    # add RegSelTool
    from RegionSelector.RegSelToolConfig import regSelTool_RPC_Cfg
    kwargs.setdefault("RegSelector", acc.popToolsAndMerge(regSelTool_RPC_Cfg(flags)))
    kwargs.setdefault("useROBs", False)
    
    # Add the RDO -> PRD alorithm
    acc.merge(MuonRdoToPrepDataAlgCfg(flags, name, **kwargs))
    return acc


def TgcRDODecodeCfg(flags, name="TgcRdoToTgcPrepData", RDOContainer = None,  **kwargs):
    acc = ComponentAccumulator()

    # We need the TGC cabling to be setup
    from MuonConfig.MuonCablingConfig import TGCCablingConfigCfg
    acc.merge(TGCCablingConfigCfg(flags))

    # Get the RDO -> PRD tool
    tool_args = {}
    if not flags.Trigger.doHLT:
       tool_args.setdefault("PrdCacheString", "")
       tool_args.setdefault("CoinCacheString", "")
    
    if RDOContainer: tool_args.setdefault("RDOContainer", RDOContainer)
    kwargs.setdefault("DecodingTool", CompFactory.Muon.TgcRdoToPrepDataToolMT(name="TgcPrepDataProviderTool", **tool_args))

    # add RegSelTool
    from RegionSelector.RegSelToolConfig import regSelTool_TGC_Cfg
    kwargs.setdefault("RegSelector", acc.popToolsAndMerge(regSelTool_TGC_Cfg(flags)))
    kwargs.setdefault("useROBs", False)
   

    ## Add the RDO -> PRD alorithm
    acc.merge(MuonRdoToPrepDataAlgCfg(flags, name, **kwargs))
    return acc

def TgcPrepDataReplicationToolAllBCto3BC(flags, name = "TgcPrepDataAllBCto3BCTool", **kwargs):
    acc = ComponentAccumulator()
    the_tool = CompFactory.Muon.TgcPrepDataReplicationToolAllBCto3BC(name, **kwargs)
    acc.setPrivateTools(the_tool)
    return acc
    
def TgcPrepDataAllBCto3BCCfg(flags, name="TgcPrepDataAllTo3Replicator", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("Tool", acc.popToolsAndMerge(TgcPrepDataReplicationToolAllBCto3BC(flags)))
    acc.addEventAlgo(CompFactory.Muon.TgcPrepDataReplicationAlg(name, **kwargs))
    return acc


def StgcRdoToPrepDataToolCfg(flags, name="STGC_PrepDataProviderTool", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("PrdCacheKey" , MuonPrdCacheNames.sTgcCache if flags.Muon.MuonTrigger else "")

    from MuonConfig.MuonRecToolsConfig import SimpleSTgcClusterBuilderToolCfg
    kwargs.setdefault("ClusterBuilderTool",result.popToolsAndMerge(SimpleSTgcClusterBuilderToolCfg(flags)))
    from MuonConfig.MuonCalibrationConfig import NSWCalibToolCfg
    kwargs.setdefault("NSWCalibTool", result.popToolsAndMerge(NSWCalibToolCfg(flags)))    
    the_tool = CompFactory.Muon.sTgcRdoToPrepDataToolMT(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result



def StgcRDODecodeCfg(flags, name="StgcRdoToStgcPrepData", **kwargs):
    acc = ComponentAccumulator()
    # Get the RDO -> PRD tool
    kwargs.setdefault("DecodingTool", acc.popToolsAndMerge(StgcRdoToPrepDataToolCfg(flags)))
    # add RegSelTool
    from RegionSelector.RegSelToolConfig import regSelTool_STGC_Cfg
    kwargs.setdefault("RegSelector", acc.popToolsAndMerge(regSelTool_STGC_Cfg(flags)))
    kwargs.setdefault("useROBs", False)
    ## Add the RDO -> PRD alorithm
    acc.merge(MuonRdoToPrepDataAlgCfg(flags, name, **kwargs))
    return acc




def MMRdoToPrepDataToolCfg(flags, name="MmRdoToPrepDataTool", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("PrdCacheKey" , MuonPrdCacheNames.MmCache if flags.Trigger.doHLT  else "")

    from MuonConfig.MuonRecToolsConfig import SimpleMMClusterBuilderToolCfg
    kwargs.setdefault("ClusterBuilderTool",result.popToolsAndMerge(SimpleMMClusterBuilderToolCfg(flags)))
    from MuonConfig.MuonCalibrationConfig import NSWCalibToolCfg
    kwargs.setdefault("NSWCalibTool", result.popToolsAndMerge(NSWCalibToolCfg(flags)))    
    the_tool = CompFactory.Muon.MmRdoToPrepDataToolMT(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result

def MMRDODecodeCfg(flags, name="MM_RdoToMM_PrepData", **kwargs):
    acc = ComponentAccumulator()
    ## Get the RDO -> PRD tool
    kwargs.setdefault("DecodingTool", acc.popToolsAndMerge(MMRdoToPrepDataToolCfg(flags)))
    # add RegSelTool
    from RegionSelector.RegSelToolConfig import regSelTool_MM_Cfg
    kwargs.setdefault("RegSelector", acc.popToolsAndMerge(regSelTool_MM_Cfg(flags)))
    kwargs.setdefault("useROBs", False)
    # Add the RDO -> PRD alorithm
    acc.merge(MuonRdoToPrepDataAlgCfg(flags, name, **kwargs))
    return acc


def MdtRDODecodeCfg(flags, name="MdtRdoToMdtPrepData", RDOContainer = None, **kwargs):
    acc = ComponentAccumulator()
    from MuonConfig.MuonCalibrationConfig import MdtCalibrationToolCfg

    # We need the MDT cabling to be setup
    from MuonConfig.MuonCablingConfig import MDTCablingConfigCfg
    acc.merge(MDTCablingConfigCfg(flags))

    tool_kwargs = {}
    tool_kwargs["MdtxAODKey"] =  "MDT_DriftCircles" if flags.Muon.writexAODPRD else ""
    tool_kwargs["UseTwin"] = True
    tool_kwargs["CalibrationTool"] = acc.popToolsAndMerge(MdtCalibrationToolCfg(flags, TimeWindowSetting = 2, DoPropagationCorrection = False))
    if RDOContainer: tool_kwargs["RDOContainer"] = RDOContainer
    # Get the RDO -> PRD tool
    kwargs.setdefault("DecodingTool", CompFactory.Muon.MdtRdoToPrepDataToolMT(name="MdtPrepDataProviderTool", **tool_kwargs))

    # add RegSelTool
    from RegionSelector.RegSelToolConfig import regSelTool_MDT_Cfg
    kwargs.setdefault("RegSelector", acc.popToolsAndMerge(regSelTool_MDT_Cfg(flags)))

    # Add the RDO -> PRD alorithm
    acc.merge(MuonRdoToPrepDataAlgCfg(flags, name, **kwargs))
    return acc


def CscRDODecodeCfg(flags, name="CscRdoToCscPrepData", RDOContainer = None, **kwargs):
    acc = ComponentAccumulator()

    # We need the CSC cabling to be setup
    from MuonConfig.MuonCablingConfig import CSCCablingConfigCfg # Not yet been prepared
    acc.merge(CSCCablingConfigCfg(flags))

    from MuonConfig.MuonCondAlgConfig import CscCondDbAlgCfg
    acc.merge(CscCondDbAlgCfg(flags))

    # Get the RDO -> PRD tool
    
    kwargs.setdefault("useROBs", False)
    kwargs.setdefault("DecodingTool", CompFactory.Muon.CscRdoToCscPrepDataToolMT(name="CscPrepDataProviderTool"))
    if RDOContainer:
        kwargs["DecodingTool"].RDOContainer = RDOContainer
    # add RegSelTool
    from RegionSelector.RegSelToolConfig import regSelTool_CSC_Cfg
    kwargs.setdefault("RegSelector", acc.popToolsAndMerge(regSelTool_CSC_Cfg(flags)))

    # Add the RDO -> PRD alorithm
    acc.merge(MuonRdoToPrepDataAlgCfg(flags, name, **kwargs))
    return acc


def CscClusterBuildCfg(flags, name="CscThresholdClusterBuilder"):
    acc = ComponentAccumulator()
    from MuonConfig.MuonGeometryConfig import MuonIdHelperSvcCfg
    from MuonConfig.MuonSegmentFindingConfig import CalibCscStripFitterCfg, QratCscClusterFitterCfg, CscAlignmentTool
    from MuonConfig.MuonCalibrationConfig import CscCalibToolCfg

    # Get cluster creator tool

    MuonIdHelperSvc = acc.getPrimaryAndMerge( MuonIdHelperSvcCfg(flags) )
    CalibCscStripFitter = acc.getPrimaryAndMerge( CalibCscStripFitterCfg(flags) )
    QratCscClusterFitter = acc.getPrimaryAndMerge( QratCscClusterFitterCfg(flags) )
    SimpleCscClusterFitter = CompFactory.SimpleCscClusterFitter(CscAlignmentTool = CscAlignmentTool(flags) )
    CscSplitClusterFitter = CompFactory.CscSplitClusterFitter(  precision_fitter = QratCscClusterFitter, 
                                                                default_fitter = SimpleCscClusterFitter )
    CscCalibTool        = acc.getPrimaryAndMerge( CscCalibToolCfg(flags) )
    CscThresholdClusterBuilderTool=CompFactory.CscThresholdClusterBuilderTool
    CscClusterBuilderTool = CscThresholdClusterBuilderTool(name = "CscThresholdClusterBuilderTool" , 
                                                           MuonIdHelperSvc = MuonIdHelperSvc,
                                                           strip_fitter = CalibCscStripFitter,
                                                           precision_fitter = QratCscClusterFitter,
                                                           default_fitter = SimpleCscClusterFitter,
                                                           split_fitter = CscSplitClusterFitter,
                                                           cscCalibTool = CscCalibTool)

    #CSC cluster building
    CscThresholdClusterBuilder=CompFactory.CscThresholdClusterBuilder
    CscClusterBuilder = CscThresholdClusterBuilder(name            = name,
                                                   cluster_builder = CscClusterBuilderTool,
                                                   MuonIdHelperSvc = MuonIdHelperSvc
                                                    )
    acc.addEventAlgo(CscClusterBuilder)

    return acc


def MuonPRD_MultiTruthMakerCfg(flags, name="MuonPRD_MultiTruthMaker", **kwargs):
    from MuonConfig.MuonGeometryConfig import MuonIdHelperSvcCfg
    acc = MuonIdHelperSvcCfg(flags)
    kwargs.setdefault("TGC_PrepRawDataContainer", 'TGC_MeasurementsAllBCs' if not flags.Muon.useTGCPriorNextBC else 'TGC_Measurements')
    # The availability of the other containers, e.g. CSC is controlled in MuonPRD_MultiTruthMaker::initialize() by checking m_idHelperSvc
    acc.addEventAlgo(CompFactory.MuonPRD_MultiTruthMaker(name, **kwargs))
    return acc


def MuonRDOtoPRDConvertorsCfg(flags):
    # Schedule RDO conversion
    acc = ComponentAccumulator()

    if flags.Detector.GeometryMDT:
        acc.merge(MdtRDODecodeCfg(flags))

    if flags.Detector.GeometryRPC:
        acc.merge(RpcRDODecodeCfg(flags))

    if flags.Detector.GeometryTGC:
        acc.merge(TgcRDODecodeCfg(flags))

    if flags.Detector.GeometrysTGC:
        acc.merge(StgcRDODecodeCfg(flags))

    if flags.Detector.GeometryMM:
        acc.merge(MMRDODecodeCfg(flags))


    if flags.Detector.GeometryCSC:
        acc.merge(CscRDODecodeCfg(flags))
        acc.merge(CscClusterBuildCfg(flags))

    if flags.Input.isMC:
        acc.merge(MuonPRD_MultiTruthMakerCfg(flags))

    return acc


# This function runs the decoding on a data file
def muonRdoDecodeTestData( forTrigger = False ):
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2

    # Set global tag by hand for now
    flags.IOVDb.GlobalTag = "CONDBR2-BLKPA-2018-13"#"CONDBR2-BLKPA-2015-17"
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2

    flags.lock()
    flags.dump()

    from AthenaCommon.Logging import log

    log.setLevel(INFO)
    log.info('About to setup Raw data decoding')

    cfg=ComponentAccumulator()


    # Add the MuonCache to ComponentAccumulator for trigger/RoI testing mode
    if forTrigger:
        # cache creators loaded independently
        from MuonConfig.MuonBytestreamDecodeConfig import MuonCacheCfg
        cfg.merge( MuonCacheCfg(flags) )

    if flags.Input.Format is Format.BS:
        from MuonConfig.MuonBytestreamDecodeConfig import MuonByteStreamDecodersCfg
        cfg.merge( MuonByteStreamDecodersCfg( flags) )

    cfg.merge( MuonRDOtoPRDConvertorsCfg( flags) )

    log.info('Print Config')
    cfg.printConfig(withDetails=True)

    if forTrigger:
        pklName = 'MuonRdoDecode_Cache.pkl'
    else:
        pklName = 'MuonRdoDecode.pkl'

    # Store config as pickle
    log.info('Save Config')
    with open(pklName,'wb') as f:
        cfg.store(f)
        f.close()
    return cfg

# This function runs the decoding on a MC file
def muonRdoDecodeTestMC():

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TriggerTest/valid1.110401.PowhegPythia_P2012_ttbar_nonallhad.recon.RDO.e3099_s2578_r7572_tid07644622_00/RDO.07644622._000001.pool.root.1"]

    flags.lock()
    flags.dump()

    from AthenaCommon.Logging import log

    log.setLevel(DEBUG)
    log.info('About to setup Rpc RDO data decoding')

    cfg=ComponentAccumulator()

    # We are reading a pool file for this test
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))

    # Schedule RDO conversion
    cfg.merge( MuonRDOtoPRDConvertorsCfg( flags) )

    log.info('Print Config')
    cfg.printConfig(withDetails=True)

    # Store config as pickle
    log.info('Save Config')
    with open('MuonRdoDecode.pkl','wb') as f:
        cfg.store(f)
        f.close()
    return cfg

if __name__=="__main__":
    # To run this, do e.g.
    # python ../athena/MuonSpectrometer/MuonConfig/python/MuonRdoDecodeConfig.py
    cfg = muonRdoDecodeTestData()
    #muonRdoDecodeTestMC()


