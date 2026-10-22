#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

'''
@file InDetSelectionConfig.py
@author M. Aparo
@date 02-10-2023
@brief CA-based python configurations for selection tools in this package
'''

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory



def TruthPerigeeDecoratorCfg( flags, name="InDetPhysValTruthDecoratorAlg", **kwargs ):
    '''
    create decoration algorithm which decorates
    truth particles with track parameters at the perigee.
    '''
    acc = ComponentAccumulator()

    from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
    extrapolator = acc.popToolsAndMerge( AtlasExtrapolatorCfg( flags ) )
    acc.addPublicTool( extrapolator )
    kwargs.setdefault( "Extrapolator", extrapolator )

    # what is this for ???
    #    kwargs.setdefault( 'TruthParticleIndexDecoration', 'origTruthIndex' )
    kwargs.setdefault( "TruthParticleIndexDecoration", "" )

    acc.addEventAlgo( CompFactory.InDetPhysValTruthDecoratorAlg( name, **kwargs ) )
    return acc




def TruthSelectorBaseCfg( flags, name="TruthSelectionBaseTool", **kwargs ) :
    '''
    Copy of InDetPhysValMonitoring.InDetPhysValMonitoringConfig InDetRttTruthSelectionToolCfg
    to handle flags internally in IDTPM2, i.e. not relying on IDPVM's (default) flags
    '''
    acc = ComponentAccumulator()
    
    #    iflags, iname = get_flags( flags, name )

    ## Baseline requirements to be applied to all analyses
    kwargs.setdefault( "requireStable", True )
    kwargs.setdefault( "requireCharged", True )

    # whny are these flags all set to none somewhere ??
    # this configuration is far, far, FAR too complicated,
    # setting should be set in ONE PLACE, and ONE PLACE ONLY
    # unless the user explicitly wants to change some later

    # values in case the geometry is not set, AAAAARRRGH!!!!!
    # this is all so a***d up, what about mineEta ?? minEta is
    # set elsewhere, why isn;t everything set consistently ?
    if kwargs.get("maxEta") is None:
        kwargs["maxEta"] = 2.5
    else:
        # this clause won't do anything, because it is already set
        kwargs.setdefault( "maxEta", 2.5 )

    if kwargs.get("minPt") is None:
        # values in case the geometry is not set
        kwargs["minPt"] = 500
    else:
        kwargs.setdefault( "minPt", 500 ) 

    if flags.has( "Detector.GeometryITk" ) :
        #  do anything is the parameter is already set
        kwargs.setdefault( "maxEta", 4.0 if flags.Detector.GeometryITk else 2.5 )
        kwargs.setdefault( "minPt", 1000 if flags.Detector.GeometryITk else 500 )

    kwargs.setdefault( "requireOnlyPrimary", True )
    kwargs.setdefault( "maxProdVertRadius", 300. )

    kwargs.setdefault( "ancestorList", [] )
    kwargs.setdefault( "requireSiHit", 0 )
    kwargs.setdefault( "Extrapolator", None )

    acc.setPrivateTools( CompFactory.AthTruthSelectionTool( name, **kwargs ) )
    return acc



