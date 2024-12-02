# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def MuonPhiHoughTransformAlgCfg(flags, name = "MuonPhiHoughTransformAlg", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("downWeightPrdMultiplicity", True)
    theAlg = CompFactory.MuonR4.PhiHoughTransformAlg(name, **kwargs)
    result.addEventAlgo(theAlg, primary=True)
    return result

def MuonEtaHoughTransformAlgCfg(flags, name = "MuonEtaHoughTransformAlg", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("downWeightPrdMultiplicity", True)
    theAlg = CompFactory.MuonR4.EtaHoughTransformAlg(name, **kwargs)
    result.addEventAlgo(theAlg, primary=True)
    return result

def MuonSegmentFittingAlgCfg(flags, name = "MuonSegmentFittingAlg", **kwargs):
    result = ComponentAccumulator()
    from MuonSpacePointCalibrator.CalibrationConfig import MuonSpacePointCalibratorCfg
    kwargs.setdefault("Calibrator", result.popToolsAndMerge(MuonSpacePointCalibratorCfg(flags, mdtErrorScaleFactor=2.0, MdtPropagationTimeUncert=True)))
    kwargs.setdefault("ResoSeedHitAssoc", 5. )
    kwargs.setdefault("RecoveryPull", 3.)
    kwargs.setdefault("useMinuit", False)
    kwargs.setdefault("fitSegmentT0", False)
    kwargs.setdefault("SeedRefine", False)
    kwargs.setdefault("doBeamspotConstraint", True)
    
    theAlg = CompFactory.MuonR4.SegmentFittingAlg(name, **kwargs)
    result.addEventAlgo(theAlg, primary=True)
    return result

def MuonPatternRecognitionCfg(flags): 
    result = ComponentAccumulator()
    from ActsAlignmentAlgs.AlignmentAlgsConfig import ActsGeometryContextAlgCfg
    result.merge(ActsGeometryContextAlgCfg(flags))
    result.merge(MuonEtaHoughTransformAlgCfg(flags))
    result.merge(MuonPhiHoughTransformAlgCfg(flags))
    return result
