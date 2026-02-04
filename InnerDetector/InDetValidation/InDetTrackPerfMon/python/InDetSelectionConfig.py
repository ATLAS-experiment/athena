#
#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
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


def VertexRoiSelectionToolCfg( flags, name="VertexRoiSelectionTool", **kwargs ):
    acc = ComponentAccumulator()

    kwargs.setdefault( "TriggerVertexContainerName",
                       flags.PhysVal.IDTPM.currentTrkAna.TrigVtxKey )

    acc.setPrivateTools( CompFactory.IDTPM.VertexRoiSelectionTool( name, **kwargs ) )
    return acc


def TrackObjectSelectionToolCfg( flags, name="TrackObjectSelectionTool", **kwargs ):
    acc = ComponentAccumulator()

    objStr = flags.PhysVal.IDTPM.currentTrkAna.SelectOfflineObject
    objQuality = flags.PhysVal.IDTPM.currentTrkAna.ObjectQuality
    if objQuality == "Medium" and "Jet" in objStr :
        ## changing default for jets
        objQuality = "DRtruthJet"
    kwargs.setdefault( "ObjectType",    objStr )
    kwargs.setdefault( "ObjectQuality", objQuality )

    if "Tau" in objStr:
        kwargs.setdefault( "TauType",    flags.PhysVal.IDTPM.currentTrkAna.TauType )
        kwargs.setdefault( "TauNprongs", flags.PhysVal.IDTPM.currentTrkAna.TauNprongs )

    if "Truth" in objStr:
        kwargs.setdefault( "MatchingTruthProb", flags.PhysVal.IDTPM.currentTrkAna.TruthProbMin )

    acc.setPrivateTools( CompFactory.IDTPM.TrackObjectSelectionTool( name, **kwargs ) )
    return acc

def OfflineQualitySelectionCfg( flags, name="OfflineSelectionTool", **kwargs ) :
    acc = ComponentAccumulator()

    # Default configurations 
    # ----------------------
    minHitsVector = flags.PhysVal.IDTPM.currentTrkAna.offlMinHitsVector
    minPtVector = flags.PhysVal.IDTPM.currentTrkAna.offlMinPtVector
    maxD0Vector = flags.PhysVal.IDTPM.currentTrkAna.offlMaxD0Vector
    maxZ0Vector = flags.PhysVal.IDTPM.currentTrkAna.offlMaxZ0Vector
    etaBins = flags.PhysVal.IDTPM.currentTrkAna.offlEtaBins
    qualityWP = flags.PhysVal.IDTPM.currentTrkAna.OfflineQualityWP

    #if flags.PhysVal.IDTPM.currentTrkAna.CustomOfflSel == "EFTracking": # Default selection for EFTracking studies
    ## Selection Working Point common for EF Tracking studies
    if qualityWP == "EFTracking" :
        etaBins = [-1., 2., 2.6, 9999.]
        minHitsVector = [9, 8, 7]
        minPtVector = [900., 400., 400.]
        maxD0Vector = [2., 2., 10.]
        maxZ0Vector = [150., 150., 150.]
        qualityWP = "" # to avoid conflicts with InDetTrackSelectionTool options

    kwargs_InDetTrackSelectionTool = {}

    if flags.PhysVal.IDTPM.currentTrkAna.offlMinPt!=-9999.: 
        kwargs_InDetTrackSelectionTool.setdefault( "minPt", flags.PhysVal.IDTPM.currentTrkAna.offlMinPt )
    if flags.PhysVal.IDTPM.currentTrkAna.offlMaxAbsEta!=-9999.: 
        kwargs_InDetTrackSelectionTool.setdefault( "maxAbsEta", flags.PhysVal.IDTPM.currentTrkAna.offlMaxAbsEta )
    if flags.PhysVal.IDTPM.currentTrkAna.offlMaxZ0SinTheta!=-9999.: 
        kwargs_InDetTrackSelectionTool.setdefault( "maxZ0SinTheta", flags.PhysVal.IDTPM.currentTrkAna.offlMaxZ0SinTheta )
    if flags.PhysVal.IDTPM.currentTrkAna.offlMaxZ0!=-9999.: 
        kwargs_InDetTrackSelectionTool.setdefault( "maxZ0", flags.PhysVal.IDTPM.currentTrkAna.offlMaxZ0 )
    if flags.PhysVal.IDTPM.currentTrkAna.offlMaxD0!=-9999.: 
        kwargs_InDetTrackSelectionTool.setdefault( "maxD0", flags.PhysVal.IDTPM.currentTrkAna.offlMaxD0 )
    if flags.PhysVal.IDTPM.currentTrkAna.offlMinNInnermostLayerHits!=-9999.: 
        kwargs_InDetTrackSelectionTool.setdefault( "minNInnermostLayerHits", flags.PhysVal.IDTPM.currentTrkAna.offlMinNInnermostLayerHits )
    if flags.PhysVal.IDTPM.currentTrkAna.offlMinNBothInnermostLayersHits!=-9999.: 
        kwargs_InDetTrackSelectionTool.setdefault( "minNBothInnermostLayersHits", flags.PhysVal.IDTPM.currentTrkAna.offlMinNBothInnermostLayersHits )
    if flags.PhysVal.IDTPM.currentTrkAna.offlMaxNInnermostLayerSharedHits!=-9999.: 
        kwargs_InDetTrackSelectionTool.setdefault( "maxNInnermostLayerSharedHits", flags.PhysVal.IDTPM.currentTrkAna.offlMaxNInnermostLayerSharedHits )
    if flags.PhysVal.IDTPM.currentTrkAna.offlMinNSiHits!=-9999.: 
        kwargs_InDetTrackSelectionTool.setdefault( "minNSiHits", flags.PhysVal.IDTPM.currentTrkAna.offlMinNSiHits )
    if flags.PhysVal.IDTPM.currentTrkAna.offlMaxNSiSharedHits!=-9999.: 
        kwargs_InDetTrackSelectionTool.setdefault( "maxNSiSharedHits", flags.PhysVal.IDTPM.currentTrkAna.offlMaxNSiSharedHits )
    if flags.PhysVal.IDTPM.currentTrkAna.offlMaxNSiHoles!=-9999.: 
        kwargs_InDetTrackSelectionTool.setdefault( "maxNSiHoles", flags.PhysVal.IDTPM.currentTrkAna.offlMaxNSiHoles )
    if flags.PhysVal.IDTPM.currentTrkAna.offlMinNPixelHits!=-9999.: 
        kwargs_InDetTrackSelectionTool.setdefault( "minNPixelHits", flags.PhysVal.IDTPM.currentTrkAna.offlMinNPixelHits )
    if flags.PhysVal.IDTPM.currentTrkAna.offlMaxNPixelSharedHits!=-9999.: 
        kwargs_InDetTrackSelectionTool.setdefault( "maxNPixelSharedHits", flags.PhysVal.IDTPM.currentTrkAna.offlMaxNPixelSharedHits )
    if flags.PhysVal.IDTPM.currentTrkAna.offlMaxNPixelHoles!=-9999.: 
        kwargs_InDetTrackSelectionTool.setdefault( "maxNPixelHoles", flags.PhysVal.IDTPM.currentTrkAna.offlMaxNPixelHoles )
    if flags.PhysVal.IDTPM.currentTrkAna.offlMinNSctHits!=-9999.: 
        kwargs_InDetTrackSelectionTool.setdefault( "minNSctHits", flags.PhysVal.IDTPM.currentTrkAna.offlMinNSctHits )
    if flags.PhysVal.IDTPM.currentTrkAna.offlMaxNSctSharedHits!=-9999.: 
        kwargs_InDetTrackSelectionTool.setdefault( "maxNSctSharedHits", flags.PhysVal.IDTPM.currentTrkAna.offlMaxNSctSharedHits )
    if flags.PhysVal.IDTPM.currentTrkAna.offlMaxNSctHoles!=-9999.: 
        kwargs_InDetTrackSelectionTool.setdefault( "maxNSctHoles", flags.PhysVal.IDTPM.currentTrkAna.offlMaxNSctHoles )
    if flags.PhysVal.IDTPM.currentTrkAna.offlMaxChiSq!=-9999.: 
        kwargs_InDetTrackSelectionTool.setdefault( "maxChiSq", flags.PhysVal.IDTPM.currentTrkAna.offlMaxChiSq )
    if flags.PhysVal.IDTPM.currentTrkAna.offlMaxChiSqperNdf!=-9999.: 
        kwargs_InDetTrackSelectionTool.setdefault( "maxChiSqperNdf", flags.PhysVal.IDTPM.currentTrkAna.offlMaxChiSqperNdf )
    if flags.PhysVal.IDTPM.currentTrkAna.offlMinProb!=-9999.: 
        kwargs_InDetTrackSelectionTool.setdefault( "minProb", flags.PhysVal.IDTPM.currentTrkAna.offlMinProb )
    kwargs_InDetTrackSelectionTool.setdefault( "CutLevel", qualityWP )

    from InDetTrackSelectionTool.InDetTrackSelectionToolConfig import (
        InDetTrackSelectionToolCfg)
    offlineSelectionTool = acc.popToolsAndMerge( InDetTrackSelectionToolCfg( flags, **kwargs_InDetTrackSelectionTool) )

    kwargs.setdefault( "offlineTool", offlineSelectionTool )
    kwargs.setdefault( "maxPt", flags.PhysVal.IDTPM.currentTrkAna.offlMaxPt )
    kwargs.setdefault( "minEta", flags.PhysVal.IDTPM.currentTrkAna.offlMinEta )
    kwargs.setdefault( "minPhi", flags.PhysVal.IDTPM.currentTrkAna.offlMinPhi )
    kwargs.setdefault( "maxPhi", flags.PhysVal.IDTPM.currentTrkAna.offlMaxPhi )
    kwargs.setdefault( "minD0", flags.PhysVal.IDTPM.currentTrkAna.offlMinD0 )
    kwargs.setdefault( "minZ0", flags.PhysVal.IDTPM.currentTrkAna.offlMinZ0 )
    kwargs.setdefault( "minQoPT", flags.PhysVal.IDTPM.currentTrkAna.offlMinQoPT )
    kwargs.setdefault( "maxQoPT", flags.PhysVal.IDTPM.currentTrkAna.offlMaxQoPT )
    kwargs.setdefault( "minAbsEta", flags.PhysVal.IDTPM.currentTrkAna.offlMinAbsEta )
    kwargs.setdefault( "minAbsPhi", flags.PhysVal.IDTPM.currentTrkAna.offlMinAbsPhi )
    kwargs.setdefault( "maxAbsPhi", flags.PhysVal.IDTPM.currentTrkAna.offlMaxAbsPhi )
    kwargs.setdefault( "minAbsD0", flags.PhysVal.IDTPM.currentTrkAna.offlMinAbsD0 )
    kwargs.setdefault( "maxAbsD0", flags.PhysVal.IDTPM.currentTrkAna.offlMaxAbsD0 )
    kwargs.setdefault( "minAbsZ0", flags.PhysVal.IDTPM.currentTrkAna.offlMinAbsZ0 )
    kwargs.setdefault( "maxAbsZ0", flags.PhysVal.IDTPM.currentTrkAna.offlMaxAbsZ0 )
    kwargs.setdefault( "minAbsQoPT", flags.PhysVal.IDTPM.currentTrkAna.offlMinAbsQoPT )
    kwargs.setdefault( "maxAbsQoPT", flags.PhysVal.IDTPM.currentTrkAna.offlMaxAbsQoPT )
    kwargs.setdefault( "etaBins", etaBins ) 
    kwargs.setdefault( "minHitsVec", minHitsVector )
    kwargs.setdefault( "minPtVec", minPtVector )
    kwargs.setdefault( "maxD0Vec", maxD0Vector )
    kwargs.setdefault( "maxZ0Vec", maxZ0Vector )

    acc.setPrivateTools( CompFactory.IDTPM.OfflineTrackQualitySelectionTool( name, **kwargs ) )

    return acc    


def TruthSelectionBaseToolCfg( flags, name="TruthSelectionBaseTool", **kwargs ) :
    '''
    Copy of InDetPhysValMonitoring.InDetPhysValMonitoringConfig InDetRttTruthSelectionToolCfg
    to handle flags internally in IDTPM, i.e. not relying on IDPVM's (default) flags
    '''
    acc = ComponentAccumulator()

    ## Baseline requirements to be applied to all analyses
    kwargs.setdefault( "requireStable", True )
    kwargs.setdefault( "requireCharged", True )
    kwargs.setdefault( "selectedCharge", 0 )
    kwargs.setdefault( "maxEta", 4.0 if flags.Detector.GeometryITk else 2.5 )

    kwargs.setdefault( "requireOnlyPrimary", True )
    kwargs.setdefault( "maxProdVertRadius", 300. )

    kwargs.setdefault( "minPt", 1000 if flags.Detector.GeometryITk else 500 )
    kwargs.setdefault( "ancestorList", [] )
    kwargs.setdefault( "requireSiHit", 0 )
    kwargs.setdefault( "Extrapolator", None )

    acc.setPrivateTools( CompFactory.AthTruthSelectionTool( name, **kwargs ) )
    return acc


def TruthQualitySelectionToolCfg( flags, name="TruthQualitySelectionTool", **kwargs ) :
    acc = ComponentAccumulator()

    # Default configurations 
    # ----------------------
    truthMinPt      = flags.PhysVal.IDTPM.currentTrkAna.truthMinPt
    truthMaxPt      = flags.PhysVal.IDTPM.currentTrkAna.truthMaxPt
    truthMaxAbsEta  = flags.PhysVal.IDTPM.currentTrkAna.truthMaxAbsEta
    truthPdgId      = flags.PhysVal.IDTPM.currentTrkAna.truthPdgId

    truthMinParentPt = flags.PhysVal.IDTPM.currentTrkAna.truthMinParentPt
    truthMaxParentPt = flags.PhysVal.IDTPM.currentTrkAna.truthMaxParentPt

    ## SelectTruthObject: customised Pt range selection
    if "HighPt" in flags.PhysVal.IDTPM.currentTrkAna.SelectTruthObject :
        truthMinPt = 10000  # 10 GeV
        truthMaxPt = -9999. # +inf
    elif "VeryLowPt" in flags.PhysVal.IDTPM.currentTrkAna.SelectTruthObject :
        truthMinPt = 1000   # 1 GeV
        truthMaxPt = 2000   # 2 GeV
    elif "LowPt" in flags.PhysVal.IDTPM.currentTrkAna.SelectTruthObject :
        truthMinPt = 1000   # 1 GeV
        truthMaxPt = 10000  # 10 GeV

    ## SelectTruthObjec: cutomised selections
    if "Muon" in flags.PhysVal.IDTPM.currentTrkAna.SelectTruthObject :
        truthPdgId = 13 
    if "Electron" in flags.PhysVal.IDTPM.currentTrkAna.SelectTruthObject :
        truthPdgId = 11
        ## adjusting pT ranges for low/highPt electrons
        if "HighPt" in flags.PhysVal.IDTPM.currentTrkAna.SelectTruthObject :
            truthMinPt = 20000  # 20 GeV
            truthMaxPt = -9999. # +inf
        if "LowPt" in flags.PhysVal.IDTPM.currentTrkAna.SelectTruthObject :
            truthMinPt = 10000  # 10 GeV
            truthMaxPt = 20000  # 20 GeV
    truthIsHadron   = ( "Hadron" in flags.PhysVal.IDTPM.currentTrkAna.SelectTruthObject )
    truthIsPion     = ( "Pion" in flags.PhysVal.IDTPM.currentTrkAna.SelectTruthObject )

    ## SelectTruthObjec: cutomised truth origin selections
    truthIsFromB = ( "FromB" in flags.PhysVal.IDTPM.currentTrkAna.SelectTruthObject )
    truthIsFromC = ( "FromC" in flags.PhysVal.IDTPM.currentTrkAna.SelectTruthObject )
    truthIsFromHeavyFlav = ( "FromHeavyFlav" in flags.PhysVal.IDTPM.currentTrkAna.SelectTruthObject )
    truthIsFromLightFlav = ( "FromLightFlav" in flags.PhysVal.IDTPM.currentTrkAna.SelectTruthObject )
    truthIsFromTau = ( "FromTau" in flags.PhysVal.IDTPM.currentTrkAna.SelectTruthObject)

    # TruthSelectionBaseTool properties
    # ---------------------------------
    kwargs_base = {}
    if truthMinPt!=-9999.       : kwargs_base.setdefault( "minPt",  truthMinPt )
    if truthMaxPt!=-9999.       : kwargs_base.setdefault( "maxPt",  truthMaxPt )
    if truthMaxAbsEta!=-9999.   : kwargs_base.setdefault( "maxEta", truthMaxAbsEta )
    if truthPdgId!=-9999.       : kwargs_base.setdefault( "pdgId",  truthPdgId )

    ## remove only primary requirements for Heavy Flavour truth selection - removed for now
    #doHF = truthIsFromB or truthIsFromC or truthIsFromHeavyFlav
    #if doHF                         : kwargs_base.setdefault( "requireOnlyPrimary", False )
    #if doHF or truthIsFromLightFlav : kwargs_base.setdefault( "maxProdVertRadius", -1. )

    kwargs.setdefault( "truthTool" ,
        acc.popToolsAndMerge( TruthSelectionBaseToolCfg( flags, **kwargs_base ) ) )

    # Additional properties
    # ---------------------
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
    kwargs.setdefault( "minAbsEta", flags.PhysVal.IDTPM.currentTrkAna.truthMinAbsEta )
    kwargs.setdefault( "minAbsPhi", flags.PhysVal.IDTPM.currentTrkAna.truthMinAbsPhi )
    kwargs.setdefault( "maxAbsPhi", flags.PhysVal.IDTPM.currentTrkAna.truthMaxAbsPhi )
    kwargs.setdefault( "minAbsD0", flags.PhysVal.IDTPM.currentTrkAna.truthMinAbsD0 )
    kwargs.setdefault( "maxAbsD0", flags.PhysVal.IDTPM.currentTrkAna.truthMaxAbsD0 )
    kwargs.setdefault( "minAbsZ0", flags.PhysVal.IDTPM.currentTrkAna.truthMinAbsZ0 )
    kwargs.setdefault( "maxAbsZ0", flags.PhysVal.IDTPM.currentTrkAna.truthMaxAbsZ0 )
    kwargs.setdefault( "minAbsQoPT", flags.PhysVal.IDTPM.currentTrkAna.truthMinAbsQoPT )
    kwargs.setdefault( "maxAbsQoPT", flags.PhysVal.IDTPM.currentTrkAna.truthMaxAbsQoPT )
    kwargs.setdefault( "isHadron", truthIsHadron )
    kwargs.setdefault( "isPion", truthIsPion )
    kwargs.setdefault( "isFromB", truthIsFromB )
    kwargs.setdefault( "isFromC", truthIsFromC )
    kwargs.setdefault( "isFromHeavyFlav", truthIsFromHeavyFlav )
    kwargs.setdefault( "isFromLightFlav", truthIsFromLightFlav )
    kwargs.setdefault( "isFromTau", truthIsFromTau )
    if truthMinParentPt!=-9999.    :kwargs.setdefault( "minParentPt", truthMinParentPt)
    if truthMaxParentPt!=-9999.    :kwargs.setdefault( "maxParentPt", truthMaxParentPt)

    acc.setPrivateTools( CompFactory.IDTPM.TruthQualitySelectionTool( name, **kwargs ) )

    return acc


def TrackQualitySelectionToolCfg( flags, name="TrackQualitySelectionTool", **kwargs ):
    acc = ComponentAccumulator()

    ## Offline tracks quality selection
    if flags.PhysVal.IDTPM.currentTrkAna.OfflineQualityWP != "" or flags.PhysVal.IDTPM.currentTrkAna.DoOfflineSelection:
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


def VertexQualitySelectionToolCfg( flags, name="VertexQualitySelectionTool", **kwargs ):
    acc = ComponentAccumulator()

    ## TODO: here other selector tools for vertices

    acc.setPrivateTools( CompFactory.IDTPM.VertexQualitySelectionTool( name, **kwargs ) )
    return acc
