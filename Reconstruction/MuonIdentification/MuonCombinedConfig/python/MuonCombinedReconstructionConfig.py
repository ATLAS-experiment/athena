# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType


def MuonCaloTagAlgCfg(flags, name="MuonCaloTagAlg", **kwargs):
    from MuonCombinedConfig.MuonCombinedRecToolsConfig import MuonCaloTagToolCfg
    result = ComponentAccumulator()
    kwargs.setdefault("MuonCombinedInDetExtensionTool",
                      result.popToolsAndMerge(MuonCaloTagToolCfg(flags)))
    kwargs.setdefault("TagMap", "caloTagMap")
    kwargs.setdefault("CombinedTrackCollection", "")
    kwargs.setdefault("METrackCollection", "")
    kwargs.setdefault("usePRDs", False)
    alg = CompFactory.MuonCombinedInDetExtensionAlg(name, **kwargs)
    result.addEventAlgo(alg, primary=True)
    return result


def LRT_MuonCaloTagAlgCfg(flags, name="MuonCaloTagAlg_LRT", **kwargs):
    kwargs.setdefault("TagMap", "caloTagMap_LRT")
    kwargs.setdefault("InDetCandidateLocation", "TrackParticleCandidateLRT")
    return MuonCaloTagAlgCfg(flags, name, **kwargs)


def MuonSegmentTagAlgCfg(flags, name="MuonSegmentTagAlg", **kwargs):
    from MuonCombinedConfig.MuonCombinedRecToolsConfig import MuonSegmentTagToolCfg
    result = MuonSegmentTagToolCfg(flags)
    muon_segment_tag_tool = result.popPrivateTools()
    kwargs.setdefault("MuonSegmentTagTool", muon_segment_tag_tool)
    kwargs.setdefault("InDetCandidateLocation",
                      "InDetCandidates" if not flags.MuonCombined.doCombinedFit else "InDetCandidatesSystemExtened")
    alg = CompFactory.MuonSegmentTagAlg(name, **kwargs)
    result.addEventAlgo(alg, primary=True)
    return result


def LRT_MuonSegmentTagAlgCfg(flags, name="MuonSegmentTagAlg_LRT", **kwargs):
    kwargs.setdefault("TagMap", "segmentTagMap_LRT")
    kwargs.setdefault("InDetCandidateLocation", "TrackParticleCandidateLRT"
                      if not flags.MuonCombined.doCombinedFit else "InDetCandidateLRT_SystemExtended")
    return MuonSegmentTagAlgCfg(flags, name, **kwargs)


def MuonInsideOutRecoAlgCfg(flags, name="MuonInsideOutRecoAlg", **kwargs):
    from MuonCombinedConfig.MuonCombinedRecToolsConfig import MuonInsideOutRecoToolCfg
    result = ComponentAccumulator()
    kwargs.setdefault("MuonCombinedInDetExtensionTool",
                      result.popToolsAndMerge(MuonInsideOutRecoToolCfg(flags)))
    kwargs.setdefault("usePRDs", True)
    kwargs.setdefault("HasCSC", flags.Detector.GeometryCSC)
    kwargs.setdefault("HasSTgc", flags.Detector.GeometrysTGC)
    kwargs.setdefault("HasMM", flags.Detector.GeometryMM)
    kwargs.setdefault("TagMap", "muGirlTagMap")
    kwargs.setdefault("SegmentCollection", "MuGirlSegments")
    kwargs.setdefault("TGCPrepDataLocation",
                      'TGC_MeasurementsAllBCs' if not flags.Muon.useTGCPriorNextBC else 'TGC_Measurements')
    kwargs.setdefault("InDetCandidateLocation",
                      "InDetCandidates" if not flags.MuonCombined.doCombinedFit else "InDetCandidatesSystemExtened")

    alg = CompFactory.MuonCombinedInDetExtensionAlg(name, **kwargs)
    result.addEventAlgo(alg, primary=True)
    return result


def LRT_MuGirlAlgCfg(flags, name="MuGirlAlg_LRT", **kwargs):
    kwargs.setdefault("TagMap", "MuGirlMap_LRT")
    kwargs.setdefault("METrackCollection", "MuGirlMETracks_LRT")
    kwargs.setdefault("SegmentCollection", "MuGirlSegments_LRT")
    kwargs.setdefault("CombinedTrackCollection",
                      "MuGirlCombinedMuonContainerLRT")
    kwargs.setdefault("InDetCandidateLocation", "TrackParticleCandidateLRT"
                      if not flags.MuonCombined.doCombinedFit else "InDetCandidateLRT_SystemExtended")
    return MuonInsideOutRecoAlgCfg(flags, name, **kwargs)


def MuGirlStauAlgCfg(flags, name="MuGirlStauAlg", **kwargs):
    from MuonCombinedConfig.MuonCombinedRecToolsConfig import MuonStauRecoToolCfg
    result = ComponentAccumulator()
    kwargs.setdefault("MuonCombinedInDetExtensionTool",
                      result.popToolsAndMerge(MuonStauRecoToolCfg(flags)))
    kwargs.setdefault("TagMap", "stauTagMap")
    kwargs.setdefault("HasCSC", flags.Detector.GeometryCSC)
    kwargs.setdefault("HasSTgc", False)
    kwargs.setdefault("HasMM", False)
    # kwargs.setdefault("TGCPrepDataLocation",
    #                   'TGC_MeasurementsAllBCs' if not flags.Muon.useTGCPriorNextBC else 'TGC_Measurements')
    # This is probably wrong, but matches old-config. #FIXME
    kwargs.setdefault("TGCPrepDataLocation", 'TGC_Measurements')
    kwargs.setdefault("CombinedTrackCollection", "MuGirlStauCombinedTracks")
    kwargs.setdefault("METrackCollection", "")
    kwargs.setdefault("SegmentCollection", "TrkStauSegments")
    kwargs.setdefault("InDetCandidateLocation",
                      "InDetCandidates" if not flags.MuonCombined.doCombinedFit else "InDetCandidatesStaus")
    alg = CompFactory.MuonCombinedInDetExtensionAlg(name, **kwargs)
    result.addEventAlgo(alg, primary=True)
    return result


def MuonCombinedMuonCandidateAlgCfg(flags, name="MuonCombinedMuonCandidateAlg", **kwargs):
    from MuonCombinedConfig.MuonCombinedRecToolsConfig import MuonCandidateToolCfg

    result = ComponentAccumulator()

    # EJWM - not completely sure where this comes from. Perhaps should be retrieved by a sub-tool?
    from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
    result.merge(CaloNoiseCondAlgCfg(flags, "totalNoise"))

    tool_kwargs = {}
    if flags.Beam.Type is BeamType.Cosmics:
        tool_kwargs.setdefault("ExtrapolationStrategy", 1)

    acc = MuonCandidateToolCfg(flags, **tool_kwargs)
    kwargs.setdefault("MuonCandidateTool", acc.popPrivateTools())
    result.merge(acc)

    alg = CompFactory.MuonCombinedMuonCandidateAlg(name, **kwargs)
    result.addEventAlgo(alg, primary=True)
    return result


def EMEO_MuonCombinedMuonCandidateAlgCfg(flags, name="MuonCombinedMuonCandidateAlg_EMEO"):
    result = ComponentAccumulator()

    from MuonCombinedConfig.MuonCombinedRecToolsConfig import EMEO_CombinedMuonTrackBuilderCfg
    from MuonCombinedConfig.MuonCombinedRecToolsConfig import MuonCandidateToolCfg

    track_builder = result.popToolsAndMerge(
        EMEO_CombinedMuonTrackBuilderCfg(flags))
    acc = MuonCandidateToolCfg(flags,
                               name="MuonCandidateTool_EMEO",
                               TrackBuilder=track_builder,
                               Commissioning=True)

    candidate_tool = result.popToolsAndMerge(acc)
    alg = CompFactory.MuonCombinedMuonCandidateAlg(name,
                                                   MuonCandidateTool=candidate_tool,
                                                   MuonSpectrometerTrackParticleLocation="EMEO_MuonSpectrometerTrackParticles",
                                                   MuonCandidateLocation="MuonCandidates_EMEO",
                                                   MSOnlyExtrapolatedTrackLocation="EMEO_MSOnlyExtrapolatedTracks")
    result.addEventAlgo(alg, primary=True)

    return result


def MuonCombinedInDetCandidateAlgCfg(flags, name="MuonCombinedInDetCandidateAlg", **kwargs):
    from InDetConfig.InDetTrackSelectorToolConfig import MuonCombinedInDetDetailedTrackSelectorToolCfg, MuonCombinedInDetDetailedForwardTrackSelectorToolCfg
    from MuonCombinedConfig.MuonCombinedRecToolsConfig import MuonSystemExtensionToolCfg
    result = MuonCombinedInDetDetailedTrackSelectorToolCfg(flags)
    kwargs.setdefault("TrackSelector", result.popPrivateTools())
    if flags.MuonCombined.doSiAssocForwardMuons and flags.Tracking.doForwardTracks:
        kwargs.setdefault("DoSiliconAssocForwardMuons", True)
        kwargs.setdefault("InDetForwardTrackSelector", result.popToolsAndMerge(
            MuonCombinedInDetDetailedForwardTrackSelectorToolCfg(flags)))

    muon_ext_tool = result.popToolsAndMerge(
        MuonSystemExtensionToolCfg(flags))
    kwargs.setdefault("MuonSystemExtensionTool", muon_ext_tool)

    # Switch off the muon system extensions if we fit combined muons
    # The MuonInDetToSystemExtensionAlg will perform the system extensions then
    kwargs.setdefault("ExtendBulk", not flags.MuonCombined.doCombinedFit)

    if flags.Reco.EnableCaloExtension and not flags.Muon.MuonTrigger:
        kwargs.setdefault("CaloExtensionLocation", ["ParticleCaloExtension"])

    alg = CompFactory.MuonCombinedInDetCandidateAlg(name, **kwargs)
    result.addEventAlgo(alg, primary=True)
    return result


def MuonInDetToMuonSystemExtensionAlgCfg(flags, name="MuonInDetToMuonSystemExtensionAlg",  **kwargs):
    result = ComponentAccumulator()
    from MuonCombinedConfig.MuonCombinedRecToolsConfig import MuonSystemExtensionToolCfg
    muon_ext_tool = result.popToolsAndMerge(
        MuonSystemExtensionToolCfg(flags))
    kwargs.setdefault("MuonSystemExtensionTool", muon_ext_tool)
    kwargs.setdefault("noNSWforSeeding", True)
    kwargs.setdefault("WriteStauCandidates", "InDetCandidatesStausPrompt"
                      if flags.Tracking.doLargeD0 else "InDetCandidatesStaus")

    if not flags.Muon.MuonTrigger:
        from MuonConfig.MuonSegmentFindingConfig import MuonLayerHoughAlgCfg
        result.merge(MuonLayerHoughAlgCfg(flags))
    kwargs.setdefault("UseOnlyHitSectors",
                      not flags.Muon.MuonTrigger and flags.Beam.Type is BeamType.Collisions)
    alg = CompFactory.MuonInDetToMuonSystemExtensionAlg(name, **kwargs)
    result.addEventAlgo(alg, primary=True)
    return result


def LRT_MuonInDetToMuonSystemExtensionAlgCfg(flags, name="MuonInDetToMuonSystemExtensionAlg_LRT",  **kwargs):
    kwargs.setdefault("WriteStauCandidates", "InDetCandidatesStausLRT")
    kwargs.setdefault("WriteInDetCandidates",
                      "InDetCandidateLRT_SystemExtended")
    kwargs.setdefault("InputInDetCandidates", "TrackParticleCandidateLRT")
    kwargs.setdefault("CombinedTagMap", "muidcoTagMap_LRT")
    return MuonInDetToMuonSystemExtensionAlgCfg(flags, name, **kwargs)


def LRT_MuonCombinedInDetCandidateAlgCfg(flags, name="MuonCombinedInDetCandidateAlg_LRT", **kwargs):
    from InDetConfig.InDetTrackSelectorToolConfig import MuonCombinedInDetDetailedForwardTrackSelectorToolCfg, MuonCombinedInDetDetailedTrackSelectorTool_LRTCfg
    result = ComponentAccumulator()

    kwargs.setdefault("TrackSelector", result.popToolsAndMerge(
        MuonCombinedInDetDetailedTrackSelectorTool_LRTCfg(flags)))

    # Use the Standard Track particle container in cases where no separate containters will be
    # saved for the LRT tracking
    kwargs.setdefault("TrackParticleLocation", ["InDetLargeD0TrackParticles"])
    kwargs.setdefault("InDetCandidateLocation", "TrackParticleCandidateLRT")
    kwargs.setdefault("DoSiliconAssocForwardMuons", False)

    kwargs.setdefault("InDetForwardTrackSelector", result.popToolsAndMerge(
        MuonCombinedInDetDetailedForwardTrackSelectorToolCfg(flags)))

    if flags.Reco.EnableCaloExtension and not flags.Muon.MuonTrigger:
        kwargs.setdefault("CaloExtensionLocation", [
                          "ParticleCaloExtension_LRT"])
    cand_alg = MuonCombinedInDetCandidateAlgCfg(flags, name, **kwargs)
    result.merge(cand_alg)
    return result


def MuonCombinedAlgCfg(flags, name="MuonCombinedAlg", **kwargs):
    from MuonCombinedConfig.MuonCombinedRecToolsConfig import MuonCombinedToolCfg
    result = ComponentAccumulator()
    kwargs.setdefault("MuonCombinedTool", result.popToolsAndMerge(
        MuonCombinedToolCfg(flags)))
    tagmaps = []
    # CombinedTagMaps must be in a 1-1 correspondence
    # with MuonCombinedTagTools.
    for h in kwargs['MuonCombinedTool'].MuonCombinedTagTools:
        if str(h).find('FitTagTool') >= 0:
            tagmaps.append('muidcoTagMap')
        elif str(h).find('StacoTagTool') >= 0:
            tagmaps.append('stacoTagMap')
    kwargs.setdefault("CombinedTagMaps", tagmaps)
    alg = CompFactory.MuonCombinedAlg(name, **kwargs)
    result.addEventAlgo(alg, primary=True)
    return result


def EMEO_MuonCombinedAlgCfg(flags, name="MuonCombinedAlg_EMEO", **kwargs):
    from MuonCombinedConfig.MuonCombinedRecToolsConfig import EMEO_MuonCombinedToolCfg
    result = ComponentAccumulator()
    kwargs.setdefault("MuonCombinedTool", result.popToolsAndMerge(
        EMEO_MuonCombinedToolCfg(flags)))
    kwargs.setdefault("CombinedTagMaps", [
                      "muidcoTagMap_EMEO", "stacoTagMap_EMEO"])
    kwargs.setdefault("MuidCombinedTracksLocation", "MuidCombinedTracks_EMEO")
    kwargs.setdefault("MuidMETracksLocation", "MuidMETracks_EMEO")
    kwargs.setdefault("MuonCandidateLocation", "MuonCandidates_EMEO")
    alg = CompFactory.MuonCombinedAlg(name, **kwargs)
    result.addEventAlgo(alg, primary=True)
    return result


def LRT_MuonCombinedAlgCfg(flags, name="MuonCombinedAlg_LRT", **kwargs):
    kwargs.setdefault("InDetCandidateLocation", "TrackParticleCandidateLRT")
    kwargs.setdefault("CombinedTagMaps", [
                      "muidcoTagMap_LRT", "stacoTagMap_LRT"])
    kwargs.setdefault("MuidCombinedTracksLocation", "MuidCombinedTracks_LRT")
    kwargs.setdefault("MuidMETracksLocation", "MuidMETracks_LRT")
    return MuonCombinedAlgCfg(flags, name, **kwargs)


def MuonCreatorAlgCfg(flags, name="MuonCreatorAlg", **kwargs):
    from MuonCombinedConfig.MuonCombinedRecToolsConfig import MuonCreatorToolCfg
    result = MuonCreatorToolCfg(flags, name="MuonCreatorTool")
    kwargs.setdefault("MuonCreatorTool", result.popPrivateTools())

    # if muGirl is off, remove "muGirlTagMap" from "TagMaps"
    # but don't set this default in case the StauCreatorAlg is created (see below)
    tag_maps = []
    if flags.MuonCombined.doMuGirl:
        tag_maps += ["muGirlTagMap"]
    if flags.MuonCombined.doStatisticalCombination:
        tag_maps += ["stacoTagMap"]
    if flags.MuonCombined.doCombinedFit:
        tag_maps += ["muidcoTagMap"]
    if flags.MuonCombined.doCaloTrkMuId:
        tag_maps += ["caloTagMap"]
    if flags.MuonCombined.doMuonSegmentTagger:
        tag_maps += ["segmentTagMap"]
    kwargs.setdefault("TagMaps", tag_maps)
    if flags.Muon.MuonTrigger:
        kwargs.setdefault("MakeClusters", False)
        kwargs.setdefault("ClusterContainerName", "")
        kwargs.setdefault("SegmentContainerName", "")
        kwargs.setdefault("TagToSegmentKey", "")
        if flags.Muon.SAMuonTrigger:
            kwargs.setdefault("CreateSAmuons", True)
            kwargs.setdefault("TagMaps", [])

    alg = CompFactory.MuonCreatorAlg(name, **kwargs)
    result.addEventAlgo(alg, primary=True)
    return result


def LRT_MuonCreatorAlgCfg(flags, name="MuonCreatorAlg_LRT", **kwargs):
    result = ComponentAccumulator()
    from MuonCombinedConfig.MuonCombinedRecToolsConfig import MuonCreatorToolCfg
    creatorTool = result.popToolsAndMerge(
        MuonCreatorToolCfg(flags, "MuonCreatorTool_LRT", RequireIDTrack=True))
    kwargs.setdefault("MuonCreatorTool", creatorTool)
    # In cases we want to switch them off we should add the flags here
    tag_maps = []
    if flags.MuonCombined.doMuGirl:
        tag_maps += ["MuGirlMap_LRT"]
    if flags.MuonCombined.doStatisticalCombination:
        tag_maps += ["stacoTagMap_LRT"]
    if flags.MuonCombined.doCombinedFit:
        tag_maps += ["muidcoTagMap_LRT"]
    if flags.MuonCombined.doCaloTrkMuId:
        tag_maps += ["caloTagMap_LRT"]
    if flags.MuonCombined.doMuonSegmentTagger:
        tag_maps += ["segmentTagMap_LRT"]
    kwargs.setdefault("TagMaps", tag_maps)
    kwargs.setdefault("MuonContainerLocation", "MuonsLRT")
    kwargs.setdefault("ExtrapolatedLocation", "ExtraPolatedMuonsLRT")
    kwargs.setdefault("MSOnlyExtrapolatedLocation",
                      "MSOnlyExtraPolatedMuonsLRT")
    kwargs.setdefault("CombinedLocation", "CombinedMuonsLRT")
    kwargs.setdefault("BuildSlowMuon", False)
    kwargs.setdefault("MakeClusters", False)
    kwargs.setdefault("ClusterContainerName", "")
    result.merge(MuonCreatorAlgCfg(flags, name=name, **kwargs))
    return result


def EMEO_MuonCreatorAlgCfg(flags, name="MuonCreatorAlg_EMEO", **kwargs):
    muon_maps = ["MuonCandidates_EMEO"]
    combined_maps = []
    if flags.MuonCombined.doStatisticalCombination:
        combined_maps += ["stacoTagMap_EMEO"]
    if flags.MuonCombined.doCombinedFit:
        combined_maps += ["muidcoTagMap_EMEO"]
    kwargs.setdefault("TagMaps", combined_maps)
    kwargs.setdefault("MuonCandidateLocation", muon_maps)
    kwargs.setdefault("MuonContainerLocation", "EMEO_Muons")
    kwargs.setdefault("ExtrapolatedLocation", "EMEO_ExtrapolatedMuon")
    kwargs.setdefault("MSOnlyExtrapolatedLocation",
                      "EMEO_MSOnlyExtrapolatedMuon")
    kwargs.setdefault("CombinedLocation", "EMEO_CombinedMuon")
    kwargs.setdefault("BuildSlowMuon", False)
    kwargs.setdefault("MakeClusters", False)
    kwargs.setdefault("ClusterContainerName", "")
    return MuonCreatorAlgCfg(flags, name=name, **kwargs)


def StauCreatorAlgCfg(flags, name="StauCreatorAlg", **kwargs):
    from MuonCombinedConfig.MuonCombinedRecToolsConfig import MuonCreatorToolCfg
    result = MuonCreatorToolCfg(
        flags, BuildStauContainer=True, name='StauCreatorTool')
    kwargs.setdefault("MuonCreatorTool", result.popPrivateTools())
    kwargs.setdefault("MuonContainerLocation", "Staus")
    kwargs.setdefault("CombinedLocation", "CombinedStau")
    kwargs.setdefault("ExtrapolatedLocation", "ExtrapolatedStau")
    kwargs.setdefault("MSOnlyExtrapolatedLocation", "MSOnlyExtrapolatedStau")
    kwargs.setdefault("MuonCandidateLocation", [])
    kwargs.setdefault("SegmentContainerName", "StauSegments")
    kwargs.setdefault("TagToSegmentKey", "")
    kwargs.setdefault("BuildSlowMuon", 1)
    kwargs.setdefault("ClusterContainerName", "SlowMuonClusterCollection")
    kwargs.setdefault("TagMaps", ["stauTagMap"])
    acc = MuonCreatorAlgCfg(flags, name, **kwargs)
    result.merge(acc)
    return result  # don't have the right algorithm being primary here, but should be okay?


def MuonSegContainerMergerAlgCfg(flags, name="MuonSegContainerMergerAlg", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("saveUnusedSegments",
                      flags.MuonCombined.writeUnAssocSegments)
    tag_maps = []
    if flags.MuonCombined.doMuGirl:
        tag_maps += ["muGirlTagMap"]
    if flags.MuonCombined.doStatisticalCombination:
        tag_maps += ["stacoTagMap"]
    if flags.MuonCombined.doCombinedFit:
        tag_maps += ["muidcoTagMap"]
    if flags.MuonCombined.doMuonSegmentTagger:
        tag_maps += ["segmentTagMap"]
    muon_maps = ["MuonCandidates"]
    if flags.Muon.runCommissioningChain:
        if flags.MuonCombined.doStatisticalCombination:
            tag_maps += ["stacoTagMap_EMEO"]
        if flags.MuonCombined.doCombinedFit:
            tag_maps += ["muidcoTagMap_EMEO"]
        muon_maps += ["MuonCandidates_EMEO"]
    if flags.Tracking.doLargeD0:
        if flags.MuonCombined.doMuGirl:
            tag_maps += ["MuGirlMap_LRT"]
        if flags.MuonCombined.doStatisticalCombination:
            tag_maps += ["stacoTagMap_LRT"]
        if flags.MuonCombined.doCombinedFit:
            tag_maps += ["muidcoTagMap_LRT"]
        if flags.MuonCombined.doMuonSegmentTagger:
            tag_maps += ["segmentTagMap_LRT"]
    kwargs.setdefault("MuonCandidateMaps", muon_maps)
    kwargs.setdefault("TagMaps", tag_maps)
    from MuonConfig.MuonRecToolsConfig import MuonAmbiProcessorCfg
    kwargs.setdefault("AmbiguityProcessor", result.popToolsAndMerge(
        MuonAmbiProcessorCfg(flags)))

    the_alg = CompFactory.MuonSegContainerMergerAlg(name, **kwargs)
    result.addEventAlgo(the_alg)
    return result


def MuonInDetExtensionMergerAlgCfg(flags, name="MuonInDetExtensionMergerAlg", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault(
        "ToMerge", ["InDetCandidatesStausPrompt", "InDetCandidatesStausLRT"])
    kwargs.setdefault("ToWrite", "InDetCandidatesStaus")
    the_alg = CompFactory.MuonInDetExtensionMergerAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary=True)
    return result

# Returns a pair vectors containing th names of the
# track particle collections associated with combined muon tracks


def GetCombinedTrkContainers(flags):
    tp_coll = []
    track_coll = []
    tp_coll = ["ExtrapolatedMuonTrackParticles",
               "CombinedMuonTrackParticles",
               "MSOnlyExtrapolatedMuonTrackParticles"]
    track_coll = ["ExtrapolatedMuonTracks",
                  "CombinedMuonTracks",
                  "MSOnlyExtrapolatedTracks"]
    if flags.Muon.runCommissioningChain:
        tp_coll += ["EMEO_ExtrapolatedMuonTrackParticles",
                    "EMEO_CombinedMuonTrackParticles",
                    "EMEO_MSOnlyExtrapolatedMuonTrackParticles"]
        track_coll += ["EMEO_ExtrapolatedMuonTracks",
                       "EMEO_CombinedMuonTracks",
                       "EMEO_MSOnlyExtrapolatedTracks"]
    if flags.MuonCombined.doMuGirl and flags.MuonCombined.doMuGirlLowBeta:
        tp_coll += ["CombinedStauTrackParticles",
                    "ExtrapolatedStauTrackParticles"]
        track_coll += ["CombinedStauTracks",
                       "ExtrapolatedStauTracks"]
    if flags.Tracking.doLargeD0:
        tp_coll += ["CombinedMuonsLRTTrackParticles",
                    "ExtraPolatedMuonsLRTTrackParticles",
                    "MSOnlyExtraPolatedMuonsLRTTrackParticles"]
        track_coll += ["CombinedMuonsLRTTracks",
                       "ExtraPolatedMuonsLRTTracks",
                       "MSOnlyExtraPolatedMuonsLRTTrackParticlesTracks"]
    return tp_coll, track_coll


def MuonTrkIDMSScatterDecorAlgCfg(flags, name="MuonCombIDMSScatterDecorAlg", **kwargs):
    result = ComponentAccumulator()
    the_alg = CompFactory.MuonTrkIDMSScatterDecorAlg(name=name, **kwargs)
    result.addEventAlgo(the_alg)
    return result


def MuonTrkAEOTDecorAlgCfg(flags, name="MuonCombAEOTDecorAlg", **kwargs):
    result = ComponentAccumulator()
    the_alg = CompFactory.MuonTrkAEOTDecorationAlg(name=name, **kwargs)
    result.addEventAlgo(the_alg)
    return result


def MuonPrecisionLayerDecorAlgCfg(flags, name="MuonPrecisionLayerDecorAlg", **kwargs):
    result = ComponentAccumulator()
    the_alg = CompFactory.MuonPrecisionLayerDecorAlg(name=name, **kwargs)
    result.addEventAlgo(the_alg)
    return result


def MuonDecorationAlgsCfg(flags):
    result = ComponentAccumulator()
    trk_cols = ["ExtrapolatedMuonTrackParticles",
                "CombinedMuonTrackParticles",
                "MSOnlyExtrapolatedMuonTrackParticles"]
    stau_coll = ["CombinedStauTrackParticles",
                 "ExtrapolatedStauTrackParticles"] if flags.MuonCombined.doMuGirl and flags.MuonCombined.doMuGirlLowBeta else []
    track_coll_lrt = ["CombinedMuonsLRTTrackParticles",
                      "ExtraPolatedMuonsLRTTrackParticles",
                      "MSOnlyExtraPolatedMuonsLRTTrackParticles"] if flags.Tracking.doLargeD0 else []

    # Decorate the muon tracks
    for coll in trk_cols + stau_coll + track_coll_lrt:
        result.merge(MuonTrkIDMSScatterDecorAlgCfg(flags, "MuonCombIDMSScatterDecorAlg"+coll,
                                                   TrackContainer=coll))

        result.merge(MuonTrkAEOTDecorAlgCfg(flags, "MuonCombAEOTDecorAlg"+coll,
                                            TrackContainer=coll))
    # Proceed with the precision layer decoration
    prec_trk_cols = ["ExtrapolatedMuonTrackParticles",
                     "CombinedMuonTrackParticles",
                     "MSOnlyExtrapolatedMuonTrackParticles"]
    result.merge(MuonPrecisionLayerDecorAlgCfg(flags, "MuonCombPrecisionLayerDecorAlg",
                                               MuonContainer="Muons",
                                               TrackContainer=prec_trk_cols))
    if flags.MuonCombined.doMuGirl and flags.MuonCombined.doMuGirlLowBeta:

        result.merge(MuonPrecisionLayerDecorAlgCfg(flags, "MuonCombStauPrecisionLayerDecorAlg",
                                                   MuonContainer="Staus",
                                                   TrackContainer=stau_coll))
    if flags.Tracking.doLargeD0:

        result.merge(MuonPrecisionLayerDecorAlgCfg(flags, "MuonCombLRTPrecisionLayerDecorAlg",
                                                   MuonContainer="MuonsLRT",
                                                   TrackContainer=track_coll_lrt))

    return result


def CombinedMuonTrackTruthAlgsCfg(flags):
    result = ComponentAccumulator()
    particle_cols, trk_cols = GetCombinedTrkContainers(flags)

    from MuonConfig.MuonTruthAlgsConfig import MuonDetailedTrackTruthMakerCfg
    result.merge(MuonDetailedTrackTruthMakerCfg(flags, name="MuonCombinedDetailedTrackTruthMaker",
                                                TrackCollectionNames=trk_cols))

    for i in range(len(trk_cols)):
        from TrkConfig.TrkTruthAlgsConfig import TrackTruthSelectorCfg, TrackParticleTruthAlgCfg
        result.merge(TrackTruthSelectorCfg(flags, tracks=trk_cols[i]))
        result.merge(TrackParticleTruthAlgCfg(flags, tracks=trk_cols[i],
                                              TrackParticleName=particle_cols[i]))

    return result


def CombinedMuonTruthAssociationAlgsCfg(flags):
    result = ComponentAccumulator()
    trk_cols = GetCombinedTrkContainers(flags)[0]

    from MuonConfig.MuonTruthAlgsConfig import MuonTruthAssociationAlgCfg
    result.merge(MuonTruthAssociationAlgCfg(flags))

    if flags.Tracking.doLargeD0:
        result.merge(MuonTruthAssociationAlgCfg(flags, name="MuonTruthAssociationAlgLRT",
                                                MuonContainerName="MuonsLRT",
                                                RecoLinkName="recoMuonLinkLRT",
                                                TrackContainers=trk_cols))

    if flags.Muon.runCommissioningChain:
        result.merge(MuonTruthAssociationAlgCfg(flags, name="MuonTruthAssociationAlg_EMEO",
                                                MuonContainerName="EMEO_Muons",
                                                RecoLinkName="",
                                                TrackContainers=trk_cols))

    if flags.MuonCombined.doMuGirl and flags.MuonCombined.doMuGirlLowBeta:
        result.merge(MuonTruthAssociationAlgCfg(flags, name="MuonTruthAssociationAlgStau",
                                                MuonContainerName="Staus",
                                                RecoLinkName="",
                                                TrackContainers=trk_cols))

    return result

def CombinedMuonOutputCfg(flags):
    from OutputStreamAthenaPool.OutputStreamConfig import addToESD, addToAOD
    result = ComponentAccumulator()

    # FIXME! Fix for ATLASRECTS-5151. Remove when better solution found.
    from TrkEventCnvTools.TrkEventCnvToolsConfig import (
        TrkEventCnvSuperToolCfg)
    result.merge(TrkEventCnvSuperToolCfg(flags))

    # Avoid old-style import from from IsolationAlgs.IsoUpdatedTrackCones import iso_vars
    # But shouldn't be here.
    iso_vars = []
    for track_pt in 500, 1000:
        for cone_str in "", "LooseCone":
            name = f"Nonprompt_All_MaxWeightTTVA{cone_str}_pt{track_pt}"
            iso_vars += ["ptconeCorrBitset_"+name,
                         "ptconecoreTrackPtrCorrection_"+name]
            for cone_size in 20, 30, 40:
                for var_str in "", "var":
                    iso_vars.append(f"pt{var_str}cone{cone_size}_{name}")

    aod_items = []
    # Segments
    aod_items += ["xAOD::MuonSegmentContainer#MuonSegments"]
    aod_items += ["xAOD::MuonSegmentAuxContainer#MuonSegmentsAux."]
    if flags.MuonCombined.writeUnAssocSegments:
        aod_items += ["xAOD::MuonSegmentContainer#UnAssocMuonSegments"]
        aod_items += ["xAOD::MuonSegmentAuxContainer#UnAssocMuonSegmentsAux."]
    if flags.MuonCombined.doMuGirlLowBeta:
        aod_items += ["xAOD::MuonSegmentContainer#StauSegments"]
        aod_items += ["xAOD::MuonSegmentAuxContainer#StauSegmentsAux."]

    particle_col, trk_col = GetCombinedTrkContainers(flags)
    if flags.Detector.EnableCalo:
        aod_items += ["xAOD::CaloClusterContainer#MuonClusterCollection"]
        aod_items += ["xAOD::CaloClusterAuxContainer#MuonClusterCollectionAux."]
        aod_items += ["CaloClusterCellLinkContainer#MuonClusterCollection_links"]
        from CaloRec.CaloThinCellsByClusterAlgConfig import CaloThinCellsByClusterAlgCfg
        result.merge(CaloThinCellsByClusterAlgCfg(flags, streamName="StreamAOD",
                                                  clusters="MuonClusterCollection",
                                                  samplings=["TileGap1", "TileGap2", "TileGap3", "TileBar0", "TileExt0", "HEC0"]))

    # Adding the xAOD content by default
    excludedAuxData = '-clusterAssociation'
    aod_items += ["xAOD::TrackParticleContainer#"+col for col in particle_col]
    aod_items += ["xAOD::TrackParticleAuxContainer#"+col +
                  "Aux." + excludedAuxData for col in particle_col]

    aod_items += ["xAOD::MuonContainer#Muons"]
    aod_items += ["xAOD::MuonContainer#MuonsLRT"]

    # FIXME! Next two lines are hack to remove derivation framework variables that are added by DRAW building and are supposed to be transient
    wp_decors = ["DFCommonMuonsTight", "DFCommonGoodMuon", "DFCommonMuonsMedium",
                 "DFCommonMuonsLoose", "InnerDetectorPt", "MuonSpectrometerPt", "isMedium_DRAWZmumu"]
    # Remove GlobalFELinks and related variables - these are links between FlowElement (FE) containers created in jet finding and muons.
    # Since these transient FE containers are not in the ESD/AOD, we should not write out these links.
    gpf_vars = ["chargedGlobalFELinks", "neutralGlobalFELinks",
                "deltaR_muon_clus_GlobalFEalg", "muon_efrac_matched_GlobalFE"]

    excludedMuonAuxData = ".-"+".-".join(iso_vars+wp_decors+gpf_vars)

    aod_items += ["xAOD::MuonAuxContainer#MuonsAux" + excludedMuonAuxData]
    aod_items += ["xAOD::MuonAuxContainer#MuonsLRTAux" + excludedMuonAuxData]

    # stau
    aod_items += ["xAOD::MuonContainer#Staus"]
    aod_items += ["xAOD::MuonAuxContainer#StausAux" + excludedMuonAuxData]
    aod_items += ["xAOD::SlowMuonContainer#SlowMuons"]
    aod_items += ["xAOD::SlowMuonAuxContainer#SlowMuonsAux."]

    if flags.Muon.runCommissioningChain:
        aod_items += ["xAOD::MuonContainer#EMEO_Muons"]
        aod_items += ["xAOD::MuonAuxContainer#EMEO_MuonsAux" +
                      excludedMuonAuxData]
    # +++++ ESD +++++

    # Tracks
    esd_items = ["TrackCollection#"+col for col in trk_col]
    esd_items += ["Trk::SegmentCollection#TrkMuonSegments"]
    if flags.MuonCombined.writeUnAssocSegments:
        esd_items += ["Trk::SegmentCollection#UnAssocMuonTrkSegments"]
    # Truth
    if flags.Input.isMC:
        esd_items += ["DetailedTrackTruthCollection#ExtrapolatedMuonTracksTruth"]
        esd_items += ["DetailedTrackTruthCollection#CombinedMuonTracksTruth"]
        esd_items += ["DetailedTrackTruthCollection#ExtrapolatedMuonTrackParticlesTruth"]
        esd_items += ["DetailedTrackTruthCollection#CombinedMuonTrackParticlesTruth"]

    if flags.MuonCombined.doCosmicSplitTracks:
        esd_items += ["TrackCollection#Combined_Tracks_split"]
        esd_items += ["TrackCollection#Tracks_split"]
        esd_items += ["Rec::TrackParticleContainer#TrackParticleCandidate_split"]

    esd_items += aod_items

    result.merge(addToESD(flags, esd_items))
    result.merge(addToAOD(flags, aod_items))

    # Leaving in for the moment, because we might still need this. Will remove once it's confirmed we don't
    # if flags.Output.doWriteESD:
    #
    # if runOnESD:
    #     # Need to make it possible to write Object ... so rename read containers
    #     from AthenaCommon.Logging import log
    #     from SGComps.AddressRemappingConfig import AddressRemappingCfg
    #     rename_maps = list(map(lambda item:'%s->%s' % (item, 'old_'+item.split('#')[1]), esd_items))
    #     log.info('Since we are running on ESD, rename inputs as follows:'+str(rename_maps))
    #     cfg.merge( AddressRemappingCfg(rename_maps) )

    return result


def MuonCombinedReconstructionCfg(flags):
    from MuonConfig.MuonGeometryConfig import MuonIdHelperSvcCfg

    # Many components need these services, so setup once here.
    result = MuonIdHelperSvcCfg(flags)

    from CaloRec.CaloTopoClusterConfig import CaloTopoClusterCfg
    result.merge(CaloTopoClusterCfg(flags))
    from TrackToCalo.CaloExtensionBuilderAlgCfg import CaloExtensionBuilderCfg
    result.merge(CaloExtensionBuilderCfg(flags))

    from TrackingGeometryCondAlg.AtlasTrackingGeometryCondAlgConfig import TrackingGeometryCondAlgCfg
    result.merge(TrackingGeometryCondAlgCfg(flags))
    from MuonConfig.MuonRecToolsConfig import MuonEDMHelperSvcCfg
    result.merge(MuonEDMHelperSvcCfg(flags))

    # Set up to read Tracks.
    from TrkConfig.TrackCollectionReadConfig import TrackCollectionReadCfg
    result.merge(TrackCollectionReadCfg(flags, 'Tracks'))

    # We do not need to schedule the Inner detector selector algs if none
    # of the combined algs executed
    if flags.MuonCombined.doMuGirl or \
       flags.MuonCombined.doStatisticalCombination or \
       flags.MuonCombined.doCaloTrkMuId or \
       flags.MuonCombined.doCombinedFit or \
       flags.MuonCombined.doMuonSegmentTagger:

        result.merge(MuonCombinedInDetCandidateAlgCfg(flags))
        if flags.Tracking.doLargeD0:
            result.merge(LRT_MuonCombinedInDetCandidateAlgCfg(flags))

    result.merge(MuonCombinedMuonCandidateAlgCfg(flags))

    if flags.MuonCombined.doStatisticalCombination or flags.MuonCombined.doCombinedFit:
        result.merge(MuonCombinedAlgCfg(flags))
        if flags.Tracking.doLargeD0:
            result.merge(LRT_MuonCombinedAlgCfg(flags))

    # Perform system extensions on ID tracks where MuidCo did not succeed
    if flags.MuonCombined.doCombinedFit:
        result.merge(MuonInDetToMuonSystemExtensionAlgCfg(flags))
        if flags.Tracking.doLargeD0:
            result.merge(LRT_MuonInDetToMuonSystemExtensionAlgCfg(flags))
    if flags.MuonCombined.doMuGirl:
        # Use only ID tracks rejected by MuidCo
        result.merge(MuonInsideOutRecoAlgCfg(flags, name="MuonInsideOutRecoAlg",
                                             InDetCandidateLocation="InDetCandidates"
                                             if not flags.MuonCombined.doCombinedFit else "InDetCandidatesSystemExtened"))
        if flags.MuonCombined.doMuGirlLowBeta:
            if flags.Tracking.doLargeD0:
                result.merge(MuonInDetExtensionMergerAlgCfg(flags))
            # Use the InDetCandidateStaus as InDetCandidates as they've also the extensions
            # from the MuidCo tracks
            result.merge(MuGirlStauAlgCfg(flags))
        if flags.Tracking.doLargeD0:
            result.merge(LRT_MuGirlAlgCfg(flags))

    if flags.MuonCombined.doCaloTrkMuId:
        result.merge(MuonCaloTagAlgCfg(flags))
        if flags.Tracking.doLargeD0:
            result.merge(LRT_MuonCaloTagAlgCfg(flags))

    if flags.MuonCombined.doMuonSegmentTagger:
        result.merge(MuonSegmentTagAlgCfg(flags))
        if flags.Tracking.doLargeD0:
            result.merge(LRT_MuonSegmentTagAlgCfg(flags))

    if flags.Muon.runCommissioningChain:
        result.merge(EMEO_MuonCombinedMuonCandidateAlgCfg(flags))
        result.merge(EMEO_MuonCombinedAlgCfg(flags))

    result.merge(MuonSegContainerMergerAlgCfg(flags))
    from MuonConfig.MuonSegmentFindingConfig import MuonSegmentCnvAlgCfg
    result.merge(MuonSegmentCnvAlgCfg(flags, "MuonSegmentCnvAlg",
                                      SegmentContainerName="TrkMuonSegments",
                                      xAODContainerName="MuonSegments"))
    if flags.MuonCombined.writeUnAssocSegments:
        result.merge(MuonSegmentCnvAlgCfg(flags, "UnAssocMuonSegmentCnvAlg",
                                          SegmentContainerName="UnAssocMuonTrkSegments",
                                          xAODContainerName="UnAssocMuonSegments"))
    if flags.MuonCombined.doMuGirlLowBeta:
        result.merge(MuonSegmentCnvAlgCfg(flags, "MuonStauSegmentCnvAlg",
                                          SegmentContainerName="TrkStauSegments",
                                          xAODContainerName="StauSegments"))
    if flags.Muon.runCommissioningChain:
        result.merge(EMEO_MuonCreatorAlgCfg(flags))
    # runs over outputs and create xAODMuon collection
    result.merge(MuonCreatorAlgCfg(flags))
    if flags.Tracking.doLargeD0:
        result.merge(LRT_MuonCreatorAlgCfg(flags))

    if flags.MuonCombined.doMuGirlLowBeta:
        # Has to be at end if not using sequencer. If we drop this requirement, can be moved above
        result.merge(StauCreatorAlgCfg(flags))

    # FIXME
    # I see errors related to unconfigured public tools. This is a quick fix, but should really move to where it is called.
    from MuonConfig.MuonRecToolsConfig import MuonTrackScoringToolCfg
    result.addPublicTool(result.popToolsAndMerge(
        MuonTrackScoringToolCfg(flags)))
    from TrkConfig.TrkTrackSummaryToolConfig import MuonCombinedTrackSummaryToolCfg
    result.addPublicTool(result.popToolsAndMerge(
        MuonCombinedTrackSummaryToolCfg(flags)))

    # post processing
    if flags.Input.isMC:
        result.merge(CombinedMuonTrackTruthAlgsCfg(flags))
        result.merge(CombinedMuonTruthAssociationAlgsCfg(flags))
        if not flags.Muon.usePhaseIIGeoSetup and  \
           "MuonSegments" not in flags.Input.Collections:
            # Segment truth association decorations, but only if they are not already there (e.g. when running on ESDs)
            from MuonConfig.MuonTruthAlgsConfig import MuonSegmentTruthAssociationAlgCfg
            result.merge(MuonSegmentTruthAssociationAlgCfg(flags))

    result.addEventAlgo(CompFactory.ClusterMatching.CaloClusterMatchLinkAlg(
        "MuonTCLinks", ClustersToDecorate="MuonClusterCollection"))
    result.merge(MuonDecorationAlgsCfg(flags))
    # Setup output
    result.merge(CombinedMuonOutputCfg(flags))

    return result


if __name__ == "__main__":
    # To run this, do e.g.
    # python -m MuonCombinedConfig.MuonCombinedReconstructionConfig --run --threads=1

    from MuonConfig.MuonConfigUtils import SetupMuonStandaloneCA
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Input.Files = [
        '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonCombinedConfig/myESD_q445_unslimmedTracks.pool.root']
    from MuonConfig.MuonConfigUtils import configureCondTag
    configureCondTag(flags)
   
    flags.Muon.useTGCPriorNextBC = False
    # This fails due to "Hough data per sector vector not found"
    flags.MuonCombined.doMuGirlLowBeta = False

   
    flags.fillFromArgs()

    flags.lock()
    flags.dump()

    cfg = SetupMuonStandaloneCA(flags)

    from MuonConfig.MuonPrepDataConvConfig import MuonPrepDataConvCfg
    cfg.merge(MuonPrepDataConvCfg(flags))

    # "Fixes" to get this working standalone i.e. from ESD
    # Configure topocluster algorithms, and associated conditions
    cfg.merge(MuonCombinedReconstructionCfg(flags))

    # This causes a stall due to missing HoughDataPerSectorVec
    cfg.getEventAlgo("MuonInDetToMuonSystemExtensionAlg").UseOnlyHitSectors = False
    cfg.getEventAlgo("MuonInDetToMuonSystemExtensionAlg_LRT").UseOnlyHitSectors = False

    # This causes a stall. See https://its.cern.ch/jira/browse/ATEAM-825
    # Leaving here for the moment, for convenience investigating this bug.
    # muonSegmentCnvTool = cfg.popToolsAndMerge( MuonSegmentConverterToolCfg(ConfigFlags, OutputLevel=0) )
    # cfg.addEventAlgo(CompFactory.xAODMaker.MuonSegmentCnvAlg("MuonSegmentCnvAlg", MuonSegmentConverterTool=muonSegmentCnvTool))

    # Keep this in, since it makes debugging easier to simply uncomment and change Algo/Service name,
    # from AthenaCommon.Constants import VERBOSE
    # tmp = cfg.getEventAlgo("MuonCombinedMuonCandidateAlg")
    # tmp.OutputLevel=VERBOSE
    # tmp = cfg.getService("StoreGateSvc")
    # tmp.OutpuDumptLevel=True

    # Need to make it possible to write Muons ... so rename read containers
    from SGComps.AddressRemappingConfig import AddressRemappingCfg

    rename_maps = ['%s#%s->%s' % ("xAOD::MuonContainer", "Muons", "old_Muons"),
                   '%s#%s->%s' % ("xAOD::MuonAuxContainer",
                                  "MuonsAux.", "old_MuonsAux."),
                   '%s#%s->%s' % ("xAOD::MuonContainer",
                                  "Muons.rpcHitIdentifier", "old_Muons.rpcHitIdentifier"),
                   '%s#%s->%s' % ("xAOD::TruthParticleContainer",
                                  "MuonTruthParticles.recoMuonLink", "MuonTruthParticles.old_recoMuonLink")
                   ]
    cfg.merge(AddressRemappingCfg(rename_maps))

    from MuonConfig.MuonConfigUtils import executeTest
    executeTest(cfg)
        
