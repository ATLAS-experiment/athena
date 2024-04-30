# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of SeedToTrackConversionTool package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def SeedToTrackConversionToolCfg(
        flags, name="SeedToTrackConversionTool", **kwargs):
    acc = ComponentAccumulator()

    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import InDetExtrapolatorCfg
        InDetExtrapolator = acc.popToolsAndMerge(InDetExtrapolatorCfg(flags))
        acc.addPublicTool(InDetExtrapolator)
        kwargs.setdefault("Extrapolator", InDetExtrapolator)

    if "RIO_OnTrackCreator" not in kwargs:
        from TrkConfig.TrkRIO_OnTrackCreatorConfig import (
            InDetRotCreatorDigitalCfg)
        RotCreator = acc.popToolsAndMerge(InDetRotCreatorDigitalCfg(flags))
        acc.addPublicTool(RotCreator)
        kwargs.setdefault("RIO_OnTrackCreator", RotCreator)

    extension = flags.Tracking.ActiveConfig.extension
    if extension == flags.Tracking.PrimaryPassConfig.value:
        extension = ""
    kwargs.setdefault("OutputName",
                      f"SiSPSeedSegments{extension}")

    acc.setPrivateTools(
        CompFactory.InDet.SeedToTrackConversionTool(name, **kwargs))
    return acc


def ITkSeedToTrackConversionToolCfg(
        flags, name="ITkSeedToTrackConversionTool", **kwargs):
    acc = ComponentAccumulator()

    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
        AtlasExtrapolator = acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags))
        acc.addPublicTool(AtlasExtrapolator)
        kwargs.setdefault("Extrapolator", AtlasExtrapolator)

    if "RIO_OnTrackCreator" not in kwargs:
        from TrkConfig.TrkRIO_OnTrackCreatorConfig import ITkRotCreatorCfg
        kwargs.setdefault("RIO_OnTrackCreator", acc.popToolsAndMerge(ITkRotCreatorCfg(flags)))

    extension = flags.Tracking.ActiveConfig.extension
    if extension == flags.Tracking.PrimaryPassConfig.value:
        extension = ""
    kwargs.setdefault("OutputName",
                      f"SiSPSeedSegments{extension}")

    acc.setPrivateTools(
        CompFactory.InDet.SeedToTrackConversionTool(name, **kwargs))
    return acc
