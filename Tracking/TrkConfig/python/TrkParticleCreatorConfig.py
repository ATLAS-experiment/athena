# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of TrkParticleCreator package
# Creating xAOD::TrackParticles starting from
# input Trk::Tracks

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType, LHCPeriod


####################################
#####        InDet/ITk         #####
####################################
def TrackParticleCreatorToolCfg(flags,
                                name="InDetxAODParticleCreatorTool",
                                **kwargs):

    if flags.Detector.GeometryITk:
        name = name.replace("InDet", "ITk")
        return ITkTrackParticleCreatorToolCfg(flags, name, **kwargs)

    # To produce InDet::BeamSpotData CondHandle
    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
    result = BeamSpotCondAlgCfg(flags)

    if "TrackToVertex" not in kwargs:
        from TrackToVertex.TrackToVertexConfig import TrackToVertexCfg
        kwargs.setdefault("TrackToVertex", result.popToolsAndMerge(
            TrackToVertexCfg(flags)))

    if "TrackSummaryTool" not in kwargs:
        from TrkConfig.TrkTrackSummaryToolConfig import InDetTrackSummaryToolCfg
        TrackSummaryTool = result.popToolsAndMerge(
            InDetTrackSummaryToolCfg(flags))
        result.addPublicTool(TrackSummaryTool)
        kwargs.setdefault("TrackSummaryTool", TrackSummaryTool)

    if "TRT_ElectronPidTool" not in kwargs and flags.Detector.EnableTRT:
        from InDetConfig.TRT_ElectronPidToolsConfig import (
            TRT_ElectronPidToolCfg)
        kwargs.setdefault("TRT_ElectronPidTool", result.popToolsAndMerge(
            TRT_ElectronPidToolCfg(flags, name="InDetTRT_ElectronPidTool")))

    if 'PixelToTPIDTool' not in kwargs and flags.Detector.EnablePixel:
        from InDetConfig.PixelToTPIDToolConfig import PixelToTPIDToolCfg
        kwargs.setdefault("PixelToTPIDTool", result.popToolsAndMerge(
            PixelToTPIDToolCfg(flags)))

    if 'TestPixelLayerTool' not in kwargs and flags.Detector.EnablePixel:
        from InDetConfig.InDetTestPixelLayerConfig import (
            InDetTestPixelLayerToolInnerCfg)
        kwargs.setdefault("TestPixelLayerTool", result.popToolsAndMerge(
            InDetTestPixelLayerToolInnerCfg(flags)))

    kwargs.setdefault("ComputeAdditionalInfo", True)
    kwargs.setdefault("AssociationMapName", "")
    kwargs.setdefault("DoSharedSiHits", flags.Tracking.doSharedHits
                      and kwargs["AssociationMapName"] != "")
    kwargs.setdefault("DoSharedTRTHits", flags.Tracking.doSharedHits
                      and flags.Detector.EnableTRT
                      and kwargs["AssociationMapName"] != "")
    kwargs.setdefault("RunningTIDE_Ambi", flags.Tracking.doTIDE_Ambi)

    # Select the mode to identify suspicous pixel cluster
    kwargs.setdefault("BadClusterID", 3)

    kwargs.setdefault("KeepParameters", True)
    kwargs.setdefault("KeepFirstParameters", False)
    kwargs.setdefault("PerigeeExpression", flags.Tracking.perigeeExpression)

    result.setPrivateTools(
        CompFactory.Trk.TrackParticleCreatorTool(name, **kwargs))
    return result


def TrackParticleCreatorToolPIDCheckCfg(flags,
                                        name="InDetxAODParticleCreatorTool",
                                        **kwargs):

    # Used only through tracking passes, where ActiveConfig flags are defined
    if not flags.Tracking.ActiveConfig.RunTRTPID:
        kwargs.setdefault("TRT_ElectronPidTool", None)
    if not flags.Tracking.ActiveConfig.RunPixelPID:
        kwargs.setdefault("PixelToTPIDTool", None)
        kwargs.setdefault("TestPixelLayerTool", None)

    # have to create special public instance depending on PID tool configuration
    if name == "InDetxAODParticleCreatorTool":
        pixel_pid = flags.Tracking.ActiveConfig.RunPixelPID
        trt_pid = flags.Tracking.ActiveConfig.RunTRTPID
        if not trt_pid and not pixel_pid:
            name += "NoPID"
        elif not trt_pid:
            name += "NoTRTPID"
        elif not pixel_pid:
            name += "NoPixPID"

    return TrackParticleCreatorToolCfg(flags, name, **kwargs)


def TrackParticleCreatorToolNoPIDCfg(flags,
                                     name="InDetxAODParticleCreatorToolNoPID",
                                     **kwargs):
    kwargs.setdefault("TRT_ElectronPidTool", None)
    kwargs.setdefault("PixelToTPIDTool", None)
    kwargs.setdefault("TestPixelLayerTool", None)
    return TrackParticleCreatorToolCfg(flags, name, **kwargs)


def InDetTrigParticleCreatorToolCfg(flags,
                                         name="InDetTrigParticleCreatorTool",
                                         **kwargs):
    kwargs.setdefault("TRT_ElectronPidTool", None)
    return InDetTrigParticleCreatorToolTRTPidCfg(flags, name, **kwargs)

    
def InDetTrigParticleCreatorToolTRTPidCfg(flags,
                                         name="InDetTrigParticleCreatorToolTRTPid",
                                         **kwargs):
    
    result = ComponentAccumulator()

    kwargs.setdefault("PixelToTPIDTool", None)

    if "TRT_ElectronPidTool" not in kwargs and flags.Detector.EnableTRT:
        from InDetConfig.TRT_ElectronPidToolsConfig import (
            TrigTRT_ElectronPidToolCfg)
        kwargs.setdefault("TRT_ElectronPidTool", result.popToolsAndMerge(
            TrigTRT_ElectronPidToolCfg(flags)))

    if "TrackSummaryTool" not in kwargs:
        from TrkConfig.TrkTrackSummaryToolConfig import InDetTrigTrackSummaryToolCfg
        TrackSummaryTool = result.popToolsAndMerge(
            InDetTrigTrackSummaryToolCfg(flags))
        result.addPublicTool(TrackSummaryTool)
        kwargs.setdefault("TrackSummaryTool", TrackSummaryTool)

    if 'TestPixelLayerTool' not in kwargs and flags.Detector.EnablePixel:
        from InDetConfig.InDetTestPixelLayerConfig import (
            InDetTrigTestPixelLayerToolInnerCfg)
        kwargs.setdefault("TestPixelLayerTool", result.popToolsAndMerge(
            InDetTrigTestPixelLayerToolInnerCfg(flags)))

    kwargs.setdefault("RunningTIDE_Ambi", False)   #flags.Tracking.doTIDE_Ambi)

    kwargs.setdefault("BadClusterID", 3)    #2023fix

    kwargs.setdefault("KeepParameters", True)   #TODO #2023fix not necessary for all signatures
    kwargs.setdefault("PerigeeExpression", "BeamLine")

    result.setPrivateTools(result.popToolsAndMerge(TrackParticleCreatorToolCfg(flags, name, **kwargs)))
    return result


def InDetTrigParticleCreatorToolFTFCfg(flags,
                                       name="InDetTrigParticleCreatorToolFTF",
                                       **kwargs):
    result = ComponentAccumulator()

    if "TrackToVertex" not in kwargs:
        from TrackToVertex.TrackToVertexConfig import TrackToVertexCfg
        kwargs.setdefault("TrackToVertex", result.popToolsAndMerge(
            TrackToVertexCfg(flags)))

    if "TrackSummaryTool" not in kwargs:
        from TrkConfig.TrkTrackSummaryToolConfig import (
            InDetTrigFastTrackSummaryToolCfg)
        TrackSummaryTool = result.popToolsAndMerge(
            InDetTrigFastTrackSummaryToolCfg(flags))
        result.addPublicTool(TrackSummaryTool)
        kwargs.setdefault("TrackSummaryTool", TrackSummaryTool)
    # 2023fix
    # if 'TestPixelLayerTool' not in kwargs:
    #     from InDetConfig.InDetTestPixelLayerConfig import InDetTrigTestPixelLayerToolInnerCfg
    #     kwargs.setdefault("TestPixelLayerTool", result.popToolsAndMerge(InDetTrigTestPixelLayerToolInnerCfg(flags)))
    kwargs.setdefault("TestPixelLayerTool", "")  #must be empty key not None to prevent retrieval of unconfigured Extrapolator

    kwargs.setdefault("KeepParameters", True)
    kwargs.setdefault("ComputeAdditionalInfo", True)
    kwargs.setdefault("AssociationMapName", "")
    kwargs.setdefault("DoSharedSiHits", kwargs["AssociationMapName"] != "")

    result.setPrivateTools(
        CompFactory.Trk.TrackParticleCreatorTool(name, **kwargs))
    return result



def ITkTrackParticleCreatorToolCfg(flags,
                                   name="ITkTrackParticleCreatorTool",
                                   **kwargs):
    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
    # To produce InDet::BeamSpotData CondHandle
    result = BeamSpotCondAlgCfg(flags)

    if "TrackToVertex" not in kwargs:
        from TrackToVertex.TrackToVertexConfig import TrackToVertexCfg
        kwargs.setdefault("TrackToVertex", result.popToolsAndMerge(
            TrackToVertexCfg(flags)))

    if "TrackSummaryTool" not in kwargs:
        from TrkConfig.TrkTrackSummaryToolConfig import ITkTrackSummaryToolCfg
        TrackSummaryTool = result.popToolsAndMerge(
            ITkTrackSummaryToolCfg(flags))
        result.addPublicTool(TrackSummaryTool)
        kwargs.setdefault("TrackSummaryTool", TrackSummaryTool)

    if "TestPixelLayerTool" not in kwargs and flags.Detector.EnableITkPixel:
        from InDetConfig.InDetTestPixelLayerConfig import (
            ITkTestPixelLayerToolInnerCfg)
        kwargs.setdefault("TestPixelLayerTool", result.popToolsAndMerge(
            ITkTestPixelLayerToolInnerCfg(flags)))

    kwargs.setdefault("ComputeAdditionalInfo", True)
    kwargs.setdefault("AssociationMapName", "")
    kwargs.setdefault("DoSharedSiHits",
                      flags.Tracking.doSharedHits and
                      kwargs["AssociationMapName"] != "")
    kwargs.setdefault("RunningTIDE_Ambi", True)

    # Select the mode to identify suspicous pixel cluster
    kwargs.setdefault("BadClusterID", 3)

    kwargs.setdefault("KeepParameters", True)
    kwargs.setdefault("KeepFirstParameters", False)
    kwargs.setdefault("PerigeeExpression", flags.Tracking.perigeeExpression)

    kwargs.setdefault("IBLParameterSvc", "")
    kwargs.setdefault("DoITk", True)

    result.setPrivateTools(
        CompFactory.Trk.TrackParticleCreatorTool(name, **kwargs))
    return result


# egamma : Used to create TrackParticels from GSF Tracks after refit
def GSFBuildInDetParticleCreatorToolCfg(flags,
                                        name="GSFBuildInDetParticleCreatorTool",
                                        isTrigger = False,
                                        **kwargs):
    result = ComponentAccumulator()

    if "TrackToVertex" not in kwargs:
        from TrackToVertex.TrackToVertexConfig import TrackToVertexCfg
        kwargs.setdefault("TrackToVertex", result.popToolsAndMerge(
            TrackToVertexCfg(flags)))

    if "TrackSummaryTool" not in kwargs:
        from TrkConfig.TrkTrackSummaryToolConfig import GSFTrackSummaryToolCfg
        TrackSummaryTool = result.popToolsAndMerge(
            GSFTrackSummaryToolCfg(flags))
        result.addPublicTool(TrackSummaryTool)
        kwargs.setdefault("TrackSummaryTool", TrackSummaryTool)

    if flags.GeoModel.Run < LHCPeriod.Run4 and "PixelToTPIDTool" not in kwargs:
        from InDetConfig.PixelToTPIDToolConfig import PixelToTPIDToolCfg
        kwargs.setdefault("PixelToTPIDTool", result.popToolsAndMerge(
            PixelToTPIDToolCfg(flags)))

    if flags.Detector.EnableTRT and "TRT_ElectronPidTool" not in kwargs:
        from InDetConfig.TRT_ElectronPidToolsConfig import (
            GSFBuildTRT_ElectronPidToolCfg)
        kwargs.setdefault("TRT_ElectronPidTool", result.popToolsAndMerge(
            GSFBuildTRT_ElectronPidToolCfg(flags, isTrigger = isTrigger)))

    if ((flags.Detector.EnablePixel or flags.Detector.EnableITkPixel)
            and "TestPixelLayerTool" not in kwargs):
        from InDetConfig.InDetTestPixelLayerConfig import (
            InDetTestPixelLayerToolInnerCfg)
        kwargs.setdefault("TestPixelLayerTool", result.popToolsAndMerge(
            InDetTestPixelLayerToolInnerCfg(flags)))

    kwargs.setdefault("ComputeAdditionalInfo", True)
    kwargs.setdefault("KeepParameters", True)
    kwargs.setdefault("BadClusterID", 0)
    kwargs.setdefault("IBLParameterSvc",
                      "IBLParameterSvc" if flags.Detector.GeometryID else "")

    result.setPrivateTools(
        CompFactory.Trk.TrackParticleCreatorTool(name, **kwargs))
    return result


####################################
#####          Muons           #####
####################################
def MuonParticleCreatorToolCfg(flags, name="MuonParticleCreatorTool", **kwargs):
    result = ComponentAccumulator()

    if "TrackToVertex" not in kwargs:
        from TrackToVertex.TrackToVertexConfig import TrackToVertexCfg
        kwargs.setdefault("TrackToVertex", result.popToolsAndMerge(
            TrackToVertexCfg(flags)))

    if "TrackSummaryTool" not in kwargs:
        from TrkConfig.TrkTrackSummaryToolConfig import MuonTrackSummaryToolCfg
        kwargs.setdefault("TrackSummaryTool", result.popToolsAndMerge(
            MuonTrackSummaryToolCfg(flags)))

    if "MuonSummaryTool" not in kwargs:
        from MuonConfig.MuonRecToolsConfig import MuonHitSummaryToolCfg
        kwargs.setdefault("MuonSummaryTool", result.popToolsAndMerge(
            MuonHitSummaryToolCfg(flags)))

    kwargs.setdefault("KeepAllPerigee", True)
    kwargs.setdefault("PerigeeExpression", "Origin")
    kwargs.setdefault("IBLParameterSvc",
                      "IBLParameterSvc" if flags.Detector.GeometryID else "")

    result.setPrivateTools(
        CompFactory.Trk.TrackParticleCreatorTool(name, **kwargs))
    return result


def MuonCombinedParticleCreatorCfg(flags,
                                   name="MuonCombinedParticleCreator",
                                   **kwargs):
    result = ComponentAccumulator()

    if "TrackSummaryTool" not in kwargs:
        if flags.Muon.SAMuonTrigger:
            from TrkConfig.TrkTrackSummaryToolConfig import (
                MuonTrackSummaryToolCfg)
            kwargs.setdefault("TrackSummaryTool", result.popToolsAndMerge(
                MuonTrackSummaryToolCfg(flags)))
        else:
            from TrkConfig.TrkTrackSummaryToolConfig import (
                MuonCombinedTrackSummaryToolCfg)
            kwargs.setdefault("TrackSummaryTool", result.popToolsAndMerge(
                MuonCombinedTrackSummaryToolCfg(flags)))

    if "TrackToVertex" not in kwargs:
        from TrackToVertex.TrackToVertexConfig import TrackToVertexCfg
        kwargs.setdefault('TrackToVertex', result.popToolsAndMerge(
            TrackToVertexCfg(flags)))

    if "MuonSummaryTool" not in kwargs:
        from MuonConfig.MuonRecToolsConfig import MuonHitSummaryToolCfg
        kwargs.setdefault("MuonSummaryTool", result.popToolsAndMerge(
            MuonHitSummaryToolCfg(flags)))

    if not flags.Muon.MuonTrigger:

        if 'PixelToTPIDTool' not in kwargs and flags.Detector.EnablePixel:
            from InDetConfig.PixelToTPIDToolConfig import PixelToTPIDToolCfg
            kwargs.setdefault("PixelToTPIDTool", result.popToolsAndMerge(
                PixelToTPIDToolCfg(flags)))

        if ('TestPixelLayerTool' not in kwargs and
                (flags.Detector.EnablePixel or flags.Detector.EnableITkPixel)):
            from InDetConfig.InDetTestPixelLayerConfig import (
                InDetTestPixelLayerToolInnerCfg)
            kwargs.setdefault("TestPixelLayerTool", result.popToolsAndMerge(
                InDetTestPixelLayerToolInnerCfg(flags)))

        if "TRT_ElectronPidTool" not in kwargs and flags.Detector.EnableTRT:
            from InDetConfig.TRT_ElectronPidToolsConfig import (
                TRT_ElectronPidToolCfg)
            kwargs.setdefault("TRT_ElectronPidTool", result.popToolsAndMerge(
                TRT_ElectronPidToolCfg(flags)))

        kwargs.setdefault("ComputeAdditionalInfo", True)

    kwargs.setdefault("KeepAllPerigee", True)
    if flags.Beam.Type is BeamType.Cosmics:
        kwargs.setdefault("PerigeeExpression", "Origin")
    kwargs.setdefault("IBLParameterSvc",
                      "IBLParameterSvc" if flags.Detector.GeometryID else "")

    kwargs.setdefault("TrackingVolumesSvc", "TrackingVolumesSvc")
    result.setPrivateTools(
        CompFactory.Trk.TrackParticleCreatorTool(name, **kwargs))
    return result


def MuonCaloParticleCreatorCfg(flags, name="MuonCaloParticleCreator", **kwargs):
    result = ComponentAccumulator()

    if "TrackToVertex" not in kwargs:
        from TrackToVertex.TrackToVertexConfig import TrackToVertexCfg
        kwargs.setdefault("TrackToVertex", result.popToolsAndMerge(
            TrackToVertexCfg(flags, name='TrackToVertex')))

    if "TrackSummaryTool" not in kwargs:
        if flags.Muon.SAMuonTrigger:
            from TrkConfig.TrkTrackSummaryToolConfig import (
                MuonTrackSummaryToolCfg)
            kwargs.setdefault("TrackSummaryTool", result.popToolsAndMerge(
                MuonTrackSummaryToolCfg(flags)))
        else:
            from TrkConfig.TrkTrackSummaryToolConfig import (
                MuonCombinedTrackSummaryToolCfg)
            kwargs.setdefault("TrackSummaryTool", result.popToolsAndMerge(
                MuonCombinedTrackSummaryToolCfg(flags)))

    kwargs.setdefault("KeepAllPerigee", True)
    kwargs.setdefault("PerigeeExpression", "Origin")
    kwargs.setdefault("IBLParameterSvc",
                      "IBLParameterSvc" if flags.Detector.GeometryID else "")

    kwargs.setdefault("TrackingVolumesSvc", "TrackingVolumesSvc")

    result.setPrivateTools(
        CompFactory.Trk.TrackParticleCreatorTool(name, **kwargs))
    return result
