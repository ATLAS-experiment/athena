#Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MuonPatternCnvAlgCfg(flags, name="MuonPatternCnvAlg", **kwargs):
    result = ComponentAccumulator()

    from ActsAlignmentAlgs.AlignmentAlgsConfig import ActsGeometryContextAlgCfg
    result.merge(ActsGeometryContextAlgCfg(flags))

    if not flags.Detector.GeometryMDT: kwargs.setdefault("MdtKey" ,"")    
    if not flags.Detector.GeometryRPC: kwargs.setdefault("RpcKey" ,"")
    if not flags.Detector.GeometryTGC: kwargs.setdefault("TgcKey" ,"")
    if not flags.Detector.GeometryMM: kwargs.setdefault("MmKey" ,"")
    if not flags.Detector.GeometrysTGC: kwargs.setdefault("sTgcKey" ,"")

    the_alg = CompFactory.MuonR4.PatternCnvAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result