# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Configuration of TrkAmbiguitySolver
# The Ambiguity Solver algorithm wraps a dedicated
# ambiguity processor tool and performs
# the ambiguity resolution step.
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def TrkAmbiguityScoreCfg(flags,
                         name="InDetAmbiguityScore",
                         SiSPSeededTrackCollectionKey=None,
                         ClusterSplitProbContainer='',
                         **kwargs):
    acc = ComponentAccumulator()

    if flags.Tracking.ActiveConfig.useTIDE_Ambi:
        from TrkConfig.TrkAmbiguityProcessorConfig import (
            DenseEnvironmentsAmbiguityScoreProcessorToolCfg)
        InDetAmbiguityScoreProcessor = acc.popToolsAndMerge(
            DenseEnvironmentsAmbiguityScoreProcessorToolCfg(
                flags,
                ClusterSplitProbContainer=ClusterSplitProbContainer))
    else:
        InDetAmbiguityScoreProcessor = None

    #
    # --- configure Ambiguity (score) solver
    #
    kwargs.setdefault("TrackInput",
                      [SiSPSeededTrackCollectionKey])
    kwargs.setdefault("TrackOutput", (
        'ScoredMapInDetAmbiguityScore' + flags.Tracking.ActiveConfig.extension))
    kwargs.setdefault("AmbiguityScoreProcessor",
                      InDetAmbiguityScoreProcessor)  # TODO: check the case when it is None object

    acc.addEventAlgo(CompFactory.Trk.TrkAmbiguityScore(
        name+flags.Tracking.ActiveConfig.extension, **kwargs))
    return acc


def TrkAmbiguityScore_TRT_Cfg(
        flags,
        name='InDetTRT_SeededAmbiguityScore',
        **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("TrackInput",
                      ['TRTSeededTracks'])
    kwargs.setdefault("TrackOutput",
                      'ScoredMapInDetTRT_SeededAmbiguityScore')

    InDetAmbiguityScore = CompFactory.Trk.TrkAmbiguityScore(
        name=name,
        **kwargs)
    acc.addEventAlgo(InDetAmbiguityScore)
    return acc


def TrkAmbiguityScore_Trig_Cfg(
        flags,
        name='InDetTrig_SeededAmbiguityScore',
        **kwargs):

    kwargs.setdefault("TrackInput", [flags.Tracking.ActiveConfig.trkTracks_FTF])
    kwargs.setdefault("TrackOutput",
                      f"ScoreMap{flags.Tracking.ActiveConfig.input_name}")
    kwargs.setdefault("AmbiguityScoreProcessor", None)

    #allow internal useTIDE_Ambi control
    return TrkAmbiguityScoreCfg(flags, name, **kwargs)

def ITkTrkAmbiguityScoreCfg(
        flags,
        name="ITkAmbiguityScore",
        SiSPSeededTrackCollectionKey=None,
        ClusterSplitProbContainer='',
        **kwargs):
    acc = ComponentAccumulator()

    if flags.Tracking.ActiveConfig.useTIDE_Ambi:
        from TrkConfig.TrkAmbiguityProcessorConfig import (
            ITkDenseEnvironmentsAmbiguityScoreProcessorToolCfg)
        ITkAmbiguityScoreProcessor = acc.popToolsAndMerge(
            ITkDenseEnvironmentsAmbiguityScoreProcessorToolCfg(
                flags,
                ClusterSplitProbContainer=ClusterSplitProbContainer))
    else:
        ITkAmbiguityScoreProcessor = None

    #
    # --- configure Ambiguity (score) solver
    #
    kwargs.setdefault("TrackInput",
                      [SiSPSeededTrackCollectionKey])
    kwargs.setdefault("TrackOutput", (
        'ScoredMapITkAmbiguityScore' + flags.Tracking.ActiveConfig.extension))
    # TODO: check the case when it is None object
    kwargs.setdefault("AmbiguityScoreProcessor",  ITkAmbiguityScoreProcessor)

    scorename = name + flags.Tracking.ActiveConfig.extension
    kwargs.setdefault("name", scorename)
    
    acc.addEventAlgo(CompFactory.Trk.TrkAmbiguityScore(**kwargs))
    return acc

def TrigITkTrkAmbiguityScoreCfg(
        flags,
        name='InDetTrig_SeededAmbiguityScore',
        **kwargs):
    kwargs.setdefault("TrackInput", [flags.Tracking.ActiveConfig.trkTracks_FTF])
    kwargs.setdefault("TrackOutput",
                      f"ScoreMap{flags.Tracking.ActiveConfig.input_name}")
    kwargs.setdefault("AmbiguityScoreProcessor", None)
    
    return ITkTrkAmbiguityScoreCfg(flags, name, **kwargs)


def TrkAmbiguitySolverCfg(
        flags,
        name="InDetAmbiguitySolver",
        ResolvedTrackCollectionKey=None,
        ClusterSplitProbContainer='', **kwargs):
    acc = ComponentAccumulator()

    if flags.Tracking.ActiveConfig.useTIDE_Ambi:
        from TrkConfig.TrkAmbiguityProcessorConfig import (
            DenseEnvironmentsAmbiguityProcessorToolCfg)
        InDetAmbiguityProcessor = acc.popToolsAndMerge(
            DenseEnvironmentsAmbiguityProcessorToolCfg(flags))
    else:
        from TrkConfig.TrkAmbiguityProcessorConfig import (
            SimpleAmbiguityProcessorToolCfg)
        InDetAmbiguityProcessor = acc.popToolsAndMerge(
            SimpleAmbiguityProcessorToolCfg(
                flags,
                ClusterSplitProbContainer=ClusterSplitProbContainer))

    #
    # --- configure Ambiguity solver
    #
    kwargs.setdefault("TrackInput", (
        'ScoredMapInDetAmbiguityScore' + flags.Tracking.ActiveConfig.extension))
    kwargs.setdefault("TrackOutput", ResolvedTrackCollectionKey)
    kwargs.setdefault("AmbiguityProcessor", InDetAmbiguityProcessor)

    acc.addEventAlgo(CompFactory.Trk.TrkAmbiguitySolver(
        name+flags.Tracking.ActiveConfig.extension, **kwargs))
    return acc


def TrkAmbiguitySolver_TRT_Cfg(
        flags,
        name='InDetTRT_SeededAmbiguitySolver',
        ClusterSplitProbContainer='',
        **kwargs):

    acc = ComponentAccumulator()
    from TrkConfig.TrkAmbiguityProcessorConfig import (
        SimpleAmbiguityProcessorTool_TRT_Cfg)
    InDetTRT_SeededAmbiguityProcessor = acc.popToolsAndMerge(
        SimpleAmbiguityProcessorTool_TRT_Cfg(
            flags,
            ClusterSplitProbContainer=ClusterSplitProbContainer))

    kwargs.setdefault("TrackInput", 'ScoredMapInDetTRT_SeededAmbiguityScore')
    kwargs.setdefault("TrackOutput", 'ResolvedTRTSeededTracks')
    kwargs.setdefault("AmbiguityProcessor", InDetTRT_SeededAmbiguityProcessor)

    InDetTRT_SeededAmbiguitySolver = CompFactory.Trk.TrkAmbiguitySolver(
        name=name, **kwargs)
    acc.addEventAlgo(InDetTRT_SeededAmbiguitySolver)
    return acc


def TrkAmbiguitySolver_Trig_Cfg(
        flags,
        name='InDetTrig_SeededAmbiguitySolver',
        **kwargs):
    acc = ComponentAccumulator()

    if flags.Detector.GeometryITk:
        from TrkConfig.TrkAmbiguityProcessorConfig import (
            SimpleAmbiguityProcessorTool_ITkTrig_Cfg)
        processorTool = acc.popToolsAndMerge(
            SimpleAmbiguityProcessorTool_ITkTrig_Cfg(
                flags,
                name=f"TrigAmbiguityProcessor_{flags.Tracking.ActiveConfig.name}"))
    else:
        from TrkConfig.TrkAmbiguityProcessorConfig import (
            SimpleAmbiguityProcessorTool_Trig_Cfg)
        processorTool = acc.popToolsAndMerge(
            SimpleAmbiguityProcessorTool_Trig_Cfg(
                flags,
                name=f"TrigAmbiguityProcessor_{flags.Tracking.ActiveConfig.name}"))

    kwargs.setdefault(
        "TrackInput", f"ScoreMap{flags.Tracking.ActiveConfig.input_name}")
    kwargs.setdefault(
        "TrackOutput", flags.Tracking.ActiveConfig.trkTracks_IDTrig+"_Amb")
    kwargs.setdefault("AmbiguityProcessor", processorTool)

    acc.addEventAlgo(CompFactory.Trk.TrkAmbiguitySolver(name, **kwargs))
    return acc


def ITkTrkAmbiguitySolverCfg(
        flags,
        name="ITkAmbiguitySolver",
        ResolvedTrackCollectionKey=None, **kwargs):
    acc = ComponentAccumulator()

    from TrkConfig.TrkAmbiguityProcessorConfig import (
        ITkDenseEnvironmentsAmbiguityProcessorToolCfg)
    ITkAmbiguityProcessor = acc.popToolsAndMerge(
        ITkDenseEnvironmentsAmbiguityProcessorToolCfg(flags))

    #
    # --- configure Ambiguity solver
    #
    kwargs.setdefault("TrackInput", (
        'ScoredMapITkAmbiguityScore' + flags.Tracking.ActiveConfig.extension))
    kwargs.setdefault("TrackOutput", ResolvedTrackCollectionKey)
    kwargs.setdefault("AmbiguityProcessor", ITkAmbiguityProcessor)

    acc.addEventAlgo(CompFactory.Trk.TrkAmbiguitySolver(
        name+flags.Tracking.ActiveConfig.extension, **kwargs))
    return acc
