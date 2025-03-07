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

    kwargs.setdefault("DetEleCollKey", "ITkPixelDetectorElementCollection")
    kwargs.setdefault("PixelOfflineCalibData", "ITkPixelOfflineCalibData")
    kwargs.setdefault("PerformCovarianceCalibration", flags.Acts.OnTrackCalibration.performCovarianceCalibration)
    
    if 'PixelLorentzAngleTool' not in kwargs:
        from SiLorentzAngleTool.ITkPixelLorentzAngleConfig import ITkPixelLorentzAngleToolCfg
        kwargs.setdefault("PixelLorentzAngleTool", acc.popToolsAndMerge(ITkPixelLorentzAngleToolCfg(flags)))

    acc.setPrivateTools(CompFactory.ActsTrk.ITkAnalogueClusteringTool(name, **kwargs))
    return acc
