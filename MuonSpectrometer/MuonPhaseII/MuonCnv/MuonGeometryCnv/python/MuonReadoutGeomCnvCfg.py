
#Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MuonReadoutGeometryCnvAlgCfg(flags,name="MuonDetectorCondAlg", **kwargs):
    from MuonGeoModelR4.MuonGeoModelConfig import MuonAlignStoreCfg
    
    result = ComponentAccumulator()
    result.merge(MuonAlignStoreCfg(flags))
    alignStores = []

    if flags.Detector.GeometryMDT:  
        alignStores+=["MdtActsAlignContainer"]
    if flags.Detector.GeometryRPC:  
        alignStores+=["RpcActsAlignContainer"]
    if flags.Detector.GeometryTGC:  
        alignStores+=["TgcActsAlignContainer"]
    if flags.Detector.GeometrysTGC: 
        alignStores+=["sTgcActsAlignContainer"]
    if flags.Detector.GeometryMM:
        alignStores+=["MmActsAlignContainer"]
    kwargs.setdefault("AlignmentKeys", alignStores)
    the_alg = CompFactory.MuonGMR4.ReadoutGeomCnvAlg(name=name, **kwargs)
    result.addCondAlgo(the_alg, primary = True)
    return result
