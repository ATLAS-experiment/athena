# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of TRT_TrackExtensionAlg package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def TRT_TrackExtensionAlgCfg(flags, name = 'InDetTRT_Extension', **kwargs):
    acc = ComponentAccumulator()
    doTrackOverlay = flags.TrackOverlay.isTrackOverlaySeq
    prefix = "TrackOverlay_" if doTrackOverlay else ''
    if "TrackExtensionTool" not in kwargs:
        from InDetConfig.TRT_TrackExtensionToolConfig import TRT_TrackExtensionToolCfg
        kwargs.setdefault("TrackExtensionTool", acc.popToolsAndMerge(
            TRT_TrackExtensionToolCfg(flags)))

    acc.addEventAlgo(CompFactory.InDet.TRT_TrackExtensionAlg(
        prefix + name + flags.Tracking.ActiveConfig.extension, **kwargs))
    return acc

def TRT_Phase_TrackExtensionAlgCfg(flags, name = 'InDetTRT_Phase_Extension', **kwargs):
    acc = ComponentAccumulator()

    if "TrackExtensionTool" not in kwargs:
        from InDetConfig.TRT_TrackExtensionToolConfig import TRT_TrackExtensionToolPhaseCfg
        kwargs.setdefault("TrackExtensionTool", acc.popToolsAndMerge(
            TRT_TrackExtensionToolPhaseCfg(flags)))

    acc.addEventAlgo(CompFactory.InDet.TRT_TrackExtensionAlg(name, **kwargs))
    return acc

def Trig_TRT_TrackExtensionAlgCfg(flags, inputTracks, name = 'TrigTrackExtensionAlg', **kwargs):
    acc = ComponentAccumulator()

    if "TrackExtensionTool" not in kwargs:
        from InDetConfig.TRT_TrackExtensionToolConfig import Trig_TRT_TrackExtensionToolCfg
        kwargs.setdefault("TrackExtensionTool", acc.popToolsAndMerge(
            Trig_TRT_TrackExtensionToolCfg(flags)))

    kwargs["InputTracksLocation"] = inputTracks
    kwargs.setdefault("ExtendedTracksLocation", "ExtendedTrackMap")

    acc.addEventAlgo(CompFactory.InDet.TRT_TrackExtensionAlg(f"{name}_{flags.Tracking.ActiveConfig.input_name}", **kwargs))
    return acc
