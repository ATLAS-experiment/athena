# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

def ITkPixelGeoModelCfg(flags):
    from AtlasGeoModel.GeoModelConfig import GeoModelCfg
    acc = GeoModelCfg(flags)
    geoModelSvc = acc.getPrimary()

    from AthenaConfiguration.ComponentFactory import CompFactory
    ITkPixelDetectorTool = CompFactory.ITk.PixelDetectorTool()
    # ITkPixelDetectorTool.useDynamicAlignFolders = flags.GeoModel.Align.Dynamic
    ITkPixelDetectorTool.Alignable = flags.ITk.Geometry.pixelAlignable
    ITkPixelDetectorTool.AlignmentFolderName = flags.ITk.Geometry.alignmentFolder
    ITkPixelDetectorTool.DetectorName = "ITkPixel"
    if flags.ITk.Geometry.PixelLocal:
        # Setting this filename triggers reading from local file rather than DB
        ITkPixelDetectorTool.GmxFilename = flags.ITk.Geometry.PixelFilename
    if flags.ITk.Geometry.PixelClobOutputName:
        ITkPixelDetectorTool.ClobOutputName = flags.ITk.Geometry.PixelClobOutputName
    geoModelSvc.DetectorTools += [ ITkPixelDetectorTool ]
    return acc


def ITkPixelAlignmentCfg(flags):
    if flags.GeoModel.Align.LegacyConditionsAccess:  # revert to old style CondHandle in case of simulation
        from IOVDbSvc.IOVDbSvcConfig import addFoldersSplitOnline
        return addFoldersSplitOnline(flags, "INDET", "/Indet/Onl/Align", flags.ITk.Geometry.alignmentFolder)
    else:
        from PixelConditionsAlgorithms.ITkPixelConditionsConfig import ITkPixelAlignCondAlgCfg
        return ITkPixelAlignCondAlgCfg(flags)


def ITkPixelSimulationGeometryCfg(flags):
    # main GeoModel config
    acc = ITkPixelGeoModelCfg(flags)
    acc.merge(ITkPixelAlignmentCfg(flags))
    return acc


def ITkPixelReadoutGeometryCfg(flags):
    # main GeoModel config
    acc = ITkPixelGeoModelCfg(flags)
    acc.merge(ITkPixelAlignmentCfg(flags))
    from PixelConditionsAlgorithms.ITkPixelConditionsConfig import ITkPixelDetectorElementCondAlgCfg
    acc.merge(ITkPixelDetectorElementCondAlgCfg(flags))
    return acc
