#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

'''@file InDetSelectionConfig.py
@author M. Aparo
@date 02-10-2023
@brief CA-based python configurations for selection tools in this package
'''

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def RoiSelectionToolCfg( flags, name="RoiSelectionTool", **kwargs ) :
    '''
    CA-based configuration for the Tool to retrieve and select RoIs 
    '''
    acc = ComponentAccumulator()

    kwargs.setdefault( "RoiKey",        flags.PhysVal.IDTPM.currentTrkAna.RoiKey )
    kwargs.setdefault( "ChainLeg",      flags.PhysVal.IDTPM.currentTrkAna.ChainLeg )
    kwargs.setdefault( "doTagNProbe",   flags.PhysVal.IDTPM.currentTrkAna.doTagNProbe )
    kwargs.setdefault( "RoiKeyTag",     flags.PhysVal.IDTPM.currentTrkAna.RoiKeyTag )
    kwargs.setdefault( "ChainLegTag",   flags.PhysVal.IDTPM.currentTrkAna.ChainLegTag )
    kwargs.setdefault( "RoiKeyProbe",   flags.PhysVal.IDTPM.currentTrkAna.RoiKeyProbe )
    kwargs.setdefault( "ChainLegProbe", flags.PhysVal.IDTPM.currentTrkAna.ChainLegProbe )

    acc.setPrivateTools( CompFactory.IDTPM.RoiSelectionTool( name, **kwargs ) )
    return acc


def TrackRoiSelectionToolCfg( flags, name="TrackRoiSelectionTool", **kwargs ):
    acc = ComponentAccumulator()

    kwargs.setdefault( "TriggerTrkParticleContainerName",
                       flags.PhysVal.IDTPM.currentTrkAna.TrigTrkKey )

    acc.setPrivateTools( CompFactory.IDTPM.TrackRoiSelectionTool( name, **kwargs ) )
    return acc


def TrackObjectSelectionToolCfg( flags, name="TrackObjectSelectionTool", **kwargs ):
    acc = ComponentAccumulator()

    objStr = flags.PhysVal.IDTPM.currentTrkAna.SelectOfflineObject
    kwargs.setdefault( "ObjectType",    objStr )
    kwargs.setdefault( "ObjectQuality", flags.PhysVal.IDTPM.currentTrkAna.ObjectQuality )

    if "Tau" in objStr:
        kwargs.setdefault( "TauType",    flags.PhysVal.IDTPM.currentTrkAna.TauType )
        kwargs.setdefault( "TauNprongs", flags.PhysVal.IDTPM.currentTrkAna.TauNprongs )

    if "Truth" in objStr:
        kwargs.setdefault( "MatchingTruthProb", flags.PhysVal.IDTPM.currentTrkAna.TruthProbMin )

    acc.setPrivateTools( CompFactory.IDTPM.TrackObjectSelectionTool( name, **kwargs ) )
    return acc


def TrackQualitySelectionToolCfg( flags, name="TrackQualitySelectionTool", **kwargs ):
    acc = ComponentAccumulator()

    ## offline track-object selection
    if flags.PhysVal.IDTPM.currentTrkAna.SelectOfflineObject:
        kwargs.setdefault( "DoObjectSelection", True )
    
        if "TrackObjectSelectionTool" not in kwargs:
           kwargs.setdefault( "TrackObjectSelectionTool", acc.popToolsAndMerge(
               TrackObjectSelectionToolCfg( flags,
                   name="TrackObjectSelectionTool" + flags.PhysVal.IDTPM.currentTrkAna.anaTag ) ) )

    acc.setPrivateTools( CompFactory.IDTPM.TrackQualitySelectionTool( name, **kwargs ) )
    return acc
