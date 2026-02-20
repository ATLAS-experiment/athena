# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def GraphInferenceAlgCfg(flags, name = "GraphInferenceAlg", **kwargs):
    result  = ComponentAccumulator()
    the_alg = CompFactory.MuonML.InferenceAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def GraphSPFilterToolCfg(flags, name ="GraphSPFilterTool", **kwargs):
    
    from AthOnnxComps.OnnxRuntimeSessionConfig import OnnxRuntimeSessionToolCfg

    result = ComponentAccumulator()
    kwargs.setdefault("ModelSession", result.popToolsAndMerge(OnnxRuntimeSessionToolCfg(flags, model_fname="/eos/atlas/atlascerngroupdisk/data-art/grid-input/MuonRecRTT/TestModel.onnx")))
    kwargs.setdefault("MLFilterCut", -3.6) # Working point cut
    
    the_tool = CompFactory.MuonML.GraphSPFilterTool(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result

def GraphBucketFilterToolCfg(flags, name ="GraphBucketFilterTool", **kwargs):

    from AthOnnxComps.OnnxRuntimeSessionConfig import OnnxRuntimeSessionToolCfg

    result = ComponentAccumulator()
    ### File will be moved to calibration area once the model is finalized
    kwargs.setdefault("ModelSession", result.popToolsAndMerge(OnnxRuntimeSessionToolCfg(flags, model_fname="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/edgecnn_multi_bucket_sparse_meta.onnx")))
    kwargs.setdefault("BiasClass0", 1.0) # Working point selection bias for multi-class comparison
    kwargs.setdefault("OutputLevel", 3)  # DEBUG level (1=VERBOSE, 2=DEBUG, 3=INFO, 4=WARNING, 5=ERROR, 6=FATAL)

    the_tool = CompFactory.MuonML.GraphBucketFilterTool(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result

