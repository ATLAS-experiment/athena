# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""RoI-limited silicon overlay for the track overlay primary pass.
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

RoIPixelRDOs = "RoI_PixelRDOs"
RoISCT_RDOs = "RoI_SCT_RDOs"


def TrackOverlayRoISiRDOMergerAlgCfg(flags, name="TrackOverlayRoISiRDOMergerAlg", **kwargs):
    acc = ComponentAccumulator()

    # EM topo clusters available before ID tracking (same copier as the calo-seeded RoIs)
    from egammaAlgs.egammaTopoClusterCopierConfig import indetTopoClusterCopierCfg
    acc.merge(indetTopoClusterCopierCfg(flags))

    from RegionSelector.RegSelToolConfig import regSelTool_Pixel_Cfg, regSelTool_SCT_Cfg
    kwargs.setdefault("RegSelTool_Pixel", acc.popToolsAndMerge(regSelTool_Pixel_Cfg(flags)))
    kwargs.setdefault("RegSelTool_SCT", acc.popToolsAndMerge(regSelTool_SCT_Cfg(flags)))

    kwargs.setdefault("CaloClusterKey", flags.Tracking.EgammaTopoClusters)
    kwargs.setdefault("SignalPixelRDOKey", flags.Overlay.SigPrefix + "PixelRDOs")
    kwargs.setdefault("MergedPixelRDOKey", "PixelRDOs")
    kwargs.setdefault("OutputPixelRDOKey", RoIPixelRDOs)
    kwargs.setdefault("SignalSCT_RDOKey", flags.Overlay.SigPrefix + "SCT_RDOs")
    kwargs.setdefault("MergedSCT_RDOKey", "SCT_RDOs")
    kwargs.setdefault("OutputSCT_RDOKey", RoISCT_RDOs)
    kwargs.setdefault("MinClusterEt", flags.TrackOverlay.RoIMinClusterEt)
    kwargs.setdefault("EtaHalfWidth", flags.TrackOverlay.RoIEtaHalfWidth)
    kwargs.setdefault("PhiHalfWidth", flags.TrackOverlay.RoIPhiHalfWidth)
    kwargs.setdefault("ZHalfWidth", flags.TrackOverlay.RoIZHalfWidth)

    acc.addEventAlgo(CompFactory.TrackOverlay.TrackOverlayRoISiRDOMergerAlg(name, **kwargs))
    return acc


def TrackOverlaySignalTrackFilterAlgCfg(flags, name="TrackOverlaySignalTrackFilterAlg", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("SignalPixelRDOKey", flags.Overlay.SigPrefix + "PixelRDOs")
    kwargs.setdefault("SignalSCT_RDOKey", flags.Overlay.SigPrefix + "SCT_RDOs")
    kwargs.setdefault("MinSignalClusterFraction", flags.TrackOverlay.RoIMinSignalClusterFraction)
    acc.addEventAlgo(CompFactory.TrackOverlay.TrackOverlaySignalTrackFilterAlg(name, **kwargs))
    return acc
