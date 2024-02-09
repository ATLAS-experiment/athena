# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

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

    session_tool = acc.popToolsAndMerge(OnnxRuntimeSessionToolCfg(flags, model_fname, execution_provider))
    kwargs["ORTSessionTool"] = session_tool
    acc.setPrivateTools(CompFactory.AthOnnx.OnnxRuntimeInferenceTool(name, **kwargs))
    return acc
