# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ActsAnalogueClusteringToolBaseCfg(flags,
                               name: str='ActsAnalogueClusteringTool',
                               ClusteringToolType=CompFactory.ActsTrk.ITkAnalogueClusteringTool,
                               **kwargs) -> ComponentAccumulator:

    if not flags.Detector.GeometryITk:
        raise Exception("Acts Analogue Clustering calibration only supports ITk!")

    acc = ComponentAccumulator()

    from PixelConditionsAlgorithms.ITkPixelConditionsConfig import ITkPixelOfflineCalibCondAlgCfg
    acc.merge(ITkPixelOfflineCalibCondAlgCfg(flags))

    from ActsConfig.ActsConfigFlags import PixelErrorStrategy

    kwargs.setdefault("PerformCovarianceCalibration", flags.Acts.OnTrackCalibration.performCovarianceCalibration)
    kwargs.setdefault("CalibrateAfterMeasurementSelection", not flags.Acts.PixelCalibrationStrategy.calibrateBeforeSelection())
    kwargs.setdefault("PixelOfflineCalibData", "ITkPixelOfflineCalibData")
    kwargs.setdefault("errorStrategy", PixelErrorStrategy.PITCH.value if flags.Acts.Clusters.UsePixelBroadErrors
                      else PixelErrorStrategy.CALIBRATED.value)

    if 'PixelLorentzAngleTool' not in kwargs:
        from SiLorentzAngleTool.ITkPixelLorentzAngleConfig import ITkPixelLorentzAngleToolCfg
        kwargs.setdefault("PixelLorentzAngleTool", acc.popToolsAndMerge(ITkPixelLorentzAngleToolCfg(flags)))

    acc.setPrivateTools(ClusteringToolType(name, **kwargs))
    return acc

def ActsAnalogueClusteringToolCfg(flags,
                                  name: str='ActsAnalogueClusteringTool',
                                  **kwargs) :
    return ActsAnalogueClusteringToolBaseCfg(flags,
                                             name,
                                             CompFactory.ActsTrk.ITkAnalogueClusteringTool,
                                             **kwargs)

def ActsNNClusteringToolCfg(flags,
                            name: str='ActsNNClusteringTool',
                            **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    from InDetConfig.SiClusterizationToolConfig import OnnxNNCondAlgCfg
    acc.merge(OnnxNNCondAlgCfg(flags,
                               NumberNetworkPath=flags.Acts.PixelNNCalibrationModelsFolder+'number.onnx',
                               PositionNetwork1Path=flags.Acts.PixelNNCalibrationModelsFolder+'pos1.onnx',
                               PositionNetwork2Path=flags.Acts.PixelNNCalibrationModelsFolder+'pos2.onnx',
                               PositionNetwork3Path=flags.Acts.PixelNNCalibrationModelsFolder+'pos3.onnx'))
    kwargs.setdefault("minClusterChargeForNN", 15000.0)
    tools = acc.popToolsAndMerge(ActsAnalogueClusteringToolBaseCfg(flags,
                                                                   name,
                                                                   CompFactory.ActsTrk.ITkNNClusterCalibratorTool,
                                                                   **kwargs))
    acc.setPrivateTools(tools)
    return acc

def ActsTruthClusterSplittingCalibrationToolCfg(flags, name='ActsTruthClusterSplittingCalibrationTool', **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("CalibrateAfterMeasurementSelection", not flags.Acts.PixelCalibrationStrategy.calibrateBeforeSelection())

    if 'PixelLorentzAngleTool' not in kwargs:
        from SiLorentzAngleTool.ITkPixelLorentzAngleConfig import ITkPixelLorentzAngleToolCfg
        kwargs.setdefault("PixelLorentzAngleTool", acc.popToolsAndMerge(ITkPixelLorentzAngleToolCfg(flags)))

    if 'TruthSelectionTool' not in kwargs:
        from InDetPhysValMonitoring.InDetPhysValMonitoringConfig import InDetRttTruthSelectionToolCfg
        kwargs.setdefault("TruthSelectionTool", acc.popToolsAndMerge(InDetRttTruthSelectionToolCfg(flags)))

    acc.setPrivateTools(CompFactory.ActsTrk.ITkTruthClusterSplittingTool(name, **kwargs))
    return acc

def ActsPixelCalibrationToolCfg(flags,
                                **kwargs) -> ComponentAccumulator :
    from ActsConfig.ActsConfigFlags import PixelCalibrationStrategy
    if flags.Acts.PixelCalibrationStrategy is PixelCalibrationStrategy.AnalogueClusteringBeforeSelection :
        kwargs.setdefault("CalibrateAfterMeasurementSelection", False)
        return ActsAnalogueClusteringToolCfg(flags,**kwargs)
    elif flags.Acts.PixelCalibrationStrategy is PixelCalibrationStrategy.AnalogueClustering :
        kwargs.setdefault("CalibrateAfterMeasurementSelection", True)
        return ActsAnalogueClusteringToolCfg(flags,**kwargs)
    elif flags.Acts.PixelCalibrationStrategy is PixelCalibrationStrategy.NNClusteringBeforeSelection :
        kwargs.setdefault("CalibrateAfterMeasurementSelection", False)
        return ActsNNClusteringToolCfg(flags,**kwargs)
    elif flags.Acts.PixelCalibrationStrategy is PixelCalibrationStrategy.NNClustering :
        kwargs.setdefault("CalibrateAfterMeasurementSelection", True)
        return ActsNNClusteringToolCfg(flags,**kwargs)
    elif flags.Acts.PixelCalibrationStrategy is PixelCalibrationStrategy.TruthClusterSplitting :
        kwargs.setdefault("CalibrateAfterMeasurementSelection", True)
        return ActsTruthClusterSplittingCalibrationToolCfg(flags,**kwargs)
    else :
        raise RuntimeError(f"No pixel calibration tool for calibration strategy {flags.Acts.PixelCalibrationStrategy}")

def ActsStripCalibrationToolCfg(flags,
                                  name: str='ActsStripCalibrationTool',
                                  **kwargs) -> ComponentAccumulator:

    if not flags.Detector.GeometryITk:
        raise Exception("Acts Strip calibration only supports ITk!")

    acc = ComponentAccumulator()

    from ActsConfig.ActsConfigFlags import StripClusteringErrorMode,StripErrorStrategy

    kwargs.setdefault("PerformCovarianceCalibration", True)
    kwargs.setdefault("CalibrateAfterMeasurementSelection", not flags.Acts.StripCalibrationStrategy.calibrateBeforeSelection())
    kwargs.setdefault("errorStrategy", StripErrorStrategy.PITCH.value if flags.Acts.Clusters.StripClusteringErrorMode == StripClusteringErrorMode.WIDTH
                      else StripErrorStrategy.CLUSTERING.value)

    acc.setPrivateTools(CompFactory.ActsTrk.ITkStripCalibrationTool(name, **kwargs))
    return acc
