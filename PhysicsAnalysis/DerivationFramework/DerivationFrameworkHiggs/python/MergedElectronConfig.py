# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

#==============================================================================
# Provides configs for the tools used for decorating merged-electron-related
# variables to the DAODs
#==============================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

#Merged electron details decorator
def MergedElectronDetailsDecoratorCfg(flags, name, **kwargs):
    """Configure the track particle merger tool"""
    acc = ComponentAccumulator()

    if "EMExtrapolationTool" not in kwargs:
        from egammaTrackTools.egammaTrackToolsConfig import EMExtrapolationToolsCfg
        egammaExtrapolationTool = acc.popToolsAndMerge(EMExtrapolationToolsCfg(flags, 
                                                                               name = "egammaExtrapolationTool",
                                                                               NarrowDeltaEta = 0.5,
                                                                               NarrowDeltaPhi = 0.5))
        kwargs.setdefault("EMExtrapolationTool", egammaExtrapolationTool)
    acc.addPublicTool(kwargs["EMExtrapolationTool"])

    if "VertexFitterTool" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
        AtlasExtrapolator = acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags))
        acc.addPublicTool(AtlasExtrapolator)
        
        from TrkConfig.TrkVKalVrtFitterConfig import TrkVKalVrtFitterCfg
        TrkVKalVrtFitter = acc.popToolsAndMerge(TrkVKalVrtFitterCfg(flags,
                                                                    Extrapolator = AtlasExtrapolator))
        kwargs.setdefault("VertexFitterTool", TrkVKalVrtFitter)
    acc.addPublicTool(kwargs["VertexFitterTool"])
    
    if "V0Tools" not in kwargs:
        from TrkConfig.TrkVertexAnalysisUtilsConfig import V0ToolsCfg
        TrkV0Tools = acc.popToolsAndMerge(V0ToolsCfg(flags, 
                                                     name = name+"_V0Tools"))
        kwargs.setdefault("V0Tools", TrkV0Tools)
    acc.addPublicTool(kwargs["V0Tools"])

    MergedElectronDetailsDecorator = CompFactory.DerivationFramework.MergedElectronDetailsDecorator
    acc.addPublicTool(MergedElectronDetailsDecorator(name, **kwargs),
                      primary = True)

    return acc
