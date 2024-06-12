#Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def sTgcMeasViewAlgCfg(flags, name="sTgcMeasViewAlg", **kwargs):
    result = ComponentAccumulator()
    if not flags.Detector.GeometrysTGC:
        return result
    the_alg = CompFactory.MuonR4.sTgcMeasViewAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def RpcMeasViewAlgCfg(flags, name="RpcMeasViewAlg", **kwargs):
    result = ComponentAccumulator()
    if not flags.Detector.GeometryRPC:
        return result
    the_alg = CompFactory.MuonR4.RpcMeasViewAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result