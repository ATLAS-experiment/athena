"""Define ComponentAccumulator functions for configuration of muon data conversions

Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import ProductionStep
from MuonConfig.MuonGeometryConfig import MuonIdHelperSvcCfg
from MuonConfig.MuonCablingConfig import MDTCablingConfigCfg


def MdtRDO_DecoderCfg(flags, name="Muon::MdtRDO_Decoder", **kwargs):
    acc = MuonIdHelperSvcCfg(flags)
    acc.merge(MDTCablingConfigCfg(flags)) # To provide the Cabling map
    kwargs.setdefault("MuonIdHelperSvc", acc.getService("MuonIdHelperSvc"))
    acc.setPrivateTools(CompFactory.Muon.MdtRDO_Decoder(name, **kwargs))
    return acc


def MdtRdoToMdtDigitCfg(flags, name="MdtRdoToMdtDigitAlg", **kwargs):
    """Return ComponentAccumulator with configured MdtRdoToMdtDigit algorithm"""
    acc = MuonIdHelperSvcCfg(flags)
    kwargs.setdefault("mdtRdoDecoderTool", acc.popToolsAndMerge(MdtRDO_DecoderCfg(flags)))   
    if flags.Common.isOverlay:
        kwargs.setdefault("MdtRdoContainer", f"{flags.Overlay.BkgPrefix}MDTCSM")
        kwargs.setdefault("MdtDigitContainer", f"{flags.Overlay.BkgPrefix}MDT_DIGITS")
    else:
        kwargs.setdefault("MdtRdoContainer", "MDTCSM")
        kwargs.setdefault("MdtDigitContainer", "MDT_DIGITS")

    if flags.Common.isOverlay and not flags.Overlay.DataOverlay:
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        acc.merge(SGInputLoaderCfg(flags, [f'MdtCsmContainer#{kwargs["MdtRdoContainer"]}']))

    acc.addEventAlgo(CompFactory.MdtRdoToMdtDigit(name, **kwargs))
    return acc


def RpcRdoToRpcDigitCfg(flags, name="RpcRdoToRpcDigitAlg", **kwargs):
    """Return ComponentAccumulator with configured RpcRdoToRpcDigit algorithm"""
    result = ComponentAccumulator()
    
    if flags.Common.isOverlay:
        kwargs.setdefault("RpcRdoContainer", f"{flags.Overlay.BkgPrefix}RPCPAD")
        kwargs.setdefault("RpcDigitContainer", f"{flags.Overlay.BkgPrefix}RPC_DIGITS")
        kwargs.setdefault("NRpcRdoContainer", f"{flags.Overlay.BkgPrefix}NRPCRDO")
    else:
        kwargs.setdefault("RpcRdoContainer", "RPCPAD")
        kwargs.setdefault("RpcDigitContainer", "RPC_DIGITS")
        kwargs.setdefault("NRpcRdoContainer", "NRPCRDO")

    container = [x for x in flags.Input.TypedCollections \
                    if x == "RpcPadContainer#{cont_name}".format(cont_name=kwargs["RpcRdoContainer"])  or \
                       x == "xAOD::NRPCRDOContainer#{cont_name}".format(cont_name=kwargs["NRpcRdoContainer"])  or \
                       x == "xAOD::NRPCRDOAuxContainer#{cont_name}Aux.".format(cont_name=kwargs["NRpcRdoContainer"])  ]
    
    
    ### If the length of the list is > 1, then the NRPC container is in the file
    ### and shall be decoded. Same is true if the length is 1 or 3
    kwargs.setdefault("DecodeNrpcRDO", len(container) > 1 )
    ### If it's dataoverlay, there's only the legacy container...
    kwargs.setdefault("DecodeLegacyRDO", len(container) % 2 or \
                                         not flags.Input.isMC)
    if kwargs["DecodeNrpcRDO"]:
        from MuonConfig.MuonCablingConfig import NRPCCablingConfigCfg
        result.merge(NRPCCablingConfigCfg(flags))
    if kwargs["DecodeLegacyRDO"]:
        from MuonConfig.MuonCablingConfig import RPCLegacyCablingConfigCfg
        result.merge(RPCLegacyCablingConfigCfg(flags))
    if flags.Common.isOverlay and not flags.Overlay.DataOverlay:
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        result.merge(SGInputLoaderCfg(flags, container))

    #Set N BCs and central BC consistently with RPC readout settings
    rpcrdo_decode = CompFactory.Muon.RpcRDO_Decoder("RpcRDO_Decoder", BCZERO=flags.Trigger.L1MuonSim.RPCNBCZ)
    kwargs.setdefault("rpcRdoDecoderTool", rpcrdo_decode)

    result.addEventAlgo(CompFactory.Muon.RpcRdoToRpcDigit(name, **kwargs))
    return result


def TgcRdoToTgcDigitCfg(flags, name="TgcRdoToTgcDigitAlg", **kwargs):
    """Return ComponentAccumulator with configured TgcRdoToTgcDigit algorithm"""
    acc = ComponentAccumulator()
    from MuonConfig.MuonCablingConfig import TGCCablingConfigCfg
    acc.merge(TGCCablingConfigCfg(flags))

    if flags.Common.isOverlay:
        kwargs.setdefault("TgcRdoContainer", f"{flags.Overlay.BkgPrefix}TGCRDO")
        kwargs.setdefault("TgcDigitContainer",f"{flags.Overlay.BkgPrefix}TGC_DIGITS")
    else:
        kwargs.setdefault("TgcRdoContainer", "TGCRDO")
        kwargs.setdefault("TgcDigitContainer", "TGC_DIGITS")

    if flags.Common.isOverlay and not flags.Overlay.DataOverlay:
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        acc.merge(SGInputLoaderCfg(flags, [f'TgcRdoContainer#{kwargs["TgcRdoContainer"]}']))

    acc.addEventAlgo(CompFactory.TgcRdoToTgcDigit(name, **kwargs))
    return acc


def STGC_RdoToDigitCfg(flags, name="STGC_RdoToDigitAlg", **kwargs):
    """Return ComponentAccumulator with configured STGC_RdoToDigit algorithm"""
    acc = ComponentAccumulator()
    if flags.Common.isOverlay:
        kwargs.setdefault("sTgcRdoContainer", f"{flags.Overlay.BkgPrefix}sTGCRDO")
        kwargs.setdefault("sTgcDigitContainer",f"{flags.Overlay.BkgPrefix}sTGC_DIGITS")
    else:
        kwargs.setdefault("sTgcRdoContainer", "sTGCRDO")
        kwargs.setdefault("sTgcDigitContainer", "sTGC_DIGITS")

    kwargs.setdefault("sTgcRdoDecoderTool", acc.popToolsAndMerge(STgcRdoDecoderCfg(flags)))

    if flags.Common.isOverlay and not flags.Overlay.DataOverlay:
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        acc.merge(SGInputLoaderCfg(flags, [f'Muon::STGC_RawDataContainer#{kwargs["sTgcRdoContainer"]}']))

    acc.addEventAlgo(CompFactory.STGC_RdoToDigit(name, **kwargs))
    return acc


def MM_RdoToDigitCfg(flags, name="MM_RdoToDigitAlg", **kwargs):
    """Return ComponentAccumulator with configured MM_RdoToDigit algorithm"""
    acc = ComponentAccumulator()
    if flags.Common.isOverlay:
        kwargs.setdefault("MmRdoContainer", f"{flags.Overlay.BkgPrefix}MMRDO")
        kwargs.setdefault("MmDigitContainer", f"{flags.Overlay.BkgPrefix}MM_DIGITS")
    else:
        kwargs.setdefault("MmRdoContainer", "MMRDO")
        kwargs.setdefault("MmDigitContainer", "MM_DIGITS")

    kwargs.setdefault("mmRdoDecoderTool", acc.popToolsAndMerge(MMRdoDecoderCfg(flags)))

    if flags.Common.isOverlay and not flags.Overlay.DataOverlay:
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        acc.merge(SGInputLoaderCfg(flags, [f'Muon::MM_RawDataContainer#{kwargs["MmRdoContainer"]}']))

    acc.addEventAlgo(CompFactory.MM_RdoToDigit(name, **kwargs))
    return acc


def MdtDigitToMdtRDOCfg(flags, name="MdtDigitToMdtRDO", **kwargs):
    """Return ComponentAccumulator with configured MdtDigitToMdtRDO algorithm"""
    acc = ComponentAccumulator()
    kwargs.setdefault("MuonIdHelperSvc", acc.getPrimaryAndMerge(MuonIdHelperSvcCfg(flags)))
    if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        kwargs.setdefault("OutputObjectName", f"{flags.Overlay.BkgPrefix}MDTCSM")
    else:
        kwargs.setdefault("OutputObjectName", "MDTCSM")

    acc.addEventAlgo(CompFactory.MdtDigitToMdtRDO(name, **kwargs))
    return acc


def RpcDigitToRpcRDOCfg(flags, name="RpcDigitToRpcRDO", **kwargs):
    """Return ComponentAccumulator with configured RpcDigitToRpcRDO algorithm"""
    acc = ComponentAccumulator()
    from MuonConfig.MuonCondAlgConfig import RpcCondDbAlgCfg
    acc.merge(RpcCondDbAlgCfg(flags))
    from MuonConfig.MuonCablingConfig import RPCCablingConfigCfg
    acc.merge(RPCCablingConfigCfg(flags))
    kwargs.setdefault("MuonIdHelperSvc", acc.getPrimaryAndMerge(MuonIdHelperSvcCfg(flags)))

    if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        kwargs.setdefault("OutputObjectName", flags.Overlay.BkgPrefix + "RPCPAD")
    else:
        kwargs.setdefault("OutputObjectName", "RPCPAD")

    kwargs.setdefault("NOBXS", flags.Trigger.L1MuonSim.RPCNBX)
    kwargs.setdefault("BCZERO", flags.Trigger.L1MuonSim.RPCNBCZ)

    acc.addEventAlgo(CompFactory.RpcDigitToRpcRDO(name, **kwargs))
    return acc


def NrpcDigitToNrpcRDOCfg(flags, name="NrpcDigitToNrpcRDO", **kwargs):
    """Return ComponentAccumulator with configured NrpcDigitToNrpcRDO algorithm"""
    acc = ComponentAccumulator()
    kwargs.setdefault("MuonIdHelperSvc", acc.getPrimaryAndMerge(MuonIdHelperSvcCfg(flags)))
    
    from MuonConfig.MuonCablingConfig import NRPCCablingConfigCfg
    acc.merge(NRPCCablingConfigCfg(flags))

    if flags.Muon.usePhaseIIGeoSetup:
        kwargs.setdefault("ConvertHitsFromStations", [])
    
    if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        kwargs.setdefault("NrpcRdoKey", flags.Overlay.BkgPrefix + "NRPCRDO")
    else:
        kwargs.setdefault("NrpcRdoKey", "NRPCRDO")

    acc.addEventAlgo(CompFactory.Muon.NrpcDigitToNrpcRDO(name, **kwargs))
    return acc


def TgcDigitToTgcRDOCfg(flags, name="TgcDigitToTgcRDO", **kwargs):
    """Return ComponentAccumulator with configured TgcDigitToTgcRDO algorithm"""
    acc = ComponentAccumulator()
    kwargs.setdefault("MuonIdHelperSvc", acc.getPrimaryAndMerge(MuonIdHelperSvcCfg(flags)))

    if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        kwargs.setdefault("OutputObjectName", f"{flags.Overlay.BkgPrefix}TGCRDO")
    else:
        kwargs.setdefault("OutputObjectName", "TGCRDO")

    acc.addEventAlgo(CompFactory.TgcDigitToTgcRDO(name, **kwargs))
    return acc


def CscDigitToCscRDOToolCfg(flags, name="CscDigitToCscRDOTool", **kwargs):
    """Return ComponentAccumulator with configured CscDigitToCscRDOTool"""
    acc = ComponentAccumulator()
    # configure basic parameters
    kwargs.setdefault("MuonIdHelperSvc", acc.getPrimaryAndMerge(MuonIdHelperSvcCfg(flags)))
    from MuonConfig.MuonCalibrationConfig import CscCalibToolCfg
    kwargs.setdefault("cscCalibTool", acc.popToolsAndMerge(CscCalibToolCfg(flags)))
    kwargs.setdefault("NumSamples", 4)
    kwargs.setdefault("Latency", 0)
    kwargs.setdefault("addNoise", not flags.Common.isOverlay) # doMuonNoise flag not migrated

    if flags.Common.isOverlay:
        kwargs.setdefault("InputObjectName", f"{flags.Overlay.SigPrefix}CSC_DIGITS")
        kwargs.setdefault("OutputObjectName", f"{flags.Overlay.SigPrefix}CSCRDO")
    elif flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        kwargs.setdefault("OutputObjectName", f"{flags.Overlay.BkgPrefix}CSCRDO")
    else:
        kwargs.setdefault("OutputObjectName", "CSCRDO")

    from RngComps.RngCompsConfig import AthRNGSvcCfg
    kwargs.setdefault("RndmSvc", acc.getPrimaryAndMerge(AthRNGSvcCfg(flags)).name)
    acc.setPrivateTools(CompFactory.CscDigitToCscRDOTool("CscDigitToCscRDOTool", **kwargs))
    return acc


def CscDigitToCscRDOCfg(flags, name="CscDigitToCscRDO", **kwargs):
    """Return ComponentAccumulator with configured CscDigitToCscRDO algorithm"""
    # for CSC, configuration is in the tool CscDigitToCscRDOTool
    acc = ComponentAccumulator()
    kwargs.setdefault("CscDigitToRDOTool", acc.popToolsAndMerge(CscDigitToCscRDOToolCfg(flags)))

    if flags.Concurrency.NumThreads > 0:
        kwargs.setdefault("Cardinality", flags.Concurrency.NumThreads)

    acc.addEventAlgo(CompFactory.CscDigitToCscRDO(name, **kwargs))
    return acc


def STGC_DigitToRDOCfg(flags, name="STGC_DigitToRDO", **kwargs):
    """Return ComponentAccumulator with configured STGC_DigitToRDO algorithm"""
    acc = ComponentAccumulator()
    kwargs.setdefault("MuonIdHelperSvc", acc.getPrimaryAndMerge(MuonIdHelperSvcCfg(flags)))

    if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        kwargs.setdefault("OutputObjectName", f"{flags.Overlay.BkgPrefix}sTGCRDO")
    else:
        kwargs.setdefault("OutputObjectName", "sTGCRDO")

    from MuonConfig.MuonCalibrationConfig import NSWCalibToolCfg
    kwargs.setdefault("CalibrationTool", acc.popToolsAndMerge(NSWCalibToolCfg(flags)))
    the_alg  = CompFactory.STGC_DigitToRDO(name, **kwargs)
    acc.addEventAlgo(the_alg)
    return acc


def MM_DigitToRDOCfg(flags, name="MM_DigitToRDO", **kwargs):
    """Return ComponentAccumulator with configured MM_DigitToRDO algorithm"""
    acc = ComponentAccumulator()
    kwargs.setdefault("MuonIdHelperSvc", acc.getPrimaryAndMerge(MuonIdHelperSvcCfg(flags)))

    if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        kwargs.setdefault("OutputObjectName", f"{flags.Overlay.BkgPrefix}MMRDO")
    else:
        kwargs.setdefault("OutputObjectName", "MMRDO")

    from MuonConfig.MuonCalibrationConfig import NSWCalibToolCfg
    kwargs.setdefault("CalibrationTool", acc.popToolsAndMerge(NSWCalibToolCfg(flags)))    
    the_alg =  CompFactory.MM_DigitToRDO(name, **kwargs) 
    acc.addEventAlgo(the_alg)
    return acc


def SigMdtDigitToMdtRDOCfg(flags, name="SigMdtDigitToMdtRDO", **kwargs):
    """Return ComponentAccumulator with configured MdtDigitToMdtRDO algorithm"""
    acc = ComponentAccumulator()
    kwargs.setdefault("MuonIdHelperSvc", acc.getPrimaryAndMerge(MuonIdHelperSvcCfg(flags)))
    kwargs.setdefault("InputObjectName", f"{flags.Overlay.SigPrefix}MDT_DIGITS")
    kwargs.setdefault("OutputObjectName", f"{flags.Overlay.SigPrefix}MDTCSM")
    acc.addEventAlgo(CompFactory.MdtDigitToMdtRDO(name, **kwargs))
    return acc


def SigRpcDigitToRpcRDOCfg(flags, name="SigRpcDigitToRpcRDO", **kwargs):
    """Return ComponentAccumulator with configured RpcDigitToRpcRDO algorithm"""
    acc = ComponentAccumulator()
    kwargs.setdefault("MuonIdHelperSvc", acc.getPrimaryAndMerge(MuonIdHelperSvcCfg(flags)))
    kwargs.setdefault("InputObjectName", f"{flags.Overlay.SigPrefix}RPC_DIGITS")
    kwargs.setdefault("OutputObjectName", f"{flags.Overlay.SigPrefix}RPCPAD")
    acc.addEventAlgo(CompFactory.RpcDigitToRpcRDO(name, **kwargs))
    return acc


def SigTgcDigitToTgcRDOCfg(flags, name="SigTgcDigitToTgcRDO", **kwargs):
    """Return ComponentAccumulator with configured TgcDigitToTgcRDO algorithm"""
    acc = ComponentAccumulator()
    kwargs.setdefault("MuonIdHelperSvc", acc.getPrimaryAndMerge(MuonIdHelperSvcCfg(flags)))
    kwargs.setdefault("InputObjectName", f"{flags.Overlay.SigPrefix}TGC_DIGITS")
    kwargs.setdefault("OutputObjectName", f"{flags.Overlay.SigPrefix}TGCRDO")
    acc.addEventAlgo(CompFactory.TgcDigitToTgcRDO(name, **kwargs))
    return acc

def STgcRdoDecoderCfg(flags, name="STGC_RDO_Decoder", **kwargs):
    result = ComponentAccumulator()
    from MuonConfig.MuonCalibrationConfig import NSWCalibToolCfg
    kwargs.setdefault("CalibrationTool", result.popToolsAndMerge(NSWCalibToolCfg(flags)))    
    the_tool = CompFactory.Muon.STGC_RDO_Decoder(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result
    
def MMRdoDecoderCfg(flags, name="MM_RDO_Decoder", **kwargs):
    result = ComponentAccumulator()
    from MuonConfig.MuonCalibrationConfig import NSWCalibToolCfg
    kwargs.setdefault("CalibrationTool", result.popToolsAndMerge(NSWCalibToolCfg(flags)))    
    the_tool = CompFactory.Muon.MM_RDO_Decoder(name, **kwargs)
    result.setPrivateTools(the_tool)   
    return result

def MdtRdoDecoderCfg(flags,name= "MDT_RDO_Decoder", **kwargs):
    result = ComponentAccumulator()
    from MuonConfig.MuonCablingConfig import MDTCablingConfigCfg
    result.merge(MDTCablingConfigCfg(flags))
    the_tool = CompFactory.Muon.MdtRDO_Decoder(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result
  

