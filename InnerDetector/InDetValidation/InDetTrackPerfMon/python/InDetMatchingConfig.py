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


def DeltaRMatchingTool_trkTruthCfg( flags, name="DeltaRMatchingTool_trkTruth", **kwargs ):
    '''
    Tool for Track->Truth matching via DeltaR (and/or pT resolution)
    '''
    acc = ComponentAccumulator()

    kwargs.setdefault( "dRmax",    flags.PhysVal.IDTPM.currentTrkAna.dRmax    )
    kwargs.setdefault( "pTResMax", flags.PhysVal.IDTPM.currentTrkAna.pTResMax )

    acc.setPrivateTools(
        CompFactory.IDTPM.DeltaRMatchingTool_trkTruth( name, **kwargs ) )
    return acc


def DeltaRMatchingTool_truthTrkCfg( flags, name="DeltaRMatchingTool_truthTrk", **kwargs ):
    '''
    Tool for Truth->Track matching via DeltaR (and/or pT resolution)
    '''
    acc = ComponentAccumulator()

    kwargs.setdefault( "dRmax",    flags.PhysVal.IDTPM.currentTrkAna.dRmax    )
    kwargs.setdefault( "pTResMax", flags.PhysVal.IDTPM.currentTrkAna.pTResMax )

    acc.setPrivateTools(
        CompFactory.IDTPM.DeltaRMatchingTool_truthTrk( name, **kwargs ) )
    return acc


def DeltaRMatchingTool_trkCfg( flags, name="DeltaRMatchingTool_trk", **kwargs ):
    '''
    Tool for Track->Track matching via DeltaR (and/or pT resolution)
    '''
    acc = ComponentAccumulator()

    kwargs.setdefault( "dRmax",    flags.PhysVal.IDTPM.currentTrkAna.dRmax    )
    kwargs.setdefault( "pTResMax", flags.PhysVal.IDTPM.currentTrkAna.pTResMax )

    acc.setPrivateTools(
        CompFactory.IDTPM.DeltaRMatchingTool_trk( name, **kwargs ) )
    return acc


def TrackTruthMatchingToolCfg( flags, name="TrackTruthMatchingTool", **kwargs ):
    '''
    Tool for Track->Truth matching via 'truthParticleLink' decorations
    '''
    acc = ComponentAccumulator()

    kwargs.setdefault( "MatchingTruthProb", flags.PhysVal.IDTPM.currentTrkAna.truthProbCut )

    acc.setPrivateTools(
        CompFactory.IDTPM.TrackTruthMatchingTool( name, **kwargs ) )
    return acc


def TruthTrackMatchingToolCfg( flags, name="TruthTrackMatchingTool", **kwargs ):
    '''
    Tool for Truth->Track matching via 'truthParticleLink' decorations
    '''
    acc = ComponentAccumulator()

    kwargs.setdefault( "MatchingTruthProb", flags.PhysVal.IDTPM.currentTrkAna.truthProbCut )

    acc.setPrivateTools(
        CompFactory.IDTPM.TruthTrackMatchingTool( name, **kwargs ) )
    return acc


def TrackMatchingToolCfg( flags, **kwargs ):
    '''
    CA-based configuration for the test-reference matching Tool 
    '''
    log = logging.getLogger( "TrackMatchingToolCfg" )

    ## DeltaR matching
    if flags.PhysVal.IDTPM.currentTrkAna.MatchingType == "DeltaRMatch":

        ## Track->Truth via DeltaR
        if "Truth" in flags.PhysVal.IDTPM.currentTrkAna.RefType :
            return DeltaRMatchingTool_trkTruthCfg(
                flags, name = "DeltaRMatchingTool_trkTruth" +
                    flags.PhysVal.IDTPM.currentTrkAna.anaTag, **kwargs )

        ## Truth->Track via DeltaR
        if "Truth" in flags.PhysVal.IDTPM.currentTrkAna.TestType :
            return DeltaRMatchingTool_truthTrkCfg(
                flags, name = "DeltaRMatchingTool_truthTrk" +
                    flags.PhysVal.IDTPM.currentTrkAna.anaTag, **kwargs )

        ## Track->Track via DeltaR
        return DeltaRMatchingTool_trkCfg(
            flags, name="DeltaRMatchingTool_trk" +
                flags.PhysVal.IDTPM.currentTrkAna.anaTag, **kwargs )

    ## Matching via truthParticleLink decorations
    if flags.PhysVal.IDTPM.currentTrkAna.MatchingType == "TruthMatch":

        ## Track->Truth via truthParticleLink decorations
        if "Truth" in flags.PhysVal.IDTPM.currentTrkAna.RefType :
            return TrackTruthMatchingToolCfg(
                flags, name="TrackTruthMatchingTool" +
                    flags.PhysVal.IDTPM.currentTrkAna.anaTag, **kwargs )

        ## Truth->Track via truthParticleLink decorations
        if "Truth" in flags.PhysVal.IDTPM.currentTrkAna.TestType :
            return TruthTrackMatchingToolCfg(
                flags, name="TruthTrackMatchingTool" +
                    flags.PhysVal.IDTPM.currentTrkAna.anaTag, **kwargs )

        log.warning( "TruthMatch via decorations not configurable if Test or Ref isn't Truth" )
        log.warning( "Matching will not be executed for TrkAnalysis %s",
                     flags.PhysVal.IDTPM.currentTrkAna.anaTag )
        return None

    log.warning( "Requested not supported matching type: %s",
                 flags.PhysVal.IDTPM.currentTrkAna.MatchingType )
    log.warning( "Matching will not be executed for TrkAnalysis %s",
                 flags.PhysVal.IDTPM.currentTrkAna.anaTag )
    return None
