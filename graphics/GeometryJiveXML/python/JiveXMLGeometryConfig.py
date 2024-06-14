# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AllConfigFlags import initConfigFlags


def GeometryWriterCfg(flags, **kwargs):
    result = ComponentAccumulator()
    # Add Geometry tool
    result.addPublicTool(CompFactory.JiveXML.GeometryWriter(name = "GeometryWriter", **kwargs))
    return result


def MuonGeometryWriterCfg(flags, **kwargs):
    result = ComponentAccumulator()
    # Add Muon Geometry tool
    result.addPublicTool(CompFactory.JiveXML.MuonGeometryWriter(name = "MuonGeometryWriter", **kwargs))
    return result


def GeometryJiveXMLCfg(flags):
    result = ComponentAccumulator()
    result.merge(GeometryWriterCfg(flags))
    result.merge(MuonGeometryWriterCfg(flags))
    return result


def AlgoJiveXMLGeometryCfg(flags, **kwargs):
    result = ComponentAccumulator()

    kwargs.setdefault("WriteToFile", True)
    kwargs.setdefault("StreamToServerTool", None)
    kwargs.setdefault("WriteGeometry", True)

    ### Enable this to recreate the geometry XML files for Atlantis
    result.merge(GeometryJiveXMLCfg(flags))

    the_alg = CompFactory.JiveXML.AlgoJiveXML(name="AlgoJiveXML", **kwargs)
    result.addEventAlgo(the_alg, primary=True)
    return result

def main():
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN3
    flags.Exec.MaxEvents = 0
    flags.GeoModel.AtlasVersion = 'ATLAS-R3S-2021-03-02-00'
    flags.IOVDb.GlobalTag = 'CONDBR2-BLKPA-2023-02'
    flags.lock()
    flags.dump()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    result = MainServicesCfg(flags)

    if flags.Detector.GeometryBpipe:
        from BeamPipeGeoModel.BeamPipeGMConfig import BeamPipeGeometryCfg
        result.merge(BeamPipeGeometryCfg(flags))

    if flags.Detector.GeometryPixel:
        from PixelGeoModel.PixelGeoModelConfig import PixelReadoutGeometryCfg
        result.merge(PixelReadoutGeometryCfg(flags))

    if flags.Detector.GeometrySCT:
        from SCT_GeoModel.SCT_GeoModelConfig import SCT_ReadoutGeometryCfg
        result.merge(SCT_ReadoutGeometryCfg(flags))

    if flags.Detector.GeometryTRT:
        from TRT_GeoModel.TRT_GeoModelConfig import TRT_ReadoutGeometryCfg
        result.merge(TRT_ReadoutGeometryCfg(flags))

    if flags.Detector.GeometryITkPixel:
        from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
        result.merge(ITkPixelReadoutGeometryCfg(flags))

    if flags.Detector.GeometryITkStrip:
        from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
        result.merge(ITkStripReadoutGeometryCfg(flags))

    if flags.Detector.GeometryHGTD:
        from HGTD_GeoModelXml.HGTD_GeoModelConfig import HGTD_ReadoutGeometryCfg
        result.merge(HGTD_ReadoutGeometryCfg(flags))

    if flags.Detector.GeometryLAr:
        from LArGeoAlgsNV.LArGMConfig import LArGMCfg
        result.merge(LArGMCfg(flags))

    if flags.Detector.GeometryTile:
        from TileGeoModel.TileGMConfig import TileGMCfg
        result.merge(TileGMCfg(flags))

    if flags.Detector.GeometryMuon:
        from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
        result.merge(MuonGeoModelCfg(flags))

    geoModel = result.getService("GeoModelSvc")
    geoModel.DetectorTools["LArDetectorToolNV"].GeometryConfig = "FULL"

    result.merge(AlgoJiveXMLGeometryCfg(flags))

    import sys
    sys.exit(not result.run().isSuccess())

if __name__ == "__main__":
    main()
