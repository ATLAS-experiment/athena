# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
# Configuration of InDetTrackSelectorTool package

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod, BeamType

def InDetConversionTrackSelectorToolCfg(flags, name="TrackSelector", **kwargs):
    acc = ComponentAccumulator()

    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
        kwargs.setdefault("Extrapolator", acc.popToolsAndMerge(
            AtlasExtrapolatorCfg(flags)))

    kwargs.setdefault("RatioCut1", flags.Egamma.PhotonConv.TrkSel.RatioCut1)
    kwargs.setdefault("RatioCut2", flags.Egamma.PhotonConv.TrkSel.RatioCut2)
    kwargs.setdefault("RatioCut3", flags.Egamma.PhotonConv.TrkSel.RatioCut3)
    kwargs.setdefault("TRTTrksBinnedRatioTRT",
                      flags.Egamma.PhotonConv.TrkSel.TRTTrksBinnedRatioTRT)
    kwargs.setdefault("TRTTrksEtaBins",
                      flags.Egamma.PhotonConv.TrkSel.TRTTrksEtaBins)
    kwargs.setdefault("RatioTRT", flags.Egamma.PhotonConv.TrkSel.RatioTRT)
    kwargs.setdefault("minPt",    flags.Egamma.PhotonConv.TrkSel.minPt)

    kwargs.setdefault("maxSiD0",  10000.)
    kwargs.setdefault("maxSiZ0",  10000.)
    kwargs.setdefault("maxTrtD0", 10000.)
    kwargs.setdefault("maxTrtZ0", 10000.)
    kwargs.setdefault("significanceD0_Si", -1.)
    kwargs.setdefault("RatioV0", -1.)
    kwargs.setdefault("IsConversion", True)

    acc.setPrivateTools(
        CompFactory.InDet.InDetConversionTrackSelectorTool(name, **kwargs))
    return acc

def V0InDetConversionTrackSelectorToolCfg(flags, name='InDetV0VxTrackSelector', **kwargs):
    acc = ComponentAccumulator()

    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
        kwargs.setdefault("Extrapolator", acc.popToolsAndMerge(
            AtlasExtrapolatorCfg(flags)))

    kwargs.setdefault("maxTrtD0", 50.)
    kwargs.setdefault("maxSiZ0",  250.)
    kwargs.setdefault("significanceD0_Si", 1.)
    kwargs.setdefault("significanceD0_Trt", 1.)
    kwargs.setdefault("significanceZ0_Trt", 3.)
    kwargs.setdefault("minPt", 400.0)
    kwargs.setdefault("IsConversion", False)
    kwargs.setdefault("UseEventInfoBS", True)

    acc.setPrivateTools(
        CompFactory.InDet.InDetConversionTrackSelectorTool(name, **kwargs))
    return acc

def InDetV0VxTrackSelectorLooseCfg(flags, name='InDetV0VxTrackSelectorLoose', **kwargs):
    acc = ComponentAccumulator()

    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import InDetExtrapolatorCfg
        kwargs.setdefault("Extrapolator", acc.popToolsAndMerge(
            InDetExtrapolatorCfg(flags)))

    kwargs.setdefault("maxSiD0"           , 99999.)
    kwargs.setdefault("maxTrtD0"          , 99999.)
    kwargs.setdefault("maxSiZ0"           , 99999.)
    kwargs.setdefault("maxTrtZ0"          , 99999.)
    kwargs.setdefault("minPt"             , 500.0)
    kwargs.setdefault("significanceD0_Si" , 0.)
    kwargs.setdefault("significanceD0_Trt", 0.)
    kwargs.setdefault("significanceZ0_Trt", 0.)
    kwargs.setdefault("IsConversion"      , False)

    acc.setPrivateTools(
        CompFactory.InDet.InDetConversionTrackSelectorTool(name, **kwargs))
    return acc


def InDetTrackSelectorToolCfg(flags, name='InDetTrackSelectorTool', **kwargs):
    result = ComponentAccumulator()

    if "UseEventInfoBS" not in kwargs or kwargs["UseEventInfoBS"] is False:
        # To produce the input InDet::BeamSpotData CondHandle
        from BeamSpotConditions.BeamSpotConditionsConfig import (
            BeamSpotCondAlgCfg)
        result.merge(BeamSpotCondAlgCfg(flags))

    # To produce the input AtlasFieldCacheCondObj
    from MagFieldServices.MagFieldServicesConfig import (
        AtlasFieldCacheCondAlgCfg)
    result.merge(AtlasFieldCacheCondAlgCfg(flags))

    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
        kwargs.setdefault("Extrapolator", result.popToolsAndMerge(
            AtlasExtrapolatorCfg(flags)))

    result.setPrivateTools(
        CompFactory.InDet.InDetDetailedTrackSelectorTool(name, **kwargs))
    return result

def InDetImprovedJetFitterTrackSelectorToolCfg(flags, name='InDetImprovedJFTrackSelTool', **kwargs):
    kwargs.setdefault("pTMin",      500.0)
    kwargs.setdefault("IPd0Max",    7.0)
    kwargs.setdefault("IPz0Max",    10.0)
    kwargs.setdefault("sigIPd0Max", 0.35)
    kwargs.setdefault("sigIPz0Max", 2.5)
    kwargs.setdefault("etaMax",     9999.0)
    kwargs.setdefault("nHitBLayer", 0)
    kwargs.setdefault("nHitPix",    1)
    kwargs.setdefault("nHitSct",
                      4 if flags.GeoModel.Run < LHCPeriod.Run4 else 0)
    kwargs.setdefault("nHitSi",     7)
    kwargs.setdefault("nHitTrt",    0)
    kwargs.setdefault("fitChi2OnNdfMax", 3.5)
    kwargs.setdefault("useTrackSummaryInfo", True)
    kwargs.setdefault("useSharedHitInfo",    False)
    kwargs.setdefault("useTrackQualityInfo", True)
    kwargs.setdefault("TrackSummaryTool", "")
    return InDetTrackSelectorToolCfg(flags, name, **kwargs)

def MuonCombinedInDetDetailedTrackSelectorToolCfg(flags, name="MuonCombinedInDetDetailedTrackSelectorTool", **kwargs):
    if flags.Beam.Type is BeamType.Collisions:
        kwargs.setdefault("pTMin",             2000)
        kwargs.setdefault("nHitBLayer",        0)
        kwargs.setdefault("nHitBLayerPlusPix", 0)
        kwargs.setdefault("nHitTrt",           0)
        kwargs.setdefault("useTrackQualityInfo", False)
        if flags.Muon.MuonTrigger:
            kwargs.setdefault("IPd0Max",       19999.0)
            kwargs.setdefault("IPz0Max",       19999.0)
            kwargs.setdefault("z0Max",         19999.0)
            kwargs.setdefault("nHitPix",       0)
            kwargs.setdefault("nHitSct",       0)
            kwargs.setdefault("nHitSi",        0)
            kwargs.setdefault("useTrackSummaryInfo", False)
        else:
            kwargs.setdefault("IPd0Max",       50.0)
            kwargs.setdefault("IPz0Max",       9999.0)
            kwargs.setdefault("z0Max",         9999.0)
            kwargs.setdefault("nHitPix",       1)
            kwargs.setdefault("nHitSct",       3)
            kwargs.setdefault("nHitSi",        4)
            kwargs.setdefault("useTrackSummaryInfo", True)
    else:
        kwargs.setdefault("pTMin",             500)
        kwargs.setdefault("IPd0Max",           19999.0)
        kwargs.setdefault("IPz0Max",           19999.0)
        kwargs.setdefault("z0Max",             19999.0)
        kwargs.setdefault("useTrackSummaryInfo", False)
        kwargs.setdefault("useTrackQualityInfo", False)

    kwargs.setdefault("TrackSummaryTool", "")
    return InDetTrackSelectorToolCfg(flags, name, **kwargs)

def MuonCombinedInDetDetailedTrackSelectorTool_LRTCfg(flags, name='MuonCombinedInDetDetailedTrackSelectorTool_LRT', **kwargs):
    kwargs.setdefault("pTMin",      2000)
    kwargs.setdefault("IPd0Max",    1.e4)
    kwargs.setdefault("IPz0Max",    1.e4)
    kwargs.setdefault("z0Max",      1.e4)
    kwargs.setdefault("nHitBLayer", 0)
    kwargs.setdefault("nHitPix",    0)
    kwargs.setdefault("nHitBLayerPlusPix", 0)
    kwargs.setdefault("nHitSct",    4)
    kwargs.setdefault("nHitSi",     4)
    kwargs.setdefault("nHitTrt",    0)
    kwargs.setdefault("useTrackSummaryInfo", True)
    kwargs.setdefault("useTrackQualityInfo", False)
    return MuonCombinedInDetDetailedTrackSelectorToolCfg(flags, name, **kwargs)

def MuonCombinedInDetDetailedForwardTrackSelectorToolCfg(flags, name='MuonCombinedInDetDetailedForwardTrackSelectorTool', **kwargs):
    kwargs.setdefault("nHitSct", 0)
    return MuonCombinedInDetDetailedTrackSelectorToolCfg(flags, name, **kwargs)

def CaloTrkMuIdAlgTrackSelectorToolCfg(flags, name='CaloTrkMuIdAlgTrackSelectorTool', **kwargs):
    result = ComponentAccumulator()

    kwargs.setdefault("pTMin",      5000.)
    kwargs.setdefault("IPd0Max",    7.)
    kwargs.setdefault("IPz0Max",    130.) # (tuned on Z)
    kwargs.setdefault("nHitBLayer", 0)
    kwargs.setdefault("nHitPix",    1)
    kwargs.setdefault("nHitSct",    5)
    kwargs.setdefault("nHitSi",     7)
    kwargs.setdefault("nHitTrt",    0)

    from TrkConfig.TrkTrackSummaryToolConfig import (
        MuonCombinedTrackSummaryToolCfg)
    kwargs.setdefault("TrackSummaryTool", result.popToolsAndMerge(
        MuonCombinedTrackSummaryToolCfg(flags)))

    result.setPrivateTools(result.popToolsAndMerge(
        InDetTrackSelectorToolCfg(flags, name, **kwargs)))
    return result

def TauRecInDetTrackSelectorToolCfg(flags, name='tauRec_InDetTrackSelectorTool', **kwargs):
    kwargs.setdefault("pTMin",      1000.0)
    kwargs.setdefault("IPd0Max",    1.)
    kwargs.setdefault("IPz0Max",    1.5)
    kwargs.setdefault("nHitBLayer", 0)
    kwargs.setdefault("nHitPix",    2) # PixelHits + PixelDeadSensors
    kwargs.setdefault("nHitSct",    0) # SCTHits + SCTDeadSensors
    kwargs.setdefault("nHitSi",     7) # PixelHits + SCTHits + PixelDeadSensors + SCTDeadSensors
    kwargs.setdefault("nHitTrt",    0)
    kwargs.setdefault("fitChi2OnNdfMax", 99999)
    kwargs.setdefault("useTrackSummaryInfo", True)
    kwargs.setdefault("useSharedHitInfo",    False)
    kwargs.setdefault("useTrackQualityInfo", True)
    kwargs.setdefault("TrackSummaryTool", "")
    return InDetTrackSelectorToolCfg(flags, name, **kwargs)

def BPHY_InDetDetailedTrackSelectorToolCfg(flags, name='BPHY_InDetDetailedTrackSelectorTool', **kwargs):
    acc = ComponentAccumulator()

    # Different from other InDetTrackSelectorToolCfg
    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import InDetExtrapolatorCfg
        kwargs.setdefault("Extrapolator", acc.popToolsAndMerge(
            InDetExtrapolatorCfg(flags)))

    kwargs.setdefault("pTMin"                , 400.0)
    kwargs.setdefault("IPd0Max"              , 10000.0)
    kwargs.setdefault("IPz0Max"              , 10000.0)
    kwargs.setdefault("z0Max"                , 10000.0)
    kwargs.setdefault("sigIPd0Max"           , 10000.0)
    kwargs.setdefault("sigIPz0Max"           , 10000.0)
    kwargs.setdefault("d0significanceMax"    , -1.)
    kwargs.setdefault("z0significanceMax"    , -1.)
    kwargs.setdefault("etaMax"               , 9999.)
    kwargs.setdefault("useTrackSummaryInfo"  , True)
    kwargs.setdefault("nHitBLayer"           , 0)
    kwargs.setdefault("nHitPix"              , 1)
    kwargs.setdefault("nHitBLayerPlusPix"    , 1)
    kwargs.setdefault("nHitSct"              , 2)
    kwargs.setdefault("nHitSi"               , 3)
    kwargs.setdefault("nHitTrt"              , 0)
    kwargs.setdefault("nHitTrtHighEFractionMax", 10000.0)
    kwargs.setdefault("useSharedHitInfo"     , False)
    kwargs.setdefault("useTrackQualityInfo"  , True)
    kwargs.setdefault("fitChi2OnNdfMax"      , 10000.0)
    kwargs.setdefault("TrtMaxEtaAcceptance"  , 1.9)
    kwargs.setdefault("UseEventInfoBS"       , True)
    kwargs.setdefault("TrackSummaryTool"     , None)

    acc.setPrivateTools(acc.popToolsAndMerge(InDetTrackSelectorToolCfg(flags, name, **kwargs)))
    return acc


def InDetTRTDriftCircleCutToolCfg(flags, name='InDetTRTDriftCircleCutTool', **kwargs):
    from TRT_ConditionsAlgs.TRT_ConditionsAlgsConfig import TRTActiveCondAlgCfg
    result = TRTActiveCondAlgCfg(flags) # To produce the input TRTCond::ActiveFraction CondHandle
    kwargs.setdefault("MinOffsetDCs", 5)
    kwargs.setdefault("UseNewParameterization",
                      flags.Tracking.ActiveConfig.useNewParameterizationTRT)
    kwargs.setdefault("UseActiveFractionSvc", flags.Detector.EnableTRT)
    result.setPrivateTools(CompFactory.InDet.InDetTrtDriftCircleCutTool(name, **kwargs))
    return result

def InDetTrigTRTDriftCircleCutToolCfg(flags, name='InDetTrigTRTDriftCircleCutTool', **kwargs):
    kwargs.setdefault("UseNewParameterization", True)
    kwargs.setdefault("UseActiveFractionSvc", True)
    return InDetTRTDriftCircleCutToolCfg(flags, name, **kwargs)



# Tool to select tracks to be used for TRT calibration
def InDetDetailedTrackSelectorToolCfg(flags,name="InDetDetailedTrackSelectorTool",**kwargs):
    
    from AthenaCommon.SystemOfUnits import GeV, mm
    
    acc = ComponentAccumulator()      
    kwargs.setdefault("pTMin", 1.0*GeV)     
    kwargs.setdefault("fitChi2OnNdfMax",50.0)
    kwargs.setdefault("z0Max",99999.0*mm)   
    kwargs.setdefault("IPd0Max",10.0*mm)    
    kwargs.setdefault("IPd0Max",300.0*mm)   
    kwargs.setdefault("etaMax",2.1)
    kwargs.setdefault("nHitBLayer",0)
    kwargs.setdefault("nHitPix",2)
    kwargs.setdefault("nHitBLayerPlusPix",0)
    kwargs.setdefault("nHitSct",0)
    kwargs.setdefault("nHitSi",7)
    kwargs.setdefault("nHitTrtPlusOutliers",20)
    kwargs.setdefault("nHitTrtPlusOutliersHighE",20)
    kwargs.setdefault("nHitTrtHighE",0)
    
    from TrkConfig.AtlasExtrapolatorConfig import InDetExtrapolatorCfg
    kwargs.setdefault("Extrapolator", acc.popToolsAndMerge(InDetExtrapolatorCfg(flags)))
    
    acc.setPrivateTools(acc.popToolsAndMerge(InDetTrackSelectorToolCfg(flags, name=name , **kwargs)))
    return acc
