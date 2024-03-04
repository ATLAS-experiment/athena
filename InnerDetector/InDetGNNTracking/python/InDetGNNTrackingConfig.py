#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from pathlib import Path

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


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

    kwargs.setdefault("NtupleFileName", "/DumpObjects/")
    kwargs.setdefault("NtupleTreeName", "GNN4ITk")
    kwargs.setdefault("rootFile", True)

    acc.addEventAlgo(CompFactory.InDet.DumpObjects(name, **kwargs))
    return acc

def GNNTrackFinderToolCfg(flags, name='GNNTrackFinderTool', **kwargs):
    """Sets up a GNNTrackFinderTool tool and returns it."""
    acc = ComponentAccumulator()
    
    ### parameters for GNNTrackFinderTool
    kwargs.setdefault("embeddingDim", 8)
    kwargs.setdefault("rVal", 1.7)
    kwargs.setdefault("knnVal", 500)
    kwargs.setdefault("filterCut", 0.21)
    kwargs.setdefault("inputMLModelDir", "TrainedMLModels4ITk")
    kwargs.setdefault("UseCUDA", False)

    from AthOnnxComps.OnnxRuntimeInferenceConfig import OnnxRuntimeInferenceToolCfg
    kwargs.setdefault("Embedding", acc.popToolsAndMerge(
        OnnxRuntimeInferenceToolCfg(flags, Path("TrainedMLModels4ITk") / "embedding.onnx")
    ))
    kwargs.setdefault("Filtering", acc.popToolsAndMerge(
        OnnxRuntimeInferenceToolCfg(flags, Path("TrainedMLModels4ITk") / "filtering.onnx")
    ))
    kwargs.setdefault("GNN", acc.popToolsAndMerge(
        OnnxRuntimeInferenceToolCfg(flags, Path("TrainedMLModels4ITk") / "gnn.onnx")
    ))
    
    acc.setPrivateTools(CompFactory.InDet.SiGNNTrackFinderTool(name, **kwargs))
    return acc


def SeedFitterToolCfg(flags, name="SeedFitterTool", **kwargs):
    """Sets up a SeedFitter tool and returns it."""
    acc = ComponentAccumulator()
    
    ### parameters for SeedFitter
    acc.setPrivateTools(CompFactory.InDet.SeedFitterTool(name, **kwargs))
    return acc

def GNNTrackReaderToolCfg(flags, name='GNNTrackReaderTool', **kwargs):
    """Set up a GNNTrackReader tool and return it."""
    acc = ComponentAccumulator()

    ### parameters for GNNTrackReader
    kwargs.setdefault("inputTracksDir", "gnntracks")
    kwargs.setdefault("csvPrefix", "track")

    acc.setPrivateTools(CompFactory.InDet.GNNTrackReaderTool(name, **kwargs))
    return acc

def GNNTrackMakerCfg(flags, name="GNNTrackMaker", **kwargs):
    """Sets up a GNNTrackMaker algorithm and returns it."""
    
    acc = ComponentAccumulator()
    
    ## tools 
    SeedFitterTool = acc.popToolsAndMerge(SeedFitterToolCfg(flags))
    kwargs.setdefault("SeedFitterTool", SeedFitterTool)
    
    from TrkConfig.CommonTrackFitterConfig import ITkTrackFitterCfg
    InDetTrackFitter = acc.popToolsAndMerge(ITkTrackFitterCfg(flags))
    kwargs.setdefault("TrackFitter", InDetTrackFitter)

    if flags.Tracking.GNN.useTrackFinder:
        InDetGNNTrackFinderTool = acc.popToolsAndMerge(GNNTrackFinderToolCfg(flags))
        kwargs.setdefault("GNNTrackFinderTool", InDetGNNTrackFinderTool)
        kwargs.setdefault("GNNTrackReaderTool", None)
        kwargs.setdefault("UseTrackFinder", True)
        kwargs.setdefault("UseTrackReader", False)
    elif flags.Tracking.GNN.useTrackReader:
        InDetGNNTrackReader = acc.popToolsAndMerge(GNNTrackReaderToolCfg(flags))
        kwargs.setdefault("GNNTrackReaderTool", InDetGNNTrackReader)
        kwargs.setdefault("GNNTrackFinderTool", None)
        kwargs.setdefault("UseTrackFinder", False)
        kwargs.setdefault("UseTrackReader", True)
    else:
        raise RuntimeError("GNNTrackFinder or GNNTrackReader must be enabled!")

    acc.addEventAlgo(CompFactory.InDet.SiSPGNNTrackMaker(name, **kwargs))
    return acc
