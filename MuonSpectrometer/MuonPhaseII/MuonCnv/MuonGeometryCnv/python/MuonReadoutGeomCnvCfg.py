
#Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MuonReadoutGeometryCnvAlgCfg(flags,name="MuonDetectorManagerCondAlg", **kwargs):
    from ActsAlignmentAlgs.AlignmentAlgsConfig import ActsGeometryContextAlgCfg
    from MuonGeoModelR4.MuonGeoModelConfig import MuonAlignStoreCfg
    
    result = ComponentAccumulator()
    result.merge(ActsGeometryContextAlgCfg(flags))
    result.merge(MuonAlignStoreCfg(flags))
    the_alg = CompFactory.MuonReadoutGeomCnvAlg(name=name, **kwargs)
    result.addCondAlgo(the_alg, primary = True)
    return result
