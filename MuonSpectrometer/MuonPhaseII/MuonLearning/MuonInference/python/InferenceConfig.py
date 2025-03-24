# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def GraphInferenceAlgCfg(flags, name = "GraphInferenceAlg", **kwargs):
    result  = ComponentAccumulator()
    the_alg = CompFactory.MuonML.GraphInferenceAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def GraphBucketFilterToolCfg(flags, name ="GraphBucketFilterTool", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("ModelPath", "/eos/atlas/atlascerngroupdisk/data-art/grid-input/MuonRecRTT/TestModel.onnx")
    the_tool = CompFactory.MuonML.GraphBucketFilterTool(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result