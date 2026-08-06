# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ActsAnalogueClusteringToolCfg(flags,
                                  name: str='ActsAnalogueClusteringTool',
                                  **kwargs) -> ComponentAccumulator:

    if not flags.Detector.GeometryITk:
        raise Exception("Acts Analogue Clustering calibration only supports ITk!")

    acc = ComponentAccumulator()

    from PixelConditionsAlgorithms.ITkPixelConditionsConfig import ITkPixelOfflineCalibCondAlgCfg
    acc.merge(ITkPixelOfflineCalibCondAlgCfg(flags))

    from ActsConfig.ActsConfigFlags import PixelErrorStrategy

    kwargs.setdefault("PerformCovarianceCalibration", flags.Acts.OnTrackCalibration.performCovarianceCalibration)
    kwargs.setdefault("PixelOfflineCalibData", "ITkPixelOfflineCalibData")
    kwargs.setdefault("errorStrategy", PixelErrorStrategy.PITCH.value if flags.Acts.Clusters.UsePixelBroadErrors
                      else PixelErrorStrategy.CALIBRATED.value)

    if 'PixelLorentzAngleTool' not in kwargs:
        from SiLorentzAngleTool.ITkPixelLorentzAngleConfig import ITkPixelLorentzAngleToolCfg
        kwargs.setdefault("PixelLorentzAngleTool", acc.popToolsAndMerge(ITkPixelLorentzAngleToolCfg(flags)))


    from ActsConfig.ActsConfigFlags import PixelCalibrationStrategy
    ClusteringToolType = None
    if flags.Acts.PixelCalibrationStrategy is  PixelCalibrationStrategy.NNClustering:
        ClusteringToolType = CompFactory.ActsTrk.ITkNNClusterCalibratorTool
        kwargs.setdefault("minClusterChargeForNN", 15000.0)
        from InDetConfig.SiClusterizationToolConfig import OnnxNNCondAlgCfg
        acc.merge(OnnxNNCondAlgCfg(flags,
                                   NumberNetworkPath=flags.Acts.PixelNNCalibrationModelsFolder+'number.onnx',
                                   PositionNetwork1Path=flags.Acts.PixelNNCalibrationModelsFolder+'pos1.onnx',
                                   PositionNetwork2Path=flags.Acts.PixelNNCalibrationModelsFolder+'pos2.onnx',
                                   PositionNetwork3Path=flags.Acts.PixelNNCalibrationModelsFolder+'pos3.onnx'))
    else:
        ClusteringToolType = CompFactory.ActsTrk.ITkAnalogueClusteringTool


    acc.setPrivateTools(ClusteringToolType(name, **kwargs))
    return acc

def ActsStripCalibrationToolCfg(flags,
                                  name: str='ActsStripCalibrationTool',
                                  **kwargs) -> ComponentAccumulator:

    if not flags.Detector.GeometryITk:
        raise Exception("Acts Strip calibration only supports ITk!")

    acc = ComponentAccumulator()

    from ActsConfig.ActsConfigFlags import StripClusteringErrorMode,StripErrorStrategy

    kwargs.setdefault("PerformCovarianceCalibration", True)
    kwargs.setdefault("errorStrategy", StripErrorStrategy.PITCH.value if flags.Acts.Clusters.StripClusteringErrorMode == StripClusteringErrorMode.WIDTH
                      else StripErrorStrategy.CLUSTERING.value)

    acc.setPrivateTools(CompFactory.ActsTrk.ITkStripCalibrationTool(name, **kwargs))
    return acc
