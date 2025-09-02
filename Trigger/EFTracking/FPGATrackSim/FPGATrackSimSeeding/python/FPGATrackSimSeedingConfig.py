# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def FPGATrackSimSeedingCfg(flags, name='FPGATrackSimSeedingAlg', **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault('FPGATrackSimTrackKey', "FPGATracks")
    kwargs.setdefault('FPGAPixelClustersKey', "ITkPixelClusters")
    kwargs.setdefault('FPGASpacePointsKey', "ITkPixelSpacePoints")
    kwargs.setdefault('OutputSeeds', "ActsValidateF150PixelSeeds")
    kwargs.setdefault('MinSpacePointsPerSeed', 4)
    kwargs.setdefault('MaxSpacePointsPerSeed', 5)
    
    acc.addEventAlgo(CompFactory.FPGATrackSim.FPGATrackSimSeedingAlg(name, **kwargs))
    return acc