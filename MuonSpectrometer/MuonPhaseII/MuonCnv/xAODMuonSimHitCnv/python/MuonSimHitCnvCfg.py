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
        from MuonConfig.RPC_DigitizationConfig import RPC_DigitizationDigitToRDOCfg
        result.merge(RPC_DigitizationDigitToRDOCfg(flags))
        from MuonConfig.MuonRdoDecodeConfig import RpcRDODecodeCfg
        result.merge(RpcRDODecodeCfg(flags))

    if flags.Detector.GeometryTGC:
        #result.merge(xAODSimHitToTgcMeasCnvAlgCfg(flags))
        from MuonConfig.TGC_DigitizationConfig import TGC_DigitizationDigitToRDOCfg
        result.merge(TGC_DigitizationDigitToRDOCfg(flags))
        from MuonConfig.MuonRdoDecodeConfig import TgcRDODecodeCfg
        result.merge(TgcRDODecodeCfg(flags))


    if flags.Detector.GeometrysTGC:
        #result.merge(xAODSimHitTosTGCMeasCnvAlgCfg(flags))
        from MuonConfig.sTGC_DigitizationConfig import sTGC_DigitizationDigitToRDOCfg
        result.merge(sTGC_DigitizationDigitToRDOCfg(flags))
        from MuonConfig.MuonRdoDecodeConfig import StgcRDODecodeCfg
        result.merge(StgcRDODecodeCfg(flags))
    if flags.Detector.GeometryMM:    
        #result.merge(xAODSimHitToMmMeasCnvAlgCfg(flags))
        from MuonConfig.MM_DigitizationConfig import MM_DigitizationDigitToRDOCfg
        result.merge(MM_DigitizationDigitToRDOCfg(flags))
        from MuonConfig.MuonRdoDecodeConfig import MMRDODecodeCfg
        result.merge(MMRDODecodeCfg(flags))

    return result
