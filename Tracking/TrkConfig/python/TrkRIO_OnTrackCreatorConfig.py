# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Configuration of TrkRIO_OnTrackCreator package
# RIO (Reconstruction Input Objects) On Track (ROT)
# Represent clusters, drift circles (PrepRawData)
# with additonal detector specific calibration corrections.

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod


def InDetRotCreatorCfg(flags, name='InDetRotCreator', **kwargs):
    if flags.Detector.GeometryITk:
        name = name.replace("InDet", "ITk")
        return ITkRotCreatorCfg(flags, name, **kwargs)

    acc = ComponentAccumulator()

    if 'ToolPixelCluster' not in kwargs:
        if flags.Tracking.useBroadPixClusterErrors:
            from InDetConfig.SiClusterOnTrackTool_PixelConfig import (
                InDetBroadPixelClusterOnTrackToolCfg)
            ToolPixelCluster = acc.popToolsAndMerge(
                InDetBroadPixelClusterOnTrackToolCfg(flags))
        else:
            from InDetConfig.SiClusterOnTrackTool_PixelConfig import (
                InDetPixelClusterOnTrackToolCfg)
            ToolPixelCluster = acc.popToolsAndMerge(
                InDetPixelClusterOnTrackToolCfg(flags))
        kwargs.setdefault("ToolPixelCluster", ToolPixelCluster)

    if 'ToolSCT_Cluster' not in kwargs:
        if flags.Tracking.useBroadSCTClusterErrors:
            from InDetConfig.SiClusterOnTrackTool_SCTStripConfig import (
                InDetBroadSCT_ClusterOnTrackToolCfg)
            ToolSCT_Cluster = acc.popToolsAndMerge(
                InDetBroadSCT_ClusterOnTrackToolCfg(flags))
        else:
            from InDetConfig.SiClusterOnTrackTool_SCTStripConfig import (
                InDetSCT_ClusterOnTrackToolCfg)
            ToolSCT_Cluster = acc.popToolsAndMerge(
                InDetSCT_ClusterOnTrackToolCfg(flags))
        kwargs.setdefault("ToolSCT_Cluster", ToolSCT_Cluster)

    if 'ToolTRT_DriftCircle' not in kwargs:
        from InDetConfig.TRT_DriftCircleOnTrackToolConfig import (
            TRT_DriftCircleOnTrackToolCfg)
        kwargs.setdefault("ToolTRT_DriftCircle", acc.popToolsAndMerge(
            TRT_DriftCircleOnTrackToolCfg(flags)))

    kwargs.setdefault('Mode', 'indet')

    acc.setPrivateTools(CompFactory.Trk.RIO_OnTrackCreator(name, **kwargs))
    return acc


def InDetBroadRotCreatorCfg(flags, name='InDetBroadInDetRotCreator', **kwargs):
    acc = ComponentAccumulator()

    if 'ToolPixelCluster' not in kwargs:
        from InDetConfig.SiClusterOnTrackTool_PixelConfig import (
            InDetBroadPixelClusterOnTrackToolCfg)
        kwargs.setdefault('ToolPixelCluster', acc.popToolsAndMerge(
            InDetBroadPixelClusterOnTrackToolCfg(flags)))

    if 'ToolSCT_Cluster' not in kwargs:
        from InDetConfig.SiClusterOnTrackTool_SCTStripConfig import (
            InDetBroadSCT_ClusterOnTrackToolCfg)
        kwargs.setdefault('ToolSCT_Cluster', acc.popToolsAndMerge(
            InDetBroadSCT_ClusterOnTrackToolCfg(flags)))

    if flags.Detector.EnableTRT and 'ToolTRT_DriftCircle' not in kwargs:
        from InDetConfig.TRT_DriftCircleOnTrackToolConfig import (
            TRT_DriftCircleOnTrackNoDriftTimeToolCfg)
        kwargs.setdefault('ToolTRT_DriftCircle', acc.popToolsAndMerge(
            TRT_DriftCircleOnTrackNoDriftTimeToolCfg(flags)))

    acc.setPrivateTools(acc.popToolsAndMerge(
        InDetRotCreatorCfg(flags, name, **kwargs)))
    return acc


def InDetRefitRotCreatorCfg(flags, name='InDetRefitRotCreator', **kwargs):
    acc = ComponentAccumulator()

    if flags.Detector.EnableTRT:
        if 'ToolTRT_DriftCircle' not in kwargs:
            from InDetConfig.TRT_DriftCircleOnTrackToolConfig import (
                TRT_DriftCircleOnTrackUniversalToolCfg)
            kwargs.setdefault("ToolTRT_DriftCircle", acc.popToolsAndMerge(
                TRT_DriftCircleOnTrackUniversalToolCfg(flags)))

    acc.setPrivateTools(acc.popToolsAndMerge(
        InDetRotCreatorCfg(flags, name, **kwargs)))
    return acc


def InDetRotCreatorDigitalCfg(flags, name='InDetRotCreatorDigital', **kwargs):
    acc = ComponentAccumulator()

    if 'ToolPixelCluster' not in kwargs:
        from InDetConfig.SiClusterOnTrackTool_PixelConfig import (
            InDetPixelClusterOnTrackToolDigitalCfg)
        kwargs.setdefault('ToolPixelCluster', acc.popToolsAndMerge(
            InDetPixelClusterOnTrackToolDigitalCfg(flags)))

    acc.setPrivateTools(acc.popToolsAndMerge(
        InDetRotCreatorCfg(flags, name, **kwargs)))
    return acc


def TrigRotCreatorCfg(flags, name='InDetTrigRotCreator', **kwargs):
    acc = ComponentAccumulator()

    if 'ToolPixelCluster' not in kwargs:
        from InDetConfig.SiClusterOnTrackTool_PixelConfig import (
            TrigPixelClusterOnTrackToolBaseCfg)
        kwargs.setdefault("ToolPixelCluster", acc.popToolsAndMerge(
            TrigPixelClusterOnTrackToolBaseCfg(flags)))

    if 'ToolSCT_Cluster' not in kwargs:
        from InDetConfig.SiClusterOnTrackTool_SCTStripConfig import (
            InDetSCT_ClusterOnTrackToolCfg)
        kwargs.setdefault("ToolSCT_Cluster", acc.popToolsAndMerge(
            InDetSCT_ClusterOnTrackToolCfg(flags)))

    kwargs.setdefault('Mode', 'indet')

    acc.setPrivateTools(CompFactory.Trk.RIO_OnTrackCreator(name, **kwargs))
    return acc


def ITkRotCreatorCfg(flags, name='ITkRotCreator', **kwargs):
    acc = ComponentAccumulator()

    if 'ToolPixelCluster' not in kwargs:
        from InDetConfig.SiClusterOnTrackTool_PixelConfig import (
            ITkPixelClusterOnTrackToolCfg)
        kwargs.setdefault("ToolPixelCluster", acc.popToolsAndMerge(
            ITkPixelClusterOnTrackToolCfg(flags)))

    if 'ToolSCT_Cluster' not in kwargs:
        from InDetConfig.SiClusterOnTrackTool_SCTStripConfig import (
            ITkStripClusterOnTrackToolCfg)
        kwargs.setdefault("ToolSCT_Cluster", acc.popToolsAndMerge(
            ITkStripClusterOnTrackToolCfg(flags)))

    kwargs.setdefault("ToolTRT_DriftCircle", None)
    kwargs.setdefault('Mode', 'indet')

    acc.setPrivateTools(CompFactory.Trk.RIO_OnTrackCreator(name, **kwargs))
    return acc


def ITkBroadRotCreatorCfg(flags, name='ITkBroadRotCreator', **kwargs):
    acc = ComponentAccumulator()

    if 'ToolPixelCluster' not in kwargs:
        from InDetConfig.SiClusterOnTrackTool_PixelConfig import (
            ITkBroadPixelClusterOnTrackToolCfg)
        kwargs.setdefault('ToolPixelCluster', acc.popToolsAndMerge(
            ITkBroadPixelClusterOnTrackToolCfg(flags)))

    if 'ToolSCT_Cluster' not in kwargs:
        from InDetConfig.SiClusterOnTrackTool_SCTStripConfig import (
            ITkBroadStripClusterOnTrackToolCfg)
        kwargs.setdefault('ToolSCT_Cluster', acc.popToolsAndMerge(
            ITkBroadStripClusterOnTrackToolCfg(flags)))

    acc.setPrivateTools(acc.popToolsAndMerge(
        ITkRotCreatorCfg(flags, name, **kwargs)))
    return acc


def MuonRotCreatorCfg(flags, name="MuonRotCreator", **kwargs):
    result = ComponentAccumulator()

    from MuonConfig.MuonRIO_OnTrackCreatorToolConfig import (
        MdtDriftCircleOnTrackCreatorCfg, MuonClusterOnTrackCreatorCfg)
    mdt_rot_creator = result.popToolsAndMerge(
        MdtDriftCircleOnTrackCreatorCfg(flags))
    cluster_rot_creator = result.popToolsAndMerge(
        MuonClusterOnTrackCreatorCfg(flags))

    kwargs.setdefault("ToolMuonDriftCircle", mdt_rot_creator)
    kwargs.setdefault("ToolMuonCluster", cluster_rot_creator)    
    kwargs.setdefault("ToolPixelCluster", None)
    kwargs.setdefault("ToolSCT_Cluster", None)
    kwargs.setdefault("ToolTRT_DriftCircle", None)
    kwargs.setdefault("Mode", 'muon')

    result.setPrivateTools(CompFactory.Trk.RIO_OnTrackCreator(name, **kwargs))
    return result

### ROT creator for combined ID & Muon fit useage
def CombinedRotCreatorCfg(flags, name="TrkRotCreator", **kwargs):
    result = ComponentAccumulator()
    #### Inner detector part
    if flags.Detector.GeometryITk:
        from InDetConfig.SiClusterOnTrackTool_PixelConfig import ITkPixelClusterOnTrackToolCfg
        kwargs.setdefault("ToolPixelCluster", result.popToolsAndMerge(ITkPixelClusterOnTrackToolCfg(flags)))
        from InDetConfig.SiClusterOnTrackTool_SCTStripConfig import ITkStripClusterOnTrackToolCfg
        kwargs.setdefault("ToolSCT_Cluster", result.popToolsAndMerge(ITkStripClusterOnTrackToolCfg(flags)))
    elif flags.Detector.GeometryID:
        if flags.Tracking.useBroadPixClusterErrors:
            from InDetConfig.SiClusterOnTrackTool_PixelConfig import InDetBroadPixelClusterOnTrackToolCfg
            kwargs.setdefault("ToolPixelCluster", 
                              result.popToolsAndMerge(InDetBroadPixelClusterOnTrackToolCfg(flags)))
        else:
            from InDetConfig.SiClusterOnTrackTool_PixelConfig import InDetPixelClusterOnTrackToolCfg
            kwargs.setdefault("ToolPixelCluster", 
                              result.popToolsAndMerge(InDetPixelClusterOnTrackToolCfg(flags)))
        if flags.Tracking.useBroadSCTClusterErrors:
            from InDetConfig.SiClusterOnTrackTool_SCTStripConfig import InDetBroadSCT_ClusterOnTrackToolCfg
            kwargs.setdefault("ToolSCT_Cluster", 
                             result.popToolsAndMerge(InDetBroadSCT_ClusterOnTrackToolCfg(flags)))
        else:
            from InDetConfig.SiClusterOnTrackTool_SCTStripConfig import InDetSCT_ClusterOnTrackToolCfg
            kwargs.setdefault("ToolSCT_Cluster", result.popToolsAnsMerge(InDetSCT_ClusterOnTrackToolCfg(flags)))
    else: ### Just take the muon calibrator
        result.setPrivateTools(result.popToolsAndMerge(MuonRotCreatorCfg(flags, name=name, **kwargs)))
        return result
    #### TRT part
    if flags.Detector.GeometryTRT:
        from InDetConfig.TRT_DriftCircleOnTrackToolConfig import TRT_DriftCircleOnTrackToolCfg
        kwargs.setdefault("ToolTRT_DriftCircle", result.popToolsAndMerge(TRT_DriftCircleOnTrackToolCfg(flags)))
    #### Muon part
    if flags.Detector.GeometryMuon:
        from MuonConfig.MuonRIO_OnTrackCreatorToolConfig import MdtDriftCircleOnTrackCreatorCfg
        from MuonConfig.MuonRIO_OnTrackCreatorToolConfig import MuonClusterOnTrackCreatorCfg
        kwargs.setdefault("ToolMuonDriftCircle", result.popToolsAndMerge(MdtDriftCircleOnTrackCreatorCfg(flags)))
        kwargs.setdefault("ToolMuonCluster", result.popToolsAndMerge(MuonClusterOnTrackCreatorCfg(flags)))    
    else:
        result.setPrivateTools(result.popToolsAndMerge(InDetRotCreatorCfg(flags,name=name, **kwargs)))
        return result
    result.setPrivateTools(CompFactory.Trk.RIO_OnTrackCreator(name, **kwargs))
    return result

def RIO_OnTrackErrorScalingCondAlgCfg(flags,
                                      name='RIO_OnTrackErrorScalingCondAlg',
                                      **kwargs):
    acc = ComponentAccumulator()

    if flags.GeoModel.Run is LHCPeriod.Run1:
        error_scaling_type = ["PixelRIO_OnTrackErrorScalingRun1"]
        error_scaling_outkey = ["/Indet/TrkErrorScalingPixel"]
    else:  # Run 2 and 3
        error_scaling_type = ["PixelRIO_OnTrackErrorScaling"]
        error_scaling_outkey = ["/Indet/TrkErrorScalingPixel"]

    error_scaling_type += ["SCTRIO_OnTrackErrorScaling"]
    error_scaling_outkey += ["/Indet/TrkErrorScalingSCT"]

    # TODO: cover Run 4 and beyond

    if flags.GeoModel.Run <= LHCPeriod.Run3:
        error_scaling_type += ["TRTRIO_OnTrackErrorScaling"]
        error_scaling_outkey += ["/Indet/TrkErrorScalingTRT"]

    from IOVDbSvc.IOVDbSvcConfig import addFoldersSplitOnline
    acc.merge(addFoldersSplitOnline(flags,
                                    'INDET',
                                    '/Indet/Onl/TrkErrorScaling',
                                    '/Indet/TrkErrorScaling',
                                    className="CondAttrListCollection"))

    kwargs.setdefault("ReadKey", "/Indet/TrkErrorScaling")
    kwargs.setdefault("ErrorScalingType", error_scaling_type)
    kwargs.setdefault("OutKeys", error_scaling_outkey)

    acc.addCondAlgo(CompFactory.RIO_OnTrackErrorScalingCondAlg(name, **kwargs))
    return acc
