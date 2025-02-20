# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def MuonPhiHoughTransformAlgCfg(flags, name = "MuonPhiHoughTransformAlg", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("downWeightPrdMultiplicity", True)
    theAlg = CompFactory.MuonR4.PhiHoughTransformAlg(name, **kwargs)
    result.addEventAlgo(theAlg, primary=True)
    return result


def MuonNSWPhiSeedFinderAlgCfg(flags, name = "MuonNswPhiSeedFinderAlg", **kwargs):
    result = ComponentAccumulator()
    theAlg = CompFactory.MuonR4.CombinatorialNSWSeedFinderAlg(name, **kwargs)
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
    kwargs.setdefault("Calibrator", result.popToolsAndMerge(MuonSpacePointCalibratorCfg(flags)))
    kwargs.setdefault("ResoSeedHitAssoc", 5. )
    kwargs.setdefault("RecoveryPull", 3.)
    kwargs.setdefault("fitSegmentT0", False)
    kwargs.setdefault("recalibInFit", True)
    kwargs.setdefault("SeedRefine", False)
    kwargs.setdefault("doBeamspotConstraint", True)
    
    theAlg = CompFactory.MuonR4.SegmentFittingAlg(name, **kwargs)
    result.addEventAlgo(theAlg, primary=True)
    return result

def MuonPatternRecognitionCfg(flags): 
    result = ComponentAccumulator()
    from ActsAlignmentAlgs.AlignmentAlgsConfig import ActsGeometryContextAlgCfg
    result.merge(ActsGeometryContextAlgCfg(flags))
    if flags.Detector.GeometrysTGC or flags.Detector.GeometryMM:
        result.merge(MuonEtaHoughTransformAlgCfg(flags,name="MuonNswEtaHoughTransformAlg", EtaHoughMaxContainer = "MuonHoughNswMaxima", SpacePointContainer = "NswSpacePoints"))
        result.merge(MuonNSWPhiSeedFinderAlgCfg(flags, name="MuonNswPhiSeedFinderAlg", CombinatorialPhiWriteKey = "MuonHoughNswSegmentSeeds", CombinatorialReadKey = "MuonHoughNswMaxima"))
    if flags.Detector.GeometryMDT or flags.Detector.GeometryRPC or flags.Detector.GeometryTGC:
        result.merge(MuonEtaHoughTransformAlgCfg(flags))
        result.merge(MuonPhiHoughTransformAlgCfg(flags))
    return result
