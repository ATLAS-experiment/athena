#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from pathlib import Path

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from InDetGNNTracking.InDetGNNTrackingConfigFlags import GNNTrackFinderToolType


def DumpObjectsCfg(
        flags, name="DumpObjects", outfile="Dump_GNN4Itk.root", **kwargs):
    '''
    create algorithm which dumps GNN training information to ROOT file
    '''
    acc = ComponentAccumulator()

    acc.addService(
        CompFactory.THistSvc(
            Output=[f"{name} DATAFILE='{outfile}', OPT='RECREATE'"]
        )
    )

    kwargs.setdefault("NtupleFileName", flags.Tracking.GNN.DumpObjects.NtupleFileName)
    kwargs.setdefault("NtupleTreeName", flags.Tracking.GNN.DumpObjects.NtupleTreeName)
    kwargs.setdefault("rootFile", True)

    acc.addEventAlgo(CompFactory.InDet.DumpObjects(name, **kwargs))
    return acc

def GNNTrackFinderToolCfg(flags, name='GNNTrackFinderTool', **kwargs):
    """Sets up a GNNTrackFinderTool tool and returns it."""
    acc = ComponentAccumulator()
    
    ### parameters for GNNTrackFinderTool
    kwargs.setdefault("embeddingDim", flags.Tracking.GNN.TrackFinder.embeddingDim)
    kwargs.setdefault("rVal", flags.Tracking.GNN.TrackFinder.rVal)
    kwargs.setdefault("knnVal", flags.Tracking.GNN.TrackFinder.knnVal)
    kwargs.setdefault("filterCut", flags.Tracking.GNN.TrackFinder.filterCut)
    kwargs.setdefault("inputMLModelDir", flags.Tracking.GNN.TrackFinder.inputMLModelDir)
    kwargs.setdefault("ccCut", flags.Tracking.GNN.TrackFinder.ccCut)
    kwargs.setdefault("walkMin", flags.Tracking.GNN.TrackFinder.walkMin)
    kwargs.setdefault("walkMax", flags.Tracking.GNN.TrackFinder.walkMax)
    kwargs.setdefault("EmbeddingFeatureNames", flags.Tracking.GNN.TrackFinder.EmbeddingFeatureNames)
    kwargs.setdefault("EmbeddingFeatureScales", flags.Tracking.GNN.TrackFinder.EmbeddingFeatureScales)
    kwargs.setdefault("FilterFeatureNames", flags.Tracking.GNN.TrackFinder.FilterFeatureNames)
    kwargs.setdefault("FilterFeatureScales", flags.Tracking.GNN.TrackFinder.FilterFeatureScales)
    kwargs.setdefault("GNNFeatureNames", flags.Tracking.GNN.TrackFinder.GNNFeatureNames)
    kwargs.setdefault("GNNFeatureScales", flags.Tracking.GNN.TrackFinder.GNNFeatureScales)

    from AthOnnxComps.OnnxRuntimeInferenceConfig import OnnxRuntimeInferenceToolCfg
    ort_exe_provider = flags.Tracking.GNN.TrackFinder.ORTExeProvider
    kwargs.setdefault("Embedding", acc.popToolsAndMerge(
        OnnxRuntimeInferenceToolCfg(flags, str(Path(kwargs["inputMLModelDir"]) / "embedding.onnx"), 
                                    ort_exe_provider, name="Embedding")
    ))
    kwargs.setdefault("Filtering", acc.popToolsAndMerge(
        OnnxRuntimeInferenceToolCfg(flags, str(Path(kwargs["inputMLModelDir"]) / "filtering.onnx"), 
                                    ort_exe_provider, name="Filtering")
    ))
    kwargs.setdefault("GNN", acc.popToolsAndMerge(
        OnnxRuntimeInferenceToolCfg(flags, str(Path(kwargs["inputMLModelDir"]) / "gnn.onnx"), 
                                    ort_exe_provider, name="GNN")
    ))
    
    acc.setPrivateTools(CompFactory.InDet.SiGNNTrackFinderTool(name, **kwargs))
    return acc


def GNNTrackFinderTritonToolCfg(flags, name='GNNTrackFinderTritonTool', **kwargs):
    """Sets up a GNNTrackFinderTritonTool tool and returns it."""
    from AthTritonComps.TritonToolConfig import TritonToolCfg

    acc = ComponentAccumulator()

    kwargs.setdefault("TritonTool", acc.popToolsAndMerge(
        TritonToolCfg(flags, flags.Tracking.GNN.Triton.model, flags.Tracking.GNN.Triton.url))
    )
    kwargs.setdefault("SpacepointFeatureTool", acc.popToolsAndMerge(SpacepointFeatureToolCfg(flags)))
    
    acc.setPrivateTools(CompFactory.InDet.GNNTrackFinderTritonTool(name, **kwargs))
    return acc  


def SeedFitterToolCfg(flags, name="SeedFitterTool", **kwargs):
    """Sets up a SeedFitter tool and returns it."""
    acc = ComponentAccumulator()
    
    ### parameters for SeedFitter
    acc.setPrivateTools(CompFactory.InDet.SeedFitterTool(name, **kwargs))
    return acc


def SpacepointFeatureToolCfg(flags, name="SpacepointFeatureTool", **kwargs):
    """Sets up a SpacepointFeature tool and returns it."""
    acc = ComponentAccumulator()
    
    ### parameters for SpacepointFeature
    acc.setPrivateTools(CompFactory.InDet.SpacepointFeatureTool(name, **kwargs))
    return acc


def GNNTrackReaderToolCfg(flags, name='GNNTrackReaderTool', **kwargs):
    """Set up a GNNTrackReader tool and return it."""
    acc = ComponentAccumulator()

    ### parameters for GNNTrackReader
    kwargs.setdefault("inputTracksDir", flags.Tracking.GNN.TrackReader.inputTracksDir)
    kwargs.setdefault("csvPrefix", flags.Tracking.GNN.TrackReader.csvPrefix)

    acc.setPrivateTools(CompFactory.InDet.GNNTrackReaderTool(name, **kwargs))
    return acc

def GNNTrackMakerCfg(flags, name="GNNTrackMaker", **kwargs):
    """Sets up a GNNTrackMaker algorithm and returns it."""

    if flags.Tracking.GNN.usePixelHitsOnly:
        return GNNSeedingTrackMakerCfg(flags, name, **kwargs)
    
    return GNNEndToEndTrackMaker(flags, name, **kwargs)

def GNNEndToEndTrackMaker(flags, name="GNNEndToEndTrackMaker", **kwargs):
    """Sets up a GNNTrackMaker algorithm and returns it."""
    
    acc = ComponentAccumulator()
    
    ## tools 
    SeedFitterTool = acc.popToolsAndMerge(SeedFitterToolCfg(flags))
    kwargs.setdefault("SeedFitterTool", SeedFitterTool)
    
    from TrkConfig.CommonTrackFitterConfig import ITkTrackFitterCfg
    InDetTrackFitter = acc.popToolsAndMerge(ITkTrackFitterCfg(flags))
    kwargs.setdefault("TrackFitter", InDetTrackFitter)

    if "TrackSummaryTool" not in kwargs:
        from TrkConfig.TrkTrackSummaryToolConfig import ITkTrackSummaryToolCfg

        kwargs.setdefault(
            "TrackSummaryTool", acc.popToolsAndMerge(ITkTrackSummaryToolCfg(flags))
        )

    if flags.Tracking.GNN.ToolType == GNNTrackFinderToolType.TrackFinder:
        InDetGNNTrackFinderTool = acc.popToolsAndMerge(GNNTrackFinderToolCfg(flags))
        kwargs.setdefault("GNNTrackFinderTool", InDetGNNTrackFinderTool)
        kwargs.setdefault("GNNTrackReaderTool", None)
    elif flags.Tracking.GNN.ToolType == GNNTrackFinderToolType.TrackReader:
        InDetGNNTrackReader = acc.popToolsAndMerge(GNNTrackReaderToolCfg(flags))
        kwargs.setdefault("GNNTrackReaderTool", InDetGNNTrackReader)
        kwargs.setdefault("GNNTrackFinderTool", None)
    elif flags.Tracking.GNN.ToolType == GNNTrackFinderToolType.Triton:
        InDetGNNTrackFinderTool = acc.popToolsAndMerge(GNNTrackFinderTritonToolCfg(flags))
        kwargs.setdefault("GNNTrackReaderTool", None)
        kwargs.setdefault("GNNTrackFinderTool", InDetGNNTrackFinderTool)
    else:
        raise RuntimeError("GNNTrackFinder or GNNTrackReader must be enabled!")

    kwargs.setdefault("areInputClusters", flags.Tracking.GNN.useClusterTracks)
    kwargs.setdefault("doRecoTrackCuts", flags.Tracking.GNN.doRecoTrackCuts)

    # add eta dependent cut service
    if "InDetEtaDependentCutSvc" not in kwargs:
        from InDetConfig.InDetEtaDependentCutsConfig import ITkEtaDependentCutsSvcCfg
        acc.merge(ITkEtaDependentCutsSvcCfg(flags))
        kwargs.setdefault("InDetEtaDependentCutsSvc", acc.getService("ITkEtaDependentCutsSvc"+flags.Tracking.ActiveConfig.extension))

    kwargs.setdefault("minClusters", flags.Tracking.GNN.minClusters)
    kwargs.setdefault("pTmin", flags.Tracking.GNN.pTmin)
    kwargs.setdefault("etamax", flags.Tracking.GNN.etamax)
    kwargs.setdefault("minPixelClusters", flags.Tracking.GNN.minPixelClusters)
    kwargs.setdefault("minStripClusters", flags.Tracking.GNN.minStripClusters)

    acc.addEventAlgo(CompFactory.InDet.SiSPGNNTrackMaker(name, **kwargs))
    return acc

def GNNSeedingTrackMakerCfg(flags, name="GNNSeedingTrackMaker", **kwargs):
    """Sets up a GNN for seeding algorithm and returns it."""
    acc = ComponentAccumulator()

    from InDetConfig.SiCombinatorialTrackFinderToolConfig import SiDetElementBoundaryLinksCondAlg_xk_ITkPixel_Cfg, SiDetElementBoundaryLinksCondAlg_xk_ITkStrip_Cfg
    acc.merge(SiDetElementBoundaryLinksCondAlg_xk_ITkPixel_Cfg(flags))
    acc.merge(SiDetElementBoundaryLinksCondAlg_xk_ITkStrip_Cfg(flags))

    # To produce AtlasFieldCacheCondObj
    from MagFieldServices.MagFieldServicesConfig import (
        AtlasFieldCacheCondAlgCfg)
    acc.merge(AtlasFieldCacheCondAlgCfg(flags))

    from TrkConfig.TrkRIO_OnTrackCreatorConfig import ITkRotCreatorCfg
    ITkRotCreator = acc.popToolsAndMerge(ITkRotCreatorCfg(
        flags, name="ITkRotCreator"+flags.Tracking.ActiveConfig.extension))
    acc.addPublicTool(ITkRotCreator)
    kwargs.setdefault("RIOonTrackTool", ITkRotCreator)

    from TrkConfig.TrkExRungeKuttaPropagatorConfig import (
        RungeKuttaPropagatorCfg)
    ITkPatternPropagator = acc.popToolsAndMerge(
        RungeKuttaPropagatorCfg(flags, name="ITkPatternPropagator"))
    acc.addPublicTool(ITkPatternPropagator)
    kwargs.setdefault("PropagatorTool", ITkPatternPropagator)

    from TrkConfig.TrkMeasurementUpdatorConfig import KalmanUpdator_xkCfg
    ITkPatternUpdator = acc.popToolsAndMerge(
        KalmanUpdator_xkCfg(flags, name="ITkPatternUpdator"))
    acc.addPublicTool(ITkPatternUpdator)
    kwargs.setdefault("UpdatorTool", ITkPatternUpdator)

    from InDetConfig.InDetBoundaryCheckToolConfig import ITkBoundaryCheckToolCfg
    kwargs.setdefault("BoundaryCheckTool", acc.popToolsAndMerge(
        ITkBoundaryCheckToolCfg(flags)))

    from PixelConditionsTools.ITkPixelConditionsSummaryConfig import (
        ITkPixelConditionsSummaryCfg)
    kwargs.setdefault("PixelSummaryTool", acc.popToolsAndMerge(
        ITkPixelConditionsSummaryCfg(flags)))

    from SCT_ConditionsTools.ITkStripConditionsToolsConfig import (
        ITkStripConditionsSummaryToolCfg)
    kwargs.setdefault("StripSummaryTool", acc.popToolsAndMerge(
        ITkStripConditionsSummaryToolCfg(flags)))

    if flags.Tracking.GNN.useTrackFinder:
        kwargs.setdefault("GNNTrackFinderTool", acc.popToolsAndMerge(GNNTrackFinderToolCfg(flags)))
        kwargs.setdefault("GNNTrackReaderTool", None)
    elif flags.Tracking.GNN.useTrackReader:
        kwargs.setdefault("GNNTrackReaderTool", acc.popToolsAndMerge(GNNTrackReaderToolCfg(flags)))
        kwargs.setdefault("GNNTrackFinderTool", None)
    else:
        raise RuntimeError("GNNTrackFinder or GNNTrackReader must be enabled!")

    kwargs.setdefault("SeedFitterTool", acc.popToolsAndMerge(SeedFitterToolCfg(flags)))

    from TrkConfig.CommonTrackFitterConfig import ITkTrackFitterCfg
    kwargs.setdefault("TrackFitter", acc.popToolsAndMerge(ITkTrackFitterCfg(flags)))

    from InDetConfig.SiDetElementsRoadToolConfig import ITkSiDetElementsRoadMaker_xkCfg
    kwargs.setdefault("RoadTool", acc.popToolsAndMerge(ITkSiDetElementsRoadMaker_xkCfg(flags)))

    # configurations for Kalman filter.
    # similar to https://gitlab.cern.ch/atlas/athena/-/blob/main/InnerDetector/InDetConfig/python/SiTrackMakerConfig.py#L188
    kwargs.setdefault("nClustersMin", flags.Tracking.ActiveConfig.minClusters[0])
    kwargs.setdefault("nWeightedClustersMin", flags.Tracking.ActiveConfig.nWeightedClustersMin[0])
    kwargs.setdefault("nHolesMax", flags.Tracking.ActiveConfig.nHolesMax[0])
    kwargs.setdefault("nHolesGapMax", flags.Tracking.ActiveConfig.nHolesGapMax[0])

    kwargs.setdefault("pTmin", flags.Tracking.ActiveConfig.minPT[0])
    kwargs.setdefault("pTminBrem", flags.Tracking.ActiveConfig.minPTBrem[0])
    kwargs.setdefault("Xi2max", flags.Tracking.ActiveConfig.Xi2max[0])
    kwargs.setdefault("Xi2maxNoAdd", flags.Tracking.ActiveConfig.Xi2maxNoAdd[0])
    kwargs.setdefault("Xi2maxMultiTracks", flags.Tracking.ActiveConfig.Xi2max[0])
    kwargs.setdefault("doMultiTracksProd", False)

    acc.addEventAlgo(CompFactory.InDet.GNNSeedingTrackMaker(name, **kwargs))
    return acc
