# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MuonSpacePointMakerAlgCfg(flags, name = "MuonSpacePointMakerAlg", **kwargs):
    result = ComponentAccumulator()
    if not flags.Detector.GeometryMDT: kwargs.setdefault("MdtKey" ,"")    
    if not flags.Detector.GeometryRPC: kwargs.setdefault("RpcKey" ,"")
    if not flags.Detector.GeometryTGC: kwargs.setdefault("TgcKey" ,"")
    if not flags.Detector.GeometryMM: kwargs.setdefault("MmKey" ,"")
    if not flags.Detector.GeometrysTGC: kwargs.setdefault("sTgcKey" ,"")
    
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    result.merge(MuonGeoModelCfg(flags))
    the_alg = CompFactory.MuonR4.SpacePointMakerAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def MuonSpacePointFormationCfg(flags):
    result = ComponentAccumulator()
    from ActsAlignmentAlgs.AlignmentAlgsConfig import ActsGeometryContextAlgCfg
    result.merge(ActsGeometryContextAlgCfg(flags))
    result.merge(MuonSpacePointMakerAlgCfg(flags, MmKey = "", sTgcKey = ""))
    ### Split the Nsw hits into a separate space point container
    if flags.Detector.GeometrysTGC or flags.Detector.GeometryMM:
        result.merge(MuonSpacePointMakerAlgCfg(flags, 
                                               name="MuonNswSpacePointMakerAlg",
                                               MdtKey="", RpcKey = "", TgcKey ="",
                                               WriteKey = "NswSpacePoints"))
    return result