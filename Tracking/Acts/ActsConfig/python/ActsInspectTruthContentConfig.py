# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def isdet(flags,
          *,
          pixel: list = None,
          strip: list = None,
          hgtd: list = None) -> list:
    keys = []
    if flags.Detector.EnableITkPixel and pixel is not None:
        keys += pixel
    if flags.Detector.EnableITkStrip and strip is not None:
        keys += strip
    if flags.Detector.EnableHGTD and flags.Acts.useHGTDClusterInTrackFinding and hgtd is not None:
        keys += hgtd
    return keys


def ActsInspectTruthContentAlgCfg(flags,
                                  name: str = "ActsInspectTruthContentAlg",
                                  **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault('Clusters', isdet(flags,
                                        pixel=['ITkPixelClusters'],
                                        strip=['ITkStripClusters']))
    kwargs.setdefault('TruthAssociationMaps', isdet(flags,
                                                    pixel=['ITkPixelClustersToTruthParticles'],
                                                    strip=['ITkStripClustersToTruthParticles']))
    
    kwargs.setdefault('Seeds', ['ActsFastPixelSeeds'] if flags.Tracking.doITkFastTracking else ['ActsPixelSeeds', 'ActsStripSeeds'])
    kwargs.setdefault('Tracks', ['ActsTracks'] if not flags.Acts.doAmbiguityResolution else ['ActsResolvedTracks'])

    acc.addEventAlgo(CompFactory.ActsTrk.ActsInspectTruthContentAlg(name, **kwargs))
    return acc

