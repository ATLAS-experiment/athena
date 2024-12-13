# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def OverlayVertexSkimmingAlgCfg(flags, name="OverlayVertexSkimmingAlg",  **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("PrimaryVertexContainerName", f"{flags.Overlay.BkgPrefix}PrimaryVertices")
    acc.addEventAlgo(CompFactory.OverlayVertexSkimmingAlg(name, **kwargs))
    return acc
