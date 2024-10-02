# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def SegmentMarkerAlgCfg(flags, name = "SegmentMarkerAlg", **kwargs):
    result = ComponentAccumulator()
    the_alg = CompFactory.MuonR4.SegmentMarkerAlg(name, **kwargs)
    result.addEventAlgo(the_alg)
    return result

def MeasurementMarkerAlgCfg(flags, name = "MeasurementMarkerAlg", **kwargs) :
    result = ComponentAccumulator()
    PrdContainers = []
    if flags.Detector.EnableMDT: 
        PrdContainers+=['xAODMdtCircles']
    if flags.Detector.EnableRPC: 
        PrdContainers+=['xRpcMeasurements']
    if flags.Detector.EnableTGC: 
        PrdContainers+=['xTgcStrips'] 
    if flags.Detector.EnableMM:
        PrdContainers+=['xAODMMClusters']
    if flags.Detector.EnablesTGC:
        PrdContainers+=['xAODsTgcStrips']
        PrdContainers+=['xAODsTgcWires']
        PrdContainers+=['xAODsTgcPads']
    kwargs.setdefault("PrdContainer", PrdContainers)
    the_alg = CompFactory.MuonR4.MeasurementMarkerAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def TruthMeasMarkerAlgCfg(flags, name = "TruthMeasMarkerAlg", **kwargs):
    result = ComponentAccumulator()
    PrdContainers = []
    if flags.Detector.EnableMDT: 
        PrdContainers+=['xAODMdtCircles']
    if flags.Detector.EnableRPC: 
        PrdContainers+=['xRpcMeasurements']
    if flags.Detector.EnableTGC: 
        PrdContainers+=['xTgcStrips'] 
    if flags.Detector.EnableMM:
        PrdContainers+=['xAODMMClusters']
    if flags.Detector.EnablesTGC:
        PrdContainers+=['xAODsTgcStrips']
        PrdContainers+=['xAODsTgcWires']
        PrdContainers+=['xAODsTgcPads']
    kwargs.setdefault("PrdContainer", PrdContainers)
    kwargs.setdefault("SegmentLinkKey", "truthSegLinks")
    the_alg = CompFactory.MuonR4.TruthMeasMarkerAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result
def MuonSegmentFitParDecorAlgCfg(flags,name = "MuonSegmentFitParDecorAlg", **kwargs):
    result = ComponentAccumulator()

    from ActsAlignmentAlgs.AlignmentAlgsConfig import ActsGeometryContextAlgCfg
    result.merge(ActsGeometryContextAlgCfg(flags))

    if not flags.Detector.GeometryMDT: kwargs.setdefault("MdtKey" ,"")    
    if not flags.Detector.GeometryRPC: kwargs.setdefault("RpcKey" ,"")
    if not flags.Detector.GeometryTGC: kwargs.setdefault("TgcKey" ,"")
    if not flags.Detector.GeometryMM: kwargs.setdefault("MmKey" ,"")
    if not flags.Detector.GeometrysTGC: kwargs.setdefault("sTgcKey" ,"")  
    the_alg = CompFactory.MuonR4.SegmentFitParDecorAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result
