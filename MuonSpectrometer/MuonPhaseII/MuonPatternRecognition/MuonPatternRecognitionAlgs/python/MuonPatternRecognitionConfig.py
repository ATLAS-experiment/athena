# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def MuonPhiHoughTransformAlgCfg(flags, name = "MuonPhiHoughTransformAlg", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("downWeightPrdMultiplicity", True)
    theAlg = CompFactory.MuonR4.PhiHoughTransformAlg(name, **kwargs)
    result.addEventAlgo(theAlg, primary=True)
    return result


def MuonNSWSegmentFinderAlgCfg(flags, name = "MuonNswSegmentFinderAlg", **kwargs):
    result = ComponentAccumulator()
    from MuonSpacePointCalibrator.CalibrationConfig import MuonSpacePointCalibratorCfg
    kwargs.setdefault("Calibrator", result.popToolsAndMerge(MuonSpacePointCalibratorCfg(flags)))
    theAlg = CompFactory.MuonR4.NswSegmentFinderAlg(name, **kwargs)
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
    kwargs.setdefault("recalibInFit", False)
    kwargs.setdefault("useFastFitter", False)
    kwargs.setdefault("doBeamspotConstraint", True)
    
    theAlg = CompFactory.MuonR4.SegmentFittingAlg(name, **kwargs)
    result.addEventAlgo(theAlg, primary=True)
    return result

def ActsMuonSegmentRefitAlgCfg(flags,name="ActsMuonSegmentRefitAlg", **kwargs):
    result = ComponentAccumulator()
    from MuonTrackFindingAlgs.TrackFindingConfig import SegmentSelectorCfg, MSTrackFitterCfg
    kwargs.setdefault("SegmentSelectionTool", result.popToolsAndMerge(SegmentSelectorCfg(flags)))
    
    kwargs.setdefault("FittingTool", result.popToolsAndMerge(MSTrackFitterCfg(flags,
                                                                              DoStraightLine=True)))       
    from ActsConfig.ActsGeometryConfig import ActsExtrapolationToolCfg
    kwargs.setdefault("ExtrapolationTool", result.popToolsAndMerge(ActsExtrapolationToolCfg(flags, MaxSteps=10000)))
    from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
    kwargs.setdefault("TrackingGeometryTool", result.getPrimaryAndMerge(ActsTrackingGeometryToolCfg(flags)))
    kwargs.setdefault("SegmentContainer", "MuonSegmentsFromR4")
    theAlg = CompFactory.MuonR4.SegmentActsRefitAlg(name, **kwargs)
    result.addEventAlgo(theAlg, primary = True)
    return result

def MuonPatternRecognitionCfg(flags): 
    result = ComponentAccumulator()
    from ActsAlignmentAlgs.AlignmentAlgsConfig import ActsGeometryContextAlgCfg
    result.merge(ActsGeometryContextAlgCfg(flags))
    segmentContainers = []
    if flags.Detector.GeometrysTGC or flags.Detector.GeometryMM:
        segmentContainers+=["MuonNswSegments"]
        result.merge(MuonEtaHoughTransformAlgCfg(flags, name="MuonNswEtaHoughTransformAlg", 
                                                        EtaHoughMaxContainer = "MuonHoughNswMaxima", 
                                                        SpacePointContainer = "NswSpacePoints"))
        result.merge(MuonNSWSegmentFinderAlgCfg(flags, name="MuonNswSegmentFinderAlg", 
                                                       MuonNswSegmentWriteKey = segmentContainers[-1], 
                                                       MuonNswSegmentSeedWriteKey = "MuonNswSegmentSeeds",
                                                       CombinatorialReadKey = "MuonHoughNswMaxima"))
       
    if flags.Detector.GeometryMDT or flags.Detector.GeometryRPC or flags.Detector.GeometryTGC:
        result.merge(MuonEtaHoughTransformAlgCfg(flags))
        result.merge(MuonPhiHoughTransformAlgCfg(flags))
        segmentContainers+=["R4MuonSegments"]
    
        result.merge(MuonSegmentFittingAlgCfg(flags,  OutSegmentContainer=segmentContainers[-1]))
        
    from MuonSegmentCnv.MuonSegmentCnvConfig import xAODSegmentCnvAlgCfg
    result.merge(xAODSegmentCnvAlgCfg(flags, InSegmentKeys = segmentContainers))
    if flags.Input.isMC:
        from MuonTruthAlgsR4.MuonTruthAlgsConfig import RecoSegmentTruthAssocCfg
        result.merge(RecoSegmentTruthAssocCfg(flags,
                                                name="MuonSegmentsFromR4TruthMatching",
                                                SegmentKey="MuonSegmentsFromR4"))
    if flags.Muon.scheduleActsReco:
        from MuonSegmentCnv.MuonSegmentCnvConfig import MuonR4SegmentCnvAlgCfg
        result.merge(MuonR4SegmentCnvAlgCfg(flags,
                                         ReadSegments = segmentContainers,
                                         WriteKey="TrackMuonSegments"))
    return result
