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
    
    kwargs.setdefault('UseWeightedPosition', flags.Acts.Clusters.UseWeightedPosition)
    kwargs.setdefault("PerformCovarianceCalibration", flags.Acts.OnTrackCalibration.performCovarianceCalibration)
    kwargs.setdefault("DetEleCollKey", "ITkPixelDetectorElementCollection")
    kwargs.setdefault("PixelOfflineCalibData", "ITkPixelOfflineCalibData")
    kwargs.setdefault("errorStrategy", PixelErrorStrategy.PITCH.value if flags.Acts.Clusters.UsePixelBroadErrors
                      else PixelErrorStrategy.CALIBRATED.value)

    # For default configuration we set a lower cap on the calibrated covariance
    # For FT we have inflated chi2 instead
    # This applies to all tracking passes, main and secondaries alike
    if not flags.Tracking.doITkFastTracking:
        kwargs.setdefault("CalibratedCovarianceLowerBound", 0.75)

    if 'PixelLorentzAngleTool' not in kwargs:
        from SiLorentzAngleTool.ITkPixelLorentzAngleConfig import ITkPixelLorentzAngleToolCfg
        kwargs.setdefault("PixelLorentzAngleTool", acc.popToolsAndMerge(ITkPixelLorentzAngleToolCfg(flags)))

    acc.setPrivateTools(CompFactory.ActsTrk.ITkAnalogueClusteringTool(name, **kwargs))
    return acc

def ActsStripCalibrationToolCfg(flags,
                                  name: str='ActsStripCalibrationTool',
                                  **kwargs) -> ComponentAccumulator:

    if not flags.Detector.GeometryITk:
        raise Exception("Acts Strip calibration only supports ITk!")
    
    acc = ComponentAccumulator()

    from ActsConfig.ActsConfigFlags import StripClusteringErrorMode,StripErrorStrategy
    
    
    kwargs.setdefault("DetEleCollKey", "ITkStripDetectorElementCollection")
    kwargs.setdefault("PerformCovarianceCalibration", True)
    kwargs.setdefault("errorStrategy", StripErrorStrategy.PITCH.value if flags.Acts.Clusters.StripClusteringErrorMode == StripClusteringErrorMode.WIDTH
                      else StripErrorStrategy.CLUSTERING.value)
    
    acc.setPrivateTools(CompFactory.ActsTrk.ITkStripCalibrationTool(name, **kwargs))
    return acc
