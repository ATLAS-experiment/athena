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

    myargs.setdefault("VertexFitterTool", acc.popToolsAndMerge(TrkVKalVrtFitterCfg(flags)))
    myargs.setdefault("ExtrapolatorName", acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags)))
    myargs.setdefault("TrackToVertexTool", acc.popToolsAndMerge(TrackToVertexCfg(flags)))
    myargs.setdefault("CutPt" , 500.)
    myargs.setdefault("CutBLayHits" , 1 )
    myargs.setdefault("CutPixelHits" , 3 )
    myargs.setdefault("CutSiHits" ,  8 )
    myargs.setdefault("CutTRTHits" , 10 )
    myargs.setdefault("useVertexCleaning"  ,  True)
    myargs.setdefault("MultiWithOneTrkVrt" ,  True)
    myargs.setdefault("removeTrkMatSignif" , -1.)     # No additional material rejection
    myargs.setdefault("AntiPileupSigRCut" ,  2.0)
    myargs.setdefault("TrkSigCut"      ,  2.0)
    myargs.setdefault("SelVrtSigCut"   ,  3.0)
    myargs.setdefault("v2tIniBDTCut"   , -0.7)
    myargs.setdefault("v2tFinBDTCut"   ,  0.0)
    myargs.setdefault("cosSVPVCut"     ,  0.4)
    myargs.setdefault("FastZSVCut"     ,  5.)
    myargs.setdefault("VertexMergeCut" , 4.)
    myargs.setdefault("MaxSVRadiusCut" , 50.)
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

    myargs.setdefault("VertexFitterTool" ,  acc.popToolsAndMerge(TrkVKalVrtFitterCfg(flags)))
    myargs.setdefault("ExtrapolatorName" ,  acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags)))
    myargs.setdefault("TrackToVertexTool", acc.popToolsAndMerge(TrackToVertexCfg(flags)))
    myargs.setdefault("CutPt"        , 500.)
    myargs.setdefault("CutBLayHits"  , 0)
    myargs.setdefault("CutPixelHits" , 2)
    myargs.setdefault("CutSiHits"    , 8)
    myargs.setdefault("CutTRTHits"   , 10)
    myargs.setdefault("useVertexCleaning"  , True)
    myargs.setdefault("MultiWithOneTrkVrt" , True)
    myargs.setdefault("removeTrkMatSignif" , -1.)     # No additional material rejection
    myargs.setdefault("AntiPileupSigRCut"  , 2.0)
    myargs.setdefault("TrkSigCut"          , 2.0)
    myargs.setdefault("SelVrtSigCut"   ,  3.0)
    myargs.setdefault("v2tIniBDTCut"   , -0.7)
    myargs.setdefault("v2tFinBDTCut"   , -0.2)
    myargs.setdefault("cosSVPVCut"     ,  0.5)
    myargs.setdefault("FastZSVCut"     ,  8.)

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

    myargs.setdefault("VertexFitterTool" , acc.popToolsAndMerge(TrkVKalVrtFitterCfg(flags)))
    myargs.setdefault("ExtrapolatorName" , acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags)))
    myargs.setdefault("TrackToVertexTool", acc.popToolsAndMerge(TrackToVertexCfg(flags)))
    myargs.setdefault("CutPt"        , 1000.)
    myargs.setdefault("CutBLayHits"  , 0)
    myargs.setdefault("CutPixelHits" , 2)
    myargs.setdefault("CutSiHits"    , 8)
    myargs.setdefault("CutTRTHits"   , 10)
    myargs.setdefault("useVertexCleaning"  , True)
    myargs.setdefault("MultiWithOneTrkVrt" , True)
    myargs.setdefault("removeTrkMatSignif" , -1.)     # No additional material rejection
    myargs.setdefault("AntiPileupSigRCut"  , 2.0)
    myargs.setdefault("TrkSigCut"      , 2.0)
    myargs.setdefault("SelVrtSigCut"   , 3.0)
    myargs.setdefault("v2tIniBDTCut"   ,-0.6)
    myargs.setdefault("v2tFinBDTCut"   , 0.2)
    myargs.setdefault("cosSVPVCut"     , 0.7)
    myargs.setdefault("FastZSVCut"     , 8.)

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

    myargs.setdefault("VertexFitterTool" , acc.popToolsAndMerge(TrkVKalVrtFitterCfg(flags)))
    myargs.setdefault("ExtrapolatorName" , acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags)))
    myargs.setdefault("TrackToVertexTool", acc.popToolsAndMerge(TrackToVertexCfg(flags)))
    myargs.setdefault("CutPt"        , 500.)
    myargs.setdefault("CutBLayHits"  , 0)
    myargs.setdefault("CutPixelHits" , 1)
    myargs.setdefault("CutSiHits"    , 8)
    myargs.setdefault("CutTRTHits"   , 10)
    myargs.setdefault("useVertexCleaning"  , False)
    myargs.setdefault("MultiWithOneTrkVrt" , False)
    myargs.setdefault("removeTrkMatSignif" , -1.)    # No additional material rejection
    myargs.setdefault("AntiPileupSigRCut"  , 5.0)
    myargs.setdefault("TrkSigCut"      , 5.0)
    myargs.setdefault("SelVrtSigCut"   , 10.0)
    myargs.setdefault("v2tIniBDTCut"   ,-1.01)       #Remove BDT selection
    myargs.setdefault("v2tFinBDTCut"   ,-1.01)       #Remove BDT selection
    myargs.setdefault("cosSVPVCut"     , 0.)
    myargs.setdefault("FastZSVCut"     , 10.)
    myargs.setdefault("VrtMassLimit"   , 8000.)
    myargs.setdefault("Vrt2TrMassLimit", 8000.)

    MaterialSVFinder = CompFactory.Rec.NewVrtSecInclusiveTool(name,**myargs)
    acc.setPrivateTools(MaterialSVFinder)
    mlog.info("MaterialSVFinderTool created")
 
    return acc

#######################################################################
# Configuration for LLP search using LRT 
#------------------------------------
def DVFinderToolCfg(flags, name="DVFinderTool", **myargs):

    mlog.info("entering DVFinderTool configuration")
    acc = ComponentAccumulator()
    acc.merge(BeamSpotCondAlgCfg(flags))

    myargs.setdefault("VertexFitterTool" , acc.popToolsAndMerge(TrkVKalVrtFitterCfg(flags)))
    myargs.setdefault("ExtrapolatorName" , acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags)))
    myargs.setdefault("TrackToVertexTool", acc.popToolsAndMerge(TrackToVertexCfg(flags)))
    myargs.setdefault("CutPt"        , 1000.)
    myargs.setdefault("CutBLayHits"  , 0)
    myargs.setdefault("CutPixelHits" , 0)
    myargs.setdefault("CutSiHits"    , 7)
    myargs.setdefault("CutTRTHits"   , 0)
    myargs.setdefault("useVertexCleaning"  , False)
    myargs.setdefault("MultiWithOneTrkVrt" , False)
    myargs.setdefault("removeTrkMatSignif" , -1.)    # No additional material rejection
    myargs.setdefault("AntiPileupSigRCut"  , 6.0)
    myargs.setdefault("TrkSigCut"      , 10.0)
    myargs.setdefault("SelVrtSigCut"   , 8.0)
    myargs.setdefault("v2tIniBDTCut"   ,-1.01)       # BDT selection is disabled
    myargs.setdefault("v2tFinBDTCut"   ,-1.01)       # BDT selection is disabled
    myargs.setdefault("cosSVPVCut"     , 0.)
    myargs.setdefault("FastZSVCut"     , 30.)
    myargs.setdefault("VrtMassLimit"   , 1000000.)
    myargs.setdefault("Vrt2TrMassLimit", 1000000.)
    myargs.setdefault("VertexMergeCut" , 10.)
    myargs.setdefault("MaxSVRadiusCut" , 350.)
    myargs.setdefault("CutD0Max"       , 1000.)   # Maximal track impact parameter
    myargs.setdefault("CutD0Min"       , 0.)      # Minimal track impact parameter
    myargs.setdefault("MaxZVrt"        , 100.)
    myargs.setdefault("MinZVrt"        , 0.)
    myargs.setdefault("TwoTrkVtxFormingD0Cut", 1.0) # 2-track forming cut
    myargs.setdefault("do2TrkIBLChecks", False)   # Do not explicitly require IBL/BL hits

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

    myargs.setdefault("VertexFitterTool" , acc.popToolsAndMerge(TrkVKalVrtFitterCfg(flags)))
    myargs.setdefault("ExtrapolatorName" , acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags)))
    myargs.setdefault("TrackToVertexTool", acc.popToolsAndMerge(TrackToVertexCfg(flags)))
    myargs.setdefault("FillHist"     , True)
    myargs.setdefault("CutPt"        , 400.)
    myargs.setdefault("CutBLayHits"  , 0)
    myargs.setdefault("CutPixelHits" , 1)
    myargs.setdefault("CutSiHits"    , 8)
    myargs.setdefault("CutTRTHits"   , 10)
    myargs.setdefault("useVertexCleaning"  , False)
    myargs.setdefault("MultiWithOneTrkVrt" , False)
    myargs.setdefault("removeTrkMatSignif" , -1.)    # No additional material rejection
    myargs.setdefault("AntiPileupSigRCut"  , 2.0)
    myargs.setdefault("TrkSigCut"      ,  2.0)
    myargs.setdefault("SelVrtSigCut"   ,  2.0)
    myargs.setdefault("v2tIniBDTCut"   , -1.01)       #Remove BDT selection
    myargs.setdefault("v2tFinBDTCut"   , -1.01)       #Remove BDT selection
    myargs.setdefault("cosSVPVCut"     ,  0.)
    myargs.setdefault("FastZSVCut"     ,  15.)
    myargs.setdefault("VrtMassLimit"   ,  5500.)
    myargs.setdefault("Vrt2TrMassLimit",  4000.)
    myargs.setdefault("MaxSVRadiusCut" ,  140.)
    myargs.setdefault("CutD0Max"       ,  100.)   # Maximal track impact parameter
    myargs.setdefault("CutD0Min"       ,  0.)     # Minimal track impact parameter
    myargs.setdefault("MaxZVrt"        ,  100.)


    V2TCalibration = CompFactory.Rec.NewVrtSecInclusiveTool(name,**myargs)
    acc.setPrivateTools(V2TCalibration)
    mlog.info("V2TCalibrationTool created")

    return acc


