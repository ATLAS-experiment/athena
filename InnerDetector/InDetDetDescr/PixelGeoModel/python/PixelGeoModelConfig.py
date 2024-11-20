# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.AccumulatorCache import AccumulatorCache


def PixelGeoModelCfg(flags):
    from AtlasGeoModel.GeometryDBConfig import InDetGeometryDBSvcCfg
    db = InDetGeometryDBSvcCfg(flags)

    from AtlasGeoModel.GeoModelConfig import GeoModelCfg
    acc = GeoModelCfg(flags)
    geoModelSvc = acc.getPrimary()

    from AthenaConfiguration.ComponentFactory import CompFactory
    pixelDetectorTool = CompFactory.PixelDetectorTool("PixelDetectorTool")
    pixelDetectorTool.GeometryDBSvc = db.getPrimary()
    pixelDetectorTool.BCM_Tool = CompFactory.InDetDD.BCM_Builder()
    pixelDetectorTool.BLM_Tool = CompFactory.InDetDD.BLM_Builder()
    pixelDetectorTool.useDynamicAlignFolders = flags.GeoModel.Align.Dynamic
    geoModelSvc.DetectorTools += [ pixelDetectorTool ]
    acc.merge(db)
    return acc


def PixelAlignmentCfg(flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()
    if flags.GeoModel.Align.LegacyConditionsAccess:  # revert to old style CondHandle in case of simulation
        from IOVDbSvc.IOVDbSvcConfig import addFoldersSplitOnline
        if flags.GeoModel.Align.Dynamic:
            acc.merge(addFoldersSplitOnline(flags, "INDET",
                                            ["/Indet/Onl/AlignL1/ID", "/Indet/Onl/AlignL2/PIX"],
                                            ["/Indet/AlignL1/ID", "/Indet/AlignL2/PIX"]))
            acc.merge(addFoldersSplitOnline(flags, "INDET", "/Indet/Onl/AlignL3", "/Indet/AlignL3"))
        else:
            acc.merge(addFoldersSplitOnline(flags, "INDET", "/Indet/Onl/Align", "/Indet/Align"))
            acc.merge(addFoldersSplitOnline(flags, "INDET", "/Indet/Onl/IBLDist", "/Indet/IBLDist"))
    else:
        from PixelConditionsAlgorithms.PixelConditionsConfig import PixelAlignCondAlgCfg
        acc.merge(PixelAlignCondAlgCfg(flags))
    return acc


@AccumulatorCache
def PixelSimulationGeometryCfg(flags):
    # main GeoModel config
    acc = PixelGeoModelCfg(flags)
    acc.merge(PixelAlignmentCfg(flags))
    return acc


@AccumulatorCache
def PixelReadoutGeometryCfg(flags):
    # main GeoModel config
    acc = PixelGeoModelCfg(flags)
    acc.merge(PixelAlignmentCfg(flags))
    from PixelConditionsAlgorithms.PixelConditionsConfig import PixelDetectorElementCondAlgCfg
    acc.merge(PixelDetectorElementCondAlgCfg(flags))
    return acc
