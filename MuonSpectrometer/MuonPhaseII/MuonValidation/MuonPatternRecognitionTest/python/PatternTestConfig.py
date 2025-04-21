# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaborationation

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def PatternVisualizationToolCfg(flags, name="PatternVisualizationTool", **kwargs):
    result = ComponentAccumulator()
    from MuonConfig.MuonDataPrepConfig import PrimaryMeasContNamesCfg
    kwargs.setdefault("PrdContainer", PrimaryMeasContNamesCfg(flags))
    if flags.Input.isMC:
        from MuonObjectMarker.ObjectMarkerConfig import TruthMeasMarkerAlgCfg
        markerAlg = result.getPrimaryAndMerge(TruthMeasMarkerAlgCfg(flags))
        kwargs.setdefault("TruthSegDecors", [markerAlg.SegmentLinkKey])
        kwargs["TruthSegDecors"] += [markerAlg.SegmentLinkKey]
    the_tool = CompFactory.MuonValR4.PatternVisualizationTool(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result


def MuonRecoChainTesterCfg(flags,name="MuonRecoChainTester", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("isMC", flags.Input.isMC)
    theAlg = CompFactory.MuonValR4.MuonRecoChainTester(name, **kwargs)
    result.addEventAlgo(theAlg, primary = True)
    return result

def MuonHoughTransformTesterCfg(flags, name = "MuonHoughTransformTester", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("isMC", flags.Input.isMC)
    seedKeys = []
    segmentKeys = []
    if flags.Detector.GeometryMDT or flags.Detector.GeometryRPC or flags.Detector.GeometryTGC:
        seedKeys+=["MuonHoughStationSegmentSeeds"]
        segmentKeys+=["R4MuonSegments"]
    if flags.Detector.GeometryMM or flags.Detector.GeometrysTGC:
        seedKeys+=["MuonHoughNswSegmentSeeds"]
    kwargs.setdefault("SegmentSeedKeys", seedKeys)
    kwargs.setdefault("SegmentKeys", segmentKeys)

    theAlg = CompFactory.MuonValR4.MuonHoughTransformTester(name, **kwargs) 
    result.addEventAlgo(theAlg, primary=True)
    return result

def LegacyMuonRecoChainCfg(flags):
    result = ComponentAccumulator()
    from MuonConfig.MuonSegmentFindingConfig import MuonLayerHoughAlgCfg, MuonSegmentFinderAlgCfg, MuonSegmentCnvAlgCfg

    result.merge(MuonLayerHoughAlgCfg(flags))
    result.merge(MuonSegmentFinderAlgCfg(flags, NSWSegmentCollectionName=""))
    ### MS track building
    from MuonConfig.MuonTrackBuildingConfig import MuPatTrackBuilderCfg
    result.merge(MuPatTrackBuilderCfg(flags))
    from xAODTrackingCnv.xAODTrackingCnvConfig import MuonStandaloneTrackParticleCnvAlgCfg
    result.merge(MuonStandaloneTrackParticleCnvAlgCfg(flags))
    ### Setup the MSOE track building
    from MuonCombinedConfig.MuonCombinedReconstructionConfig import  MuonCombinedMuonCandidateAlgCfg, MuonSegContainerMergerAlgCfg
    result.merge(MuonCombinedMuonCandidateAlgCfg(flags))
    
    ## Split the segment containers into associated & unassociated
    result.merge(MuonSegContainerMergerAlgCfg(flags, TagMaps =[], InputSegmentContainers =["TrackMuonSegments"]))
    ### Convert each of them into a xAOD container
    result.merge(MuonSegmentCnvAlgCfg(flags,
                                      SegmentContainerName="TrkMuonSegments",
                                      xAODContainerName="MuonSegments"))
    
    result.merge(MuonSegmentCnvAlgCfg(flags,
                                    name = "UnAssocMuonSegmentAlg",
                                    SegmentContainerName="UnAssocMuonTrkSegments",
                                    xAODContainerName="UnAssocMuonSegments"))

    #### Create the xAOD::Muon from the MSOE tracks
    from MuonCombinedConfig.MuonCombinedReconstructionConfig import MuonCreatorAlgCfg
    result.merge(MuonCreatorAlgCfg(flags, TagMaps=[], CreateSAmuons = True, MakeClusters= False,  
                                   ClusterContainerName=""))
    ### Mimimi there's no calo... mimimimimi    
    result.getEventAlgo("MuonCreatorAlg").MuonCreatorTool.RequireMSOEforSA = False

    ### Select muon-pair candidates that may originate from a Z-boson decay
    from DerivationFrameworkMuons.MuonsToolsConfig import DiMuonTaggingAlgCfg
    result.merge(DiMuonTaggingAlgCfg(flags, applyTrigger=False,  Mu1RequireQual = False, Mu2RequireQual = False, 
                                     UseTrackProbe = False, InvariantMassLow=60. * 1000., 
                                     TrackContainerKey = "MuonSpectrometerTrackParticles", BranchPrefix="HabemusZ"))

    #### Mark the associated segments 
    from MuonObjectMarker.ObjectMarkerConfig import SegmentMarkerAlgCfg, MeasurementMarkerAlgCfg
    result.merge(SegmentMarkerAlgCfg(flags, SelectMuons="passHabemusZ", SegmentKey="MuonSegments"))

    result.merge(MeasurementMarkerAlgCfg(flags, SelectSegments="passHabemusZ", 
                                         SegmentKey="MuonSegments", SegmentLinkKey="HabemusZ"))

    from MuonObjectMarker.ObjectMarkerConfig import MuonSegmentFitParDecorAlgCfg
    result.merge(MuonSegmentFitParDecorAlgCfg(flags, name="SegmentParDecorAlgMuonSegments", 
                                             SegmentKey="MuonSegments"))
    result.merge(MuonSegmentFitParDecorAlgCfg(flags, name="SegmentParDecorAlgUnAssoc", 
                                              SegmentKey="UnAssocMuonSegments"))

    if flags.Input.isMC:
        from MuonTruthAlgsR4.MuonTruthAlgsConfig import RecoSegmentTruthAssocCfg
        result.merge(RecoSegmentTruthAssocCfg(flags,
                                              name="TrkMuonSegmentsTruthMatchingAlg",
                                              SegmentKey="MuonSegments"))
        result.merge(RecoSegmentTruthAssocCfg(flags,
                                              name="UnAssocSegmentsTruthMatchingAlg",
                                              SegmentKey="UnAssocMuonSegments"))
    
    #### Build a view container for later n-tuple dumping
    from xAODMuonViewAlgs.ViewAlgsConfig import SegmentViewAlgCfg
    result.merge(SegmentViewAlgCfg(flags, 
                                   SegmentsKeys=["UnAssocMuonSegments", "MuonSegments"], 
                                   ViewKey="LegacyChainSegments",
                                   ExtraInputs = [] if not flags.Input.isMC else [( "SG::AuxVectorBase", "UnAssocMuonSegments.truthParticleLink"),
                                                                                  ( "SG::AuxVectorBase", "MuonSegments.truthParticleLink")]))
    
    return result

def MuonR4PatternRecoChainCfg(flags):
    result = ComponentAccumulator()
    from MuonPatternCnv.MuonPatternCnvConfig import MuonPatternCnvAlgCfg
    result.merge(MuonPatternCnvAlgCfg(flags,
                                      PatternCombiKey="R4HoughPatterns",
                                      HoughDataPerSecKey="R4HoughDataPerSec"))
    from MuonConfig.MuonSegmentFindingConfig import MuonSegmentFinderAlgCfg, MuonSegmentCnvAlgCfg
    result.merge(MuonSegmentFinderAlgCfg(flags,
                                         name="MuonSegmentFinderR4Pattern",
                                         MuonLayerHoughCombisKey="R4HoughPatterns",
                                         SegmentCollectionName="TrkMuonSegmentsFromHoughR4",
                                         NSWSegmentCollectionName=""))
    result.merge(MuonSegmentCnvAlgCfg(flags, "MuonSegmentCnvAlgFromHoughR4",
                                      SegmentContainerName="TrkMuonSegmentsFromHoughR4",
                                      xAODContainerName="MuonSegmentsFromHoughR4"))
    
    from MuonConfig.MuonTrackBuildingConfig import MuPatTrackBuilderCfg

    from xAODTrackingCnv.xAODTrackingCnvConfig import MuonStandaloneTrackParticleCnvAlgCfg
    result.merge(MuPatTrackBuilderCfg(flags, name="TrackBuildingFromHoughR4",
                                      MuonSegmentCollection = "TrkMuonSegmentsFromHoughR4",
                                      SpectrometerTrackOutputLocation="MuonTracksFromHoughR4"))
    result.merge(MuonStandaloneTrackParticleCnvAlgCfg(flags,"MuonXAODParticleConvFromHoughR4",
                                                   TrackContainerName="MuonTracksFromHoughR4",
                                                   xAODTrackParticlesFromTracksContainerName="MuonSpectrometerTrackParticlesFromHoughR4"))

    ### Decorate the segment parameters to the xAOD segment objects
    from MuonObjectMarker.ObjectMarkerConfig import MuonSegmentFitParDecorAlgCfg
    result.merge(MuonSegmentFitParDecorAlgCfg(flags, name="SegmentParDecorAlgHougR4", 
                                              SegmentKey="MuonSegmentsFromHoughR4"))
    if flags.Input.isMC:
        from MuonTruthAlgsR4.MuonTruthAlgsConfig import RecoSegmentTruthAssocCfg
        result.merge(RecoSegmentTruthAssocCfg(flags,
                                              name="MuonSegmentsFromHoughR4TruthMatching",
                                              SegmentKey="MuonSegmentsFromHoughR4"))
    return result

def MuonR4SegmentRecoChainCfg(flags):
    result = ComponentAccumulator()

    ### Convert the R4 segments into the legacy format
    from MuonSegmentCnv.MuonSegmentCnvConfig import MuonR4SegmentCnvAlgCfg
    result.merge(MuonR4SegmentCnvAlgCfg(flags))
    
    from MuonConfig.MuonTrackBuildingConfig import MuPatTrackBuilderCfg
    result.merge(MuPatTrackBuilderCfg(flags, name="TrackBuildingFromR4Segments",
                                             MuonSegmentCollection = "TrackMuonSegmentsR4",
                                             SpectrometerTrackOutputLocation="MuonTracksR4"))
    from xAODTrackingCnv.xAODTrackingCnvConfig import MuonStandaloneTrackParticleCnvAlgCfg

    result.merge(MuonStandaloneTrackParticleCnvAlgCfg(flags,name="MuonXAODParticleConvR4",
                                                      TrackContainerName="MuonTracksR4",
                                                      xAODTrackParticlesFromTracksContainerName="MuonSpectrometerTrackParticlesR4"))

    return result 

def TrackTruthMatchCfg(flags):
    result = ComponentAccumulator()
    if not flags.Input.isMC:
        return result
    from MuonTruthAlgsR4.MuonTruthAlgsConfig import TrackToTruthPartAssocCfg
    
    track_colstp = ["MuonSpectrometerTrackParticlesR4", "MuonSpectrometerTrackParticlesFromHoughR4", "MuonSpectrometerTrackParticles"]

    for trk in track_colstp:
        result.merge(TrackToTruthPartAssocCfg(flags, name=f"TrackToTruth{trk}",  TrackCollection=trk))

    return result
