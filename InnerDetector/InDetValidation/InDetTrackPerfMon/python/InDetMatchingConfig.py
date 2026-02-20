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


def StableDeltaRMatchingTool_trkTruthCfg( flags, name="StableDeltaRMatchingTool_trkTruth", **kwargs ):
    '''
    Tool for Track->Truth matching via DeltaR using the stable matching algorithm
    '''
    acc = ComponentAccumulator()

    kwargs.setdefault( "dRmax",    flags.PhysVal.IDTPM.currentTrkAna.dRmax    )

    acc.setPrivateTools(
        CompFactory.IDTPM.StableDeltaRMatchingTool_trkTruth( name, **kwargs ) )
    return acc


def StableDeltaRMatchingTool_truthTrkCfg( flags, name="StableDeltaRMatchingTool_truthTrk", **kwargs ):
    '''
    Tool for Truth->Track matching via DeltaR using the stable matching algorithm
    '''
    acc = ComponentAccumulator()

    kwargs.setdefault( "dRmax",    flags.PhysVal.IDTPM.currentTrkAna.dRmax    )

    acc.setPrivateTools(
        CompFactory.IDTPM.StableDeltaRMatchingTool_truthTrk( name, **kwargs ) )
    return acc


def StableDeltaRMatchingTool_trkCfg( flags, name="StableDeltaRMatchingTool_trk", **kwargs ):
    '''
    Tool for Track->Track matching via DeltaR using the stable matching algorithm
    '''
    acc = ComponentAccumulator()

    kwargs.setdefault( "dRmax",    flags.PhysVal.IDTPM.currentTrkAna.dRmax    )

    acc.setPrivateTools(
        CompFactory.IDTPM.StableDeltaRMatchingTool_trk( name, **kwargs ) )
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


def EFTrackMatchingToolCfg( flags, name="EFTrackMatchingTool", **kwargs ):
    '''
    Tool for Track->Truth matching via 'truthParticleLink' decorations
    '''
    acc = ComponentAccumulator()

    kwargs.setdefault( "MatchingTruthProb", flags.PhysVal.IDTPM.currentTrkAna.truthProbCut )

    acc.setPrivateTools(
        CompFactory.IDTPM.EFTrackMatchingTool( name, **kwargs ) )
    return acc


def TrackMatchingToolCfg( flags, **kwargs ):
    '''
    CA-based configuration for the test-reference matching Tool 
    '''
    log = logging.getLogger( "TrackMatchingToolCfg" )

    # Stable SeltaR matching
    if flags.PhysVal.IDTPM.currentTrkAna.MatchingType == "StableDeltaRMatch":
        log.debug( "Stable deltaR matching configuration chosen." )

        ## Track->Truth via stable DeltaR
        if "Truth" in flags.PhysVal.IDTPM.currentTrkAna.RefType :
            return StableDeltaRMatchingTool_trkTruthCfg(
                flags, name = "StableDeltaRMatchingTool_trkTruth" +
                flags.PhysVal.IDTPM.currentTrkAna.anaTag, **kwargs )

        ## Truth->Track via stable DeltaR
        if "Truth" in flags.PhysVal.IDTPM.currentTrkAna.TestType :
            return StableDeltaRMatchingTool_truthTrkCfg(
                flags, name = "StableDeltaRMatchingTool_truthTrk" +
                flags.PhysVal.IDTPM.currentTrkAna.anaTag, **kwargs )

        ## Track->Track via stable DeltaR
        return StableDeltaRMatchingTool_trkCfg(
            flags, name="StableDeltaRMatchingTool_trk" +
            flags.PhysVal.IDTPM.currentTrkAna.anaTag, **kwargs )

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

    ## Matching track to track via truthParticleLink decorations
    if flags.PhysVal.IDTPM.currentTrkAna.MatchingType == "EFTruthMatch":
        if not flags.Input.isMC:
            log.error( "Matching EFTruthMatch not available for non-MC samples" )
            return None

        if ( "Trigger" in flags.PhysVal.IDTPM.currentTrkAna.TestType and
             "Offline" in flags.PhysVal.IDTPM.currentTrkAna.RefType ):
            return EFTrackMatchingToolCfg(
                    flags, name="EFTrackMatchingTool" +
                        flags.PhysVal.IDTPM.currentTrkAna.anaTag, **kwargs )
        log.warning( "EFTruthMatch via decorations configurable only with Trigger as Test Offline as Ref" )
        log.warning( "Matching will not be executed for TrkAnalysis %s",
                     flags.PhysVal.IDTPM.currentTrkAna.anaTag )
        return None

    log.warning( "Requested not supported matching type: %s",
                 flags.PhysVal.IDTPM.currentTrkAna.MatchingType )
    log.warning( "Matching will not be executed for TrkAnalysis %s",
                 flags.PhysVal.IDTPM.currentTrkAna.anaTag )
    return None
