# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ActsAnalogueClusteringToolCfg(flags,
                                  name: str='ActsAnalogueClusteringTool',
                                  **kwargs) -> ComponentAccumulator:

    if not flags.Detector.GeometryITk:
        raise Exception("Acts Analogue Clustering calibration only supports ITk!")

    from PixelConditionsAlgorithms.ITkPixelConditionsConfig import ITkPixelOfflineCalibCondAlgCfg
    from SiLorentzAngleTool.ITkPixelLorentzAngleConfig import ITkPixelLorentzAngleToolCfg

    acc = ComponentAccumulator()

    acc.merge(ITkPixelOfflineCalibCondAlgCfg(flags))

    if 'DetEleCollKey' not in kwargs:
        kwargs.setdefault("DetEleCollKey", "ITkPixelDetectorElementCollection")

    if 'PixelOfflineCalibData' not in kwargs:
        kwargs.setdefault("PixelOfflineCalibData", "ITkPixelOfflineCalibData")

    if 'PixelLorentzAngleTool' not in kwargs:
        kwargs.setdefault("PixelLorentzAngleTool", acc.popToolsAndMerge(ITkPixelLorentzAngleToolCfg(flags)))

    if 'CalibrateErrors' not in kwargs:
        kwargs.setdefault("CalibrateErrors", not flags.Tracking.useBroadPixClusterErrors)

    acc.setPrivateTools(CompFactory.ActsTrk.ITkAnalogueClusteringTool(name, **kwargs))

    return acc
