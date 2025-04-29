# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
# Configuration of pixel tools of SiClusterOnTrackTool package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType
from TrkConfig.TrkConfigFlags import PixelClusterSplittingType

#############################################
### InDet PixelClusterOnTrackTool offline ###
#############################################


def InDetPixelClusterOnTrackToolBaseCfg(
        flags, name="PixelClusterOnTrackTool", **kwargs):
    # To produce PixelOfflineCalibData + PixelDistortionData
    from PixelConditionsAlgorithms.PixelConditionsConfig import (
        PixelDistortionAlgCfg, PixelOfflineCalibCondAlgCfg)
    acc = PixelOfflineCalibCondAlgCfg(flags)
    acc.merge(PixelDistortionAlgCfg(flags))

    # To produce RIO_OnTrackErrorScaling
    from TrkConfig.TrkRIO_OnTrackCreatorConfig import (
        RIO_OnTrackErrorScalingCondAlgCfg)
    acc.merge(RIO_OnTrackErrorScalingCondAlgCfg(flags))

    if 'LorentzAngleTool' not in kwargs:
        from SiLorentzAngleTool.PixelLorentzAngleConfig import (
            PixelLorentzAngleToolCfg)
        kwargs.setdefault("LorentzAngleTool", acc.popToolsAndMerge(
            PixelLorentzAngleToolCfg(flags)))

    if flags.Beam.Type is BeamType.Cosmics:
        kwargs.setdefault("ErrorStrategy", 0)
        kwargs.setdefault("PositionStrategy", 0)

    acc.setPrivateTools(
        CompFactory.InDet.PixelClusterOnTrackTool(name, **kwargs))
    return acc


def InDetPixelClusterOnTrackToolDigitalCfg(
        flags, name="InDetPixelClusterOnTrackToolDigital", **kwargs):
    kwargs.setdefault("SplitClusterAmbiguityMap", "")
    return InDetPixelClusterOnTrackToolBaseCfg(flags, name, **kwargs)


def InDetPixelClusterOnTrackToolCfg(
        flags, name="InDetPixelClusterOnTrackTool", **kwargs):
    acc = ComponentAccumulator()

    if (flags.Tracking.doPixelClusterSplitting and
        (flags.Tracking.pixelClusterSplittingType is
         PixelClusterSplittingType.NeuralNet)):

        kwargs.setdefault("applyNNcorrection", True)
        kwargs.setdefault("NNIBLcorrection", True)

        extension = flags.Tracking.ActiveConfig.extension
        if extension == flags.Tracking.PrimaryPassConfig.value:
            extension = ""
        split_cluster_map_extension = (
            extension if flags.Tracking.ActiveConfig.useTIDE_Ambi else "")
        kwargs.setdefault("SplitClusterAmbiguityMap",
                          f"SplitClusterAmbiguityMap{split_cluster_map_extension}")
        kwargs.setdefault("RunningTIDE_Ambi", flags.Tracking.doTIDE_Ambi)

        if "NnClusterizationFactory" not in kwargs:
            from InDetConfig.SiClusterizationToolConfig import (
                NnClusterizationFactoryCfg)
            kwargs.setdefault("NnClusterizationFactory", acc.popToolsAndMerge(
                NnClusterizationFactoryCfg(flags)))

    if flags.Tracking.doPixelDigitalClustering:
        kwargs.setdefault("PositionStrategy", 0)
        kwargs.setdefault("ErrorStrategy", 1)

    acc.setPrivateTools(acc.popToolsAndMerge(
        InDetPixelClusterOnTrackToolBaseCfg(flags, name, **kwargs)))
    return acc


def InDetBroadPixelClusterOnTrackToolCfg(
        flags, name='InDetBroadPixelClusterOnTrackTool', **kwargs):
    kwargs.setdefault("ErrorStrategy", 0)
    return InDetPixelClusterOnTrackToolCfg(flags, name, **kwargs)

#############################################
### InDet PixelClusterOnTrackTool trigger ###
#############################################


def TrigPixelClusterOnTrackToolBaseCfg(
        flags, name="InDetTrigPixelClusterOnTrackTool", **kwargs):
    # To produce PixelOfflineCalibData + PixelDistortionData
    from PixelConditionsAlgorithms.PixelConditionsConfig import (
        PixelDistortionAlgCfg, PixelOfflineCalibCondAlgCfg)
    acc = PixelOfflineCalibCondAlgCfg(flags)
    acc.merge(PixelDistortionAlgCfg(flags))

    # To produce RIO_OnTrackErrorScaling
    from TrkConfig.TrkRIO_OnTrackCreatorConfig import (
        RIO_OnTrackErrorScalingCondAlgCfg)
    acc.merge(RIO_OnTrackErrorScalingCondAlgCfg(flags))

    if 'LorentzAngleTool' not in kwargs:
        from SiLorentzAngleTool.PixelLorentzAngleConfig import (
            PixelLorentzAngleToolCfg)
        kwargs.setdefault("LorentzAngleTool", acc.popToolsAndMerge(
            PixelLorentzAngleToolCfg(flags)))

    if 'NnClusterizationFactory' not in kwargs:
        from InDetConfig.SiClusterizationToolConfig import (
            TrigNnClusterizationFactoryCfg)
        kwargs.setdefault("NnClusterizationFactory", acc.popToolsAndMerge(
            TrigNnClusterizationFactoryCfg(flags)))

    kwargs.setdefault("ErrorStrategy", 2)
    kwargs.setdefault("SplitClusterAmbiguityMap",
                      flags.Trigger.InDetTracking.ClusterAmbiguitiesMap)

    acc.setPrivateTools(
        CompFactory.InDet.PixelClusterOnTrackTool(name, **kwargs))
    return acc

###########################################
### ITk PixelClusterOnTrackTool offline ###
###########################################


def ITkPixelClusterOnTrackToolBaseCfg(
        flags, name="ITkPixelClusterOnTrackTool", **kwargs):
    # To produce PixelOfflineCalibData
    from PixelConditionsAlgorithms.ITkPixelConditionsConfig import (
        ITkPixelOfflineCalibCondAlgCfg)
    acc = ITkPixelOfflineCalibCondAlgCfg(flags)

    if 'LorentzAngleTool' not in kwargs:
        from SiLorentzAngleTool.ITkPixelLorentzAngleConfig import (
            ITkPixelLorentzAngleToolCfg)
        kwargs.setdefault("LorentzAngleTool", acc.popToolsAndMerge(
            ITkPixelLorentzAngleToolCfg(flags)))

    if flags.Beam.Type is BeamType.Cosmics:
        kwargs.setdefault("ErrorStrategy", 0)
        kwargs.setdefault("PositionStrategy", 0)

    kwargs.setdefault("PixelErrorScalingKey", "")

    acc.setPrivateTools(
        CompFactory.ITk.PixelClusterOnTrackTool(name, **kwargs))
    return acc


def ITkPixelClusterOnTrackToolCfg(
        flags, name='ITkPixelClusterOnTrackTool', **kwargs):
    acc = ComponentAccumulator()

    if (flags.Tracking.doPixelClusterSplitting and
        (flags.Tracking.pixelClusterSplittingType is
         PixelClusterSplittingType.NeuralNet)):

        kwargs.setdefault("applyNNcorrection", True)
        kwargs.setdefault(
            "SplitClusterAmbiguityMap",
            f"SplitClusterAmbiguityMap{flags.Tracking.ActiveConfig.extension}")
        kwargs.setdefault("RunningTIDE_Ambi", flags.Tracking.doTIDE_Ambi)

        if "NnClusterizationFactory" not in kwargs:
            from InDetConfig.SiClusterizationToolConfig import (
                ITkNnClusterizationFactoryCfg)
            kwargs.setdefault("NnClusterizationFactory", acc.popToolsAndMerge(
                ITkNnClusterizationFactoryCfg(flags)))

    if flags.Tracking.doPixelDigitalClustering:
        kwargs.setdefault("PositionStrategy", 0)
        kwargs.setdefault("ErrorStrategy", 1)

    acc.setPrivateTools(acc.popToolsAndMerge(
        ITkPixelClusterOnTrackToolBaseCfg(flags, name, **kwargs)))
    return acc


def ITkBroadPixelClusterOnTrackToolCfg(
        flags, name='ITkBroadPixelClusterOnTrackTool', **kwargs):
    kwargs.setdefault("ErrorStrategy", 0)
    return ITkPixelClusterOnTrackToolCfg(flags, name, **kwargs)
