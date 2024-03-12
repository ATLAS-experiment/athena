# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MuonSpacePointMakerAlgCfg(flags, name = "MuonSpacePointMakerAlg", **kwargs):
    result = ComponentAccumulator()
    if not flags.Detector.GeometryMDT: kwargs.set_defaults("MdtKey" ,"")    
    if not flags.Detector.GeometryRPC: kwargs.set_defaults("RpcKey" ,"")
    if not flags.Detector.GeometryTGC: kwargs.set_defaults("RpcKey" ,"")
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    result.merge(MuonGeoModelCfg(flags))
    the_alg = CompFactory.MuonR4.MuonSpacePointMakerAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result