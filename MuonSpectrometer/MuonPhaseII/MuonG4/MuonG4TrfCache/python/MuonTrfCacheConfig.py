# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

def MuonTransformCacheCfg(flags, name="MuonSimHitSortingAlg", **kwargs):
    from ROOT.ActsTrk import DetectorType
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.Enums import ProductionStep

    result = ComponentAccumulator()
    if flags.Common.ProductionStep != ProductionStep.Simulation or \
        not flags.Sim.ReleaseGeoModel or \
        not flags.Muon.usePhaseIIGeoSetup:
        return result
    if flags.Detector.GeometryMDT:  
        result.addCondAlgo(CompFactory.MuonG4.GeoModelTrfCacheAlg(name="MuonMdtTrfCacheAlg",
                                                                  writeKey="MdtActsAlignContainer",
                                                                  DetectorType=DetectorType.Mdt))
    if flags.Detector.GeometryRPC:  
        result.addCondAlgo(CompFactory.MuonG4.GeoModelTrfCacheAlg(name="MuonRpcTrfCacheAlg",
                                                                  writeKey="RpcActsAlignContainer",
                                                                  DetectorType=DetectorType.Rpc))
    if flags.Detector.GeometryTGC:  
        result.addCondAlgo(CompFactory.MuonG4.GeoModelTrfCacheAlg(name="MuonTgcTrfCacheAlg",
                                                                  writeKey="TgcActsAlignContainer",
                                                                  DetectorType=DetectorType.Tgc))
    if flags.Detector.GeometrysTGC: 
        result.addCondAlgo(CompFactory.MuonG4.GeoModelTrfCacheAlg(name="MuonsTgcTrfCacheAlg",
                                                                  writeKey="sTgcActsAlignContainer",
                                                                  DetectorType=DetectorType.sTgc))
    if flags.Detector.GeometryMM:
        result.addCondAlgo(CompFactory.MuonG4.GeoModelTrfCacheAlg(name="MuonMmTrfCacheAlg",
                                                                  writeKey="MmActsAlignContainer",
                                                                  DetectorType=DetectorType.Mm))
 
    return result