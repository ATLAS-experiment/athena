# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of SiSPSeededTrackFinder package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from TrkConfig.TrackingPassFlags import RoIStrategy
from AthenaConfiguration.Enums import ProductionStep

def SiSPSeededTrackFinderCfg(flags, name="InDetSiSpTrackFinder", **kwargs):
    acc = ComponentAccumulator()

    if "TrackTool" not in kwargs:
        from InDetConfig.SiTrackMakerConfig import SiTrackMaker_xkCfg
        kwargs.setdefault("TrackTool", acc.popToolsAndMerge(
            SiTrackMaker_xkCfg(flags)))

    if "PropagatorTool" not in kwargs:
        from TrkConfig.TrkExRungeKuttaPropagatorConfig import InDetPropagatorCfg
        InDetPropagator = acc.popToolsAndMerge(InDetPropagatorCfg(flags))
        acc.addPublicTool(InDetPropagator)
        kwargs.setdefault("PropagatorTool", InDetPropagator)

    if "TrackSummaryTool" not in kwargs:
        from TrkConfig.TrkTrackSummaryToolConfig import (
            InDetTrackSummaryToolNoHoleSearchCfg)
        kwargs.setdefault("TrackSummaryTool", acc.popToolsAndMerge(
            InDetTrackSummaryToolNoHoleSearchCfg(flags)))

    if "SeedsTool" not in kwargs:
        from InDetConfig.SiSpacePointsSeedToolConfig import (
            SiSpacePointsSeedMakerCfg)
        kwargs.setdefault("SeedsTool", acc.popToolsAndMerge(
            SiSpacePointsSeedMakerCfg(flags)))

    if flags.Tracking.ActiveConfig.usePrdAssociationTool:
        # not all classes have that property !!!
        kwargs.setdefault("PRDtoTrackMap", (
            'InDetPRDtoTrackMap' + flags.Tracking.ActiveConfig.extension))

    kwargs.setdefault("useZBoundFinding",
                      flags.Tracking.ActiveConfig.doZBoundary)
    from AthenaConfiguration.Enums import ProductionStep
    if flags.Common.ProductionStep in [ProductionStep.PileUpPretracking, ProductionStep.MinbiasPreprocessing]:
        kwargs.setdefault("EventInfoKey", flags.Overlay.BkgPrefix + "EventInfo")

    # Heavy-ion config
    kwargs.setdefault("useZvertexTool",
                      flags.Tracking.ActiveConfig.extension in ["HeavyIon", "VtxLumiHeavyIon"])
    if flags.Tracking.ActiveConfig.extension in ["HeavyIon", "VtxLumiHeavyIon"]:
        # Optimization from Igor
        kwargs.setdefault("FreeClustersCut", 2)
        kwargs.setdefault("useMBTSTimeDiff", True)

        # Z-coordinates primary vertices finder (only for collisions)
        if "ZvertexTool" not in kwargs:
            from InDetConfig.SiZvertexToolConfig import SiZvertexMaker_xkCfg
            kwargs.setdefault("ZvertexTool", acc.popToolsAndMerge(
                SiZvertexMaker_xkCfg(flags)))

    acc.addEventAlgo(CompFactory.InDet.SiSPSeededTrackFinder(
        name+flags.Tracking.ActiveConfig.extension, **kwargs))
    return acc

def TrigSiSPSeededTrackFinderCfg(flags, name="InDetTrigSiSpTrackFinder", **kwargs):
    acc = ComponentAccumulator()

    if "TracksLocation" not in kwargs:
        kwargs.setdefault("TracksLocation", flags.Tracking.ActiveConfig.trkTracks_IDTrig)

    kwargs.setdefault("SpacePointsSCTName","SCT_TrigSpacePoints")
    kwargs.setdefault("SpacePointsPixelName","PixelTrigSpacePoints")
    
    if "TrackTool" not in kwargs:
        from InDetConfig.SiTrackMakerConfig import TrigSiTrackMaker_xkCfg
        kwargs.setdefault("TrackTool", acc.popToolsAndMerge(
            TrigSiTrackMaker_xkCfg(flags)))

    #for the time being no need of InDetTrigPropagator 
    #    acc.popToolsAndMerge(InDetPropagatorCfg(flags, name="InDetTrigPropagator"))   

    if "TrackSummaryTool" not in kwargs:
        from TrkConfig.TrkTrackSummaryToolConfig import (
            InDetTrigFastTrackSummaryToolCfg)
        kwargs.setdefault("TrackSummaryTool", acc.popToolsAndMerge(
            InDetTrigFastTrackSummaryToolCfg(flags)))

    if "SeedsTool" not in kwargs:
        from InDetConfig.SiSpacePointsSeedToolConfig import (
            TrigSiSpacePointsSeedMakerCfg)
        kwargs.setdefault("SeedsTool", acc.popToolsAndMerge(
            TrigSiSpacePointsSeedMakerCfg(flags)))

    # Heavy-ion config (TODO the steering for the trigger)
    kwargs.setdefault("useMBTSTimeDiff", False)
    kwargs.setdefault("useZvertexTool", False)
    kwargs.setdefault("useZBoundFinding",
                      flags.Tracking.ActiveConfig.doZBoundary)
    if flags.Reco.EnableHI:
        # Heavy Ion optimization from Igor
        kwargs.setdefault("FreeClustersCut", 2)
    ### End of trigger-specific HI steering (TODO)
    
    if flags.Tracking.ActiveConfig.usePrdAssociationTool:
        # not all classes have that property !!!
        kwargs.setdefault("PRDtoTrackMap", (
            'InDetPRDtoTrackMap' + flags.Tracking.ActiveConfig.input_name))
    else:
        kwargs.setdefault("PRDtoTrackMap", "")

    acc.merge(SiSPSeededTrackFinderCfg(flags, name, **kwargs))
    return acc
    
def ITkSiSPSeededTrackFinderCfg(flags, name="ITkSiSpTrackFinder", **kwargs):
    acc = ComponentAccumulator()

    if "TrackTool" not in kwargs:
        from InDetConfig.SiTrackMakerConfig import ITkSiTrackMaker_xkCfg
        kwargs.setdefault("TrackTool", acc.popToolsAndMerge(
            ITkSiTrackMaker_xkCfg(flags)))

    if "PropagatorTool" not in kwargs:
        from TrkConfig.TrkExRungeKuttaPropagatorConfig import ITkPropagatorCfg
        ITkPropagator = acc.popToolsAndMerge(ITkPropagatorCfg(flags))
        acc.addPublicTool(ITkPropagator)
        kwargs.setdefault("PropagatorTool", ITkPropagator)

    if "TrackSummaryTool" not in kwargs:
        from TrkConfig.TrkTrackSummaryToolConfig import (
            ITkTrackSummaryToolNoHoleSearchCfg)
        kwargs.setdefault("TrackSummaryTool", acc.popToolsAndMerge(
            ITkTrackSummaryToolNoHoleSearchCfg(flags)))

    if "SeedsTool" not in kwargs:
        ITkSiSpacePointsSeedMaker = None

        if (flags.Tracking.ActiveConfig.extension != "Conversion" and
            flags.Tracking.ActiveConfig.doActsToAthenaSeed):
            from ActsConfig.ActsSeedingConfig import (
                ActsSiSpacePointsSeedMakerCfg)
            ITkSiSpacePointsSeedMaker = acc.popToolsAndMerge(
                ActsSiSpacePointsSeedMakerCfg(flags))
        else:
            from InDetConfig.SiSpacePointsSeedToolConfig import (
                ITkSiSpacePointsSeedMakerCfg)
            ITkSiSpacePointsSeedMaker = acc.popToolsAndMerge(
                ITkSiSpacePointsSeedMakerCfg(flags))

        kwargs.setdefault("SeedsTool", ITkSiSpacePointsSeedMaker)

    if flags.Tracking.ActiveConfig.usePrdAssociationTool:
        # not all classes have that property !!!
        kwargs.setdefault("PRDtoTrackMap", (
            'ITkPRDtoTrackMap' + flags.Tracking.ActiveConfig.extension))

    kwargs.setdefault("useZvertexTool", False)
    kwargs.setdefault("useZBoundFinding",
                      flags.Tracking.ActiveConfig.doZBoundary)
    kwargs.setdefault("ITKGeometry", True)
    kwargs.setdefault("SpacePointsSCTName", "ITkStripSpacePoints"
                      if flags.Tracking.ActiveConfig.useITkStripSeeding else "")
    kwargs.setdefault("SpacePointsPixelName", "ITkPixelSpacePoints"
                      if flags.Tracking.ActiveConfig.useITkPixelSeeding else "")
    if flags.Common.ProductionStep is ProductionStep.PileUpPretracking:
        kwargs.setdefault("EventInfoKey", flags.Overlay.BkgPrefix + "EventInfo")

    if flags.Tracking.doITkFastTracking:
        kwargs.setdefault("doFastTracking", True)
        kwargs.setdefault("writeHolesFromPattern", True)

        if 'InDetEtaDependentCutsSvc' not in kwargs:
            from InDetConfig.InDetEtaDependentCutsConfig import (
                ITkEtaDependentCutsSvcCfg)
            acc.merge(ITkEtaDependentCutsSvcCfg(flags))
            kwargs.setdefault("InDetEtaDependentCutsSvc", acc.getService(
                "ITkEtaDependentCutsSvc"+flags.Tracking.ActiveConfig.extension))

    acc.addEventAlgo(CompFactory.InDet.SiSPSeededTrackFinder(
        name+flags.Tracking.ActiveConfig.extension, **kwargs))
    return acc


def ITkSiSPSeededTrackFinderROIConvCfg(
        flags, name="ITkSiSpTrackFinderROIConv", **kwargs):
    from InDetConfig.InDetCaloClusterROISelectorConfig import (
        ITkCaloClusterROIPhiRZContainerMakerCfg)
    acc = ITkCaloClusterROIPhiRZContainerMakerCfg(flags)

    if "RegSelTool_Strip" not in kwargs:
        from RegionSelector.RegSelToolConfig import regSelTool_ITkStrip_Cfg
        kwargs.setdefault("RegSelTool_Strip", acc.popToolsAndMerge(
            regSelTool_ITkStrip_Cfg(flags)))

    kwargs.setdefault("useITkConvSeeded", True)
    kwargs.setdefault("EMROIPhiRZContainer",
                      "ITkCaloClusterROIPhiRZ15GeVUnordered")

    acc.merge(ITkSiSPSeededTrackFinderCfg(flags, name, **kwargs))
    return acc


def SiSPSeededTrackFinderRoICfg(flags, name="InDetSiSpTrackFinderRoI", **kwargs):
    acc = ComponentAccumulator()

    if "TrackTool" not in kwargs:
        from InDetConfig.SiTrackMakerConfig import SiTrackMaker_xkCfg
        kwargs.setdefault("TrackTool", acc.popToolsAndMerge(
            SiTrackMaker_xkCfg(flags)))

    if "TrackSummaryTool" not in kwargs:
        from TrkConfig.TrkTrackSummaryToolConfig import (
            InDetTrackSummaryToolNoHoleSearchCfg)
        kwargs.setdefault("TrackSummaryTool", acc.popToolsAndMerge(
            InDetTrackSummaryToolNoHoleSearchCfg(flags)))

    if "SeedsTool" not in kwargs:
        from InDetConfig.SiSpacePointsSeedToolConfig import (
            SiSpacePointsSeedMakerCfg)
        kwargs.setdefault("SeedsTool", acc.popToolsAndMerge(
            SiSpacePointsSeedMakerCfg(flags)))

    if flags.Tracking.ActiveConfig.usePrdAssociationTool:
        kwargs.setdefault("PRDtoTrackMap", (
            'InDetPRDtoTrackMap' + flags.Tracking.ActiveConfig.extension))

    if "ZWindowRoISeedTool" not in kwargs:
        if flags.Tracking.ActiveConfig.RoIStrategy is RoIStrategy.LeadTracks:
            from InDetConfig.ZWindowRoISeedToolConfig import (
                LeadTracksRoISeedToolCfg as ZWindowRoISeedToolCfg)
        elif flags.Tracking.ActiveConfig.RoIStrategy is RoIStrategy.Random:
            from InDetConfig.ZWindowRoISeedToolConfig import (
                RandomRoISeedToolCfg as ZWindowRoISeedToolCfg)
        elif flags.Tracking.ActiveConfig.RoIStrategy is RoIStrategy.File:
            from InDetConfig.ZWindowRoISeedToolConfig import (
                FileRoISeedToolCfg as ZWindowRoISeedToolCfg)
        elif flags.Tracking.ActiveConfig.RoIStrategy is RoIStrategy.TruthHS:
            from InDetConfig.ZWindowRoISeedToolConfig import (
                TruthHSRoISeedToolCfg as ZWindowRoISeedToolCfg)
        kwargs.setdefault("ZWindowRoISeedTool", acc.popToolsAndMerge(
            ZWindowRoISeedToolCfg(flags)))

    kwargs.setdefault("doRandomSpot", flags.Tracking.ActiveConfig.doRandomSpot)
    if (flags.Tracking.ActiveConfig.doRandomSpot and
        "RandomRoISeedTool" not in kwargs):
        from InDetConfig.ZWindowRoISeedToolConfig import RandomRoISeedToolCfg
        kwargs.setdefault("RandomRoISeedTool", acc.popToolsAndMerge(
            RandomRoISeedToolCfg(flags)))

    kwargs.setdefault("useRoIWidth", flags.Tracking.ActiveConfig.z0WindowRoI>0)
    kwargs.setdefault("VxOutputName",
                      "RoIVertices"+flags.Tracking.ActiveConfig.extension)

    acc.addEventAlgo(CompFactory.InDet.SiSPSeededTrackFinderRoI(
        name+flags.Tracking.ActiveConfig.extension, **kwargs))

    return acc
