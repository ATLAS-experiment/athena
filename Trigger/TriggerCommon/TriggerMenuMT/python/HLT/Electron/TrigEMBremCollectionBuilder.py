# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def TrigEMBremCollectionBuilderCfg(flags, name = "TrigEgammaBremCollectionBuilder", **kwargs):

    acc = ComponentAccumulator()
    if "TrackRefitTool" not in kwargs:
        from egammaTrackTools.egammaTrackToolsConfig import (
            egammaTrkRefitterToolCfg)
        kwargs["TrackRefitTool"] = acc.popToolsAndMerge(
            egammaTrkRefitterToolCfg(flags))

    
    if "TrackParticleCreatorTool" not in kwargs:
        from TrkConfig.TrkParticleCreatorConfig import (
            GSFBuildInDetParticleCreatorToolCfg)
        kwargs["TrackParticleCreatorTool"] = acc.popToolsAndMerge(
            GSFBuildInDetParticleCreatorToolCfg(flags, name="TrigGSFBuildInDetParticleCreatorTool", isTrigger=True))

    if "TrackSlimmingTool" not in kwargs:
        from TrkConfig.TrkTrackSlimmingToolConfig import GSFTrackSlimmingToolCfg
        kwargs["TrackSlimmingTool"] = acc.popToolsAndMerge(GSFTrackSlimmingToolCfg(flags))

    kwargs.setdefault(
        "usePixel",
        flags.Detector.EnablePixel or flags.Detector.EnableITkPixel)
    kwargs.setdefault(
        "useSCT",
        flags.Detector.EnableSCT or flags.Detector.EnableITkStrip)
    kwargs.setdefault("useTRT", flags.Detector.EnableTRT)
    kwargs.setdefault("doTruth", flags.Input.isMC)
    kwargs.setdefault("slimTrkTracks", flags.Egamma.slimGSFTrkTracks)

    # P->T conversion extra dependencies
    if flags.Detector.GeometryITk:
        kwargs.setdefault("ExtraInputs", [
            ("InDetDD::SiDetectorElementCollection",
             "ConditionStore+ITkPixelDetectorElementCollection"),
            ("InDetDD::SiDetectorElementCollection",
             "ConditionStore+ITkStripDetectorElementCollection"),
        ])
    else:
        kwargs.setdefault("ExtraInputs", [
            ("InDetDD::SiDetectorElementCollection",
             "ConditionStore+PixelDetectorElementCollection"),
            ("InDetDD::SiDetectorElementCollection",
             "ConditionStore+SCT_DetectorElementCollection"),
        ])

    alg = CompFactory.EMBremCollectionBuilder(name, **kwargs)
    acc.addEventAlgo(alg)
    return acc




