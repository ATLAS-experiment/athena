#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#


from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
# from AthenaCommon.Logging import logging
        
def TrackViewMatchingToolCfg( flags, name="TrackViewMatchingTool", **kwargs ):
    '''
    TrackView-based matching tool (TMatchingTool<const TrackView>).
    Gaudi properties: MatcherType, DeltaR, DeltaEta, DeltaPhi.
    Valid MatcherType values: DeltaR, DeltaPhi, DeltaRZ, DeltaRZSinTheta,
                              Sigma, SecondBestpT, Default.
    '''
    acc = ComponentAccumulator()

    for key, value in flags.node_flags():
        kwargs.setdefault( key, value )

    acc.setPrivateTools( CompFactory.IDTPM2.TrackViewMatchingTool( name, **kwargs ) )
    return acc



