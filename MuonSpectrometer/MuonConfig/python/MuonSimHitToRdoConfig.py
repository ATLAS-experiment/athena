#Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

#### Snippet to schedule the Muon digitization within the Phase II setup
def MuonSimHitToRdoCnvCfg(flags):
    result = ComponentAccumulator()

    if flags.Detector.GeometryMDT and ("xMdtSimHits" in flags.Input.Collections):
        from MuonConfig.MDT_DigitizationConfig import MDT_DigitizationDigitToRDOCfg
        result.merge(MDT_DigitizationDigitToRDOCfg(flags))
    if flags.Detector.GeometryRPC and ("xRpcSimHits" in flags.Input.Collections):
        from MuonConfig.RPC_DigitizationConfig import RPC_DigitizationDigitToRDOCfg
        result.merge(RPC_DigitizationDigitToRDOCfg(flags))
    if flags.Detector.GeometryTGC and ("xTgcSimHits" in flags.Input.Collections):
        from MuonConfig.TGC_DigitizationConfig import TGC_DigitizationDigitToRDOCfg
        result.merge(TGC_DigitizationDigitToRDOCfg(flags))
    if flags.Detector.GeometrysTGC and ("xStgcSimHits" in flags.Input.Collections):
        from MuonConfig.sTGC_DigitizationConfig import sTGC_DigitizationDigitToRDOCfg
        result.merge(sTGC_DigitizationDigitToRDOCfg(flags))
    if flags.Detector.GeometryMM and ("xMmSimHits" in flags.Input.Collections):
        from MuonConfig.MM_DigitizationConfig import MM_DigitizationDigitToRDOCfg
        result.merge(MM_DigitizationDigitToRDOCfg(flags))

    if ("TruthEvents" in flags.Input.Collections):
        from xAODTruthCnv.RedoTruthLinksConfig import RedoTruthLinksAlgCfg
        result.merge( RedoTruthLinksAlgCfg(flags) )
    else:
        from xAODTruthCnv.xAODTruthCnvConfig import GEN_AOD2xAODCfg
        result.merge(GEN_AOD2xAODCfg(flags))
    return result
