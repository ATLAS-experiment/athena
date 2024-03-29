# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
# Configuration of TRT_TrackSegmentsTool_xk package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def TRT_TrackSegmentsMakerCondAlg_ATLxkCfg(
        flags, name = 'InDetTRT_SeedsMakerCondAlg', **kwargs):
    from TRT_GeoModel.TRT_GeoModelConfig import TRT_ReadoutGeometryCfg
    acc = TRT_ReadoutGeometryCfg(flags) # To produce TRT_DetElementContainer

    if "PropagatorTool" not in kwargs:
        from TrkConfig.TrkExRungeKuttaPropagatorConfig import (
            RungeKuttaPropagatorCfg)
        kwargs.setdefault("PropagatorTool", acc.popToolsAndMerge(
            RungeKuttaPropagatorCfg(flags)))

    kwargs.setdefault("NumberMomentumChannel",
                      flags.Tracking.ActiveConfig.TRTSegFinderPtBins)

    acc.addCondAlgo(
        CompFactory.InDet.TRT_TrackSegmentsMakerCondAlg_ATLxk(name, **kwargs))
    return acc

def TRT_TrackSegmentsMaker_BarrelCosmicsCfg(
        flags, name='InDetTRTSegmentsMaker', **kwargs) :
    acc = ComponentAccumulator()

    kwargs.setdefault("TrtManagerLocation", 'TRT')
    kwargs.setdefault("TRT_ClustersContainer", 'TRT_DriftCirclesUncalibrated')
    kwargs.setdefault("IsMagneticFieldOn", flags.BField.solenoidOn)

    acc.setPrivateTools(
        CompFactory.InDet.TRT_TrackSegmentsMaker_BarrelCosmics(name, **kwargs))
    return acc

def TRT_TrackSegmentsMaker_ATLxkCfg(
        flags, name = 'InDetTRT_SeedsMaker',
        InputCollections = None, **kwargs):

    acc = TRT_TrackSegmentsMakerCondAlg_ATLxkCfg(
        flags,
        name = 'InDetTRT_SeedsMakerCondAlg',
        pTmin = flags.Tracking.ActiveConfig.minSecondaryPt)

    if "TrackExtensionTool" not in kwargs:
        from InDetConfig.TRT_TrackExtensionToolConfig import (
            TRT_TrackExtensionToolCfg)
        kwargs.setdefault("TrackExtensionTool", acc.popToolsAndMerge(
            TRT_TrackExtensionToolCfg(flags)))

    kwargs.setdefault("TRT_ClustersContainer", 'TRT_DriftCircles')
    kwargs.setdefault("RemoveNoiseDriftCircles", False)
    kwargs.setdefault("NumberMomentumChannel",
                      flags.Tracking.ActiveConfig.TRTSegFinderPtBins)
    if InputCollections is not None:
        kwargs.setdefault("PRDtoTrackMap", 'InDetSegmentPRDtoTrackMap')
    kwargs.setdefault("MinNumberDriftCircles", flags.Tracking.BackTracking.minTRT)
    kwargs.setdefault("pTmin", flags.Tracking.ActiveConfig.minSecondaryPt)
    kwargs.setdefault("sharedFrac",
                      flags.Tracking.ActiveConfig.maxSecondaryTRTShared)

    acc.setPrivateTools(
        CompFactory.InDet.TRT_TrackSegmentsMaker_ATLxk(name, **kwargs))
    return acc

def TRT_TrackSegmentsMaker_ATLxk_Phase_Cfg(
        flags, name = 'InDetTRT_SeedsMaker_Phase', **kwargs):
    kwargs.setdefault("PRDtoTrackMap", "")
    return TRT_TrackSegmentsMaker_ATLxkCfg(flags, name, **kwargs)

def TRT_TrackSegmentsMaker_ATLxk_TrackSegmentsCfg(
        flags, name = 'InDetTRT_SeedsMaker_TrackSegments', **kwargs):
   kwargs.setdefault("pTmin", flags.Tracking.ActiveConfig.minPT)
   kwargs.setdefault("MinNumberDriftCircles", flags.Tracking.ActiveConfig.minPT)
   kwargs.setdefault("sharedFrac", 0.7)
   kwargs.setdefault("PRDtoTrackMap", "")
   return TRT_TrackSegmentsMaker_ATLxkCfg(flags, name, **kwargs)
