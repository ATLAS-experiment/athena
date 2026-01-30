# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def TritonToolCfg(flags, model_name: str, url: str,
                  port: int = 8001, model_version: str = "",
                  timeout: float = 0., ssl: bool = False,
                  name="TritonTool", **kwargs):
    """Configure TritonTool in Control/AthOnnx/AthTritonComps/src"""

    acc = ComponentAccumulator()

    kwargs.setdefault("ModelName", model_name)
    kwargs.setdefault("URL", url)
    kwargs.setdefault("Port", port)
    kwargs.setdefault("ModelVersion", model_version)
    kwargs.setdefault("ClientTimeout", timeout)

    if port == 443: # If the port is 443, that's typically used for HTTPS.
        ssl = True

    kwargs.setdefault("UseSSL", ssl)  # Default to not using SSL


    acc.setPrivateTools(CompFactory.AthInfer.TritonTool(name=name, **kwargs))
    return acc
