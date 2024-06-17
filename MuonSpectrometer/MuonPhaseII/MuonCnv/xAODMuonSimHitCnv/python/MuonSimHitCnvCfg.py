#Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def xAODSimHitToMdtMeasCnvAlgCfg(flags,name = "SimHitToMdtMeasurementCnvAlg", **kwargs):
    result = ComponentAccumulator()
    from MuonConfig.MuonCalibrationConfig import MdtCalibDbAlgCfg
    result.merge(MdtCalibDbAlgCfg(flags))
    from RngComps.RngCompsConfig import AthRNGSvcCfg
    kwargs.setdefault("RndmSvc", result.getPrimaryAndMerge(AthRNGSvcCfg(flags)))
    the_alg = CompFactory.xAODSimHitToMdtMeasCnvAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def xAODSimHitToRpcMeasCnvAlgCfg(flags,name = "SimHitToRpcMeasurementCnvAlg", **kwargs):
    result = ComponentAccumulator()
    from RngComps.RngCompsConfig import AthRNGSvcCfg
    kwargs.setdefault("RndmSvc", result.getPrimaryAndMerge(AthRNGSvcCfg(flags)))
    the_alg = CompFactory.xAODSimHitToRpcMeasCnvAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    from xAODMuonMeasViewAlgs.ViewAlgsConfig import RpcMeasViewAlgCfg
    result.merge(RpcMeasViewAlgCfg(flags))
    return result

def xAODSimHitToTgcMeasCnvAlgCfg(flags,name = "SimHitToTgcMeasurementCnvAlg", **kwargs):
    result = ComponentAccumulator()
    from RngComps.RngCompsConfig import AthRNGSvcCfg
    kwargs.setdefault("RndmSvc", result.getPrimaryAndMerge(AthRNGSvcCfg(flags)))
    the_alg = CompFactory.xAODSimHitToTgcMeasCnvAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def xAODSimHitTosTGCMeasCnvAlgCfg(flags, name = "SimHitTosTGCMeasurementCnvAlg",**kwargs):
    result = ComponentAccumulator()
    from RngComps.RngCompsConfig import AthRNGSvcCfg
    kwargs.setdefault("RndmSvc", result.getPrimaryAndMerge(AthRNGSvcCfg(flags)))
    from MuonConfig.MuonCalibrationConfig import NswErrorCalibDbAlgCfg
    result.merge(NswErrorCalibDbAlgCfg(flags))

    the_alg = CompFactory.xAODSimHitTosTGCMeasCnvAlg(name,**kwargs)
    result.addEventAlgo(the_alg,primary=True)
    from xAODMuonMeasViewAlgs.ViewAlgsConfig import sTgcMeasViewAlgCfg
    result.merge(sTgcMeasViewAlgCfg(flags))
    return result

def xAODSimHitToMmMeasCnvAlgCfg(flags, name = "SimHitToMmMeasurementCnvAlg",**kwargs):
    result = ComponentAccumulator()
    from RngComps.RngCompsConfig import AthRNGSvcCfg
    kwargs.setdefault("RndmSvc", result.getPrimaryAndMerge(AthRNGSvcCfg(flags)))
    from MuonConfig.MuonCalibrationConfig import NswErrorCalibDbAlgCfg
    result.merge(NswErrorCalibDbAlgCfg(flags))

    the_alg = CompFactory.xAODSimHitToMmMeasCnvAlg(name,**kwargs)
    result.addEventAlgo(the_alg,primary=True)
    return result


def MmFastDigitizationCfg(flags, name="MmFastDigitizer",  **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("StreamName", "MmSimForklift")
    kwargs.setdefault("OutputSDOName", "MM_SDO")
    kwargs.setdefault("SimHitKey", "xMmSimHits")
    kwargs.setdefault("EffiDataKey", "")
    
    from MuonConfig.MuonCalibrationConfig import NswErrorCalibDbAlgCfg
    result.merge(NswErrorCalibDbAlgCfg(flags))

    from RngComps.RngCompsConfig import AthRNGSvcCfg
    kwargs.setdefault("RndmSvc", result.getPrimaryAndMerge(AthRNGSvcCfg(flags)))
    the_tool = CompFactory.MuonR4.MmFastDigiTool(name="MmDigitizationTool", **kwargs)
    the_alg = CompFactory.MuonDigitizer(name,
                                        DigitizationTool = the_tool)
    result.addEventAlgo(the_alg, primary = True)
    return result
def RpcFastDigitizationCfg(flags, name="RpcFastDigitizer", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("StreamName", "RpcSimForklift")
    kwargs.setdefault("OutputSDOName", "RPC_SDO")
    kwargs.setdefault("SimHitKey", "xRpcSimHits")
    kwargs.setdefault("EffiDataKey", "")
    from RngComps.RngCompsConfig import AthRNGSvcCfg
    kwargs.setdefault("RndmSvc", result.getPrimaryAndMerge(AthRNGSvcCfg(flags)))
    the_tool = CompFactory.MuonR4.RpcFastDigiTool(name="RpcDigitizationTool", **kwargs)
    the_alg = CompFactory.MuonDigitizer(name,
                                        DigitizationTool = the_tool)
    result.addEventAlgo(the_alg, primary = True)
    return result
def RpcDigitToMeasCnvAlgCfg(flags, name ="RpcDigitToMeasCnvAlg", **kwargs):
    result = ComponentAccumulator()
    from xAODMuonMeasViewAlgs.ViewAlgsConfig import RpcMeasViewAlgCfg
    result.merge(RpcMeasViewAlgCfg(flags))
    the_alg = CompFactory.MuonR4.RpcDigitToRpcMeasCnvAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def TgcFastDigitizationCfg(flags, name = "TgcFastDigitizer", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("StreamName", "TgcSimForklift")
    kwargs.setdefault("OutputSDOName", "TGC_SDO")
    kwargs.setdefault("SimHitKey", "xTgcSimHits")
    kwargs.setdefault("EffiDataKey", "")
    from RngComps.RngCompsConfig import AthRNGSvcCfg
    kwargs.setdefault("RndmSvc", result.getPrimaryAndMerge(AthRNGSvcCfg(flags)))
    the_tool = CompFactory.MuonR4.TgcFastDigiTool(name="TgcDigitizationTool", **kwargs)
    the_alg = CompFactory.MuonDigitizer(name,
                                        DigitizationTool = the_tool)
    result.addEventAlgo(the_alg, primary = True)
    return result
###
###  Configuration snippet to go from xAOD::MuonSimHit to xAOD::MuonPrepData    
###
def MuonSimHitToMeasurementCfg(flags):
    result = ComponentAccumulator()
    if flags.Detector.GeometryMDT:
        from MuonConfig.MDT_DigitizationConfig import MDT_DigitizationDigitToRDOCfg
        #result.merge(xAODSimHitToMdtMeasCnvAlgCfg(flags))
        from MuonConfig.MuonRdoDecodeConfig import MdtRDODecodeCfg
        result.merge(MDT_DigitizationDigitToRDOCfg(flags))
        result.merge(MdtRDODecodeCfg(flags))
    if flags.Detector.GeometryRPC:
        #result.merge(xAODSimHitToRpcMeasCnvAlgCfg(flags))
        result.merge(RpcFastDigitizationCfg(flags))
        result.merge(RpcDigitToMeasCnvAlgCfg(flags))
    if flags.Detector.GeometryTGC:
        #result.merge(xAODSimHitToTgcMeasCnvAlgCfg(flags))
        result.merge(TgcFastDigitizationCfg(flags))
        from MuonConfig.MuonByteStreamCnvTestConfig import TgcDigitToTgcRDOCfg
        from MuonConfig.MuonCablingConfig import TGCCablingConfigCfg
        result.merge(TGCCablingConfigCfg(flags))
        result.merge(TgcDigitToTgcRDOCfg(flags))
        from MuonConfig.MuonRdoDecodeConfig import TgcRDODecodeCfg
        result.merge(TgcRDODecodeCfg(flags))


    if flags.Detector.GeometrysTGC:
        result.merge(xAODSimHitTosTGCMeasCnvAlgCfg(flags))
    if flags.Detector.GeometryMM:    
        #result.merge(xAODSimHitToMmMeasCnvAlgCfg(flags))
        result.merge(MmFastDigitizationCfg(flags))
        from MuonConfig.MuonByteStreamCnvTestConfig import MM_DigitToRDOCfg
        result.merge(MM_DigitToRDOCfg(flags))
        from MuonConfig.MuonRdoDecodeConfig import MMRDODecodeCfg
        result.merge(MMRDODecodeCfg(flags))

    return result
