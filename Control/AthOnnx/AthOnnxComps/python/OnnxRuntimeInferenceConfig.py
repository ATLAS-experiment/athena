# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthOnnxComps.OnnxRuntimeFlags import OnnxRuntimeType
from typing import Optional
from AthOnnxComps.OnnxRuntimeSessionConfig import OnnxRuntimeSessionToolCfg

def OnnxRuntimeInferenceToolCfg(flags, 
                                model_fname: str = None, 
                                execution_provider: Optional[OnnxRuntimeType] = None, 
                                name="OnnxRuntimeInferenceTool", **kwargs):
    """Configure OnnxRuntimeInferenceTool in Control/AthOnnx/AthOnnxComps/src"""

    acc = ComponentAccumulator()

    if "OnnxRuntimeSvc" not in kwargs:
        from AthOnnxComps.OnnxRuntimeSvcConfig import OnnxRuntimeSvcCfg
        kwargs.setdefault("OnnxRuntimeSvc", acc.getPrimaryAndMerge(OnnxRuntimeSvcCfg(flags)))
    kwargs.setdefault("ORTSessionTool", acc.popToolsAndMerge(OnnxRuntimeSessionToolCfg(flags, model_fname, execution_provider)))
    acc.setPrivateTools(CompFactory.AthOnnx.OnnxRuntimeInferenceTool(name, **kwargs))
    return acc
