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
    calibrator_kwargs = {}
    if flags.Input.isMC:
        #See !90511, possibility to adjust precision strips with sTgcPrecCoordErrorScale and both eta and stereo MMG with mmStripErrorScale
        calibrator_kwargs["sTgcNonPrecCoordErrorScale"] = 4.

    kwargs.setdefault("Calibrator", result.popToolsAndMerge(MuonSpacePointCalibratorCfg(flags, **calibrator_kwargs)))
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
    # Configure T0 fitting, disabled by default
    kwargs.setdefault("fitSegmentT0", False)
    if kwargs.get("fitSegmentT0"):
        kwargs.setdefault("useHessianResidual", True)
        kwargs.setdefault("recalibInFit", True)
        kwargs.setdefault("maxIterations", 400)
        # temporarily disable beamspot constraint when fitting T0, as it causes FPEs
        kwargs.setdefault("doBeamspotConstraint", False)
    # Configure pre-fitting, disabled by default
    kwargs.setdefault("useFastPreFitter", False)
    if kwargs.get("useFastPreFitter"):
        kwargs.setdefault("useFastFitter", True)
    # Configure main (full) fitting
    kwargs.setdefault("useFastFitter", False)
    kwargs.setdefault("recalibInFit", False)
    kwargs.setdefault("doBeamspotConstraint", True)
    theAlg = CompFactory.MuonR4.SegmentFittingAlg(name, **kwargs)
    result.addEventAlgo(theAlg, primary=True)
    return result

def ActsMuonSegmentRefitAlgCfg(flags,name="ActsMuonSegmentRefitAlg", **kwargs):
    result = ComponentAccumulator()
    from MuonTrackFindingAlgs.TrackFindingConfig import SegmentSelectorCfg
    kwargs.setdefault("SegmentSelectionTool", result.popToolsAndMerge(SegmentSelectorCfg(flags)))
    from MuonSpacePointCalibrator.CalibrationConfig import MuonSpacePointCalibratorCfg
    kwargs.setdefault("Calibrator", result.popToolsAndMerge(MuonSpacePointCalibratorCfg(flags)))
    from ActsConfig.ActsGeometryConfig import ActsTrackingGeometrySvcCfg
    result.merge(ActsTrackingGeometrySvcCfg(flags))
    kwargs.setdefault("SegmentContainer", "MuonSegmentsFromR4")
    theAlg = CompFactory.MuonR4.SegmentActsRefitAlg(name, **kwargs)
    result.addEventAlgo(theAlg, primary = True)
    return result

### Main config fragment for pattern recognition. The suffix allows to specify in the Event Filter whether the algothms run in RoI or FS.
def MuonPatternRecognitionCfg(flags, suffix = ""): 
    result = ComponentAccumulator()
    from ActsAlignmentAlgs.AlignmentAlgsConfig import ActsGeometryContextAlgCfg
    result.merge(ActsGeometryContextAlgCfg(flags))
    
    if flags.Muon.enableMLBucketFilter:
        from MuonInference.InferenceConfig import GraphBucketFilterToolCfg, GraphInferenceAlgCfg
        bucketTool = result.popToolsAndMerge(GraphBucketFilterToolCfg(flags))
        result.merge(GraphInferenceAlgCfg(flags,
                                          name = f"GraphInferenceAlg{suffix}",
                                          InferenceTools=[bucketTool]))
    
    segmentContainers = []
    if flags.Detector.GeometrysTGC or flags.Detector.GeometryMM:
        segmentContainers+=["MuonNswSegments"]
        result.merge(MuonEtaHoughTransformAlgCfg(flags, name=f"MuonNswEtaHoughTransformAlg{suffix}", 
                                                        EtaHoughMaxContainer = "MuonHoughNswMaxima", 
                                                        SpacePointContainer = "NswSpacePoints"))
        result.merge(MuonNSWSegmentFinderAlgCfg(flags, name=f"MuonNswSegmentFinderAlg{suffix}", 
                                                       MuonNswSegmentWriteKey = segmentContainers[-1], 
                                                       MuonNswSegmentSeedWriteKey = "MuonNswSegmentSeeds",
                                                       CombinatorialReadKey = "MuonHoughNswMaxima"))
       
    if flags.Detector.GeometryMDT or flags.Detector.GeometryRPC or flags.Detector.GeometryTGC:
        if flags.Muon.enableMLBucketFilter:
            result.merge(MuonEtaHoughTransformAlgCfg(flags, name = f"MuonEtaHoughTransformAlg{suffix}",
                                                     SpacePointContainer = "FilteredMlBuckets"))
        else:
            result.merge(MuonEtaHoughTransformAlgCfg(flags, name = f"MuonEtaHoughTransformAlg{suffix}"))
        result.merge(MuonPhiHoughTransformAlgCfg(flags, name = f"MuonPhiHoughTransformAlg{suffix}"))
        segmentContainers+=["R4MuonSegments"]
    
        result.merge(MuonSegmentFittingAlgCfg(flags, 
                                              name = f"MuonSegmentFittingAlg{suffix}",
                                              OutSegmentContainer=segmentContainers[-1]))
        
    from MuonSegmentCnv.MuonSegmentCnvConfig import xAODSegmentCnvAlgCfg
    result.merge(xAODSegmentCnvAlgCfg(flags, name = f"MuonR4xAODSegmentCnvAlg{suffix}", InSegmentKeys = segmentContainers))
    if flags.Muon.setupTruthAlgorithms:
        from MuonTruthAlgsR4.MuonTruthAlgsConfig import RecoSegmentTruthAssocCfg
        result.merge(RecoSegmentTruthAssocCfg(flags,
                                              name=f"MuonSegmentsFromR4TruthMatching{suffix}",
                                              SegmentKey="MuonSegmentsFromR4"))
    return result
