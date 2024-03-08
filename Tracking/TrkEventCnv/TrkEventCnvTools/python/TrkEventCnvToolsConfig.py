# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def InDetEventCnvToolCfg(flags, name='InDetEventCnvTool', **kwargs):
    acc = ComponentAccumulator()

    # For condition data
    if flags.Detector.GeometryPixel:
        from PixelGeoModel.PixelGeoModelConfig import PixelReadoutGeometryCfg
        acc.merge(PixelReadoutGeometryCfg(flags))
    if flags.Detector.GeometrySCT:
        from SCT_GeoModel.SCT_GeoModelConfig import SCT_ReadoutGeometryCfg
        acc.merge(SCT_ReadoutGeometryCfg(flags))
    if flags.Detector.GeometryTRT:
        from TRT_GeoModel.TRT_GeoModelConfig import TRT_ReadoutGeometryCfg
        acc.merge(TRT_ReadoutGeometryCfg(flags))

    acc.setPrivateTools(CompFactory.InDet.InDetEventCnvTool(name, **kwargs))
    return acc

def ITkEventCnvToolCfg(flags, name='ITkEventCnvTool', **kwargs):
    acc = ComponentAccumulator()

    # For condition data
    if flags.Detector.GeometryITkPixel:
        from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
        acc.merge(ITkPixelReadoutGeometryCfg(flags))
    if flags.Detector.GeometryITkStrip:
        from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
        acc.merge(ITkStripReadoutGeometryCfg(flags))

    kwargs.setdefault("PixelClusterContainer", "ITkPixelClusters")
    kwargs.setdefault("SCT_ClusterContainer", "ITkStripClusters")
    kwargs.setdefault("TRT_DriftCircleContainer", "")
    kwargs.setdefault("PixelDetEleCollKey", "ITkPixelDetectorElementCollection")
    kwargs.setdefault("SCTDetEleCollKey", "ITkStripDetectorElementCollection")
    kwargs.setdefault("TRTDetEleContKey", "")

    acc.setPrivateTools(CompFactory.InDet.InDetEventCnvTool(name, **kwargs))
    return acc

def MuonCnvToolFixTGCsCfg(flags, name='MuonCnvToolFixTGCs', **kwargs):
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    acc = MuonGeoModelCfg(flags)
    kwargs.setdefault("FixTGCs", True)
    acc.setPrivateTools(CompFactory.Muon.MuonEventCnvTool(name, **kwargs))
    return acc

def TrkEventCnvSuperToolCfg(flags, name='EventCnvSuperTool', **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("DoID", flags.Reco.EnableTracking)
    kwargs.setdefault("DoMuons", flags.Detector.EnableMuon)
    kwargs.setdefault("DoTrackOverlay",
                      (flags.Common.isOverlay or flags.Output.doWriteRDO) and \
                      flags.Overlay.doTrackOverlay)

    if kwargs["DoID"] and "IdCnvTool" not in kwargs:
        if flags.Detector.GeometryID:
            IdCnvTool = acc.popToolsAndMerge(InDetEventCnvToolCfg(flags))
        elif flags.Detector.GeometryITk:
            IdCnvTool = acc.popToolsAndMerge(ITkEventCnvToolCfg(flags))
        kwargs.setdefault("IdCnvTool", IdCnvTool)

    if kwargs["DoMuons"] and "MuonCnvTool" not in kwargs:
        kwargs.setdefault("MuonCnvTool", acc.popToolsAndMerge(
            MuonCnvToolFixTGCsCfg(flags)))

    acc.addPublicTool(CompFactory.Trk.EventCnvSuperTool(name, **kwargs))
    return acc


