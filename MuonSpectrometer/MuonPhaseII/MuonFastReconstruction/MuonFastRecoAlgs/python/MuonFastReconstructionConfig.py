# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MuonFastReconstructionAlgCfg(flags, name = "MuonFastReconstructionAlg", **kwargs):
    result = ComponentAccumulator()
    SpacePointContainers = ["MuonSpacePoints"]
    if flags.Detector.GeometrysTGC or flags.Detector.GeometryMM:
        SpacePointContainers += ["NswSpacePoints"]
    kwargs.setdefault("InSpacePoints", SpacePointContainers)
    theAlg = CompFactory.MuonR4.FastReconstructionAlg(name, **kwargs)
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
                                                        SpacePointContainer = "NswSpacePointsFastReco"))
        result.merge(MuonNSWSegmentFinderAlgCfg(flags, name=f"MuonNswSegmentFinderAlg{suffix}", 
                                                       MuonNswSegmentWriteKey = segmentContainers[-1], 
                                                       MuonNswSegmentSeedWriteKey = "MuonNswSegmentSeeds",
                                                       CombinatorialReadKey = "MuonHoughNswMaxima"))
       
    if flags.Detector.GeometryMDT or flags.Detector.GeometryRPC or flags.Detector.GeometryTGC:
        result.merge(MuonEtaHoughTransformAlgCfg(flags, name = f"MuonEtaHoughTransformAlg{suffix}",
                                                        SpacePointContainer = "MuonSpacePointsFastReco"))
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