# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthOnnxComps.OnnxRuntimeFlags import OnnxRuntimeType
from typing import Optional

def OnnxRuntimeSessionToolCfg(flags,
                              model_fname: str,
                              execution_provider: Optional[OnnxRuntimeType] = None, 
                              name="OnnxRuntimeSessionTool", **kwargs):
    """"Configure OnnxRuntimeSessionTool in Control/AthOnnx/AthOnnxComps/src"""
    
    acc = ComponentAccumulator()

    
    execution_provider = flags.AthOnnx.ExecutionProvider if execution_provider is None else execution_provider
    name += execution_provider.name

    if "OnnxRuntimeSvc" not in kwargs:
        from AthOnnxComps.OnnxRuntimeSvcConfig import OnnxRuntimeSvcCfg
        kwargs.setdefault("OnnxRuntimeSvc", acc.getPrimaryAndMerge(OnnxRuntimeSvcCfg(flags)))
    kwargs.setdefault("ModelFileName", model_fname)
    if execution_provider is OnnxRuntimeType.CPU:
        acc.setPrivateTools(CompFactory.AthOnnx.OnnxRuntimeSessionToolCPU(name, **kwargs))
    elif execution_provider is OnnxRuntimeType.CUDA:
        acc.setPrivateTools(CompFactory.AthOnnx.OnnxRuntimeSessionToolCUDA(name, **kwargs))
    else:
        raise ValueError("Unknown OnnxRuntime Execution Provider: %s" % execution_provider)

    return acc
