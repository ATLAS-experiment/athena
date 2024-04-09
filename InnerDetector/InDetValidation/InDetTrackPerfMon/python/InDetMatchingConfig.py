#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

'''@file InDetMatchingConfig.py
@author M. Aparo
@date 28-03-2024
@brief CA-based python configurations for matching tools in this package
'''

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Logging import logging


## TODO - to be included in later MRs
#def DeltaRtrackMatchingTool_trkTruthCfg(
#        flags, name="DeltaRtrackMatchingTool_trkTruth", **kwargs ):
#    acc = ComponentAccumulator()
#
#    kwargs.setdefault( "dRmax",    flags.PhysVal.IDTPM.currentTrkAna.dRmax    )
#    kwargs.setdefault( "pTResMax", flags.PhysVal.IDTPM.currentTrkAna.pTResMax )
#
#    acc.setPrivateTools(
#        CompFactory.IDTPM.DeltaRtrackMatchingTool_trkTruth(name, **kwargs))
#    return acc


## TODO - to be included in later MRs
#def DeltaRtrackMatchingTool_trkCfg(
#        flags, name="DeltaRtrackMatchingTool_trk", **kwargs ):
#    acc = ComponentAccumulator()
#
#    kwargs.setdefault("dRmax",    flags.PhysVal.IDTPM.currentTrkAna.dRmax)
#    kwargs.setdefault("pTResMax", flags.PhysVal.IDTPM.currentTrkAna.pTResMax)
#
#    acc.setPrivateTools(
#        CompFactory.IDTPM.DeltaRtrackMatchingTool_trk(name, **kwargs))
#    return acc


def TrackTruthMatchingToolCfg(
        flags, name="TrackTruthMatchingTool", **kwargs ):
    '''
    Tool for Track->Truth matching via 'truthParticleLink' decorations
    '''
    acc = ComponentAccumulator()

    kwargs.setdefault( "MatchingTruthProb", flags.PhysVal.IDTPM.currentTrkAna.truthProbCut )

    acc.setPrivateTools(
        CompFactory.IDTPM.TrackTruthMatchingTool( name, **kwargs ) )
    return acc


def TrackMatchingToolCfg( flags, **kwargs ):
    '''
    CA-based configuration for the test-reference matching Tool 
    '''
    log = logging.getLogger( "TrackMatchingToolCfg" )

## TODO - to be included in later MRs
#    if flags.PhysVal.IDTPM.currentTrkAna.MatchingType == "DeltaRMatch":
# 
#        if "Truth" in flags.PhysVal.IDTPM.currentTrkAna.RefType :
#            return DeltaRtrackMatchingTool_trkTruthCfg(
#                flags, name = "DeltaRtrackMatchingTool_trkTruth" + 
#                    flags.PhysVal.IDTPM.currentTrkAna.anaTag, **kwargs)
#
#        else:
#            return DeltaRtrackMatchingTool_trkCfg(
#                flags, name="DeltaRtrackMatchingTool_trk" + 
#                    flags.PhysVal.IDTPM.currentTrkAna.anaTag, **kwargs)

    if flags.PhysVal.IDTPM.currentTrkAna.MatchingType == "TruthMatch":

        ## Track->Truth via truthParticleLink decorations
        if "Truth" in flags.PhysVal.IDTPM.currentTrkAna.RefType :
            return TrackTruthMatchingToolCfg(
                flags, name="TrackTruthMatchingTool" +
                    flags.PhysVal.IDTPM.currentTrkAna.anaTag, **kwargs )

        ## TODO - to be included in later MRs
        ## Truth->Track via truthParticleLink decorations
        #if "Truth" in flags.PhysVal.IDTPM.currentTrkAna.TestType :
        #    return TruthTrackMatchingToolCfg( flags, **kwargs )

        log.warning( "TruthMatch via decorations not configurable if Test or Ref isn't Truth" )
        log.warning( "Matching will not be executed for TrkAnalysis %s",
                     flags.PhysVal.IDTPM.currentTrkAna.anaTag )
        return None

    log.warning( "Requested not supported matching type: %s",
                 flags.PhysVal.IDTPM.currentTrkAna.MatchingType )
    log.warning( "Matching will not be executed for TrkAnalysis %s",
                 flags.PhysVal.IDTPM.currentTrkAna.anaTag )
    return None
