# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
# Author: Vadim Kostyukhin vadim.kostyukhin@cern.ch

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from TrkConfig.TrkVKalVrtFitterConfig import TrkVKalVrtFitterCfg
from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
from TrackToVertex.TrackToVertexConfig import TrackToVertexCfg
from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
 
from AthenaCommon.Logging import logging
mlog = logging.getLogger('Rec__NewVrtSecInclusiveConfig')

################################################################### 
# Search for low-pt (soft) B-hadron vertices. 
#------------------------------------
def SoftBFinderToolCfg(flags, name="SoftBFinderTool", **myargs):
 
    mlog.info("entering SoftBFinderTool configuration")
    acc = ComponentAccumulator()
    acc.merge(BeamSpotCondAlgCfg(flags))
    
    #-- 2-track vertex initial selector
    iniV2Targs = {}
    iniV2Targs.setdefault("Vrt2TrPtMin" , 1000.)
    iniV2Targs.setdefault("useVertexCleaning"  ,  True)
    iniV2Targs.setdefault("cosSVPVCut"  ,  0.4)
    iniV2Targs.setdefault("v2tBDTCut"   , -0.7)
    iniV2TSelector = CompFactory.Rec.TwoTrackVrtBDTSelector("IniV2TSelector",**iniV2Targs)
    #-- 2-track vertex final selector
    finV2Targs = {}
    finV2Targs.setdefault("Vrt2TrPtMin" , 1000.)
    finV2Targs.setdefault("useVertexCleaning"  ,  True)
    finV2Targs.setdefault("cosSVPVCut"  ,  0.4)
    finV2Targs.setdefault("v2tBDTCut"   ,  0.)
    finV2TSelector = CompFactory.Rec.TwoTrackVrtBDTSelector("FinV2TSelector",**finV2Targs)

    #-- NVSI track selection cuts
    myargs.setdefault("CutPt" , 500.)
    myargs.setdefault("CutBLayHits" , 1 )
    myargs.setdefault("CutPixelHits" , 3 )
    myargs.setdefault("CutSiHits" ,  8 )
    myargs.setdefault("CutTRTHits" , 10 )
    myargs.setdefault("AntiPileupSigRCut" ,  2.0)
    myargs.setdefault("TrkSigCut"      ,  2.0)
    #-- NVSI inclusive vertex selection
    myargs.setdefault("FastZSVCut"     ,  5.)        # Fast universal preselection of 2-track vertices 
    myargs.setdefault("MultiWithOneTrkVrt" ,  True)
    myargs.setdefault("removeTrkMatSignif" , -1.)    # No additional material rejection
    myargs.setdefault("SelVrtSigCut"   ,  2.5)

    myargs.setdefault("VertexMergeCut" , 4.)
    myargs.setdefault("MaxSVRadiusCut" , 50.)
    #-- Tools
    myargs.setdefault("TwoTrkVtxSelectorIni",iniV2TSelector)
    myargs.setdefault("TwoTrkVtxSelectorFinal",finV2TSelector)
    myargs.setdefault("VertexFitterTool", acc.popToolsAndMerge(TrkVKalVrtFitterCfg(flags)))
    myargs.setdefault("ExtrapolatorName", acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags)))
    myargs.setdefault("TrackToVertexTool", acc.popToolsAndMerge(TrackToVertexCfg(flags)))

    SoftBFinder = CompFactory.Rec.NewVrtSecInclusiveTool(name,**myargs)
    acc.setPrivateTools(SoftBFinder)
    mlog.info("SoftBFinderTool created")
 
    return acc

################################################################### 
# Configuration for B-hadron search in the ttbar phase space
#------------------------------------
def InclusiveBFinderToolCfg(flags, name="InclusiveBFinderTool", **myargs):

    mlog.info("entering InclusiveBFinderTool configuration")
    acc = ComponentAccumulator()
    acc.merge(BeamSpotCondAlgCfg(flags))

    #-- 2-track vertex initial selector
    iniV2Targs = {}
    iniV2Targs.setdefault("Vrt2TrPtMin" , 1000.)
    iniV2Targs.setdefault("cosSVPVCut"  ,  0.5)
    iniV2Targs.setdefault("v2tBDTCut"   , -0.7)
    iniV2TSelector = CompFactory.Rec.TwoTrackVrtBDTSelector("IniV2TSelector",**iniV2Targs)
    #-- 2-track vertex final selector
    finV2Targs = {}
    finV2Targs.setdefault("Vrt2TrPtMin" , 1000.)
    finV2Targs.setdefault("cosSVPVCut"  ,  0.5)
    finV2Targs.setdefault("v2tBDTCut"   , -0.2)
    finV2TSelector = CompFactory.Rec.TwoTrackVrtBDTSelector("FinV2TSelector",**finV2Targs)

    #-- NVSI track selection cuts
    myargs.setdefault("CutPt"       , 500.)
    myargs.setdefault("CutBLayHits" , 0 )
    myargs.setdefault("CutPixelHits", 2 )
    myargs.setdefault("CutSiHits"   , 8 )
    myargs.setdefault("CutTRTHits"  , 10 )
    myargs.setdefault("AntiPileupSigRCut" ,  2.0)
    myargs.setdefault("TrkSigCut"      ,  2.0)
    #-- NVSI inclusive vertex selection
    myargs.setdefault("FastZSVCut"     ,  8.0)        # Fast universal preselection of 2-track vertices 
    myargs.setdefault("MultiWithOneTrkVrt" ,  True)
    myargs.setdefault("removeTrkMatSignif" , -1.)     # No additional material rejection
    myargs.setdefault("SelVrtSigCut"   ,  3.0)


    #-- Tools
    myargs.setdefault("TwoTrkVtxSelectorIni",  iniV2TSelector)
    myargs.setdefault("TwoTrkVtxSelectorFinal",finV2TSelector)
    myargs.setdefault("VertexFitterTool",  acc.popToolsAndMerge(TrkVKalVrtFitterCfg(flags)))
    myargs.setdefault("ExtrapolatorName",  acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags)))
    myargs.setdefault("TrackToVertexTool", acc.popToolsAndMerge(TrackToVertexCfg(flags)))


    InclusiveBFinder = CompFactory.Rec.NewVrtSecInclusiveTool(name,**myargs)
    acc.setPrivateTools(InclusiveBFinder)
    mlog.info("InclusiveBFinderTool created")
 
    return acc


################################################################### 
# Configuration for B-hadron search in the high-pt phase space
#------------------------------------
def HighPtBFinderToolCfg(flags, name="HighPtBFinderTool", **myargs):

    mlog.info("entering HighPtBFinderTool configuration")
    acc = ComponentAccumulator()
    acc.merge(BeamSpotCondAlgCfg(flags))

    #-- 2-track vertex initial selector
    iniV2Targs = {}
    iniV2Targs.setdefault("Vrt2TrPtMin" , 1000.)
    iniV2Targs.setdefault("cosSVPVCut"  ,  0.7)
    iniV2Targs.setdefault("v2tBDTCut"   , -0.6)
    iniV2TSelector = CompFactory.Rec.TwoTrackVrtBDTSelector("IniV2TSelector",**iniV2Targs)
    #-- 2-track vertex final selector
    finV2Targs = {}
    finV2Targs.setdefault("Vrt2TrPtMin" , 1000.)
    finV2Targs.setdefault("cosSVPVCut"  ,  0.7)
    finV2Targs.setdefault("v2tBDTCut"   , -0.2)
    finV2TSelector = CompFactory.Rec.TwoTrackVrtBDTSelector("FinV2TSelector",**finV2Targs)

    #-- NVSI track selection cuts
    myargs.setdefault("CutPt"       , 1000.)
    myargs.setdefault("CutBLayHits" , 0 )
    myargs.setdefault("CutPixelHits", 2 )
    myargs.setdefault("CutSiHits"   , 8 )
    myargs.setdefault("CutTRTHits"  , 10 )
    myargs.setdefault("AntiPileupSigRCut", 2.0)
    myargs.setdefault("TrkSigCut"      ,   2.0)
    #-- NVSI inclusive vertex selection
    myargs.setdefault("FastZSVCut"     ,  8.0)        # Fast universal preselection of 2-track vertices 
    myargs.setdefault("MultiWithOneTrkVrt" ,  True)
    myargs.setdefault("removeTrkMatSignif" , -1.)     # No additional material rejection
    myargs.setdefault("SelVrtSigCut"   ,  3.0)

    #-- Tools
    myargs.setdefault("TwoTrkVtxSelectorIni",  iniV2TSelector)
    myargs.setdefault("TwoTrkVtxSelectorFinal",finV2TSelector)
    myargs.setdefault("VertexFitterTool",  acc.popToolsAndMerge(TrkVKalVrtFitterCfg(flags)))
    myargs.setdefault("ExtrapolatorName",  acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags)))
    myargs.setdefault("TrackToVertexTool", acc.popToolsAndMerge(TrackToVertexCfg(flags)))

    HighPtBFinder = CompFactory.Rec.NewVrtSecInclusiveTool(name,**myargs)
    acc.setPrivateTools(HighPtBFinder)
    mlog.info("HighPtBFinderTool created")
 
    return acc


################################################################### 
# Configuration for hadronic interactions in ID material studies
#------------------------------------
def MaterialSVFinderToolCfg(flags, name="MaterialSVFinderTool", **myargs):

    mlog.info("entering MaterialSVFinderTool configuration")
    acc = ComponentAccumulator()
    acc.merge(BeamSpotCondAlgCfg(flags))

    #-- 2-track vertex initial selector
    iniV2Targs = {}
    iniV2Targs.setdefault("Vrt2TrPtMin" , 1000.)
    iniV2Targs.setdefault("cosSVPVCut"  ,  0.)
    iniV2Targs.setdefault("useVertexCleaning", False)
    iniV2Targs.setdefault("v2tBDTCut"   , -1.01)       #Remove BDT selection
    iniV2Targs.setdefault("Vrt2TrMassLimit", 8000.)
    iniV2TSelector = CompFactory.Rec.TwoTrackVrtBDTSelector("IniV2TSelector",**iniV2Targs)
    #-- 2-track vertex final selector
    finV2Targs = {}
    finV2Targs.setdefault("Vrt2TrPtMin" , 1000.)
    finV2Targs.setdefault("cosSVPVCut"  ,  0.)
    finV2Targs.setdefault("useVertexCleaning", False)
    finV2Targs.setdefault("v2tBDTCut"   , -1.01)       #Remove BDT selection
    finV2Targs.setdefault("Vrt2TrMassLimit", 8000.)
    finV2TSelector = CompFactory.Rec.TwoTrackVrtBDTSelector("FinV2TSelector",**finV2Targs)

    #-- NVSI track selection cuts
    myargs.setdefault("CutPt"        , 500.)
    myargs.setdefault("CutBLayHits"  , 0 )
    myargs.setdefault("CutPixelHits" , 1 )
    myargs.setdefault("CutSiHits"    , 8 )
    myargs.setdefault("CutTRTHits"   , 10 )
    myargs.setdefault("AntiPileupSigRCut", 5.0)
    myargs.setdefault("TrkSigCut"      ,   2.0)
    #-- NVSI inclusive vertex selection
    myargs.setdefault("FastZSVCut"     ,  10.0)        # Fast universal preselection of 2-track vertices 
    myargs.setdefault("MultiWithOneTrkVrt" ,  False)
    myargs.setdefault("removeTrkMatSignif" , -1.)     # No additional material rejection
    myargs.setdefault("SelVrtSigCut"   ,  10.0)
    myargs.setdefault("VrtMassLimit", 8000.)

    #-- Tools
    myargs.setdefault("TwoTrkVtxSelectorIni",  iniV2TSelector)
    myargs.setdefault("TwoTrkVtxSelectorFinal",finV2TSelector)
    myargs.setdefault("VertexFitterTool",  acc.popToolsAndMerge(TrkVKalVrtFitterCfg(flags)))
    myargs.setdefault("ExtrapolatorName",  acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags)))
    myargs.setdefault("TrackToVertexTool", acc.popToolsAndMerge(TrackToVertexCfg(flags)))

    MaterialSVFinder = CompFactory.Rec.NewVrtSecInclusiveTool(name,**myargs)
    acc.setPrivateTools(MaterialSVFinder)
    mlog.info("MaterialSVFinderTool created")
 
    return acc

#######################################################################
# Configuration for Ks -> pi pi search using LRT 
#------------------------------------
def KsFinderToolCfg(flags, name="KsFinderTool", **myargs):

    mlog.info("entering KsFinderTool configuration")
    acc = ComponentAccumulator()
    acc.merge(BeamSpotCondAlgCfg(flags))

    #-- 2-track vertex initial selector
    iniV2Targs = {}
    iniV2Targs.setdefault("Vrt2TrPtMin" , 1000.)
    iniV2Targs.setdefault("cosSVPVCut"  ,  0.)
    iniV2Targs.setdefault("v2tBDTCut"   , -1.01)
    iniV2Targs.setdefault("MaxSVRadiusCut", 350.)
    iniV2Targs.setdefault("Vrt2TrMassLimit", 1000.)
    iniV2Targs.setdefault("useVertexCleaning"  , False)
    iniV2TSelector = CompFactory.Rec.TwoTrackVrtBDTSelector("IniV2TSelector",**iniV2Targs)

    #-- NVSI track selection cuts
    myargs.setdefault("CutPt"       , 1000.)
    myargs.setdefault("CutBLayHits" , 0 )
    myargs.setdefault("CutPixelHits", 0 )
    myargs.setdefault("CutSiHits"   , 8 )
    myargs.setdefault("CutTRTHits"  , 0 )
    myargs.setdefault("AntiPileupSigRCut", 6.0)
    myargs.setdefault("TrkSigCut"      ,  10.0)
    myargs.setdefault("CutD0Max"       , 1000.)   # Maximal track impact parameter
    myargs.setdefault("CutD0Min"       , 0.)      # Minimal track impact parameter
    myargs.setdefault("MaxZVrt"        , 100.)
    myargs.setdefault("MinZVrt"        , 0.)
    #-- NVSI inclusive vertex selection
    myargs.setdefault("FastZSVCut"     ,  30.0)        # Fast universal preselection of 2-track vertices 
    myargs.setdefault("MultiWithOneTrkVrt" ,  False)
    myargs.setdefault("removeTrkMatSignif" , -1.)     # No additional material rejection
    myargs.setdefault("SelVrtSigCut"   , 8.0)
    myargs.setdefault("VertexMergeCut" , 10.)
    myargs.setdefault("VrtMassLimit"   , 800000.)
    myargs.setdefault("MaxSVRadiusCut" , 350.)

    #-- Tools
    myargs.setdefault("TwoTrkVtxSelectorIni"  ,iniV2TSelector)
    myargs.setdefault("TwoTrkVtxSelectorFinal",iniV2TSelector) #reuse the same tool for final selection
    myargs.setdefault("VertexFitterTool",  acc.popToolsAndMerge(TrkVKalVrtFitterCfg(flags)))
    myargs.setdefault("ExtrapolatorName",  acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags)))
    myargs.setdefault("TrackToVertexTool", acc.popToolsAndMerge(TrackToVertexCfg(flags)))

    KsFinder = CompFactory.Rec.NewVrtSecInclusiveTool(name,**myargs)
    acc.setPrivateTools(KsFinder)
    mlog.info("KsFinderTool created")

    return acc

#######################################################################
# Configuration for LLP search using LRT 
#------------------------------------
def DVFinderToolCfg(flags, name="DVFinderTool", **myargs):

    mlog.info("entering DVFinderTool configuration")
    acc = ComponentAccumulator()
    acc.merge(BeamSpotCondAlgCfg(flags))

    #-- 2-track vertex initial selector
    iniV2Targs = {}
    iniV2Targs.setdefault("Vrt2TrPtMin" , 2000.)
    iniV2Targs.setdefault("cosSVPVCut"  ,  0.)
    iniV2Targs.setdefault("v2tBDTCut"   , -1.01)
    iniV2Targs.setdefault("MaxSVRadiusCut", 350.)
    iniV2Targs.setdefault("Vrt2TrMassLimit", 1000000.)
    iniV2Targs.setdefault("useVertexCleaning"  , False)
    iniV2TSelector = CompFactory.Rec.TwoTrackVrtBDTSelector("IniV2TSelector",**iniV2Targs)
    #-- 2-track vertex final selector
    finV2Targs = {}
    finV2Targs.setdefault("Vrt2TrPtMin" , 2000.)
    finV2Targs.setdefault("cosSVPVCut"  ,  0.)
    finV2Targs.setdefault("v2tBDTCut"   , -1.01)
    finV2Targs.setdefault("MaxSVRadiusCut", 350.)
    finV2Targs.setdefault("Vrt2TrMassLimit", 1000000.)
    finV2Targs.setdefault("useVertexCleaning"  , False)
    finV2TSelector = CompFactory.Rec.TwoTrackVrtBDTSelector("FinV2TSelector",**finV2Targs)

    #-- NVSI track selection cuts
    myargs.setdefault("CutPt"       , 1000.)
    myargs.setdefault("CutBLayHits" , 0 )
    myargs.setdefault("CutPixelHits", 0 )
    myargs.setdefault("CutSiHits"   , 7 )
    myargs.setdefault("CutTRTHits"  , 0 )
    myargs.setdefault("AntiPileupSigRCut", 6.0)
    myargs.setdefault("TrkSigCut"      ,  10.0)
    myargs.setdefault("CutD0Max"       , 1000.)   # Maximal track impact parameter
    myargs.setdefault("CutD0Min"       , 0.)      # Minimal track impact parameter
    myargs.setdefault("MaxZVrt"        , 100.)
    myargs.setdefault("MinZVrt"        , 0.)
    #-- NVSI inclusive vertex selection
    myargs.setdefault("FastZSVCut"     ,  30.0)        # Fast universal preselection of 2-track vertices 
    myargs.setdefault("MultiWithOneTrkVrt" ,  False)
    myargs.setdefault("removeTrkMatSignif" , -1.)     # No additional material rejection
    myargs.setdefault("SelVrtSigCut"   , 8.0)
    myargs.setdefault("VertexMergeCut" , 10.)
    myargs.setdefault("VrtMassLimit"   , 1000000.)
    myargs.setdefault("MaxSVRadiusCut" , 350.)

    #-- Tools
    myargs.setdefault("TwoTrkVtxSelectorIni"  ,iniV2TSelector)
    myargs.setdefault("TwoTrkVtxSelectorFinal",finV2TSelector)
    myargs.setdefault("VertexFitterTool",  acc.popToolsAndMerge(TrkVKalVrtFitterCfg(flags)))
    myargs.setdefault("ExtrapolatorName",  acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags)))
    myargs.setdefault("TrackToVertexTool", acc.popToolsAndMerge(TrackToVertexCfg(flags)))

    DVFinder = CompFactory.Rec.NewVrtSecInclusiveTool(name,**myargs)
    acc.setPrivateTools(DVFinder)
    mlog.info("DVFinderTool created")

    return acc

##########################################################################################################
# Configuration for creation of calibration ntuples for 2-track vertex classification BDT
# Version for B-hadrons
#----------------------------
def V2TCalibrationToolCfg(flags, name="V2TCalibrationTool", **myargs):

    mlog.info("entering V2TCalibrationTool configuration")
    acc = ComponentAccumulator()
    acc.merge(BeamSpotCondAlgCfg(flags))

    #-- 2-track vertex initial selector
    iniV2Targs = {}
    iniV2Targs.setdefault("Vrt2TrPtMin" , 2000.)
    iniV2Targs.setdefault("cosSVPVCut"  ,  0.)
    iniV2Targs.setdefault("v2tBDTCut"   , -1.01)
    iniV2Targs.setdefault("Vrt2TrMassLimit", 4000.)
    iniV2Targs.setdefault("useVertexCleaning"  , False)
    iniV2TSelector = CompFactory.Rec.TwoTrackVrtBDTSelector("IniV2TSelector",**iniV2Targs)
    #-- 2-track vertex final selector
    finV2Targs = {}
    finV2Targs.setdefault("Vrt2TrPtMin" , 2000.)
    finV2Targs.setdefault("cosSVPVCut"  ,  0.)
    finV2Targs.setdefault("v2tBDTCut"   , -1.01)
    finV2Targs.setdefault("Vrt2TrMassLimit", 4000.)
    finV2Targs.setdefault("useVertexCleaning"  , False)
    finV2TSelector = CompFactory.Rec.TwoTrackVrtBDTSelector("FinV2TSelector",**finV2Targs)

    myargs.setdefault("FillHist"     , True)
    #-- NVSI track selection cuts
    myargs.setdefault("CutPt"       , 400.)
    myargs.setdefault("CutBLayHits" , 0 )
    myargs.setdefault("CutPixelHits", 1 )
    myargs.setdefault("CutSiHits"   , 8 )
    myargs.setdefault("CutTRTHits"  , 10 )
    myargs.setdefault("AntiPileupSigRCut", 2.0)  # Should be less than TrkSigCut 
    myargs.setdefault("TrkSigCut"      ,   2.0) 
    myargs.setdefault("CutD0Max"       , 100.)   # Maximal track impact parameter
    myargs.setdefault("CutD0Min"       , 0.)      # Minimal track impact parameter
    myargs.setdefault("MaxZVrt"        , 100.)
    myargs.setdefault("MinZVrt"        , 0.)
    #-- NVSI inclusive vertex selection
    myargs.setdefault("FastZSVCut"     ,  15.0)        # Fast universal preselection of 2-track vertices 
    myargs.setdefault("MultiWithOneTrkVrt" ,  False)
    myargs.setdefault("removeTrkMatSignif" , -1.)     # No additional material rejection
    myargs.setdefault("SelVrtSigCut"   , 2.0)
    myargs.setdefault("VertexMergeCut" , 10.)
    myargs.setdefault("VrtMassLimit"   , 5500.)
    myargs.setdefault("MaxSVRadiusCut" , 140.)

    #-- Tools
    myargs.setdefault("TwoTrkVtxSelectorIni"  ,iniV2TSelector)
    myargs.setdefault("TwoTrkVtxSelectorFinal",finV2TSelector)
    myargs.setdefault("VertexFitterTool",  acc.popToolsAndMerge(TrkVKalVrtFitterCfg(flags)))
    myargs.setdefault("ExtrapolatorName",  acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags)))
    myargs.setdefault("TrackToVertexTool", acc.popToolsAndMerge(TrackToVertexCfg(flags)))

    V2TCalibration = CompFactory.Rec.NewVrtSecInclusiveTool(name,**myargs)
    acc.setPrivateTools(V2TCalibration)
    mlog.info("V2TCalibrationTool created")

    return acc


