# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.AccumulatorCache import AccumulatorCache


def SCT_GeoModelCfg(flags):
    from AtlasGeoModel.GeometryDBConfig import InDetGeometryDBSvcCfg
    db = InDetGeometryDBSvcCfg(flags)

    from AtlasGeoModel.GeoModelConfig import GeoModelCfg
    acc = GeoModelCfg(flags)
    geoModelSvc = acc.getPrimary()

    from AthenaConfiguration.ComponentFactory import CompFactory
    sctDetectorTool = CompFactory.SCT_DetectorTool()
    sctDetectorTool.GeometryDBSvc = db.getPrimary()
    sctDetectorTool.useDynamicAlignFolders = flags.GeoModel.Align.Dynamic
    sctDetectorTool.Alignable = True # make this a flag?
    geoModelSvc.DetectorTools += [ sctDetectorTool ]
    acc.merge(db)
    return acc


def SCT_AlignmentCfg(flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()
    if flags.GeoModel.Align.LegacyConditionsAccess:  # revert to old style CondHandle in case of simulation
        from IOVDbSvc.IOVDbSvcConfig import addFoldersSplitOnline
        if flags.GeoModel.Align.Dynamic:
            acc.merge(addFoldersSplitOnline(flags, "INDET",
                                            ["/Indet/Onl/AlignL1/ID", "/Indet/Onl/AlignL2/SCT"],
                                            ["/Indet/AlignL1/ID", "/Indet/AlignL2/SCT"]))
            acc.merge(addFoldersSplitOnline(flags, "INDET", "/Indet/Onl/AlignL3", "/Indet/AlignL3"))
        else:
            acc.merge(addFoldersSplitOnline(flags, "INDET", "/Indet/Onl/Align", "/Indet/Align"))
    else:
        from SCT_ConditionsAlgorithms.SCT_ConditionsAlgorithmsConfig import SCT_AlignCondAlgCfg
        acc.merge(SCT_AlignCondAlgCfg(flags))
    return acc


@AccumulatorCache
def SCT_SimulationGeometryCfg(flags):
    # main GeoModel config
    acc = SCT_GeoModelCfg(flags)
    acc.merge(SCT_AlignmentCfg(flags))
    return acc


@AccumulatorCache
def SCT_ReadoutGeometryCfg(flags):
    # main GeoModel config
    acc = SCT_GeoModelCfg(flags)
    acc.merge(SCT_AlignmentCfg(flags))
    from SCT_ConditionsAlgorithms.SCT_ConditionsAlgorithmsConfig import SCT_DetectorElementCondAlgCfg
    acc.merge(SCT_DetectorElementCondAlgCfg(flags))
    return acc
