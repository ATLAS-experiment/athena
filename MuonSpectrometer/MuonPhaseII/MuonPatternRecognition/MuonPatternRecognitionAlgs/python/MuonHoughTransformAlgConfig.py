# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def MuonPhiHoughTransformAlgCfg(flags, name = "MuonPhiHoughTransformAlg", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("downWeightPrdMultiplicity", True)
    theAlg = CompFactory.MuonR4.MuonPhiHoughTransformAlg(name, **kwargs)
    result.addEventAlgo(theAlg, primary=True)
    return result

def MuonEtaHoughTransformAlgCfg(flags, name = "MuonEtaHoughTransformAlg", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("downWeightPrdMultiplicity", True)
    theAlg = CompFactory.MuonR4.MuonEtaHoughTransformAlg(name, **kwargs)
    result.addEventAlgo(theAlg, primary=True)
    return result

def MuonSegmentFittingAlgCfg(flags, name = "MuonSegmentFittingAlg", **kwargs):
    result = ComponentAccumulator()
    theAlg = CompFactory.MuonR4.MuonSegmentFittingAlg(name, **kwargs)
    result.addEventAlgo(theAlg, primary=True)
    return result

def MuonPatternRecognitionCfg(flags): 
    result = ComponentAccumulator()
    from ActsAlignmentAlgs.AlignmentAlgsConfig import ActsGeometryContextAlgCfg
    result.merge(ActsGeometryContextAlgCfg(flags))
    result.merge(MuonEtaHoughTransformAlgCfg(flags))
    result.merge(MuonPhiHoughTransformAlgCfg(flags))
    result.merge(MuonSegmentFittingAlgCfg(flags))
    return result
