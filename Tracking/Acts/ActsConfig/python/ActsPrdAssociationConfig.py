# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ActsPrdAssociationAlgCfg(flags,
                             name: str = "ActsPrdAssociationAlg",
                             *,
                             previousActsExtension: str = None,
                             **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    acts_tracks = f"{flags.Tracking.ActiveConfig.extension}Tracks" if not flags.Acts.doAmbiguityResolution else f"{flags.Tracking.ActiveConfig.extension}ResolvedTracks"
    kwargs.setdefault('InputTrackCollection', acts_tracks)
    kwargs.setdefault('OutputPrdMap', f'{flags.Tracking.ActiveConfig.extension}PrdMap')
    if previousActsExtension is not None:
        kwargs.setdefault('InputPrdMap', f'{previousActsExtension}PrdMap')
    acc.addEventAlgo(CompFactory.ActsTrk.PrdAssociationAlg(name, **kwargs))
    return acc
