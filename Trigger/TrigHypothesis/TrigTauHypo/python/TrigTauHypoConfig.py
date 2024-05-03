# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def tauCaloRoiUpdaterCfg(flags, inputRoIs, clusters):
    acc = ComponentAccumulator()
    newflags = flags.Trigger.InDetTracking.tauCore
    alg                               = CompFactory.TrigTauCaloRoiUpdater("TauCaloRoiUpdater",
                                        etaHalfWidth                  = newflags.etaHalfWidth,
                                        phiHalfWidth                  = newflags.phiHalfWidth,
                                        z0HalfWidth                   = newflags.zedHalfWidth,
                                        RoIInputKey                   = inputRoIs,
                                        RoIOutputKey                  = 'UpdatedCaloRoI',
                                        CaloClustersKey               = clusters)
    acc.addEventAlgo(alg)
    return acc


def tauTrackRoiUpdaterCfg(flags, inputRoIs, tracks):
    acc = ComponentAccumulator()
    newflags = flags.Trigger.InDetTracking.tauIso
    alg                               = CompFactory.TrigTauTrackRoiUpdater("TrackRoiUpdater",
                                        etaHalfWidth                  = newflags.etaHalfWidth,
                                        phiHalfWidth                  = newflags.phiHalfWidth,
                                        z0HalfWidth                   = newflags.zedHalfWidth,
                                        RoIInputKey                   = inputRoIs,
                                        RoIOutputKey                  = "UpdatedTrackRoI",
                                        fastTracksKey                 = tracks,
                                        Key_trigTauJetInputContainer  = "" )
    acc.addEventAlgo(alg)
    return acc


def tauLRTRoiUpdaterCfg(flags, inputRoIs, tracks):
    acc                               = ComponentAccumulator()
    newflags = flags.Tracking.ActiveConfig
    alg                               = CompFactory.TrigTauTrackRoiUpdater("TrackRoiUpdaterLRT",
                                        etaHalfWidth                  = newflags.etaHalfWidth,
                                        phiHalfWidth                  = newflags.phiHalfWidth,
                                        z0HalfWidth                   = newflags.zedHalfWidth,
                                        RoIInputKey                   = inputRoIs,
                                        RoIOutputKey                  = "UpdatedTrackLRTRoI",
                                        fastTracksKey                 = tracks,
                                        Key_trigTauJetInputContainer  = "" )
    acc.addEventAlgo(alg)
    return acc
