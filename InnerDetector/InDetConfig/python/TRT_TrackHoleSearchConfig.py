# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
# Configuration of TRT_TrackHoleSearch package

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def TRTTrackHoleSearchToolCfg(flags, name="TRTTrackHoleSearchTool", **kwargs):
    acc = ComponentAccumulator()

    if "extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
        kwargs.setdefault("extrapolator", acc.addPublicTool(
            acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags))))
    
    acc.setPrivateTools(CompFactory.TRTTrackHoleSearchTool(name, **kwargs))
    return acc

