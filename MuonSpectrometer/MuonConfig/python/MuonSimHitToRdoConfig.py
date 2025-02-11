#Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

#### Snippet to schedule the Muon digitization within the Phase II setup
def MuonSimHitToRdoCnvCfg(flags):
    result = ComponentAccumulator()
    if flags.Detector.GeometryMDT:
        from MuonConfig.MDT_DigitizationConfig import MDT_DigitizationDigitToRDOCfg
        result.merge(MDT_DigitizationDigitToRDOCfg(flags))
    if flags.Detector.GeometryRPC:
        from MuonConfig.RPC_DigitizationConfig import RPC_DigitizationDigitToRDOCfg
        result.merge(RPC_DigitizationDigitToRDOCfg(flags))
    if flags.Detector.GeometryTGC:
        from MuonConfig.TGC_DigitizationConfig import TGC_DigitizationDigitToRDOCfg
        result.merge(TGC_DigitizationDigitToRDOCfg(flags))
    if flags.Detector.GeometrysTGC:
        from MuonConfig.sTGC_DigitizationConfig import sTGC_DigitizationDigitToRDOCfg
        result.merge(sTGC_DigitizationDigitToRDOCfg(flags))
    if flags.Detector.GeometryMM:    
        from MuonConfig.MM_DigitizationConfig import MM_DigitizationDigitToRDOCfg
        result.merge(MM_DigitizationDigitToRDOCfg(flags))

    from xAODTruthCnv.RedoTruthLinksConfig import RedoTruthLinksAlgCfg
    result.merge( RedoTruthLinksAlgCfg(flags) )
    return result
