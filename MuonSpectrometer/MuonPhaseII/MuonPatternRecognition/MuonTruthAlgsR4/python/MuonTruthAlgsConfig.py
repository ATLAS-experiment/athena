# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
def TruthSegmentMakerCfg(flags, name = "TruthSegmentMakerAlg", **kwargs):
    result = ComponentAccumulator()
    if not flags.Input.isMC:
        return result

    from ActsAlignmentAlgs.AlignmentAlgsConfig import ActsGeometryContextAlgCfg
    result.merge(ActsGeometryContextAlgCfg(flags))
    containerNames = []
    ## If the tester runs on MC add the truth information
    if flags.Detector.EnableMDT: containerNames+=["xMdtSimHits"]       
    if flags.Detector.EnableRPC: containerNames+=["xRpcSimHits"]
    if flags.Detector.EnableTGC: containerNames+=["xTgcSimHits"]
    if flags.Detector.EnableMM: containerNames+=["xMmSimHits"]
    if flags.Detector.EnablesTGC: containerNames+=["xStgcSimHits"] 
    kwargs.setdefault("SimHitKeys", containerNames)
    the_alg = CompFactory.MuonR4.TruthSegmentMaker(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def MeasToSimHitAssocAlgCfg(flags, name="MeasToSimHitConvAlg", **kwargs):
    result = ComponentAccumulator()
    from ActsAlignmentAlgs.AlignmentAlgsConfig import ActsGeometryContextAlgCfg
    result.merge(ActsGeometryContextAlgCfg(flags))
    the_alg = CompFactory.MuonR4.PrepDataToSimHitAssocAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)    
    return result

def TruthHitAssociationCfg(flags):
    result = ComponentAccumulator()
    if not flags.Input.isMC:
        return result

    if flags.Detector.EnableMDT: 
        result.merge(MeasToSimHitAssocAlgCfg(flags,
                                             name="MdtPrepDataToSimHitAssoc",
                                             SimHits = "xMdtSimHits",
                                             Measurements="xAODMdtCircles"))
    if flags.Detector.EnableRPC: 
        result.merge(MeasToSimHitAssocAlgCfg(flags,
                                             name="RpcPrepDataToSimHitAssoc",
                                             SimHits = "xRpcSimHits",
                                             Measurements="xRpcMeasurements"))
        
    if flags.Detector.EnableTGC: 
        result.merge(MeasToSimHitAssocAlgCfg(flags,
                                             name="TgcPrepDataToSimHitAssoc",
                                             SimHits = "xTgcSimHits",
                                             Measurements="xTgcStrips"))
    if flags.Detector.EnableMM: 
        result.merge(MeasToSimHitAssocAlgCfg(flags,
                                             name="MmPrepDataToSimHitAssoc",
                                             SimHits = "xMmSimHits",
                                             Measurements="xAODMMClusters"))

    if flags.Detector.EnablesTGC: 
        result.merge(MeasToSimHitAssocAlgCfg(flags,
                                             name="sTgcPrepDataToSimHitAssoc",
                                             SimHits = "xStgcSimHits",
                                             Measurements="xAODsTgcMeasurements"))

    return result

