# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def FPGATrackSimSeedingCfg(flags, name='FPGATrackSimSeedingAlg', **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault('FPGATrackSimTrackKey', "FPGATracks")
    kwargs.setdefault('FPGAPixelClustersKey', "ITkPixelClusters")
    kwargs.setdefault('FPGASpacePointsKey', "ITkPixelSpacePoints")
    if flags.Trigger.FPGATrackSim.runF150hw:
        kwargs.setdefault('OutputSeeds', "ActsValidateF150SWPixelSeeds")
    else:
        kwargs.setdefault('OutputSeeds', "ActsValidateF150PixelSeeds")

    kwargs.setdefault('MinSpacePointsPerSeed', flags.Trigger.FPGATrackSim.MinSpacePointsPerSeed)
    kwargs.setdefault('MaxSpacePointsPerSeed', flags.Trigger.FPGATrackSim.MaxSpacePointsPerSeed)
    
    acc.addEventAlgo(CompFactory.FPGATrackSim.FPGATrackSimSeedingAlg(name, **kwargs))
    return acc
