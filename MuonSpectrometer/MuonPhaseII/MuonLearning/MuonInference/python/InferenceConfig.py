# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def GraphInferenceAlgCfg(flags, name = "GraphInferenceAlg", **kwargs):
    result  = ComponentAccumulator()
    the_alg = CompFactory.MuonML.InferenceAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def GraphBucketFilterToolCfg(flags, name ="GraphBucketFilterTool", **kwargs):
    
    from AthOnnxComps.OnnxRuntimeSessionConfig import OnnxRuntimeSessionToolCfg

    result = ComponentAccumulator()
    #kwargs.setdefault("ModelSession", result.popToolsAndMerge(OnnxRuntimeSessionToolCfg(flags, model_fname="MuonSPId/EdgeGAT_FCG_8vars_quantized_metadata.onnx")))
    kwargs.setdefault("ModelSession", result.popToolsAndMerge(OnnxRuntimeSessionToolCfg(flags, model_fname="/eos/atlas/atlascerngroupdisk/data-art/grid-input/MuonRecRTT/TestModel.onnx")))
    kwargs.setdefault("MLFilterCut", -2.7)

    the_tool = CompFactory.MuonML.GraphBucketFilterTool(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result