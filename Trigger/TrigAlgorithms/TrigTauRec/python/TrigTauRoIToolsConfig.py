# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

from TrigInDetConfig.utils import getFlagsForActiveConfig

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)


def tauCaloRoiUpdaterCfg(flags: AthConfigFlags, inputRoIs: str, clusters: str) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    alg = CompFactory.TrigTauCaloRoiUpdater(name='TauCaloRoiUpdater',
                                            RoIInputKey=inputRoIs,
                                            RoIOutputKey='UpdatedCaloRoI',
                                            CaloClustersKey=clusters)
    acc.addEventAlgo(alg)
    return acc


def tauHitZRoiUpdaterCfg(
    flags: AthConfigFlags,
    inputRoIs: str,
    outputRoIs: str,
    taus: str,
    hitz_alg: str,
    max_pt: float,
    max_sigma: float,
    tracking_cfg: str | None = None,
) -> ComponentAccumulator:
    if tracking_cfg:
        flags = getFlagsForActiveConfig(flags, tracking_cfg, log)

    acc = ComponentAccumulator()
    from TriggerMenuMT.HLT.Tau.TauConfigurationTools import getHitZVariables
    alg = CompFactory.TrigTauHitZRoiUpdater(name=f'HitZRoiUpdater_{inputRoIs}',
                                             etaHalfWidth=flags.Tracking.ActiveConfig.etaHalfWidth,
                                             phiHalfWidth=flags.Tracking.ActiveConfig.phiHalfWidth,
                                             z0HalfWidth=flags.Tracking.ActiveConfig.zedHalfWidth,
                                             RoIInputKey=inputRoIs,
                                             RoIOutputKey=outputRoIs,
                                             TauKey=taus,
                                             zDecorKey=getHitZVariables(hitz_alg)[0],
                                             sigmaDecorKey=getHitZVariables(hitz_alg)[1],
                                             maxPt=max_pt,
                                             maxSigma=max_sigma)
    acc.addEventAlgo(alg)
    return acc


def tauTrackRoiUpdaterCfg(flags: AthConfigFlags, inputRoIs: str, outputRoIs: str, tracks: str, tracking_cfg: str | None = None) -> ComponentAccumulator:
    if tracking_cfg:
        flags = getFlagsForActiveConfig(flags, tracking_cfg, log)

    acc = ComponentAccumulator()
    acc.addEventAlgo(CompFactory.TrigTauTrackRoiUpdater(
        name=f'TrackRoiUpdater_{inputRoIs}',
        etaHalfWidth=flags.Tracking.ActiveConfig.etaHalfWidth,
        phiHalfWidth=flags.Tracking.ActiveConfig.phiHalfWidth,
        z0HalfWidth=flags.Tracking.ActiveConfig.zedHalfWidth,
        RoIInputKey=inputRoIs,
        RoIOutputKey=outputRoIs,
        TracksKey=tracks
    ))
    return acc
