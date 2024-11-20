#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
from AthenaConfiguration.AccumulatorCache import AccumulatorCache


def TRT_GeoModelCfg(flags):
    from AtlasGeoModel.GeometryDBConfig import InDetGeometryDBSvcCfg
    db = InDetGeometryDBSvcCfg(flags)

    from AtlasGeoModel.GeoModelConfig import GeoModelCfg
    acc = GeoModelCfg(flags)
    geoModelSvc = acc.getPrimary()

    from AthenaConfiguration.ComponentFactory import CompFactory
    trtDetectorTool = CompFactory.TRT_DetectorTool()
    trtDetectorTool.GeometryDBSvc = db.getPrimary()
    trtDetectorTool.useDynamicAlignFolders = flags.GeoModel.Align.Dynamic
    # Use default TRT active gas in geo model unless in simulation.
    from AthenaConfiguration.Enums import Project, ProductionStep
    if (flags.Common.Project is not Project.AthSimulation
            and flags.Common.ProductionStep not in [ProductionStep.Simulation, ProductionStep.FastChain]):
        trtDetectorTool.DoXenonArgonMixture = False
        trtDetectorTool.DoKryptonMixture = False

    from TRT_ConditionsServices.TRT_ConditionsServicesConfig import TRT_StrawStatusSummaryToolCfg
    acc.popToolsAndMerge(TRT_StrawStatusSummaryToolCfg(flags, forceLegacyAccess=True))  # FIXME: if we set the tool, things break for unknown reasons
    geoModelSvc.DetectorTools += [ trtDetectorTool ]
    acc.merge(db)
    return acc


def TRT_AlignmentCfg(flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()
    if flags.GeoModel.Align.LegacyConditionsAccess:  # revert to old style CondHandle in case of simulation
        from IOVDbSvc.IOVDbSvcConfig import addFoldersSplitOnline
        acc.merge(addFoldersSplitOnline(flags, "TRT", "/TRT/Onl/Calib/DX", "/TRT/Calib/DX"))
        if flags.GeoModel.Align.Dynamic:
            acc.merge(addFoldersSplitOnline(flags, "TRT", "/TRT/Onl/AlignL1/TRT", "/TRT/AlignL1/TRT"))
            acc.merge(addFoldersSplitOnline(flags, "TRT", "/TRT/Onl/AlignL2", "/TRT/AlignL2"))
        else:
            acc.merge(addFoldersSplitOnline(flags, "TRT", "/TRT/Onl/Align", "/TRT/Align"))
    else:
        from TRT_ConditionsAlgs.TRT_ConditionsAlgsConfig import TRTAlignCondAlgCfg
        acc.merge(TRTAlignCondAlgCfg(flags))
    return acc


@AccumulatorCache
def TRT_SimulationGeometryCfg(flags):
    # main GeoModel config
    acc = TRT_GeoModelCfg(flags)
    acc.merge(TRT_AlignmentCfg(flags))
    return acc


@AccumulatorCache
def TRT_ReadoutGeometryCfg(flags):
    # main GeoModel config
    acc = TRT_GeoModelCfg(flags)
    acc.merge(TRT_AlignmentCfg(flags))
    # Note: this has almost the same content but different name on purpose if
    # we ever split readout geometry in a separate conditions algorithm
    from TRT_ConditionsAlgs.TRT_ConditionsAlgsConfig import TRTAlignCondAlgCfg
    acc.merge(TRTAlignCondAlgCfg(flags))
    return acc
