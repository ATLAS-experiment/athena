"""Define methods to construct configured RPC Digitization tools and algorithms

Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod, ProductionStep
from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
from MuonConfig.MuonByteStreamCnvTestConfig import RpcDigitToRpcRDOCfg
from MuonConfig.MuonByteStreamCnvTestConfig import NrpcDigitToNrpcRDOCfg
from MuonConfig.MuonCablingConfig import RPCCablingConfigCfg
from DigitizationConfig.TruthDigitizationOutputConfig import TruthDigitizationOutputCfg
from DigitizationConfig.PileUpToolsConfig import PileUpToolsCfg
from DigitizationConfig.PileUpMergeSvcConfig import PileUpMergeSvcCfg, PileUpXingFolderCfg


# The earliest and last bunch crossing times for which interactions will be sent
# to the RpcDigitizationTool.
def RPC_FirstXing():
    return -150


def RPC_LastXing():
    return 125


def RPC_RangeCfg(flags, name="RPC_Range", **kwargs):
    """Return a PileUpXingFolder tool configured for RPC"""
    kwargs.setdefault("FirstXing", RPC_FirstXing())
    kwargs.setdefault("LastXing", RPC_LastXing())
    kwargs.setdefault("CacheRefreshFrequency", 1.0)
    if flags.Muon.usePhaseIIGeoSetup:
        kwargs.setdefault("ItemList", ["xAOD::MuonSimHitContainer#xRpcSimHits",
                                       "xAOD::MuonSimHitAuxContainer#xRpcSimHitsAux."])
    else:
        kwargs.setdefault("ItemList", ["RPCSimHitCollection#RPC_Hits"])

    return PileUpXingFolderCfg(flags, name, **kwargs)



def RPC_DigitizationToolCommonCfg(flags, name="RpcDigitizationTool", **kwargs):
    """Return ComponentAccumulator with configured RpcDigitizationTool"""
    acc = ComponentAccumulator()

    if flags.GeoModel.Run < LHCPeriod.Run3:  # Run 3 and later currently do not use conditions
        from MuonConfig.MuonCondAlgConfig import RpcCondDbAlgCfg
        acc.merge(RpcCondDbAlgCfg(flags))
    
    if flags.Digitization.DoXingByXingPileUp:
        kwargs.setdefault("FirstXing", RPC_FirstXing())
        kwargs.setdefault("LastXing", RPC_LastXing())
    if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        kwargs.setdefault("OutputSDOName", flags.Overlay.BkgPrefix + "RPC_SDO")
    else:
        kwargs.setdefault("OutputSDOName", "RPC_SDO")
 
    from RngComps.RngCompsConfig import AthRNGSvcCfg
    kwargs.setdefault("RndmSvc", acc.getPrimaryAndMerge(AthRNGSvcCfg(flags)))
    kwargs.setdefault("OutputObjectName", "RPC_DIGITS")

    ### For Phase II use the fast digitization tool
    if flags.Muon.usePhaseIIGeoSetup:
        from ActsAlignmentAlgs.AlignmentAlgsConfig import ActsGeometryContextAlgCfg
        acc.merge(ActsGeometryContextAlgCfg(flags))
        kwargs.setdefault("StreamName", "RpcSimForklift")
        kwargs.setdefault("SimHitKey", "xRpcSimHits")
        kwargs.setdefault("EffiDataKey", "")
        the_tool = CompFactory.MuonR4.RpcFastDigiTool(name="RpcDigitizationTool", **kwargs)
        acc.setPrivateTools(the_tool)       
        return acc
    
    # config
    kwargs.setdefault("DeadTime", 100)
    kwargs.setdefault("PatchForRpcTime", True)
    # kwargs.setdefault("PatchForRpcTimeShift", 9.6875)
    kwargs.setdefault("PatchForRpcTimeShift", 12.5)
    kwargs.setdefault("turnON_efficiency", True)
    kwargs.setdefault("turnON_clustersize", True)
    kwargs.setdefault("ClusterSize1_2uncorr", False)
    kwargs.setdefault("CutProjectedTracks", 100)
    kwargs.setdefault("RPCInfoFromDb", True)
    kwargs.setdefault("Efficiency_fromCOOL", True)
    kwargs.setdefault("EfficiencyPatchForBMShighEta", False)
    kwargs.setdefault("ClusterSize_fromCOOL", True)
    kwargs.setdefault("IgnoreRunDependentConfig", False)
    kwargs.setdefault("PhiAndEtaEff_A",[0.938, 0.938, 0.938, 0.938, 0.938, 0.938, 0.938, 0.938, 0.938])
    kwargs.setdefault("OnlyPhiEff_A"  ,[0.022, 0.022, 0.022, 0.022, 0.022, 0.022, 0.022, 0.022, 0.022])
    kwargs.setdefault("OnlyEtaEff_A"  ,[0.022, 0.022, 0.022, 0.022, 0.022, 0.022, 0.022, 0.022, 0.022])
    kwargs.setdefault("PhiAndEtaEff_C",[0.938, 0.938, 0.938, 0.938, 0.938, 0.938, 0.938, 0.938, 0.938])
    kwargs.setdefault("OnlyPhiEff_C"  ,[0.022, 0.022, 0.022, 0.022, 0.022, 0.022, 0.022, 0.022, 0.022])
    kwargs.setdefault("OnlyEtaEff_C"  ,[0.022, 0.022, 0.022, 0.022, 0.022, 0.022, 0.022, 0.022, 0.022])
    kwargs.setdefault("FracClusterSize1_A",   [0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664])
    kwargs.setdefault("FracClusterSize2_A",   [0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986])
    kwargs.setdefault("FracClusterSizeTail_A",[0.13035,  0.13035,  0.13035,  0.13035,  0.13035,  0.13035,  0.13035,  0.13035,  0.13035, 0.13035,  0.13035,  0.13035,  0.13035,  0.13035,  0.13035,  0.13035,  0.13035,  0.13035 ])
    kwargs.setdefault("MeanClusterSizeTail_A",[0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598])
    kwargs.setdefault("FracClusterSize1_C",   [0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664])
    kwargs.setdefault("FracClusterSize2_C",   [0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986])
    kwargs.setdefault("FracClusterSizeTail_C",[0.13035,  0.13035,  0.13035,  0.13035,  0.13035,  0.13035,  0.13035,  0.13035,  0.13035, 0.13035,  0.13035,  0.13035,  0.13035,  0.13035,  0.13035,  0.13035,  0.13035,  0.13035 ])
    kwargs.setdefault("MeanClusterSizeTail_C",[0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598])
    RpcDigitizationTool = CompFactory.RpcDigitizationTool(name, **kwargs)
    acc.setPrivateTools(RpcDigitizationTool)
    return acc


def RPC_DigitizationToolCfg(flags, name="RpcDigitizationTool", **kwargs):
    """Return ComponentAccumulator with configured RpcDigitizationTool"""
    acc = ComponentAccumulator()
    if flags.Digitization.PileUp:
        intervals = []
        if not flags.Digitization.DoXingByXingPileUp:
            intervals += [acc.popToolsAndMerge(RPC_RangeCfg(flags))]
        kwargs.setdefault("PileUpMergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=intervals)).name)
    else:
        kwargs.setdefault("PileUpMergeSvc", '')
    kwargs.setdefault("OnlyUseContainerName", flags.Digitization.PileUp)
    kwargs.setdefault("OutputObjectName", "RPC_DIGITS")
    if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        kwargs.setdefault("OutputSDOName", flags.Overlay.BkgPrefix + "RPC_SDO")
    else:
        kwargs.setdefault("OutputSDOName", "RPC_SDO")
    tool = acc.popToolsAndMerge(RPC_DigitizationToolCommonCfg(flags, name, **kwargs))
    acc.setPrivateTools(tool)
    return acc


def RPC_OverlayDigitizationToolCfg(flags, name="Rpc_OverlayDigitizationTool", **kwargs):
    """Return ComponentAccumulator with RpcDigitizationTool configured for Overlay"""
    kwargs.setdefault("OnlyUseContainerName", False)
    kwargs.setdefault("OutputObjectName", flags.Overlay.SigPrefix + "RPC_DIGITS")
    kwargs.setdefault("OutputSDOName", flags.Overlay.SigPrefix + "RPC_SDO")
    kwargs.setdefault("PileUpMergeSvc", '')
    return RPC_DigitizationToolCommonCfg(flags, name, **kwargs)


def RPC_OutputCfg(flags):
    """Return ComponentAccumulator with Output for RPC. Not standalone."""
    acc = ComponentAccumulator()
    if flags.Output.doWriteRDO:
        ItemList = []
        if not flags.Muon.usePhaseIIGeoSetup:
            ItemList+=["RpcPadContainer#*"]
            if flags.Digitization.EnableTruth:
                ItemList += ["MuonSimDataCollection#*"]
        if flags.Muon.enableNRPC:
            ItemList += [ 'xAOD::NRPCRDOContainer#*' , 'xAOD::NRPCRDOAuxContainer#*' ]
        if flags.Digitization.EnableTruth:           
            ItemList += ["xAOD::MuonSimHitContainer#*RPC_SDO",
                        "xAOD::MuonSimHitAuxContainer#*RPC_SDOAux."]
            acc.merge(TruthDigitizationOutputCfg(flags))
        acc.merge(OutputStreamCfg(flags, "RDO", ItemList))
    return acc


def RPC_DigitizationBasicCfg(flags, **kwargs):
    """Return ComponentAccumulator for RPC digitization"""
    acc = MuonGeoModelCfg(flags)
    if "PileUpTools" not in kwargs:
        PileUpTools = acc.popToolsAndMerge(RPC_DigitizationToolCfg(flags))
        kwargs["PileUpTools"] = PileUpTools
    acc.merge(PileUpToolsCfg(flags, **kwargs))
    return acc


def RPC_OverlayDigitizationBasicCfg(flags, **kwargs):
    """Return ComponentAccumulator with RPC Overlay digitization"""
    acc = MuonGeoModelCfg(flags)

    if flags.Common.ProductionStep != ProductionStep.FastChain:
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        if flags.Muon.usePhaseIIGeoSetup:
            acc.merge(SGInputLoaderCfg(flags,["xAOD::MuonSimHitContainer#xRpcSimHits",
                                              "xAOD::MuonSimHitAuxContainer#xRpcSimHitsAux."]))
        else:
            acc.merge(SGInputLoaderCfg(flags, ["RPCSimHitCollection#RPC_Hits"]))

    kwargs.setdefault("DigitizationTool", acc.popToolsAndMerge(RPC_OverlayDigitizationToolCfg(flags)))

    if flags.Concurrency.NumThreads > 0:
        kwargs.setdefault("Cardinality", flags.Concurrency.NumThreads)

    # Set common overlay extra inputs
    kwargs.setdefault("ExtraInputs", flags.Overlay.ExtraInputs)

    the_alg = CompFactory.MuonDigitizer(name="RPC_OverlayDigitizer", **kwargs)
    acc.addEventAlgo(the_alg)
    return acc


# with output defaults
def RPC_DigitizationCfg(flags, **kwargs):
    """Return ComponentAccumulator for RPC digitization and Output"""
    acc = RPC_DigitizationBasicCfg(flags, **kwargs)
    acc.merge(RPC_OutputCfg(flags))
    return acc


def RPC_DigitizationDigitToRDOCfg(flags):
    """Return ComponentAccumulator with RPC digitization and Digit to RPCPAD RDO"""
    acc = RPC_DigitizationCfg(flags)
    acc.merge(RPCCablingConfigCfg(flags))
    if not flags.Muon.usePhaseIIGeoSetup:
        acc.merge(RpcDigitToRpcRDOCfg(flags))
    if flags.Muon.enableNRPC:
        acc.merge(NrpcDigitToNrpcRDOCfg(flags))
    return acc

