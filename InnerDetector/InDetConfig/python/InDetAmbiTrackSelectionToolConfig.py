# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of InDetAmbiTrackSelectionTool package

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType, LHCPeriod


def InDetAmbiTrackSelectionToolCfg(
        flags, name="InDetAmbiTrackSelectionTool", **kwargs):
    acc = ComponentAccumulator()

    if ('UseParameterization' in kwargs and
        kwargs.get('UseParameterization', False) and
        "DriftCircleCutTool" not in kwargs):
        from InDetConfig.InDetTrackSelectorToolConfig import (
            InDetTRTDriftCircleCutToolCfg)
        DriftCircleCutTool = acc.popToolsAndMerge(
            InDetTRTDriftCircleCutToolCfg(flags))
        acc.addPublicTool(DriftCircleCutTool)
        kwargs.setdefault("DriftCircleCutTool", DriftCircleCutTool)

    if "AssociationTool" not in kwargs:
        from InDetConfig.InDetAssociationToolsConfig import (
            InDetPRDtoTrackMapToolGangedPixelsCfg)
        kwargs.setdefault("AssociationTool", acc.popToolsAndMerge(
            InDetPRDtoTrackMapToolGangedPixelsCfg(flags)))

    kwargs.setdefault("minHits", flags.Tracking.ActiveConfig.minClusters)
    kwargs.setdefault("minNotShared",
                      flags.Tracking.ActiveConfig.minSiNotShared)
    kwargs.setdefault("maxShared", flags.Tracking.ActiveConfig.maxShared)
    kwargs.setdefault("minTRTHits", 0)  # used for Si only tracking !!!
    kwargs.setdefault("UseParameterization", False)
    kwargs.setdefault("Cosmics", flags.Beam.Type is BeamType.Cosmics)
    kwargs.setdefault("doPixelSplitting",
                      flags.Tracking.doPixelClusterSplitting)

    if flags.Tracking.ActiveConfig.useTIDE_Ambi:
        kwargs.setdefault("sharedProbCut",
                          flags.Tracking.pixelClusterSplitProb1)
        kwargs.setdefault("sharedProbCut2",
                          flags.Tracking.pixelClusterSplitProb2)
        kwargs.setdefault("minSiHitsToAllowSplitting",
                          8 if flags.GeoModel.Run is LHCPeriod.Run1 else 9)
        kwargs.setdefault("minUniqueSCTHits", 4)
        kwargs.setdefault("minTrackChi2ForSharedHits", 3)
        # Only allow split clusters on track withe pt greater than this MeV
        kwargs.setdefault("minPtSplit", 1000)
        # Split cluster ROI size
        kwargs.setdefault("phiWidth", 0.05)
        kwargs.setdefault("etaWidth", 0.05)

        # Only split in cluster in region of interest
        kwargs.setdefault("doEmCaloSeed", flags.Tracking.doCaloSeededAmbi)
        kwargs.setdefault("EMROIPhiRZContainer",
                          "InDetCaloClusterROIPhiRZ10GeV")
        if flags.Tracking.doCaloSeededAmbi:
            from InDetConfig.InDetCaloClusterROISelectorConfig import (
                CaloClusterROIPhiRZContainerMakerCfg)
            acc.merge(CaloClusterROIPhiRZContainerMakerCfg(flags))

        # Do special cuts in region of interest
        kwargs.setdefault("doHadCaloSeed", flags.Tracking.doCaloSeededAmbi)
        kwargs.setdefault("HadROIPhiRZContainer",
                          "InDetHadCaloClusterROIPhiRZBjet")
        if flags.Tracking.doCaloSeededAmbi:
            from InDetConfig.InDetCaloClusterROISelectorConfig import (
                HadCaloClusterROIPhiRZContainerMakerCfg)
            acc.merge(HadCaloClusterROIPhiRZContainerMakerCfg(flags))

        # Do special cuts in region of interest
        kwargs.setdefault("minPtBjetROI", 10000)
        # Split cluster ROI size
        kwargs.setdefault("phiWidthEM", 0.05)
        kwargs.setdefault("etaWidthEM", 0.05)
        # Skip ambi solver in hadronic ROI
        kwargs.setdefault("doSkipAmbiInROI", flags.Tracking.doSkipAmbiROI)

        if (flags.Tracking.doTIDE_AmbiTrackMonitoring and
                flags.Tracking.ActiveConfig.extension == ""):
            from TrkConfig.TrkValToolsConfig import TrkObserverToolCfg
            TrkObserverTool = acc.popToolsAndMerge(TrkObserverToolCfg(flags))
            acc.addPublicTool(TrkObserverTool)
            kwargs.setdefault("ObserverTool", TrkObserverTool)

    else:
        kwargs.setdefault("sharedProbCut", 0.10)

    if flags.Tracking.ActiveConfig.useTIDE_Ambi:
        AmbiTrackSelectionTool = (
            CompFactory.InDet.InDetDenseEnvAmbiTrackSelectionTool)
    else:
        AmbiTrackSelectionTool = CompFactory.InDet.InDetAmbiTrackSelectionTool

    acc.setPrivateTools(AmbiTrackSelectionTool(
        name=name+flags.Tracking.ActiveConfig.extension, **kwargs))
    return acc


def InDetTRTAmbiTrackSelectionToolCfg(
        flags, name='InDetTRT_SeededAmbiTrackSelectionTool', **kwargs):
    acc = ComponentAccumulator()

    if "DriftCircleCutTool" not in kwargs:
        from InDetConfig.InDetTrackSelectorToolConfig import (
            InDetTRTDriftCircleCutToolCfg)
        DriftCircleCutTool = acc.popToolsAndMerge(
            InDetTRTDriftCircleCutToolCfg(flags))
        acc.addPublicTool(DriftCircleCutTool)
        kwargs.setdefault("DriftCircleCutTool", DriftCircleCutTool)

    if "AssociationTool" not in kwargs:
        from InDetConfig.InDetAssociationToolsConfig import (
            InDetPRDtoTrackMapToolGangedPixelsCfg)
        kwargs.setdefault("AssociationTool", acc.popToolsAndMerge(
            InDetPRDtoTrackMapToolGangedPixelsCfg(flags)))

    kwargs.setdefault("minScoreShareTracks", -1.)  # off !
    kwargs.setdefault("minHits", flags.Tracking.BackTracking.minClusters)
    kwargs.setdefault("minNotShared", flags.Tracking.BackTracking.minSiNotShared)
    kwargs.setdefault("maxShared", flags.Tracking.BackTracking.maxShared)
    kwargs.setdefault("minTRTHits", flags.Tracking.BackTracking.minTRTonTrk)
    kwargs.setdefault("UseParameterization",
                      flags.Tracking.ActiveConfig.useParameterizedTRTCuts)
    kwargs.setdefault("Cosmics", flags.Beam.Type is BeamType.Cosmics)
    kwargs.setdefault("doPixelSplitting",
                      flags.Tracking.doPixelClusterSplitting)

    acc.setPrivateTools(
        CompFactory.InDet.InDetAmbiTrackSelectionTool(name, **kwargs))
    return acc


def InDetTrigAmbiTrackSelectionToolCfg(
        flags, name='InDetTrigAmbiTrackSelectionTool', **kwargs):
    acc = ComponentAccumulator()
    # TODO add AmbiTrackSelectionTool for cosmics

    if "DriftCircleCutTool" not in kwargs:
        from InDetConfig.InDetTrackSelectorToolConfig import (
            InDetTrigTRTDriftCircleCutToolCfg)
        DriftCircleCutTool = acc.popToolsAndMerge(
            InDetTrigTRTDriftCircleCutToolCfg(flags))
        acc.addPublicTool(DriftCircleCutTool)
        kwargs.setdefault("DriftCircleCutTool", DriftCircleCutTool)

    if "AssociationTool" not in kwargs:
        from InDetConfig.InDetAssociationToolsConfig import (
            TrigPRDtoTrackMapToolGangedPixelsCfg)
        kwargs.setdefault("AssociationTool", acc.popToolsAndMerge(
            TrigPRDtoTrackMapToolGangedPixelsCfg(flags)))

    kwargs.setdefault("minHits", flags.Tracking.ActiveConfig.minClusters)
    kwargs.setdefault("minNotShared",
                      flags.Tracking.ActiveConfig.minSiNotShared)
    kwargs.setdefault("maxShared", flags.Tracking.ActiveConfig.maxShared)
    kwargs.setdefault("minTRTHits", 0)  # used for Si only tracking !!!
    kwargs.setdefault("Cosmics", False)  # there is a different instance
    kwargs.setdefault("UseParameterization", False)

    acc.setPrivateTools(
        CompFactory.InDet.InDetAmbiTrackSelectionTool(name, **kwargs))
    return acc

def InDetTrigAmbiTrackSelectionToolCosmicsCfg(
        flags, name='InDetTrigAmbiTrackSelectionToolCosmics', **kwargs):
    kwargs.setdefault("Cosmics", True)
    kwargs.setdefault("minHits", 0)       #not the same minClusters setting everywhere
    kwargs.setdefault("maxTracksPerSharedPRD", 10)
    return InDetTrigAmbiTrackSelectionToolCfg(flags, name, **kwargs)

def ITkAmbiTrackSelectionToolCfg(
        flags, name="ITkAmbiTrackSelectionTool", **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("DriftCircleCutTool", None)

    if "AssociationTool" not in kwargs:
        from InDetConfig.InDetAssociationToolsConfig import (
            ITkPRDtoTrackMapToolGangedPixelsCfg)
        kwargs.setdefault("AssociationTool", acc.popToolsAndMerge(
            ITkPRDtoTrackMapToolGangedPixelsCfg(flags)))

    kwargs.setdefault("minTRTHits", 0)  # used for Si only tracking !!!
    kwargs.setdefault("UseParameterization", False)
    kwargs.setdefault("Cosmics", flags.Beam.Type is BeamType.Cosmics)
    kwargs.setdefault("doPixelSplitting",
                      flags.Tracking.doPixelClusterSplitting)

    kwargs.setdefault("sharedProbCut",
                      flags.Tracking.pixelClusterSplitProb1)
    kwargs.setdefault("sharedProbCut2",
                      flags.Tracking.pixelClusterSplitProb2)
    kwargs.setdefault("minSiHitsToAllowSplitting", 9)
    kwargs.setdefault("minUniqueSCTHits", 4)
    kwargs.setdefault("minTrackChi2ForSharedHits", 3)
    # Only allow split clusters on track withe pt greater than this MeV
    kwargs.setdefault("minPtSplit", 1000)
    # Split cluster ROI size
    kwargs.setdefault("phiWidth", 0.05)
    kwargs.setdefault("etaWidth", 0.05)

    # Only split in cluster in region of interest
    kwargs.setdefault("doEmCaloSeed", flags.Tracking.ActiveConfig.doCaloSeededAmbiSi)
    kwargs.setdefault("EMROIPhiRZContainer", "ITkCaloClusterROIPhiRZ10GeV")
    if flags.Tracking.ActiveConfig.doCaloSeededAmbiSi:
        from InDetConfig.InDetCaloClusterROISelectorConfig import (
            ITkCaloClusterROIPhiRZContainerMakerCfg)
        acc.merge(ITkCaloClusterROIPhiRZContainerMakerCfg(flags))

    # Do special cuts in region of interest
    kwargs.setdefault("doHadCaloSeed", flags.Tracking.ActiveConfig.doCaloSeededAmbiSi)
    kwargs.setdefault("HadROIPhiRZContainer", "ITkHadCaloClusterROIPhiRZBjet")
    if flags.Tracking.ActiveConfig.doCaloSeededAmbiSi:
        from InDetConfig.InDetCaloClusterROISelectorConfig import (
            ITkHadCaloClusterROIPhiRZContainerMakerCfg)
        acc.merge(ITkHadCaloClusterROIPhiRZContainerMakerCfg(flags))

    # Only allow split clusters on track withe pt greater than this MeV
    kwargs.setdefault("minPtBjetROI", 10000)
    # Split cluster ROI size
    kwargs.setdefault("phiWidthEM", 0.05)
    kwargs.setdefault("etaWidthEM", 0.05)
    # Skip ambi solver in hadronic ROI
    kwargs.setdefault("doSkipAmbiInROI", flags.Tracking.doSkipAmbiROI)

    if 'InDetEtaDependentCutsSvc' not in kwargs:
        from InDetConfig.InDetEtaDependentCutsConfig import (
            ITkEtaDependentCutsSvcCfg)
        acc.merge(ITkEtaDependentCutsSvcCfg(flags))
        kwargs.setdefault("InDetEtaDependentCutsSvc", acc.getService(
            "ITkEtaDependentCutsSvc"+flags.Tracking.ActiveConfig.extension))

    acc.setPrivateTools(CompFactory.InDet.InDetDenseEnvAmbiTrackSelectionTool(
        name=name+flags.Tracking.ActiveConfig.extension, **kwargs))
    return acc
