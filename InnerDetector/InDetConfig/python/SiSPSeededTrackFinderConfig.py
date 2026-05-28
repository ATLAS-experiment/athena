# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of SiSPSeededTrackFinder package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


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

    # Heavy-ion config
    kwargs.setdefault("useMBTSTimeDiff", flags.Reco.EnableHI)

    if flags.Tracking.ActiveConfig.usePrdAssociationTool:
        # not all classes have that property !!!
        kwargs.setdefault("PRDtoTrackMap", (
            'InDetPRDtoTrackMap' + flags.Tracking.ActiveConfig.extension))

    if flags.Tracking.ActiveConfig.extension == "Forward":
        kwargs.setdefault("useZvertexTool", flags.Reco.EnableHI)
        kwargs.setdefault("useZBoundFinding", False)
    else:
        kwargs.setdefault("useZvertexTool", flags.Reco.EnableHI)
        kwargs.setdefault("useZBoundFinding",
                          flags.Tracking.ActiveConfig.doZBoundary)

    # Z-coordinates primary vertices finder (only for collisions)
    if kwargs["useZvertexTool"] and "ZvertexTool" not in kwargs:
        from InDetConfig.SiZvertexToolConfig import SiZvertexMaker_xkCfg
        kwargs.setdefault("ZvertexTool", acc.popToolsAndMerge(
            SiZvertexMaker_xkCfg(flags)))

    if flags.Reco.EnableHI:
        # Heavy Ion optimization from Igor
        kwargs.setdefault("FreeClustersCut", 2)

    acc.addEventAlgo(CompFactory.InDet.SiSPSeededTrackFinder(
        name+flags.Tracking.ActiveConfig.extension, **kwargs))
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

        from ActsInterop.TrackingComponentConfigurer import (
            TrackingComponentConfigurer)
        configuration_settings = TrackingComponentConfigurer(flags)

        if (flags.Tracking.ActiveConfig.extension != "ConversionFinding" and
            configuration_settings.ActsToAthenaSeedConverter):
            from ActsTrkSeedingTool.ActsTrkSeedingToolConfig import (
                ActsTrkSiSpacePointsSeedMakerCfg)
            ITkSiSpacePointsSeedMaker = acc.popToolsAndMerge(
                ActsTrkSiSpacePointsSeedMakerCfg(flags))
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
    kwargs.setdefault("SpacePointsSCTName", "ITkStripSpacePoints")
    kwargs.setdefault("SpacePointsPixelName", "ITkPixelSpacePoints")

    if flags.Tracking.doITkFastTracking:
        kwargs.setdefault("doFastTracking", True)

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
        
    if (flags.Tracking.ActiveConfig.RoIStrategy == "LeadTracksRoISeedTool") and ("ZWindowRoISeedTool" not in kwargs):
        from InDetConfig.ZWindowRoISeedToolConfig import LeadTracksRoISeedToolCfg
        kwargs.setdefault("ZWindowRoISeedTool", acc.popToolsAndMerge(
            LeadTracksRoISeedToolCfg(flags)))
    elif (flags.Tracking.ActiveConfig.RoIStrategy == "RandomRoISeedTool") and ("ZWindowRoISeedTool" not in kwargs):
        from InDetConfig.ZWindowRoISeedToolConfig import RandomRoISeedToolCfg
        kwargs.setdefault("ZWindowRoISeedTool", acc.popToolsAndMerge(
            RandomRoISeedToolCfg(flags)))
    elif (flags.Tracking.ActiveConfig.RoIStrategy == "FileRoISeedTool") and ("ZWindowRoISeedTool" not in kwargs):
        from InDetConfig.ZWindowRoISeedToolConfig import FileRoISeedToolCfg
        kwargs.setdefault("ZWindowRoISeedTool", acc.popToolsAndMerge(
            FileRoISeedToolCfg(flags)))
    elif (flags.Tracking.ActiveConfig.RoIStrategy == "TruthHSRoISeedTool") and ("ZWindowRoISeedTool" not in kwargs):
        from InDetConfig.ZWindowRoISeedToolConfig import TruthHSRoISeedToolCfg
        kwargs.setdefault("ZWindowRoISeedTool", acc.popToolsAndMerge(
            TruthHSRoISeedToolCfg(flags)))
    else:
        print(f"ERROR. Invalid flags.Tracking.ActiveConfig.RoIStrategy value ({flags.Tracking.ActiveConfig.RoIStrategy}). Expected a valid RoI Seed Tool name. Please check.")

    kwargs.setdefault("doRandomSpot", flags.Tracking.ActiveConfig.doRandomSpot)
    if flags.Tracking.ActiveConfig.doRandomSpot and ("RandomRoISeedTool" not in kwargs):
        from InDetConfig.ZWindowRoISeedToolConfig import RandomRoISeedToolCfg
        kwargs.setdefault("RandomRoISeedTool", acc.popToolsAndMerge(
            RandomRoISeedToolCfg(flags)))

    kwargs.setdefault("RoIWidth",flags.Tracking.ActiveConfig.z0WindowRoI)
    kwargs.setdefault("VxOutputName", "RoIVertices"+flags.Tracking.ActiveConfig.extension)

    acc.addEventAlgo(CompFactory.InDet.SiSPSeededTrackFinderRoI(
        name+flags.Tracking.ActiveConfig.extension, **kwargs))

    return acc
