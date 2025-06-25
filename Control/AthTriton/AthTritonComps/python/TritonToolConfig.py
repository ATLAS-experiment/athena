# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def TritonToolCfg(flags, model_name: str, url: str, name="TritonTool", **kwargs):
    """Configure TritonTool in Control/AthOnnx/AthTritonComps/src"""

    acc = ComponentAccumulator()

    kwargs.setdefault("ModelName", model_name)
    kwargs.setdefault("URL", url)

    acc.setPrivateTools(CompFactory.AthInfer.TritonTool(name, **kwargs))
    return acc
