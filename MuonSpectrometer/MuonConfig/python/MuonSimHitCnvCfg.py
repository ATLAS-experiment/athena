#Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
###
###  Configuration snippet to go from xAOD::MuonSimHit to xAOD::MuonPrepData    
###
def MuonSimHitToMeasurementCfg(flags):
    result = ComponentAccumulator()
    if flags.Detector.GeometryMDT:
        from MuonConfig.MDT_DigitizationConfig import MDT_DigitizationDigitToRDOCfg
        from MuonConfig.MuonRdoDecodeConfig import MdtRDODecodeCfg
        result.merge(MDT_DigitizationDigitToRDOCfg(flags))
        result.merge(MdtRDODecodeCfg(flags))
    if flags.Detector.GeometryRPC:
        from MuonConfig.RPC_DigitizationConfig import RPC_DigitizationDigitToRDOCfg
        result.merge(RPC_DigitizationDigitToRDOCfg(flags))
        from MuonConfig.MuonRdoDecodeConfig import RpcRDODecodeCfg
        result.merge(RpcRDODecodeCfg(flags))

    if flags.Detector.GeometryTGC:
        from MuonConfig.TGC_DigitizationConfig import TGC_DigitizationDigitToRDOCfg
        result.merge(TGC_DigitizationDigitToRDOCfg(flags))
        from MuonConfig.MuonRdoDecodeConfig import TgcRDODecodeCfg
        result.merge(TgcRDODecodeCfg(flags))


    if flags.Detector.GeometrysTGC:
        from MuonConfig.sTGC_DigitizationConfig import sTGC_DigitizationDigitToRDOCfg
        result.merge(sTGC_DigitizationDigitToRDOCfg(flags))
        from MuonConfig.MuonRdoDecodeConfig import StgcRDODecodeCfg
        result.merge(StgcRDODecodeCfg(flags))
    if flags.Detector.GeometryMM:    
        from MuonConfig.MM_DigitizationConfig import MM_DigitizationDigitToRDOCfg
        result.merge(MM_DigitizationDigitToRDOCfg(flags))
        from MuonConfig.MuonRdoDecodeConfig import MMRDODecodeCfg
        result.merge(MMRDODecodeCfg(flags))

    return result
