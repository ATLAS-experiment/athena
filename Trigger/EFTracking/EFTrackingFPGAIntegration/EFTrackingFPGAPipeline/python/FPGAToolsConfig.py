# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def FPGADataFormatToolCfg(flags, name="FPGADataFormatTool", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("name", name)
    acc.setPrivateTools(CompFactory.FPGADataFormatTool(**kwargs))
    return acc


def xAODClusterMakerCfg(flags, name="xAODClusterMaker", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("PixelClusterContainerKey", "FPGAPixelClusters")
    kwargs.setdefault("StripClusterContainerKey", "FPGAStripClusters")
    kwargs.setdefault("DoBulkCopy", True)
    acc.setPrivateTools(CompFactory.xAODClusterMaker(name, **kwargs))
    return acc
