# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration


from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def DumpCfg (flags, ofile, items='*', exclude=''):
    acc = ComponentAccumulator()
    from PyDumper.PyComps import PySgDumper as pyalg
    from AthenaCommon.Constants import INFO

    # Don't set up geomodel for evgen files.
    if flags.GeoModel.AtlasVersion:
        from AtlasGeoModel.GeoModelConfig import GeoModelCfg
        acc.merge (GeoModelCfg (flags))

        if flags.Detector.GeometryPixel:
            from PixelGeoModel.PixelGeoModelConfig import PixelReadoutGeometryCfg
            acc.merge(PixelReadoutGeometryCfg(flags))
        if flags.Detector.GeometrySCT:
            from SCT_GeoModel.SCT_GeoModelConfig import SCT_ReadoutGeometryCfg
            acc.merge(SCT_ReadoutGeometryCfg(flags))
        if flags.Detector.GeometryTRT:
            from TRT_GeoModel.TRT_GeoModelConfig import TRT_ReadoutGeometryCfg
            acc.merge(TRT_ReadoutGeometryCfg(flags))
        if flags.Detector.GeometryITkPixel:
            from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
            acc.merge(ITkPixelReadoutGeometryCfg(flags))
        if flags.Detector.GeometryITkStrip:
            from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
            acc.merge(ITkStripReadoutGeometryCfg(flags))
        if flags.Detector.GeometryLAr:
            from LArGeoAlgsNV.LArGMConfig import LArGMCfg
            acc.merge(LArGMCfg(flags))
        if flags.Detector.GeometryTile:
            from TileGeoModel.TileGMConfig import TileGMCfg
            acc.merge(TileGMCfg(flags))
        if flags.Detector.GeometryMuon:
            from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
            acc.merge(MuonGeoModelCfg(flags))

    acc.addEventAlgo (pyalg ('pyalg',
                             ofile = ofile,
                             items = items,
                             exclude = exclude,
                             OutputLevel = INFO))
    return acc
