# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def tauCaloRoiUpdaterCfg(flags, inputRoIs, clusters):
    acc = ComponentAccumulator()
    alg = CompFactory.TrigTauCaloRoiUpdater(name='TauCaloRoiUpdater',
                                            RoIInputKey=inputRoIs,
                                            RoIOutputKey='UpdatedCaloRoI',
                                            CaloClustersKey=clusters)
    acc.addEventAlgo(alg)
    return acc


def tauTrackRoiUpdaterCfg(flags, inputRoIs, tracks):
    acc = ComponentAccumulator()
    roi_flags = flags.Trigger.InDetTracking.tauIso
    alg = CompFactory.TrigTauTrackRoiUpdater(name='TrackRoiUpdater',
                                             etaHalfWidth=roi_flags.etaHalfWidth,
                                             phiHalfWidth=roi_flags.phiHalfWidth,
                                             z0HalfWidth=roi_flags.zedHalfWidth,
                                             RoIInputKey=inputRoIs,
                                             RoIOutputKey='UpdatedTrackRoI',
                                             TracksKey=tracks)
    acc.addEventAlgo(alg)
    return acc


def tauLRTRoiUpdaterCfg(flags, inputRoIs, tracks):
    acc = ComponentAccumulator()
    roi_flags = flags.Tracking.ActiveConfig
    alg = CompFactory.TrigTauTrackRoiUpdater(name='TrackRoiUpdaterLRT',
                                             etaHalfWidth=roi_flags.etaHalfWidth,
                                             phiHalfWidth=roi_flags.phiHalfWidth,
                                             z0HalfWidth=roi_flags.zedHalfWidth,
                                             RoIInputKey=inputRoIs,
                                             RoIOutputKey='UpdatedTrackLRTRoI',
                                             TracksKey=tracks)
    acc.addEventAlgo(alg)
    return acc
