# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration



def MuonTransformTechnologyCfg(flags, name, key="", detType=-1):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaConfiguration.ComponentFactory import CompFactory
    result = ComponentAccumulator()
    cond_alg = CompFactory.MuonG4.GeoModelTrfCacheAlg(name= f"{name}CondAlg",
                                                      writeKey=key,
                                                      DetectorType=detType)
    result.addCondAlgo(cond_alg)

    event_alg = CompFactory.MuonG4.AlignStoreProviderAlg(name=f"{name}Alg",
                                                         writeKey=key, readKey=key)
    result.addEventAlgo(event_alg)
    return result


def MuonTransformCacheCfg(flags, name="MuonSimHitSortingAlg", **kwargs):
    from ROOT.ActsTrk import DetectorType
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaConfiguration.Enums import ProductionStep

    result = ComponentAccumulator()

    if (flags.Common.ProductionStep != ProductionStep.Simulation or \
        not flags.Muon.usePhaseIIGeoSetup):
        return result
    if flags.Detector.GeometryMDT:  
        result.merge(MuonTransformTechnologyCfg(flags, name="MuonMdtTrfCache",
                                                key="MdtActsAlignContainer",
                                                detType=DetectorType.Mdt))
    if flags.Detector.GeometryRPC:  
        result.merge(MuonTransformTechnologyCfg(flags, name="MuonRpcTrfCache",
                                                key="RpcActsAlignContainer",
                                                detType=DetectorType.Rpc))
    if flags.Detector.GeometryTGC:  
        result.merge(MuonTransformTechnologyCfg(flags, name="MuonTgcTrfCache",
                                                key="TgcActsAlignContainer",
                                                detType=DetectorType.Tgc))
    if flags.Detector.GeometrysTGC:
        result.merge(MuonTransformTechnologyCfg(flags, name="MuonsTgcTrfCache",
                                                key="sTgcActsAlignContainer",
                                                detType=DetectorType.sTgc)) 
    if flags.Detector.GeometryMM:
        result.merge(MuonTransformTechnologyCfg(flags, name="MuonMmTrfCache",
                                                key="MmActsAlignContainer",
                                                detType=DetectorType.Mm))
    return result