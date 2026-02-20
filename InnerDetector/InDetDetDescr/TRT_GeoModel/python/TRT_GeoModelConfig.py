#
#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
from AthenaConfiguration.AccumulatorCache import AccumulatorCache

def TRT_GeoModelCfg(flags):
    from AtlasGeoModel.GeoModelConfig import GeoModelCfg
    acc = GeoModelCfg(flags)
    geoModelSvc = acc.getPrimary()

    from AthenaConfiguration.ComponentFactory import CompFactory
    trtDetectorTool = CompFactory.TRT_DetectorTool()
    trtDetectorTool.useDynamicAlignFolders = flags.GeoModel.Align.Dynamic
    # Use default TRT active gas in geo model unless in simulation.
    from AthenaConfiguration.Enums import LHCPeriod
    from AthenaConfiguration.Enums import Project, ProductionStep
    if (flags.Common.Project is not Project.AthSimulation
            and flags.Common.ProductionStep not in [ProductionStep.Simulation, ProductionStep.FastChain]) or flags.GeoModel.Run is LHCPeriod.Run1:
        trtDetectorTool.DoXenonArgonMixture = False
        trtDetectorTool.DoKryptonMixture = False
    if flags.GeoModel.Run is LHCPeriod.Run3:
        # TRT filled with Run 3 mixture of Xenon and Argon for pp runs [DB=791CE02A-56D5-5A4C-9150-CCADCB68B31F]
        trtDetectorTool.StrawStatusFile = 'TRTGeometry/Run3MCStrawStatus.txt'
    elif flags.GeoModel.Run is LHCPeriod.Run2:
        # TRT filled with Run 2 mixture of Xenon and Argon for pp runs [DB=8D6AE810-BB00-B44C-A21E-15DE270363BE]
        trtDetectorTool.StrawStatusFile = 'TRTGeometry/Run2MC_pp_StrawStatus.txt'
        if flags.Input.ConditionsRunNumber in [222506, 222507, 313000] or (flags.Input.ConditionsRunNumber >= 226000 and flags.Input.ConditionsRunNumber < 228000):
            # Full TRT filled with Argon for HI runs [DB=52200EDF-D5BF-E24A-AAD4-6BFFEA94BFEF]
            trtDetectorTool.StrawStatusFile = 'TRTGeometry/Run2MC_HI_StrawStatus.txt'
    elif flags.GeoModel.Run is not LHCPeriod.Run1:
        # TRT filled with pure Xenon in Run 1.
        raise ValueError(f'Cannot configure the TRT geometry for LHCPeriod {flags.GeoModel.Run.value}, please check the configuration of "flags.GeoModel.Run".')

    ## Un-comment the next line for dumping straw statuses into an ASCII file in the run directory
    # trtDetectorTool.DumpStrawStatus = True
    # from TRT_ConditionsServices.TRT_ConditionsServicesConfig import TRT_StrawStatusSummaryToolCfg
    #trtDetectorTool.SummaryTool = acc.popToolsAndMerge(TRT_StrawStatusSummaryToolCfg(flags))
    geoModelSvc.DetectorTools += [ trtDetectorTool ]
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
