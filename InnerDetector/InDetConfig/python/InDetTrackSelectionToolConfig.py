# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
# Configuration of InDetTrackSelectionTool package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod, BeamType

def InDetTrackSelectionToolCfg(flags, name="InDetTrackSelectionTool", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("maxAbsEta",
                      2.5 if flags.GeoModel.Run <= LHCPeriod.Run3 else 4.0)
    acc.setPrivateTools(CompFactory.InDet.InDetTrackSelectionTool(name, **kwargs))
    return acc

def InDetTrackSelectionTool_TrackTools_Cfg(
        flags, name="InDetTrackSelectionTool", **kwargs):
    acc = ComponentAccumulator()

    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import InDetExtrapolatorCfg
        kwargs.setdefault("Extrapolator", acc.popToolsAndMerge(InDetExtrapolatorCfg(flags)))

    if "TrackSummaryTool" not in kwargs:
        from TrkConfig.TrkTrackSummaryToolConfig import InDetTrackSummaryToolCfg
        kwargs.setdefault("TrackSummaryTool", acc.popToolsAndMerge(InDetTrackSummaryToolCfg(flags)))

    kwargs.setdefault("UseTrkTrackTools", True)

    acc.setPrivateTools(acc.popToolsAndMerge(
        InDetTrackSelectionToolCfg(flags, name, **kwargs)))
    return acc

#############################################
#####  Configs based on CutLevel Loose  #####
#############################################

def InDetTrackSelectionTool_Loose_Cfg(
        flags, name="InDetTrackSelectionTool_Loose", **kwargs):
    kwargs.setdefault("CutLevel", "Loose")
    return InDetTrackSelectionToolCfg(flags, name, **kwargs)

def isoTrackSelectionToolCfg(flags, name="isoTrackSelectionTool", **kwargs):
    kwargs.setdefault("minPt", 1000)
    return InDetTrackSelectionTool_Loose_Cfg(flags, name, **kwargs)

def InDetTrackSelectionTool_Loose_TrackTools_Cfg(
        flags, name="InDetTrackSelectionTool_Loose", **kwargs):
    kwargs.setdefault("CutLevel", "Loose")
    return InDetTrackSelectionTool_TrackTools_Cfg(flags, name, **kwargs)

####################################################
#####  Configs based on CutLevel LoosePrimary  #####
####################################################

def InDetTrackSelectionTool_LoosePrimary_Cfg(
        flags, name="InDetTrackSelectionTool_LoosePrimary", **kwargs):
    kwargs.setdefault("CutLevel", "LoosePrimary")
    return InDetTrackSelectionToolCfg(flags, name, **kwargs)

####################################################
#####  Configs based on CutLevel TightPrimary  #####
####################################################

def InDetTrackSelectionTool_TightPrimary_Cfg(
        flags, name="InDetTrackSelectionTool_TightPrimary", **kwargs):
    kwargs.setdefault("CutLevel", "TightPrimary")
    return InDetTrackSelectionToolCfg(flags, name, **kwargs)

def InDetTrackSelectionTool_HILoose_Cfg(
        flags, name="InDetTrackSelectionTool_HILoose", **kwargs):
    kwargs.setdefault("CutLevel", "HILoose")
    return InDetTrackSelectionToolCfg(flags, name, **kwargs)   

def InDetTrackSelectionTool_HITight_Cfg(
        flags, name="InDetTrackSelectionTool_HITight", **kwargs):
    kwargs.setdefault("CutLevel", "HITight")
    return InDetTrackSelectionToolCfg(flags, name, **kwargs)

def InDetTrackSelectionTool_TightPrimary_TrackTools_Cfg(
        flags, name="InDetTrackSelectionTool_TightPrimary", **kwargs):
    kwargs.setdefault("CutLevel", "TightPrimary")
    return InDetTrackSelectionTool_TrackTools_Cfg(flags, name, **kwargs)

def PFTrackSelectionToolCfg(flags, name="PFTrackSelectionTool", **kwargs):
    kwargs.setdefault("minPt", 500.0)
    return InDetTrackSelectionTool_TightPrimary_Cfg(flags, name, **kwargs)

###############################################
#####  Configs not based on any CutLevel  #####
###############################################

def VtxInDetTrackSelectionCfg(
        flags, name="VertexInDetTrackSelectionTool", **kwargs):
    for key in (
        "maxAbsEta",
        "maxD0",
        "maxNPixelHoles",
        "maxSigmaD0",
        "maxSigmaZ0SinTheta",
        "maxZ0",
        "maxZ0SinTheta",
        "minNInnermostLayerHits",
        "minNPixelHits",
        "minNSctHits",
        "minNSiHits",
        "minNTrtHits",
        "minPt",
    ):
        kwargs.setdefault(key, getattr(flags.Tracking.PriVertex, key))

    kwargs.setdefault("UseTrkTrackTools", False)

    # Cut level = NoCut for a few modes
    if flags.Reco.EnableHI or \
       flags.Tracking.doMinBias or \
       flags.Tracking.doLowMu:
        acc = ComponentAccumulator()
        acc.setPrivateTools(CompFactory.InDet.InDetTrackSelectionTool(name, **kwargs))
        return acc

    # Default is TightPrimary
    else:
        return InDetTrackSelectionTool_TightPrimary_Cfg(flags, name, **kwargs)


def TrigVtxInDetTrackSelectionCfg(flags, name="InDetTrigDetailedTrackSelectionTool", **kwargs):

    acc = ComponentAccumulator()
    import AthenaCommon.SystemOfUnits as Units
    
    kwargs.setdefault("CutLevel", "NoCut")    #fill flags rather than hardcode here
    kwargs.setdefault("minPt",           1.*Units.GeV)
    kwargs.setdefault("maxD0",           4.*Units.mm)
    kwargs.setdefault("maxZ0",        1000.*Units.mm)
    kwargs.setdefault("maxZ0SinTheta",1000.*Units.mm)
    kwargs.setdefault("maxSigmaD0", 5.)
    kwargs.setdefault("maxSigmaZ0SinTheta", 10.)
    kwargs.setdefault("maxChiSqperNdf", 3.5)
    kwargs.setdefault("maxAbsEta", 2.4)
    kwargs.setdefault("minNInnermostLayerHits", 0)
    kwargs.setdefault("minNPixelHits",          1)
    kwargs.setdefault("maxNPixelHoles",         1)
    kwargs.setdefault("minNSctHits",            4)
    kwargs.setdefault("minNTrtHits",            0)
    kwargs.setdefault("minNSiHits", flags.Tracking.ActiveConfig.minNSiHits_vtx)
    # N.B. Legacy config used to set extrapolator + trackSummary tools but since UseTrkTrackTools is not set to True, they're not used in the InDetTrackSelectionTool

    acc.setPrivateTools(CompFactory.InDet.InDetTrackSelectionTool(
        name+flags.Tracking.ActiveConfig.input_name, **kwargs))
    return acc

def Tau_InDetTrackSelectionToolForTJVACfg(flags, name="tauRec_InDetTrackSelectionToolForTJVA", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("minPt", 1000.)
    kwargs.setdefault("minNPixelHits", 2)
    kwargs.setdefault("minNSiHits", 7)
    acc.setPrivateTools(CompFactory.InDet.InDetTrackSelectionTool(name, **kwargs))
    return acc

def InDetGlobalLRTMonAlg_TrackSelectionToolCfg(flags, name="InDetGlobalLRTMonAlg_TrackSelectionTool", **kwargs):
    kwargs.setdefault("minPt", 1000.)
    kwargs.setdefault("maxNPixelHoles", 1)
    return InDetTrackSelectionTool_TrackTools_Cfg(flags, name, **kwargs)

def HI_InDetTrackSelectionToolForHITrackJetsCfg(flags, name="TrackSelHI", **kwargs):
    """Provides track selection tool for HI track jet reconstruction."""
    acc = ComponentAccumulator()
    kwargs.setdefault("minNSiHits", 7)
    kwargs.setdefault("maxAbsEta", 2.5)
    kwargs.setdefault("maxNSiHoles", 2)
    kwargs.setdefault("maxNPixelHoles", 1)
    kwargs.setdefault("minPt", 4000.)
    acc.setPrivateTools(CompFactory.InDet.InDetTrackSelectionTool(name, **kwargs))
    return acc

###############################################
#####  Configs for Sec Vtx  #####
###############################################


def InDetTrackSelectionTool_AMSVF_Cfg(flags, name='InDetTrackSelectionTool_AMSVF', **kwargs):

    kwargs.setdefault("CutLevel", "NoCut")
    kwargs.setdefault("minPt", 1000.)
    kwargs.setdefault("maxD0", 500.0)
    kwargs.setdefault("maxZ0", 1500.)
    kwargs.setdefault("maxSigmaD0", -1.0)
    kwargs.setdefault("maxSigmaZ0SinTheta", -1.0)
    kwargs.setdefault("maxChiSqperNdf", 5.0)
    kwargs.setdefault("maxAbsEta", 2.5)
    kwargs.setdefault("minNInnermostLayerHits", 0)
    kwargs.setdefault("minNPixelHits", 0)
    kwargs.setdefault("maxNPixelHoles", 1)
    kwargs.setdefault("minNSctHits", 2)
    kwargs.setdefault("minNTrtHits", 0)
    kwargs.setdefault("minNSiHits", 0)
    kwargs.setdefault("maxNSiSharedHits", 6)

    return InDetTrackSelectionTool_TrackTools_Cfg(flags, name, **kwargs)

###############################################
#####        Configs for alignment        #####
###############################################


def Align_InDetTrackSelectionToolCfg(flags, name="Align_InDetTrackSelectionTool", **kwargs):
    if flags.Beam.Type is BeamType.Cosmics:
        kwargs.setdefault("minNPixelHits", 1)
        kwargs.setdefault("minNSctHits", 8)
        kwargs.setdefault("minNSiHitsModTop", 2)
        kwargs.setdefault("minNSiHitsModBottom", 2)

    else:
        kwargs.setdefault("minPt", 3000.)
        kwargs.setdefault("maxEtaForTrtHitCuts", 1.9)
        kwargs.setdefault("maxTrtEtaAcceptance", 0)
        kwargs.setdefault("minNTrtHits", 0)
        kwargs.setdefault("maxD0", 500.)
        kwargs.setdefault("maxZ0", 500.)
        kwargs.setdefault("minNPixelHitsPhysical", 2)
        kwargs.setdefault("minNSiHitsPhysical", 10)
        kwargs.setdefault("minNSctHits", 5)
        kwargs.setdefault("minNSctHitsPhysical", 7)

    return InDetTrackSelectionTool_TightPrimary_TrackTools_Cfg(flags, name, **kwargs)

def Monitor_Align_InDetTrackSelectionToolCfg(flags, name="Monitor_Align_InDetTrackSelectionTool", **kwargs):
    kwargs.setdefault("minPt", 1000.)
    kwargs.setdefault("maxD0", 5000.)
    kwargs.setdefault("maxZ0", 5000.)
    kwargs.setdefault("minNNextToInnermostLayerHits", 1)
    return InDetTrackSelectionTool_Loose_TrackTools_Cfg(flags, name, **kwargs)
    
