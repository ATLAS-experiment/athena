# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Configuration of TrkAmbiguityProcessor
# The ambiguity processor drives the ambiguity resolution step that
# rejects lower-quality candidates sharing a large number
# of associated hits with higher-quality ones.
# Special treatment for electrons (brem)
# photons (conversion) and charged particles
# in the cores of high-energy jets, can be applied.
# The track candidates resulting after the ambiguity resolution
# are then re-fitted  to obtain the final track parameter estimate.

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType


def SimpleAmbiguityProcessorToolCfg(flags,
                                    name="InDetAmbiguityProcessor",
                                    ClusterSplitProbContainer='',
                                    **kwargs):
    acc = ComponentAccumulator()

    #
    # --- set up different Scoring Tool for collisions and cosmics
    #
    if "ScoringTool" not in kwargs:
        if (flags.Beam.Type is BeamType.Cosmics):
            from InDetConfig.InDetTrackScoringToolsConfig import (
                InDetCosmicsScoringToolCfg)
            InDetAmbiScoringTool = acc.popToolsAndMerge(
                InDetCosmicsScoringToolCfg(flags))
        elif (flags.Tracking.ActiveConfig.extension == "R3LargeD0" and
              flags.Tracking.nnCutLargeD0Threshold > 0):
            # Set up NN config
            from InDetConfig.InDetTrackScoringToolsConfig import (
                InDetNNScoringToolSiCfg)
            InDetAmbiScoringTool = acc.popToolsAndMerge(
                InDetNNScoringToolSiCfg(flags))
        else:
            from InDetConfig.InDetTrackScoringToolsConfig import (
                InDetAmbiScoringToolSiCfg)
            InDetAmbiScoringTool = acc.popToolsAndMerge(
                InDetAmbiScoringToolSiCfg(flags))
        kwargs.setdefault("ScoringTool", InDetAmbiScoringTool)

    if "Fitter" not in kwargs:
        if flags.Tracking.ActiveConfig.isLowPt:
            from TrkConfig.CommonTrackFitterConfig import (
                InDetTrackFitterLowPtCfg)
            InDetTrackFitter = acc.popToolsAndMerge(
                InDetTrackFitterLowPtCfg(flags))
        else:
            from TrkConfig.CommonTrackFitterConfig import InDetTrackFitterCfg
            InDetTrackFitter = acc.popToolsAndMerge(InDetTrackFitterCfg(flags))
        kwargs.setdefault("Fitter", InDetTrackFitter)

    if "AssociationTool" not in kwargs:
        from InDetConfig.InDetAssociationToolsConfig import (
            InDetPRDtoTrackMapToolGangedPixelsCfg)
        kwargs.setdefault("AssociationTool", acc.popToolsAndMerge(
            InDetPRDtoTrackMapToolGangedPixelsCfg(flags)))

    if "TrackSummaryTool" not in kwargs:
        from TrkConfig.TrkTrackSummaryToolConfig import (
            InDetTrackSummaryToolCfg)
        kwargs.setdefault("TrackSummaryTool", acc.popToolsAndMerge(
            InDetTrackSummaryToolCfg(flags)))

    if "SelectionTool" not in kwargs:
        from InDetConfig.InDetAmbiTrackSelectionToolConfig import (
            InDetAmbiTrackSelectionToolCfg)
        kwargs.setdefault("SelectionTool", acc.popToolsAndMerge(
            InDetAmbiTrackSelectionToolCfg(flags)))

    kwargs.setdefault("InputClusterSplitProbabilityName",
                      ClusterSplitProbContainer)
    kwargs.setdefault("OutputClusterSplitProbabilityName", (
        'InDetAmbiguityProcessorSplitProb' +
        flags.Tracking.ActiveConfig.extension))
    kwargs.setdefault("SuppressTrackFit", (
        not flags.Tracking.ActiveConfig.doAmbiguityProcessorTrackFit))
    kwargs.setdefault("SuppressHoleSearch", False)
    kwargs.setdefault("tryBremFit",
                      flags.Tracking.ActiveConfig.doBremRecoverySi)
    kwargs.setdefault("caloSeededBrem", flags.Tracking.doCaloSeededBrem)
    kwargs.setdefault("pTminBrem", flags.Tracking.ActiveConfig.minPTBrem)
    kwargs.setdefault("RefitPrds", True)
    kwargs.setdefault("MatEffects", flags.Tracking.materialInteractionsType
                      if flags.Tracking.materialInteractions else 0)

    if flags.Tracking.ActiveConfig.extension == "Pixel":
        kwargs.setdefault("SuppressHoleSearch", True)

    acc.setPrivateTools(CompFactory.Trk.SimpleAmbiguityProcessorTool(
        name+flags.Tracking.ActiveConfig.extension, **kwargs))
    return acc


def SimpleAmbiguityProcessorTool_TRT_Cfg(
        flags,
        name='InDetTRT_SeededAmbiguityProcessor',
        ClusterSplitProbContainer="",
        **kwargs):
    acc = ComponentAccumulator()

    if "Fitter" not in kwargs:
        from TrkConfig.CommonTrackFitterConfig import InDetTrackFitterBTCfg
        kwargs.setdefault("Fitter", acc.popToolsAndMerge(
            InDetTrackFitterBTCfg(flags)))

    if "AssociationTool" not in kwargs:
        from InDetConfig.InDetAssociationToolsConfig import (
            InDetPRDtoTrackMapToolGangedPixelsCfg)
        kwargs.setdefault("AssociationTool", acc.popToolsAndMerge(
            InDetPRDtoTrackMapToolGangedPixelsCfg(flags)))

    #
    # --- set up special Scoring Tool for TRT seeded tracks
    #
    if "ScoringTool" not in kwargs:
        if flags.Beam.Type is BeamType.Cosmics:
            from InDetConfig.InDetTrackScoringToolsConfig import (
                InDetCosmicScoringTool_TRTCfg)
            InDetTRT_SeededScoringTool = acc.popToolsAndMerge(
                InDetCosmicScoringTool_TRTCfg(flags))
        else:
            from InDetConfig.InDetTrackScoringToolsConfig import (
                InDetTRT_SeededScoringToolCfg)
            InDetTRT_SeededScoringTool = acc.popToolsAndMerge(
                InDetTRT_SeededScoringToolCfg(flags))
        kwargs.setdefault("ScoringTool", InDetTRT_SeededScoringTool)

    if "TrackSummaryTool" not in kwargs:
        from TrkConfig.TrkTrackSummaryToolConfig import (
            InDetTrackSummaryToolCfg)
        kwargs.setdefault("TrackSummaryTool", acc.popToolsAndMerge(
            InDetTrackSummaryToolCfg(flags)))

    if "SelectionTool" not in kwargs:
        from InDetConfig.InDetAmbiTrackSelectionToolConfig import (
            InDetTRTAmbiTrackSelectionToolCfg)
        kwargs.setdefault("SelectionTool", acc.popToolsAndMerge(
            InDetTRTAmbiTrackSelectionToolCfg(flags)))

    kwargs.setdefault("InputClusterSplitProbabilityName",
                      ClusterSplitProbContainer)
    kwargs.setdefault("OutputClusterSplitProbabilityName", (
        'InDetTRT_SeededAmbiguityProcessorSplitProb' +
        flags.Tracking.ActiveConfig.extension))
    kwargs.setdefault("RefitPrds", False)
    kwargs.setdefault("SuppressTrackFit", (
        not flags.Tracking.ActiveConfig.doAmbiguityProcessorTrackFit))
    kwargs.setdefault("SuppressHoleSearch", False)
    kwargs.setdefault("ScoringTool", InDetTRT_SeededScoringTool)
    kwargs.setdefault("MatEffects", flags.Tracking.materialInteractionsType
                      if flags.Tracking.materialInteractions else 0)

    acc.setPrivateTools(
        CompFactory.Trk.SimpleAmbiguityProcessorTool(name, **kwargs))
    return acc


def SimpleAmbiguityProcessorTool_Trig_Cfg(
        flags,
        name='InDetTrig_SeededAmbiguityProcessor',
        **kwargs):
    import AthenaCommon.SystemOfUnits as Units

    acc = ComponentAccumulator()

    kwargs.setdefault("SuppressTrackFit", (
        not flags.Tracking.ActiveConfig.doAmbiguityProcessorTrackFit))
    # TODO False if flags.Tracking.ActiveConfig.name == 'cosmics' else True
    kwargs.setdefault("SuppressHoleSearch", False)
    # kwargs.setdefault("RefitPrds", False)
    # #TODO clarify this setting False if flags.Tracking.ActiveConfig.name == 'cosmics' else True
    kwargs.setdefault("tryBremFit",
                      flags.Tracking.ActiveConfig.doBremRecoverySi)
    kwargs.setdefault("pTminBrem", 5*Units.GeV)
    kwargs.setdefault("MatEffects", 3)

    if "Fitter" not in kwargs:
        from TrkConfig.TrkGlobalChi2FitterConfig import (
            InDetTrigGlobalChi2FitterCfg,InDetTrigGlobalChi2FitterCosmicsCfg)
        if flags.Tracking.ActiveConfig.input_name == "cosmics":
            kwargs.setdefault("Fitter", acc.popToolsAndMerge(
                InDetTrigGlobalChi2FitterCosmicsCfg(flags)))
        else:
            kwargs.setdefault("Fitter", acc.popToolsAndMerge(
                InDetTrigGlobalChi2FitterCfg(flags)))

    if "ScoringTool" not in kwargs:
        from InDetConfig.InDetTrackScoringToolsConfig import (
            InDetTrigAmbiScoringToolCfg)
        kwargs.setdefault("ScoringTool", acc.popToolsAndMerge(
            InDetTrigAmbiScoringToolCfg(flags)))

    if "TrackSummaryTool" not in kwargs:
        from TrkConfig.TrkTrackSummaryToolConfig import (
            InDetTrigTrackSummaryToolCfg)
        kwargs.setdefault("TrackSummaryTool", acc.popToolsAndMerge(
            InDetTrigTrackSummaryToolCfg(flags)))

    if "AssociationTool" not in kwargs:
        from InDetConfig.InDetAssociationToolsConfig import (
            TrigPRDtoTrackMapToolGangedPixelsCfg)
        kwargs.setdefault("AssociationTool", acc.popToolsAndMerge(
            TrigPRDtoTrackMapToolGangedPixelsCfg(flags)))

    if "SelectionTool" not in kwargs:
        from InDetConfig.InDetAmbiTrackSelectionToolConfig import (
            InDetTrigAmbiTrackSelectionToolCfg,InDetTrigAmbiTrackSelectionToolCosmicsCfg)
        
        if flags.Tracking.ActiveConfig.input_name == "cosmics":
            kwargs.setdefault("SelectionTool", acc.popToolsAndMerge(
                InDetTrigAmbiTrackSelectionToolCosmicsCfg(flags)))
        else:
            kwargs.setdefault("SelectionTool", acc.popToolsAndMerge(
                InDetTrigAmbiTrackSelectionToolCfg(flags,DriftCircleCutTool=None)))

    acc.setPrivateTools(
        CompFactory.Trk.SimpleAmbiguityProcessorTool(name, **kwargs))
    return acc


def DenseEnvironmentsAmbiguityScoreProcessorToolCfg(
        flags,
        name="InDetAmbiguityScoreProcessor",
        ClusterSplitProbContainer="",
        **kwargs):

    acc = ComponentAccumulator()

    # --- set up different Scoring Tool for collisions and cosmics
    if "ScoringTool" not in kwargs:
        if (flags.Beam.Type is BeamType.Cosmics):
            from InDetConfig.InDetTrackScoringToolsConfig import (
                InDetCosmicsScoringToolCfg)
            InDetAmbiScoringTool = acc.popToolsAndMerge(
                InDetCosmicsScoringToolCfg(flags))
        elif (flags.Tracking.ActiveConfig.extension == "R3LargeD0" and
              flags.Tracking.nnCutLargeD0Threshold > 0):
            from InDetConfig.InDetTrackScoringToolsConfig import (
                InDetNNScoringToolSiCfg)
            InDetAmbiScoringTool = acc.popToolsAndMerge(
                InDetNNScoringToolSiCfg(flags))
        else:
            from InDetConfig.InDetTrackScoringToolsConfig import (
                InDetAmbiScoringToolSiCfg)
            InDetAmbiScoringTool = acc.popToolsAndMerge(
                InDetAmbiScoringToolSiCfg(flags))
        kwargs.setdefault("ScoringTool", InDetAmbiScoringTool)

    if "SplitProbTool" not in kwargs:
        from InDetConfig.SiClusterizationToolConfig import (
            NnPixelClusterSplitProbToolCfg)
        kwargs.setdefault(
            "SplitProbTool",
            (acc.popToolsAndMerge(NnPixelClusterSplitProbToolCfg(flags))
             if flags.Tracking.doPixelClusterSplitting else None))

    if "AssociationTool" not in kwargs:
        from InDetConfig.InDetAssociationToolsConfig import (
            InDetPRDtoTrackMapToolGangedPixelsCfg)
        kwargs.setdefault("AssociationTool", acc.popToolsAndMerge(
            InDetPRDtoTrackMapToolGangedPixelsCfg(flags)))

    if "AssociationToolNotGanged" not in kwargs:
        from TrkConfig.TrkAssociationToolsConfig import PRDtoTrackMapToolCfg
        kwargs.setdefault("AssociationToolNotGanged",
                          acc.popToolsAndMerge(PRDtoTrackMapToolCfg(flags)))

    kwargs.setdefault("AssociationMapName", (
        "PRDtoTrackMapAmbi_"+flags.Tracking.ActiveConfig.extension))

    if flags.Tracking.ActiveConfig.useTIDE_Ambi:
        kwargs.setdefault("sharedProbCut",
                          flags.Tracking.pixelClusterSplitProb1)
        kwargs.setdefault("sharedProbCut2",
                          flags.Tracking.pixelClusterSplitProb2)

        extension = flags.Tracking.ActiveConfig.extension
        if extension == flags.Tracking.PrimaryPassConfig.value:
            extension = ""
        kwargs.setdefault("SplitClusterMap_new", (
            f"SplitClusterAmbiguityMap{extension}"))

    kwargs.setdefault("InputClusterSplitProbabilityName",
                      ClusterSplitProbContainer)
    kwargs.setdefault("OutputClusterSplitProbabilityName",
                      f"SplitProb{flags.Tracking.ActiveConfig.extension}")

    if (flags.Tracking.doTIDE_AmbiTrackMonitoring and
            flags.Tracking.ActiveConfig.extension == ""):
        from TrkConfig.TrkValToolsConfig import TrkObserverToolCfg
        TrkObserverTool = acc.popToolsAndMerge(TrkObserverToolCfg(flags))
        acc.addPublicTool(TrkObserverTool)
        kwargs.setdefault("ObserverTool", TrkObserverTool)

    acc.setPrivateTools(
        CompFactory.Trk.DenseEnvironmentsAmbiguityScoreProcessorTool(
            f"{name}{flags.Tracking.ActiveConfig.extension}", **kwargs))
    return acc


def ITkDenseEnvironmentsAmbiguityScoreProcessorToolCfg(
        flags,
        name="ITkAmbiguityScoreProcessor",
        ClusterSplitProbContainer='',
        **kwargs):

    acc = ComponentAccumulator()

    #
    # --- set up different Scoring Tool for collisions and cosmics
    #
    if "ScoringTool" not in kwargs:
        if flags.Beam.Type is BeamType.Cosmics:
            from InDetConfig.InDetTrackScoringToolsConfig import (
                ITkCosmicsScoringToolCfg)
            ITkAmbiScoringTool = acc.popToolsAndMerge(
                ITkCosmicsScoringToolCfg(flags))
        else:
            from InDetConfig.InDetTrackScoringToolsConfig import (
                ITkAmbiScoringToolCfg)
            ITkAmbiScoringTool = acc.popToolsAndMerge(
                ITkAmbiScoringToolCfg(flags))
        kwargs.setdefault("ScoringTool", ITkAmbiScoringTool)

    if "SplitProbTool" not in kwargs:
        from InDetConfig.SiClusterizationToolConfig import (
            ITkTruthPixelClusterSplitProbToolCfg)
        kwargs.setdefault(
            "SplitProbTool",
            (acc.popToolsAndMerge(ITkTruthPixelClusterSplitProbToolCfg(flags))
             if flags.Tracking.doPixelClusterSplitting else None))

    if "AssociationTool" not in kwargs:
        from InDetConfig.InDetAssociationToolsConfig import (
            ITkPRDtoTrackMapToolGangedPixelsCfg)
        kwargs.setdefault("AssociationTool", acc.popToolsAndMerge(
            ITkPRDtoTrackMapToolGangedPixelsCfg(flags)))

    if "AssociationToolNotGanged" not in kwargs:
        from TrkConfig.TrkAssociationToolsConfig import (
            PRDtoTrackMapToolCfg)
        kwargs.setdefault("AssociationToolNotGanged", acc.popToolsAndMerge(
            PRDtoTrackMapToolCfg(flags)))

    kwargs.setdefault("sharedProbCut", flags.Tracking.pixelClusterSplitProb1)
    kwargs.setdefault("sharedProbCut2", flags.Tracking.pixelClusterSplitProb2)

    extension = flags.Tracking.ActiveConfig.extension
    if extension == flags.Tracking.PrimaryPassConfig.value:
        extension = ""
    kwargs.setdefault("SplitClusterMap_new", (
        'SplitClusterAmbiguityMap' + extension))

    kwargs.setdefault("AssociationMapName", (
        'ITkPRDtoTrackMapAmbi_' + flags.Tracking.ActiveConfig.extension))
    kwargs.setdefault("InputClusterSplitProbabilityName", (
        ClusterSplitProbContainer))
    kwargs.setdefault("OutputClusterSplitProbabilityName", (
        'SplitProb'+flags.Tracking.ActiveConfig.extension))

    acc.setPrivateTools(
        CompFactory.Trk.DenseEnvironmentsAmbiguityScoreProcessorTool(
            name+flags.Tracking.ActiveConfig.extension, **kwargs))
    return acc


def DenseEnvironmentsAmbiguityProcessorToolCfg(
        flags,
        name="InDetAmbiguityProcessor",
        **kwargs):

    acc = ComponentAccumulator()

    # --- set up different Scoring Tool for collisions and cosmics
    if "ScoringTool" not in kwargs:
        if (flags.Beam.Type is BeamType.Cosmics):
            from InDetConfig.InDetTrackScoringToolsConfig import (
                InDetCosmicsScoringToolCfg)
            InDetAmbiScoringTool = acc.popToolsAndMerge(
                InDetCosmicsScoringToolCfg(flags))
        elif (flags.Tracking.ActiveConfig.extension == "R3LargeD0" and
              flags.Tracking.nnCutLargeD0Threshold > 0):
            from InDetConfig.InDetTrackScoringToolsConfig import (
                InDetNNScoringToolSiCfg)
            InDetAmbiScoringTool = acc.popToolsAndMerge(
                InDetNNScoringToolSiCfg(flags))
        else:
            from InDetConfig.InDetTrackScoringToolsConfig import (
                InDetAmbiScoringToolSiCfg)
            InDetAmbiScoringTool = acc.popToolsAndMerge(
                InDetAmbiScoringToolSiCfg(flags))
        kwargs.setdefault("ScoringTool", InDetAmbiScoringTool)

    if "Fitter" not in kwargs:
        fitter_list = []

        if flags.Tracking.ActiveConfig.isLowPt:
            from TrkConfig.CommonTrackFitterConfig import (
                InDetTrackFitterLowPtAmbiCfg)
            InDetTrackFitterLowPt = acc.popToolsAndMerge(
                InDetTrackFitterLowPtAmbiCfg(
                    flags,
                    name=('InDetTrackFitterLowPt' +
                          flags.Tracking.ActiveConfig.extension)))
            fitter_list.append(InDetTrackFitterLowPt)
        else:
            from TrkConfig.CommonTrackFitterConfig import (
                InDetTrackFitterAmbiCfg)
            InDetTrackFitterAmbi = acc.popToolsAndMerge(
                InDetTrackFitterAmbiCfg(
                    flags,
                    name=('InDetTrackFitterAmbi' +
                          flags.Tracking.ActiveConfig.extension)))
            fitter_list.append(InDetTrackFitterAmbi)

        kwargs.setdefault("Fitter", fitter_list)

    if "AssociationTool" not in kwargs:
        from InDetConfig.InDetAssociationToolsConfig import (
            InDetPRDtoTrackMapToolGangedPixelsCfg)
        kwargs.setdefault("AssociationTool", acc.popToolsAndMerge(
            InDetPRDtoTrackMapToolGangedPixelsCfg(flags)))

    if "TrackSummaryTool" not in kwargs:
        from TrkConfig.TrkTrackSummaryToolConfig import (
            InDetTrackSummaryToolCfg)
        kwargs.setdefault("TrackSummaryTool", acc.popToolsAndMerge(
            InDetTrackSummaryToolCfg(flags)))

    if "SelectionTool" not in kwargs:
        from InDetConfig.InDetAmbiTrackSelectionToolConfig import (
            InDetAmbiTrackSelectionToolCfg)
        kwargs.setdefault("SelectionTool", acc.popToolsAndMerge(
            InDetAmbiTrackSelectionToolCfg(flags)))

    kwargs.setdefault("AssociationMapName", (
        'PRDtoTrackMapAmbi_' + flags.Tracking.ActiveConfig.extension))
    kwargs.setdefault("InputClusterSplitProbabilityName", (
        'SplitProb'+flags.Tracking.ActiveConfig.extension))
    kwargs.setdefault("OutputClusterSplitProbabilityName", (
        'InDetAmbiguityProcessorSplitProb' +
        flags.Tracking.ActiveConfig.extension))
    kwargs.setdefault("SuppressTrackFit", (
        not flags.Tracking.ActiveConfig.doAmbiguityProcessorTrackFit))
    kwargs.setdefault("SuppressHoleSearch", False)
    kwargs.setdefault("tryBremFit",
                      flags.Tracking.ActiveConfig.doBremRecoverySi)
    kwargs.setdefault("caloSeededBrem", flags.Tracking.doCaloSeededBrem)
    kwargs.setdefault("pTminBrem", flags.Tracking.ActiveConfig.minPTBrem)
    kwargs.setdefault("RefitPrds", True)
    kwargs.setdefault("KeepHolesFromBeforeRefit", False)
    kwargs.setdefault("MatEffects", flags.Tracking.materialInteractionsType
                      if flags.Tracking.materialInteractions else 0)

    if (flags.Tracking.doTIDE_AmbiTrackMonitoring and
            flags.Tracking.ActiveConfig.extension == ""):
        from TrkConfig.TrkValToolsConfig import (
            TrkObserverToolCfg, WriterTrkObserverToolCfg)
        TrkObserverTool = acc.popToolsAndMerge(TrkObserverToolCfg(flags))
        acc.addPublicTool(TrkObserverTool)
        kwargs.setdefault("ObserverTool", TrkObserverTool)

        TrkObserverToolWriter = acc.popToolsAndMerge(
            WriterTrkObserverToolCfg(flags))
        acc.addPublicTool(TrkObserverToolWriter)
        kwargs.setdefault("ObserverToolWriter", TrkObserverToolWriter)

    acc.setPrivateTools(
        CompFactory.Trk.DenseEnvironmentsAmbiguityProcessorTool(
            name+flags.Tracking.ActiveConfig.extension, **kwargs))
    return acc


def ITkDenseEnvironmentsAmbiguityProcessorToolCfg(
        flags,
        name="ITkAmbiguityProcessor",
        **kwargs):
    acc = ComponentAccumulator()

    #
    # --- set up different Scoring Tool for collisions and cosmics
    #
    if "ScoringTool" not in kwargs:
        if flags.Beam.Type is BeamType.Cosmics:
            from InDetConfig.InDetTrackScoringToolsConfig import (
                ITkCosmicsScoringToolCfg)
            ITkAmbiScoringTool = acc.popToolsAndMerge(
                ITkCosmicsScoringToolCfg(flags))
        else:
            from InDetConfig.InDetTrackScoringToolsConfig import (
                ITkAmbiScoringToolCfg)
            ITkAmbiScoringTool = acc.popToolsAndMerge(
                ITkAmbiScoringToolCfg(flags))
        kwargs.setdefault("ScoringTool", ITkAmbiScoringTool)

    if "Fitter" not in kwargs:
        from TrkConfig.CommonTrackFitterConfig import (
            ITkTrackFitterAmbiCfg)
        fitter_list = []
        ITkTrackFitterAmbi = acc.popToolsAndMerge(
            ITkTrackFitterAmbiCfg(
                flags,
                name=('ITkTrackFitterAmbi' +
                      flags.Tracking.ActiveConfig.extension)))
        fitter_list.append(ITkTrackFitterAmbi)
        kwargs.setdefault("Fitter", fitter_list)

    if "AssociationTool" not in kwargs:
        from InDetConfig.InDetAssociationToolsConfig import (
            ITkPRDtoTrackMapToolGangedPixelsCfg)
        kwargs.setdefault("AssociationTool", acc.popToolsAndMerge(
            ITkPRDtoTrackMapToolGangedPixelsCfg(flags)))

    if "TrackSummaryTool" not in kwargs:
        from TrkConfig.TrkTrackSummaryToolConfig import (
            ITkTrackSummaryToolCfg)
        kwargs.setdefault("TrackSummaryTool", acc.popToolsAndMerge(
            ITkTrackSummaryToolCfg(flags)))

    if "SelectionTool" not in kwargs:
        from InDetConfig.InDetAmbiTrackSelectionToolConfig import (
            ITkAmbiTrackSelectionToolCfg)
        kwargs.setdefault("SelectionTool", acc.popToolsAndMerge(
            ITkAmbiTrackSelectionToolCfg(flags)))

    kwargs.setdefault("AssociationMapName", (
        'ITkPRDtoTrackMapAmbi_' + flags.Tracking.ActiveConfig.extension))
    kwargs.setdefault("InputClusterSplitProbabilityName", (
        'SplitProb'+flags.Tracking.ActiveConfig.extension))
    kwargs.setdefault("OutputClusterSplitProbabilityName", (
        'ITkAmbiguityProcessorSplitProb' +
        flags.Tracking.ActiveConfig.extension))
    kwargs.setdefault("SuppressTrackFit", (
        not flags.Tracking.ActiveConfig.doAmbiguityProcessorTrackFit))
    kwargs.setdefault("SuppressHoleSearch", False)

    # Disabled for second passes in reco
    kwargs.setdefault("tryBremFit",
                      flags.Tracking.ActiveConfig.doBremRecoverySi)
    kwargs.setdefault("caloSeededBrem", flags.Tracking.ActiveConfig.doCaloSeededBremSi)
    kwargs.setdefault("pTminBrem", flags.Tracking.ActiveConfig.minPTBrem[0])
    kwargs.setdefault("RefitPrds", True)
    kwargs.setdefault("KeepHolesFromBeforeRefit", False)
    kwargs.setdefault("MatEffects", flags.Tracking.materialInteractionsType
                      if flags.Tracking.materialInteractions else 0)

    acc.setPrivateTools(
        CompFactory.Trk.DenseEnvironmentsAmbiguityProcessorTool(
            name+flags.Tracking.ActiveConfig.extension, **kwargs))
    return acc
