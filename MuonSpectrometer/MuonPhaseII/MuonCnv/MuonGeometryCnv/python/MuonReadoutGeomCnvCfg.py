
#Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MuonReadoutGeometryCnvAlgCfg(flags,name="MuonDetectorManagerCondAlg", **kwargs):
    result = ComponentAccumulator()
    from ActsConfig.ActsGeometryConfig import ActsAlignmentCondAlgCfg
    result.merge(ActsAlignmentCondAlgCfg(flags))
    the_alg = CompFactory.MuonReadoutGeomCnvAlg(name=name, **kwargs)
    result.addCondAlgo(the_alg, primary = True)
    return result
