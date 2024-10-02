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

def OfflineQualitySelectionCfg( flags, name="OfflineSelectionTool", **kwargs ) :
    acc = ComponentAccumulator()

    from InDetConfig.InDetTrackSelectionToolConfig import InDetTrackSelectionToolCfg
    offlineSelectionTool = acc.popToolsAndMerge( InDetTrackSelectionToolCfg( flags ) )
    offlineSelectionTool.CutLevel = flags.PhysVal.IDTPM.currentTrkAna.OfflineQualityWP

    kwargs.setdefault( "offlineTool", offlineSelectionTool )
    acc.setPrivateTools( CompFactory.IDTPM.OfflineTrackQualitySelectionTool( name, **kwargs ) )
    return acc    


def TruthQualitySelectionToolCfg( flags, name="TruthQualitySelectionTool", **kwargs ) :
    acc = ComponentAccumulator()

    # Default configurations 
    # ----------------------
    truthMinPt = flags.PhysVal.IDTPM.currentTrkAna.truthMinPt
    truthMaxPt = flags.PhysVal.IDTPM.currentTrkAna.truthMaxPt
    truthPdgId = flags.PhysVal.IDTPM.currentTrkAna.truthPdgId
    truthIsHadron = flags.PhysVal.IDTPM.currentTrkAna.truthIsHadron

    if "Muon" in flags.PhysVal.IDTPM.currentTrkAna.SelectTruthObject:
        truthPdgId = 13 
        if flags.PhysVal.IDTPM.currentTrkAna.SelectTruthObject == "highPTMuon": 
            truthMinPt = 20000 
            truthMaxPt = -9999. 
        if flags.PhysVal.IDTPM.currentTrkAna.SelectTruthObject == "lowPTMuon": 
            truthMinPt = 10000 
            truthMaxPt = 20000 

    elif "Hadron" in flags.PhysVal.IDTPM.currentTrkAna.SelectTruthObject:
        truthIsHadron = True
        if flags.PhysVal.IDTPM.currentTrkAna.SelectTruthObject == "highPTHadron": 
            truthMinPt = 20000 
            truthMaxPt = -9999. 
        if flags.PhysVal.IDTPM.currentTrkAna.SelectTruthObject == "lowPTHadron": 
            truthMinPt = 10000 
            truthMaxPt = 20000 
        
    elif "Electron" in flags.PhysVal.IDTPM.currentTrkAna.SelectTruthObject:
        truthPdgId = 11  
        if flags.PhysVal.IDTPM.currentTrkAna.SelectTruthObject == "highPTElectron": 
            truthMinPt = 10000 
            truthMaxPt = -9999. 
        if flags.PhysVal.IDTPM.currentTrkAna.SelectTruthObject == "lowPTElectron": 
            truthMinPt = 5000 
            truthMaxPt = 10000


    # InDetRttTruthSelectionTool properties
    # -------------------------------------
    kwargs_InDetRttTruthSelectionTool = {}
    if truthMinPt!=-9999.: kwargs_InDetRttTruthSelectionTool.setdefault( "minPt", truthMinPt )
    if truthMaxPt!=-9999.: kwargs_InDetRttTruthSelectionTool.setdefault( "maxPt", truthMaxPt )
    if flags.PhysVal.IDTPM.currentTrkAna.truthMaxAbsEta!=-9999.: kwargs_InDetRttTruthSelectionTool.setdefault( "maxEta", flags.PhysVal.IDTPM.currentTrkAna.truthMaxAbsEta )
    if truthPdgId!=-9999.: kwargs_InDetRttTruthSelectionTool.setdefault( "pdgId", truthPdgId )

    from InDetPhysValMonitoring.InDetPhysValMonitoringConfig import InDetRttTruthSelectionToolCfg
    truthSelectionTool = acc.popToolsAndMerge(InDetRttTruthSelectionToolCfg(flags, **kwargs_InDetRttTruthSelectionTool))

    # Additional properties
    # ---------------------
    kwargs.setdefault( "truthTool" , truthSelectionTool)
    kwargs.setdefault( "maxEta", flags.PhysVal.IDTPM.currentTrkAna.truthMaxEta )
    kwargs.setdefault( "minEta", flags.PhysVal.IDTPM.currentTrkAna.truthMinEta )
    kwargs.setdefault( "minPhi", flags.PhysVal.IDTPM.currentTrkAna.truthMinPhi )
    kwargs.setdefault( "maxPhi", flags.PhysVal.IDTPM.currentTrkAna.truthMaxPhi )
    kwargs.setdefault( "minD0", flags.PhysVal.IDTPM.currentTrkAna.truthMinD0 )
    kwargs.setdefault( "maxD0", flags.PhysVal.IDTPM.currentTrkAna.truthMaxD0 )
    kwargs.setdefault( "minZ0", flags.PhysVal.IDTPM.currentTrkAna.truthMinZ0 )
    kwargs.setdefault( "maxZ0", flags.PhysVal.IDTPM.currentTrkAna.truthMaxZ0 )
    kwargs.setdefault( "minQoPT", flags.PhysVal.IDTPM.currentTrkAna.truthMinQoPT )
    kwargs.setdefault( "maxQoPT", flags.PhysVal.IDTPM.currentTrkAna.truthMaxQoPT )
    kwargs.setdefault( "isHadron", truthIsHadron )
    kwargs.setdefault( "minAbsEta", flags.PhysVal.IDTPM.currentTrkAna.truthMinAbsEta )
    kwargs.setdefault( "minAbsPhi", flags.PhysVal.IDTPM.currentTrkAna.truthMinAbsPhi )
    kwargs.setdefault( "maxAbsPhi", flags.PhysVal.IDTPM.currentTrkAna.truthMaxAbsPhi )
    kwargs.setdefault( "minAbsD0", flags.PhysVal.IDTPM.currentTrkAna.truthMinAbsD0 )
    kwargs.setdefault( "maxAbsD0", flags.PhysVal.IDTPM.currentTrkAna.truthMaxAbsD0 )
    kwargs.setdefault( "minAbsZ0", flags.PhysVal.IDTPM.currentTrkAna.truthMinAbsZ0 )
    kwargs.setdefault( "maxAbsZ0", flags.PhysVal.IDTPM.currentTrkAna.truthMaxAbsZ0 )
    kwargs.setdefault( "minAbsQoPT", flags.PhysVal.IDTPM.currentTrkAna.truthMinAbsQoPT )
    kwargs.setdefault( "maxAbsQoPT", flags.PhysVal.IDTPM.currentTrkAna.truthMaxAbsQoPT )

    acc.setPrivateTools( CompFactory.IDTPM.TruthQualitySelectionTool( name, **kwargs ) )

    return acc


def TrackQualitySelectionToolCfg( flags, name="TrackQualitySelectionTool", **kwargs ):
    acc = ComponentAccumulator()

    ## Offline tracks quality selection
    if flags.PhysVal.IDTPM.currentTrkAna.OfflineQualityWP != "":
        kwargs.setdefault( "DoOfflineSelection", True )
    
        kwargs.setdefault( "OfflineSelectionTool", acc.popToolsAndMerge(
            OfflineQualitySelectionCfg( flags, name="OfflineSelectionTool"+flags.PhysVal.IDTPM.currentTrkAna.anaTag ) ) )

    ## Truth particles quality selection
    if flags.Input.isMC:
        kwargs.setdefault( "DoTruthSelection", True )
    
        kwargs.setdefault( "TruthSelectionTool", acc.popToolsAndMerge(
            TruthQualitySelectionToolCfg( flags, name="TruthQualitySelectionTool"+flags.PhysVal.IDTPM.currentTrkAna.anaTag ) ) )

    ## offline track-object selection
    if flags.PhysVal.IDTPM.currentTrkAna.SelectOfflineObject != "":
        kwargs.setdefault( "DoObjectSelection", True )
    
        if "TrackObjectSelectionTool" not in kwargs:
           kwargs.setdefault( "TrackObjectSelectionTool", acc.popToolsAndMerge(
               TrackObjectSelectionToolCfg( flags,
                   name="TrackObjectSelectionTool" + flags.PhysVal.IDTPM.currentTrkAna.anaTag ) ) )

    acc.setPrivateTools( CompFactory.IDTPM.TrackQualitySelectionTool( name, **kwargs ) )
    return acc
