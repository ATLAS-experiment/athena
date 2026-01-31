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


# from AthenaConfiguration.AthConfigFlags import ConfigFlags

# # default useful setter function ....
# def get_opt(obj, name, default=None):
#     return getattr(obj, name, default)

# # set the default kswargs only if there is such a variable in the flags object
# def kwargs_setdefault_old( kwargs, key, obj, attr=None ):
#     if attr is None:
#         attr = key
#     if hasattr(obj, attr):
#         kwargs.setdefault(key, getattr(obj, attr))

    
# def kwargs_setdefault_new( kwargs, key, obj, attr=None, default=None, skip_if_default=False):
#     if attr is None:
#         attr = key
#     val = getattr(obj, attr, default)
#     if skip_if_default and val == default:
#         return  # do not set
#     kwargs.setdefault(key, val)

# # _MISSING=objuect()
# # def setdefault(kwargs, key, obj, attr=None, default=_MISSING, skip_if_default=False):
# def kwargs_setdefault(kwargs, key, obj, attr=None, default=..., skip_if_default=False):
#     if attr is None:
#         attr = key

#     try:
#         val = getattr(obj, attr)
#         attr_exists = True
#     except AttributeError:
#         val = ...
#         attr_exists = False

#     # what if we have no attribute ? only set if we
#     # provide a default
#     if not attr_exists:
#         if default is ...:
#             return
#         val = default

#     # Now, if we want to skip it if there is no attribute etc
#     if skip_if_default and default is not ... and val == default:
#         return

#     kwargs.setdefault(key, val)
    
# # check whether the attribute has an entry wigth this value, but only if
# # the attribute is actually set
# def has_in( item, obj, attr, default=False):
#     if not hasattr(obj, attr):
#         return default
#     val = getattr(obj, attr)
#     if val is None:
#         return default
#     try:
#         return item in val
#     except TypeError:
#         return default



# def hasFlag(obj, name):
#     """
#     The athena config design is very poor *every* node should be the same, 
#     but only the top level has a hasFlag() method, so here is a helper 
#     function that can be called as if top level, or sub objects were the 
#     same 
#     """
#     # pythion complains if we have this directly ...
#     # if hasattr(obj, "hasFlag") and callable(getattr(obj, "hasFlag")):
#     f = getattr(obj, "hasFlag", None)
#     if callable(f):
#         # top-level ConfigFlags → use the real hasFlag
#         return obj.hasFlag(name)
    
#     # Sub-container → walk the dotted path
#     parts = name.split(".")
#     current = obj
#     for part in parts:
#         if not hasattr(current, part):
#             return False
#         current = getattr(current, part)
#     return True


# def sanitise(s: str) -> str:
#     """
#     Replace all ':' and '=' characters in the string with '_'.
#     """
#     return s.replace(":", "_").replace("=", "_")

# def get_flags( flags, name ):
#     if hasFlag(flags,"PhysVal.IDTPM.currentTrkAna"):
#         iflags = flags.PhysVal.IDTPM.currentTrkAna
#         iname  = name+iflags.anaTag 
#     else:
#         iflags = flags
#         iname  = sanitise(name)
#     return iflags, iname

        
from InDetTrackPerfMon.ConfigUtils import get_flags
from InDetTrackPerfMon.ConfigUtils import hasFlag
from InDetTrackPerfMon.ConfigUtils import has_in
from InDetTrackPerfMon.ConfigUtils import kwargs_setdefault
from InDetTrackPerfMon.ConfigUtils import get_opt
from InDetTrackPerfMon.ConfigUtils import sanitise

    
def RoiSelectionToolCfg( flags, name="RoiSelectionTool", **kwargs ) :
    '''
    CA-based configuration for the Tool to retrieve and select RoIs 
    '''
    acc = ComponentAccumulator()

    iflags, iname = get_flags( flags, name )
    
    kwargs_setdefault( kwargs, "RoiKey",        iflags )
    kwargs_setdefault( kwargs, "ChainLeg",      iflags )
    kwargs_setdefault( kwargs, "doTagNProbe",   iflags )
    kwargs_setdefault( kwargs, "RoiKeyTag",     iflags )
    kwargs_setdefault( kwargs, "ChainLegTag",   iflags )
    kwargs_setdefault( kwargs, "RoiKeyProbe",   iflags )
    kwargs_setdefault( kwargs, "ChainLegProbe", iflags )

#    print("DEBUG final kwargs for RoiSelectionTool:")
#    for k, v in kwargs.items():
#        print(f"  {k} = {v!r}")
       
    acc.setPrivateTools( CompFactory.IDTPM.RoiSelectionTool( iname, **kwargs ) )
    return acc


def TrackRoiSelectionToolCfg( flags, name="TrackRoiSelectionTool", **kwargs ):
    acc = ComponentAccumulator()

    iflags, iname = get_flags( flags, name )
    
    kwargs_setdefault( kwargs, "TriggerTrkParticleContainerName", iflags )

    acc.setPrivateTools( CompFactory.IDTPM.TrackRoiSelectionTool( iname, **kwargs ) )
    return acc


def VertexRoiSelectionToolCfg( flags, name="VertexRoiSelectionTool", **kwargs ):
    acc = ComponentAccumulator()

    iflags, iname = get_flags( flags, name )
        
    kwargs_setdefault( kwargs,  "TriggerVertexContainerName", iflags, "TrigVtxKey" )

    acc.setPrivateTools( CompFactory.IDTPM.VertexRoiSelectionTool( iname, **kwargs ) )
    return acc


def TrackObjectSelectionToolCfg( flags, name="TrackObjectSelectionTool", **kwargs ):
    acc = ComponentAccumulator()

    iflags, iname = get_flags( flags, name )
        
    objStr = get_opt( iflags, "SelectOfflineObject" )
    objQuality = get_opt( iflags, "ObjectQuality" )


    if objQuality and objStr: 
        if objQuality == "Medium" and "Jet" in objStr :
            ## changing default for§ jets
            objQuality = "DRtruthJet"
            
    if  objStr: 
        kwargs.setdefault( "ObjectType",    objStr )

    if objQuality:
        kwargs.setdefault( "ObjectQuality", objQuality )

    if objStr:
        if "Tau" in objStr:
            kwargs_setdefault( kwargs,  "TauType",    iflags )
            kwargs_setdefault( kwargs,  "TauNprongs", iflags )

        if "Truth" in objStr:
            kwargs_setdefault( kwargs,  "MatchingTruthProb", iflags, "TruthProbMin" )

    acc.setPrivateTools( CompFactory.IDTPM.TrackObjectSelectionTool( iname, **kwargs ) )
    return acc


def OfflineQualitySelectionCfg( flags, name="OfflineSelectionTool", **kwargs ) :
    acc = ComponentAccumulator()

    iflags, iname = get_flags( flags, name )

    # Default configurations 
    # ----------------------
    minHitsVector = get_opt( iflags, "offlMinHitsVector" )
    minPtVector = get_opt( iflags, "offlMinPtVector" )
    maxD0Vector = get_opt( iflags, "offlMaxD0Vector" )
    maxZ0Vector = get_opt( iflags, "offlMaxZ0Vector" )
    etaBins     = get_opt( iflags, "offlEtaBins" )
    qualityWP   = get_opt( iflags, "OfflineQualityWP" )

    #if iflags.CustomOfflSel == "EFTracking": # Default selection for EFTracking studies
    ## Selection Working Point common for EF Tracking studies
    if qualityWP == "EFTracking" :
        etaBins = [-1., 2., 2.6, 9999.]
        minHitsVector = [9, 8, 7]
        minPtVector = [900., 400., 400.]
        maxD0Vector = [2., 2., 10.]
        maxZ0Vector = [150., 150., 150.]
        qualityWP = "" # to avoid conflicts with InDetTrackSelectionTool options

    kwargs_InDetTrackSelectionTool = {}

    kwargs_setdefault( kwargs_InDetTrackSelectionTool, "minPt",     iflags, "offlMinPt",     -9999, True )
    kwargs_setdefault( kwargs_InDetTrackSelectionTool, "maxAbsEta", iflags, "offlMaxAbsEta", -9999, True )
    kwargs_setdefault( kwargs_InDetTrackSelectionTool, "maxZ0SinTheta", iflags, "offlMaxZ0SinTheta", -9999, True )
    kwargs_setdefault( kwargs_InDetTrackSelectionTool, "maxZ0", iflags, "offlMaxZ0", -9999, True ) 
    kwargs_setdefault( kwargs_InDetTrackSelectionTool, "maxD0", iflags, "offlMaxD0", -9999, True ) 
    kwargs_setdefault( kwargs_InDetTrackSelectionTool, "minNInnermostLayerHits",       iflags, "offlMinNInnermostLayerHits",       -9999, True ) 
    kwargs_setdefault( kwargs_InDetTrackSelectionTool, "minNBothInnermostLayersHits",  iflags, "offlMinNBothInnermostLayersHits",  -9999, True ) 
    kwargs_setdefault( kwargs_InDetTrackSelectionTool, "maxNInnermostLayerSharedHits", iflags, "offlMaxNInnermostLayerSharedHits", -9999, True )
    kwargs_setdefault( kwargs_InDetTrackSelectionTool, "minNSiHits",       iflags, "offlMinNSiHits",         -9999, True ) 
    kwargs_setdefault( kwargs_InDetTrackSelectionTool, "maxNSiSharedHits", iflags, "offlMaxNSiSharedHits",   -9999, True ) 
    kwargs_setdefault( kwargs_InDetTrackSelectionTool, "maxNSiHoles",      iflags, "offlMaxNSiHoles",        -9999, True ) 
    kwargs_setdefault( kwargs_InDetTrackSelectionTool, "minNPixelHits",    iflags, "offlMinNPixelHits",      -9999, True )
    kwargs_setdefault( kwargs_InDetTrackSelectionTool, "maxNPixelSharedHits", iflags, "offlMaxNPixelSharedHits", -9999, True )
    kwargs_setdefault( kwargs_InDetTrackSelectionTool, "maxNPixelHoles",    iflags, "offlMaxNPixelHoles",    -9999, True )
    kwargs_setdefault( kwargs_InDetTrackSelectionTool, "minNSctHits",       iflags, "offlMinNSctHits",       -9999, True )
    kwargs_setdefault( kwargs_InDetTrackSelectionTool, "maxNSctSharedHits", iflags, "offlMaxNSctSharedHits", -9999, True )
    kwargs_setdefault( kwargs_InDetTrackSelectionTool, "maxNSctHoles",      iflags, "offlMaxNSctHoles",      -9999, True )
    kwargs_setdefault( kwargs_InDetTrackSelectionTool, "maxChiSq",          iflags, "offlMaxChiSq",          -9999, True )
    kwargs_setdefault( kwargs_InDetTrackSelectionTool, "maxChiSqperNdf",    iflags, "offlMaxChiSqperNdf",    -9999, True )
    kwargs_setdefault( kwargs_InDetTrackSelectionTool, "minProb",           iflags, "offlMinProb",           -9999, True )



    kwargs_setdefault( kwargs_InDetTrackSelectionTool, "minPt",         iflags, "offlMinPt",     -9999, True )
    kwargs_setdefault( kwargs_InDetTrackSelectionTool, "maxAbsEta",     iflags, "offlMaxAbsEta", -9999, True )
    kwargs_setdefault( kwargs_InDetTrackSelectionTool, "maxZ0SinTheta", iflags, "offlMaxZ0SinTheta", -9999, True )
    kwargs_setdefault( kwargs_InDetTrackSelectionTool, "maxZ0",         iflags, "offlMaxZ0", -9999, True )
    
    kwargs_setdefault( kwargs_InDetTrackSelectionTool, "maxD0", iflags, "offlMaxD0", -9999, True )
    kwargs_setdefault( kwargs_InDetTrackSelectionTool, "minNInnermostLayerHits",       iflags, "offlMinNInnermostLayerHits",       -9999, True )
    kwargs_setdefault( kwargs_InDetTrackSelectionTool, "minNBothInnermostLayersHits",  iflags, "offlMinNBothInnermostLayersHits",  -9999, True )
    kwargs_setdefault( kwargs_InDetTrackSelectionTool, "maxNInnermostLayerSharedHits", iflags, "offlMaxNInnermostLayerSharedHits", -9999, True )
    kwargs_setdefault( kwargs_InDetTrackSelectionTool, "minNSiHits",       iflags, "offlMinNSiHits", -9999, True )
    kwargs_setdefault( kwargs_InDetTrackSelectionTool, "maxNSiSharedHits", iflags, "offlMaxNSiSharedHits", -9999, True )

         
    kwargs_InDetTrackSelectionTool.setdefaul( "CutLevel", qualityWP )

    from InDetConfig.InDetTrackSelectionToolConfig import InDetTrackSelectionToolCfg
    offlineSelectionTool = acc.popToolsAndMerge( InDetTrackSelectionToolCfg( flags, **kwargs_InDetTrackSelectionTool) )

    kwargs.setdefault( "offlineTool", offlineSelectionTool )
    
    kwargs_setdefault( kwargs,  "maxPt",     iflags, "offlMaxPt"  )
    kwargs_setdefault( kwargs,  "minEta",    iflags, "offlMinEta" )
    kwargs_setdefault( kwargs,  "minPhi",    iflags, "offlMinPhi" )
    kwargs_setdefault( kwargs,  "maxPhi",    iflags, "offlMaxPhi" )
    kwargs_setdefault( kwargs,  "minD0",     iflags, "offlMinD0"  )
    kwargs_setdefault( kwargs,  "minZ0",     iflags, "offlMinZ0"  )
    kwargs_setdefault( kwargs,  "minQoPT",   iflags, "offlMinQoPT" )
    kwargs_setdefault( kwargs,  "maxQoPT",   iflags, "offlMaxQoPT" )
    kwargs_setdefault( kwargs,  "minAbsEta", iflags, "offlMinAbsEta" )
    kwargs_setdefault( kwargs,  "minAbsPhi", iflags, "offlMinAbsPhi" )
    kwargs_setdefault( kwargs,  "maxAbsPhi", iflags, "offlMaxAbsPhi" )
    kwargs_setdefault( kwargs,  "minAbsD0",  iflags, "offlMinAbsD0" )
    kwargs_setdefault( kwargs,  "maxAbsD0",  iflags, "offlMaxAbsD0" )
    kwargs_setdefault( kwargs,  "minAbsZ0",  iflags, "offlMinAbsZ0" )
    kwargs_setdefault( kwargs,  "maxAbsZ0",  iflags, "offlMaxAbsZ0" )
    kwargs_setdefault( kwargs,  "minAbsQoPT", iflags, "offlMinAbsQoPT" )
    kwargs_setdefault( kwargs,  "maxAbsQoPT", iflags, "offlMaxAbsQoPT" )
    
    kwargs.setdefault( "etaBins",    etaBins ) 
    kwargs.setdefault( "minHitsVec", minHitsVector )
    kwargs.setdefault( "minPtVec",   minPtVector )
    kwargs.setdefault( "maxD0Vec",   maxD0Vector )
    kwargs.setdefault( "maxZ0Vec",   maxZ0Vector )

    acc.setPrivateTools( CompFactory.IDTPM.OfflineTrackQualitySelectionTool( iname, **kwargs ) )

    return acc    


def TruthSelectionBaseToolCfg( flags, name="TruthSelectionBaseTool", **kwargs ) :
    '''
    Copy of InDetPhysValMonitoring.InDetPhysValMonitoringConfig InDetRttTruthSelectionToolCfg
    to handle flags internally in IDTPM, i.e. not relying on IDPVM's (default) flags
    '''
    acc = ComponentAccumulator()
    
    iflags, iname = get_flags( flags, name )

    
    ## Baseline requirements to be applied to all analyses
    kwargs.setdefault( "requireStable", True )
    kwargs.setdefault( "requireCharged", True )
    kwargs.setdefault( "selectedCharge", 0 )

    # values in case the geometry is not set
    kwargs.setdefault( "maxEta", 2.5 )
    kwargs.setdefault( "minPt", 500 ) 

    if hasFlag( flags, "Detector.GeometryITk" ) :
        kwargs.setdefault( "maxEta", 4.0 if flags.Detector.GeometryITk else 2.5 )
        kwargs.setdefault( "minPt", 1000 if flags.Detector.GeometryITk else 500 )
        
    kwargs.setdefault( "requireOnlyPrimary", True )
    kwargs.setdefault( "maxProdVertRadius", 300. )


    kwargs.setdefault( "ancestorList", [] )
    kwargs.setdefault( "requireSiHit", 0 )
    kwargs.setdefault( "Extrapolator", None )

    acc.setPrivateTools( CompFactory.AthTruthSelectionTool( iname, **kwargs ) )
    return acc


def TruthQualitySelectionToolCfg( flags, name="TruthQualitySelectionTool", **kwargs ) :
    acc = ComponentAccumulator()

    iflags, iname = get_flags( flags, name )

    # Default configurations 
    # ----------------------
    truthMinPt      = get_opt( iflags, "truthMinPt" )
    truthMaxPt      = get_opt( iflags, "truthMaxPt" )
    truthMaxAbsEta  = get_opt( iflags, "truthMaxAbsEta" )
    truthPdgId      = get_opt( iflags, "truthPdgId" )

    truthMinParentPt = get_opt( iflags, "truthMinParentPt" )
    truthMaxParentPt = get_opt( iflags, "truthMaxParentPt" )

    ## SelectTruthObject: customised Pt range selection
    if has_in( "HighPt", iflags, "SelectTruthObject" ) :
        truthMinPt = 10000  # 10 GeV
        truthMaxPt = -9999. # +inf
    elif has_in( "VeryLowPt", iflags, "SelectTruthObject" ) :
        truthMinPt = 1000   # 1 GeV
        truthMaxPt = 2000   # 2 GeV
    elif has_in( "LowPt", iflags, "SelectTruthObject" ) :
        truthMinPt = 1000   # 1 GeV
        truthMaxPt = 10000  # 10 GeV

    ## SelectTruthObjec: cutomised selections
    if has_in( "Muon", iflags, "SelectTruthObject" ) :
        truthPdgId = 13 
    if has_in( "Electron", iflags, "SelectTruthObject" ) :
        truthPdgId = 11
        ## adjusting pT ranges for low/highPt electrons
        if has_in( "HighPt", iflags, "SelectTruthObject" ) :
            truthMinPt = 20000  # 20 GeV
            truthMaxPt = -9999. # +inf
        if has_in( "LowPt", iflags, "SelectTruthObject") :
            truthMinPt = 10000  # 10 GeV
            truthMaxPt = 20000  # 20 GeV

#    truthIsHadron   = ( "Hadron" in iflags, "SelectTruthObject" )
#    truthIsPion     = ( "Pion" in iflags, "SelectTruthObject" )

    ## SelectTruthObjec: cutomised truth origin selections
#   truthIsFromB = ( "FromB" in iflags, "SelectTruthObject" )
#   truthIsFromC = ( "FromC" in iflags, "SelectTruthObject" )
#   truthIsFromHeavyFlav = ( "FromHeavyFlav" in iflags, "SelectTruthObject" )
#   truthIsFromLightFlav = ( "FromLightFlav" in iflags, "SelectTruthObject" )
#   truthIsFromTau = ( "FromTau" in iflags.SelectTruthObject)

    truthIsHadron   = has_in( "Hadron", iflags, "SelectTruthObject" )
    truthIsPion     = has_in( "Pion", iflags, "SelectTruthObject" )

    ## SelectTruthObjec: cutomised truth origin selections
    truthIsFromB = has_in( "FromB", iflags, "SelectTruthObject" )
    truthIsFromC = has_in( "FromC", iflags, "SelectTruthObject" )
    truthIsFromHeavyFlav = has_in( "FromHeavyFlav", iflags, "SelectTruthObject" )
    truthIsFromLightFlav = has_in( "FromLightFlav", iflags, "SelectTruthObject" )
    truthIsFromTau = has_in( "FromTau", iflags, "SelectTruthObject" ) 
    
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

    kwargs.setdefault("truthTool" ,
        acc.popToolsAndMerge( TruthSelectionBaseToolCfg( flags, **kwargs_base ) ) )

    # Additional properties
    # ---------------------
    kwargs_setdefault( kwargs,  "maxEta", iflags, "truthMaxEta" )
    kwargs_setdefault( kwargs,  "minEta", iflags, "truthMinEta" )
    kwargs_setdefault( kwargs,  "minPhi", iflags, "truthMinPhi" )
    kwargs_setdefault( kwargs,  "maxPhi", iflags, "truthMaxPhi" )
    kwargs_setdefault( kwargs,  "minD0",  iflags, "truthMinD0" )
    kwargs_setdefault( kwargs,  "maxD0",  iflags, "truthMaxD0" )
    kwargs_setdefault( kwargs,  "minZ0",  iflags, "truthMinZ0" )
    kwargs_setdefault( kwargs,  "maxZ0",  iflags, "truthMaxZ0" )
    kwargs_setdefault( kwargs,  "minQoPT",    iflags, "truthMinQoPT" )
    kwargs_setdefault( kwargs,  "maxQoPT",    iflags, "truthMaxQoPT" )
    kwargs_setdefault( kwargs,  "minAbsEta",  iflags, "truthMinAbsEta" )
    kwargs_setdefault( kwargs,  "minAbsPhi",  iflags, "truthMinAbsPhi" )
    kwargs_setdefault( kwargs,  "maxAbsPhi",  iflags, "truthMaxAbsPhi" )
    kwargs_setdefault( kwargs,  "minAbsD0",   iflags, "truthMinAbsD0" )
    kwargs_setdefault( kwargs,  "maxAbsD0",   iflags, "truthMaxAbsD0" )
    kwargs_setdefault( kwargs,  "minAbsZ0",   iflags, "truthMinAbsZ0" )
    kwargs_setdefault( kwargs,  "maxAbsZ0",   iflags, "truthMaxAbsZ0" )
    kwargs_setdefault( kwargs,  "minAbsQoPT", iflags, "truthMinAbsQoPT" )
    kwargs_setdefault( kwargs,  "maxAbsQoPT", iflags, "truthMaxAbsQoPT" )

    kwargs.setdefault( "isHadron", truthIsHadron )
    kwargs.setdefault( "isPion",   truthIsPion )
    kwargs.setdefault( "isFromB",  truthIsFromB )
    kwargs.setdefault( "isFromC",  truthIsFromC )
    kwargs.setdefault( "isFromHeavyFlav", truthIsFromHeavyFlav )
    kwargs.setdefault( "isFromLightFlav", truthIsFromLightFlav )
    kwargs.setdefault( "isFromTau", truthIsFromTau )
    if truthMinParentPt!=-9999.    :kwargs.setdefault( "minParentPt", truthMinParentPt )
    if truthMaxParentPt!=-9999.    :kwargs.setdefault( "maxParentPt", truthMaxParentPt )

    acc.setPrivateTools( CompFactory.IDTPM.TruthQualitySelectionTool( iname, **kwargs ) )

    return acc


def TrackQualitySelectionToolCfg( flags, name="TrackQualitySelectionTool", **kwargs ):
    acc = ComponentAccumulator()

    iflags, iname = get_flags( flags, name )

    # don't understand the logic of all this - it is a bit of a mess
        
    ## Offline tracks quality selection
    #    if iflags.OfflineQualityWP != "" or iflags.DoOfflineSelection:
    if get_opt(iflags, "OfflineQualityWP", "") != "" or get_opt(iflags, "DoOfflineSelection", False):
        kwargs.setdefault(  "DoOfflineSelection", True )

        # naming insanity !!!!
        kwargs.setdefault( "OfflineSelectionTool", acc.popToolsAndMerge(
            OfflineQualitySelectionCfg( flags, name="OfflineSelectionTool_"+sanitise(name) ) ) )

    ## Truth particles quality selection
    # wtf is this ??? Just because it is MC does not mean that we ALWAYS
    # want the truth selection ?
    # there should be a flag DoTruthSelection so that
    # if flags.Input.isMC and DoTruthSelection: kwargs.setdefault( "DoTruthSelection", True )
    if get_opt( flags, "Input.isMC", False ):
        kwargs.setdefault( "DoTruthSelection", True )
    
        kwargs.setdefault(  "TruthSelectionTool", acc.popToolsAndMerge(
            TruthQualitySelectionToolCfg( flags, name="TruthQualitySelectionTool_"+sanitise(name) ) ) )

    ## offline track-object selection
    if get_opt( iflags, "SelectOfflineObject", False ):
        kwargs.setdefault( "DoObjectSelection", True )
    
        if "TrackObjectSelectionTool" not in kwargs:
           kwargs.setdefault( "TrackObjectSelectionTool", 
                              acc.popToolsAndMerge( TrackObjectSelectionToolCfg( flags, name="TrackObjectSelectionTool_" + sanitise(name) ) ) )

    acc.setPrivateTools( CompFactory.IDTPM.TrackQualitySelectionTool( name, **kwargs ) )
    return acc


def VertexQualitySelectionToolCfg( flags, name="VertexQualitySelectionTool", **kwargs ):
    acc = ComponentAccumulator()

    iflags, iname = get_flags( flags, name )

    ## TODO: here other selector tools for vertices

    acc.setPrivateTools( CompFactory.IDTPM.VertexQualitySelectionTool( iname, **kwargs ) )
    return acc
 
