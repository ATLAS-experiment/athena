# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MuonGlobalPatternFindingAlgCfg(flags, name = "MuonGlobalPatternFindingAlg", **kwargs):
    result = ComponentAccumulator()
    from ActsAlignmentAlgs.AlignmentAlgsConfig import ActsGeometryContextAlgCfg
    result.merge(ActsGeometryContextAlgCfg(flags))
    
    # Set defaults input space point containers
    SpacePointContainers = ["MuonSpacePoints"]
    if flags.Detector.GeometrysTGC or flags.Detector.GeometryMM:
        SpacePointContainers += ["NswSpacePoints"]
    kwargs.setdefault("InSpacePoints", SpacePointContainers)

    # Set defaults for the minimum number of layers required to form a pattern
    kwargs.setdefault("MinBendingTriggerLayers", 1)
    kwargs.setdefault("MinBendingPrecisionLayers", 8)
    kwargs.setdefault("MinPhiLayers", 1)

    theAlg = CompFactory.MuonR4.MuonGlobalPatternFindingAlg(name, **kwargs)
    result.addEventAlgo(theAlg, primary=True)
    return result

def MuonFastSegmentFittingAlgCfg(flags, name = "MuonFastSegmentFittingAlg", **kwargs):
    result = ComponentAccumulator()
    from ActsAlignmentAlgs.AlignmentAlgsConfig import ActsGeometryContextAlgCfg
    result.merge(ActsGeometryContextAlgCfg(flags))

    # Set SP calibrator tool
    from MuonSpacePointCalibrator.CalibrationConfig import MuonSpacePointCalibratorCfg
    kwargs.setdefault("Calibrator", result.popToolsAndMerge(MuonSpacePointCalibratorCfg(flags)))

    # Set the segment converter tool
    from MuonSegmentCnv.MuonSegmentCnvConfig import xAODSegmentCnvToolCfg
    kwargs.setdefault("SegmentCnvTool", result.popToolsAndMerge(xAODSegmentCnvToolCfg(flags,
                                                                                      estimateHoles=False)))
    
    theAlg = CompFactory.MuonR4.MuonFastSegmentFittingAlg(name, **kwargs)
    result.addEventAlgo(theAlg, primary=True)
    return result

def MuonFastSABuilderAlgCfg(flags, name = "MuonFastSABuilderAlg", **kwargs):
    result = ComponentAccumulator()

    # Set the track seeding tool for momentum estimation
    from MuonTrackFindingAlgs.TrackFindingConfig import MsTrackSeedingToolCfg, SegmentSelectorCfg
    seedingTool_kwargs = {}
    seedingTool_kwargs["SegmentSelectionTool"] = result.popToolsAndMerge(
        SegmentSelectorCfg(flags, minRpcPhiSeedHitsBI=1,
                                  minRpcPhiSeedHitsBM=1,
                                  minRpcPhiSeedHitsBO=1,
                                  minTgcPhiSeedHitsEI=1,
                                  minTgcPhiSeedHitsEM=1))
    seedingTool_kwargs["SegmentContainer"] = ""
    kwargs.setdefault("SeedingTool", result.popToolsAndMerge(MsTrackSeedingToolCfg(flags, **seedingTool_kwargs)))
    
    theAlg = CompFactory.MuonR4.MuonFastSABuilderAlg(name, **kwargs)
    result.addEventAlgo(theAlg, primary=True)
    return result

def MuonFastSpacepointFilteringAlgCfg(flags, name = "MuonFastSpacepointFilteringAlg", **kwargs):
    result = ComponentAccumulator()

    # Disable NSW outputspace point if needed
    if not flags.Detector.GeometrysTGC and not flags.Detector.GeometryMM:
        kwargs.setdefault("OutNswSpacePoints", "")

    theAlg = CompFactory.MuonR4.MuonFastSpacepointFilteringAlg(name, **kwargs)
    result.addEventAlgo(theAlg, primary=True)
    return result

def PatternRecognitionFromFastRecoCfg(flags, suffix = ""):
    result = ComponentAccumulator()
    from ActsAlignmentAlgs.AlignmentAlgsConfig import ActsGeometryContextAlgCfg
    result.merge(ActsGeometryContextAlgCfg(flags))
    
    from MuonPatternRecognitionAlgs.MuonPatternRecognitionConfig import MuonEtaHoughTransformAlgCfg, MuonNSWSegmentFinderAlgCfg, MuonPhiHoughTransformAlgCfg, MuonSegmentFittingAlgCfg
    segmentContainers = []
    if flags.Detector.GeometrysTGC or flags.Detector.GeometryMM:
        segmentContainers+=["MuonNswSegments"]
        result.merge(MuonEtaHoughTransformAlgCfg(flags, name=f"MuonNswEtaHoughTransformAlg{suffix}", 
                                                        EtaHoughMaxContainer = "MuonHoughNswMaxima", 
                                                        SpacePointContainer = "MuonFastRecoNswSpacePoints"))
        result.merge(MuonNSWSegmentFinderAlgCfg(flags, name=f"MuonNswSegmentFinderAlg{suffix}", 
                                                       MuonNswSegmentWriteKey = segmentContainers[-1], 
                                                       MuonNswSegmentSeedWriteKey = "MuonNswSegmentSeeds",
                                                       CombinatorialReadKey = "MuonHoughNswMaxima"))
       
    if flags.Detector.GeometryMDT or flags.Detector.GeometryRPC or flags.Detector.GeometryTGC:
        result.merge(MuonEtaHoughTransformAlgCfg(flags, name = f"MuonEtaHoughTransformAlg{suffix}",
                                                        SpacePointContainer = "MuonFastRecoSpacePoints"))
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
    if flags.Muon.scheduleActsReco:
        from MuonSegmentCnv.MuonSegmentCnvConfig import MuonR4SegmentCnvAlgCfg
        result.merge(MuonR4SegmentCnvAlgCfg(flags,
                                            name=f"MuonR4SegmentCnvAlg{suffix}",
                                            ReadSegments = segmentContainers,
                                            WriteKey="TrackMuonSegments"))
    return result