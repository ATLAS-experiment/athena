"""Define method to configure VrtSecInclusive algorithm

Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def VrtSecInclusiveCfg(flags, name="VrtSecInclusive", **kwargs):

    """Return a configured VrtSecInclusive algorithm instance"""
    acc = ComponentAccumulator()

    from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
    from TrkConfig.TrkVertexFitterUtilsConfig import TrackToVertexIPEstimatorCfg
    from TrackToVertex.TrackToVertexConfig import TrackToVertexCfg
    from TrkConfig.TrkVKalVrtFitterConfig import TrkVKalVrtFitterCfg

    kwargs.setdefault("Extrapolator"                 , acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags)))
    kwargs.setdefault("VertexFitterTool"             , acc.popToolsAndMerge(TrkVKalVrtFitterCfg(flags, IterationNumber = 30)))

    if flags.Detector.GeometryPixel:
        from PixelConditionsTools.PixelConditionsSummaryConfig import (
            PixelConditionsSummaryCfg)
        kwargs.setdefault("PixelConditionsSummaryTool", acc.popToolsAndMerge(
            PixelConditionsSummaryCfg(flags,
                                      UseByteStreamFEI4 = False,
                                      UseByteStreamFEI3 = False)))
    elif flags.Detector.GeometryITkPixel:
        from PixelConditionsTools.ITkPixelConditionsSummaryConfig import (
            ITkPixelConditionsSummaryCfg)
        kwargs.setdefault("PixelConditionsSummaryTool", acc.popToolsAndMerge(
            ITkPixelConditionsSummaryCfg(flags,
                                         UseByteStreamFEI4 = False,
                                         UseByteStreamFEI3 = False)))

    TrackToVertexTool = acc.popToolsAndMerge(TrackToVertexCfg(flags))
    acc.addPublicTool(TrackToVertexTool)
    kwargs.setdefault("TrackToVertexTool"            , TrackToVertexTool)

    TrackToVertexIPEstimatorTool = acc.popToolsAndMerge(TrackToVertexIPEstimatorCfg(flags))
    acc.addPublicTool(TrackToVertexIPEstimatorTool)
    kwargs.setdefault("TrackToVertexIPEstimatorTool" , TrackToVertexIPEstimatorTool)

    kwargs.setdefault("do_d0Cut"                               , False)
    kwargs.setdefault("do_z0Cut"                               , False)
    kwargs.setdefault("doTRTPixCut"                            , True)
    kwargs.setdefault("CheckHitPatternStrategy"                , 'ExtrapolationAssist') # Either 'Classical', 'Extrapolation' or 'ExtrapolationAssist'
    kwargs.setdefault("doReassembleVertices"                   , True)
    kwargs.setdefault("doMergeByShuffling"                     , True)
    kwargs.setdefault("doMergeFinalVerticesDistance"           , True)
    kwargs.setdefault("doAssociateNonSelectedTracks"           , True)
    kwargs.setdefault("DoTruth"                                , flags.Input.isMC)
    kwargs.setdefault("FillHist"                               , True)
    kwargs.setdefault("TruthParticleFilter"                    , "Higgs")
    kwargs.setdefault("CutSctHits"                             , 2)
    kwargs.setdefault("TrkA0ErrCut"                            , 200000)
    kwargs.setdefault("TrkZErrCut"                             , 200000)
    kwargs.setdefault("a0TrkPVDstMinCut"                       , 2.0)    # track d0 min
    kwargs.setdefault("a0TrkPVDstMaxCut"                       , 300.0)  # track d0 max: default is 1000.0
    kwargs.setdefault("zTrkPVDstMinCut"                        , 0.0)    # track z0 min: default is 0.0, just for clarification
    kwargs.setdefault("zTrkPVDstMaxCut"                        , 1500.0) # track z0 max: default is 1000.0
    kwargs.setdefault("twoTrkVtxFormingD0Cut"                  , 2.0)
    kwargs.setdefault("SelVrtChi2Cut"                          , 5.)
    kwargs.setdefault("CutSharedHits"                          , 2)
    kwargs.setdefault("TrkChi2Cut"                             , 50)
    kwargs.setdefault("TruthTrkLen"                            , 1)
    kwargs.setdefault("SelTrkMaxCutoff"                        , 2000)
    kwargs.setdefault("mergeByShufflingAllowance"              , 10.)
    kwargs.setdefault("associatePtCut"                         , 1000.)
    kwargs.setdefault("associateMinDistanceToPV"               , 2.)

    acc.addEventAlgo(CompFactory.VKalVrtAthena.VrtSecInclusive(name, **kwargs))
    return acc

