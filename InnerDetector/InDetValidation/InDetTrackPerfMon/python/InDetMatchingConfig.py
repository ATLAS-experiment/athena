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
        
from InDetTrackPerfMon.ConfigUtils import get_flags
from InDetTrackPerfMon.ConfigUtils import hasFlag
from InDetTrackPerfMon.ConfigUtils import has_in
from InDetTrackPerfMon.ConfigUtils import kwargs_setdefault
from InDetTrackPerfMon.ConfigUtils import get_opt
from InDetTrackPerfMon.ConfigUtils import sanitise


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

    iflags, iname = get_flags( flags, name )
    
    kwargs_setdefault( kwargs, "dRmax", iflags, "dRmax", 0.005 )

    acc.setPrivateTools( CompFactory.IDTPM.StableDeltaRMatchingTool_trk( iname, **kwargs ) )
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


    
def TrackMatchingToolCfg( flags, name="MatchingTool", **kwargs ):
    '''
    CA-based configuration for the test-reference matching Tool 
    '''
    log = logging.getLogger( "TrackMatchingToolCfg" )

    iflags, iname = get_flags( flags, name )

    # all these if statemenst are NOT the way to set this up ... 
    # actually all these matchers shouldn't even be tools, they
    # have only singl;e figure number of parameters, and some
    # even only have one so these could have stayed a simple
    # classes, which would have been cleaner, and more efficient 
    
    # Stable SeltaR matching
    if get_opt( iflags, "MatchingType", "" ) == "StableDeltaRMatch":
        log.debug( "Stable deltaR matching configuration chosen." )

        ## Track->Truth via stable DeltaR
        if has_in( "Truth", iflags, "RefType") :
            return StableDeltaRMatchingTool_trkTruthCfg( iflags, name = "StableDeltaRMatchingTool_trkTruth" + iname, **kwargs )

        ## Truth->Track via stable DeltaR
        if has_in( "Truth", iflags, "TestType" ):
            return StableDeltaRMatchingTool_truthTrkCfg( iflags, name = "StableDeltaRMatchingTool_truthTrk" + iname, **kwargs )

        ## Track->Track via stable DeltaR
        return StableDeltaRMatchingTool_trkCfg(iflags, name="StableDeltaRMatchingTool_trk" + iname, **kwargs )

    ## DeltaR matching
    if get_opt( iflags, "MatchingType", "" ) == "DeltaRMatch":

        ## Track->Truth via DeltaR
        if has_in( "Truth", iflags, "RefType" ) :
            return DeltaRMatchingTool_trkTruthCfg( iflags, name = "DeltaRMatchingTool_trkTruth" + iname, **kwargs )

        ## Truth->Track via DeltaR
        if has_in( "Truth", iflags, "TestType" ):
            return DeltaRMatchingTool_truthTrkCfg( iflags, name = "DeltaRMatchingTool_truthTrk" + iname, **kwargs )

        ## Track->Track via DeltaR
        return DeltaRMatchingTool_trkCfg( iflags, name="DeltaRMatchingTool_trk" + iname, **kwargs )

    ## Matching via truthParticleLink decorations
    if get_opt( iflags, "MatchingType", "" ) == "TruthMatch":

        ## Track->Truth via truthParticleLink decorations
        if has_in( "Truth", iflags, "RefType" ) :
            return TrackTruthMatchingToolCfg( iflags, name="TrackTruthMatchingTool" + iname, **kwargs )

        ## Truth->Track via truthParticleLink decorations
        if has_in( "Truth", iflags, "TestType" ) :
            return TruthTrackMatchingToolCfg( iflags, name="TruthTrackMatchingTool" + iname, **kwargs )

        log.warning( "TruthMatch via decorations not configurable if Test or Ref isn't Truth" )
        log.warning( "Matching will not be executed for TrkAnalysis %s",
                     iname )
        return None

    ## Matching track to track via truthParticleLink decorations
    if get_opt( iflags, "MatchingType", "" ) == "EFTruthMatch":
        if not get_opt( iflags, "Input.isMC", False ):
            log.error( "Matching EFTruthMatch not available for non-MC samples" )
            return None

        if ( has_in("Trigger", iflags, "TestType") and
             has_in("Offline", iflags, "RefType" ) ):
            return EFTrackMatchingToolCfg( iflags, name="EFTrackMatchingTool" + iname, **kwargs )

        # but we don'e EVER want to use "Decorations" with the trigger, we just don't
        log.warning( "EFTruthMatch via decorations configurable only with Trigger as Test Offline as Ref" )
        log.warning( "Matching will not be executed for TrkAnalysis %s", iname )
        return None

    log.warning( "Requested not supported matching type: %s", get_opt( iflags, "MatchingType", "Nope" ) )
    log.warning( "Matching will not be executed for TrkAnalysis %s", iname )
    return None
