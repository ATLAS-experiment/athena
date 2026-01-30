# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

def PLR_GeometryCfg(flags):
    from AtlasGeoModel.GeoModelConfig import GeoModelCfg
    acc = GeoModelCfg(flags)
    geoModelSvc = acc.getPrimary()

    from AthenaConfiguration.ComponentFactory import CompFactory
    plrDetectorTool = CompFactory.PLRDetectorTool()
    plrDetectorTool.DetectorName = "PLR"
    if flags.Detector.GeometryITkPixel:
        # if pixel is present, PLR will be placed inside it, so it needs to know
        # which volume to start from rather than ATLAS
        plrDetectorTool.ContainingDetector = "ITkPixel"
    if flags.ITk.Geometry.PLRLocal:
        # Setting this filename triggers reading from local file rather than DB
        plrDetectorTool.GmxFilename = flags.ITk.Geometry.PLRFilename
    if flags.ITk.Geometry.PLRClobOutputName:
        plrDetectorTool.ClobOutputName = flags.ITk.Geometry.PLRClobOutputName
    geoModelSvc.DetectorTools += [ plrDetectorTool ]
    return acc


def PLR_AlignmentCfg(flags):
    if flags.GeoModel.Align.LegacyConditionsAccess:  # revert to old style CondHandle in case of simulation
        from IOVDbSvc.IOVDbSvcConfig import addFoldersSplitOnline
        return addFoldersSplitOnline(flags, "INDET", "/Indet/Onl/Align", "/Indet/Align")
    else:
        from PixelConditionsAlgorithms.PLR_ConditionsConfig import PLR_AlignCondAlgCfg
        return PLR_AlignCondAlgCfg(flags)


def PLR_SimulationGeometryCfg(flags):
    # main GeoModel config
    acc = PLR_GeometryCfg(flags)
    acc.merge(PLR_AlignmentCfg(flags))
    return acc


def PLR_ReadoutGeometryCfg(flags, **kwargs):
    # main GeoModel config
    acc = PLR_GeometryCfg(flags)
    acc.merge(PLR_AlignmentCfg(flags))
    from PixelConditionsAlgorithms.PLR_ConditionsConfig import PLR_DetectorElementCondAlgCfg
    acc.merge(PLR_DetectorElementCondAlgCfg(flags))
    return acc
