# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
# Configuration of InDetVKalVxInJetTool package

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod

def TCTDecorCheckInToolCfg(flags, name="TCTDecorCheckInTool", **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("JetContainer", "AntiKt4EMPFlowJets")
    kwargs.setdefault("TrackClassificationTool",acc.popToolsAndMerge(
        InDetTrkInJetTypeCfg(flags, JetCollection=kwargs["JetContainer"])))
                 
    acc.addEventAlgo(CompFactory.TCTDecorCheckInTool(name, **kwargs))
    return acc

def InDetTrkInJetTypeCfg(flags, name="TrkInJetType", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("trkSctHits", 4 if flags.GeoModel.Run <= LHCPeriod.Run3 else -1)
    kwargs.setdefault("useFivePtJetBinVersion", flags.BTagging.TrkClassFiveBinMode)

    if "VertexFitterTool" not in kwargs:
        from TrkConfig.TrkVKalVrtFitterConfig import TrkVKalVrtFitterCfg
        kwargs.setdefault("VertexFitterTool", acc.popToolsAndMerge(
            TrkVKalVrtFitterCfg(flags,"VKalVrtFitter")))
    
    acc.setPrivateTools(CompFactory.InDet.InDetTrkInJetType(name, **kwargs))
    return acc

def InDetVKalVxInJetToolCfg(flags, name="InDetVKalVxInJetTool", **kwargs):
    acc = ComponentAccumulator()

    from TrkConfig.TrkVKalVrtFitterConfig import BTAG_TrkVKalVrtFitterCfg
    VertexFitter = acc.popToolsAndMerge(BTAG_TrkVKalVrtFitterCfg(flags,"VKalVrtFitter"))

    if "TrackClassTool" not in kwargs:
         kwargs.setdefault("TrackClassTool", acc.popToolsAndMerge(
             InDetTrkInJetTypeCfg(flags,VertexFitterTool=VertexFitter)))

    kwargs.setdefault("ExistIBL", flags.GeoModel.Run in [LHCPeriod.Run2, LHCPeriod.Run3])
    kwargs.setdefault("getNegativeTag", "Flip" in name)
    kwargs.setdefault("UseFrozenVersion", True)
    kwargs.setdefault("VertexFitterTool", VertexFitter)

    if flags.GeoModel.Run >= LHCPeriod.Run4:
        from InDetConfig.InDetEtaDependentCutsConfig import IDEtaDependentCuts_SV1_SvcCfg
        acc.merge(IDEtaDependentCuts_SV1_SvcCfg(flags, name="IDEtaDepCutsSvc_" + name))
        kwargs.setdefault("InDetEtaDependentCutsSvc", acc.getService("IDEtaDepCutsSvc_" + name))
        kwargs.setdefault("useVertexCleaningPix", False) # Would use hardcoded InDet Pixel geometry
        kwargs.setdefault("useITkMaterialRejection", True)

    acc.setPrivateTools(CompFactory.InDet.InDetVKalVxInJetTool(name,**kwargs))
    return acc

def MSV_InDetVKalVxInJetToolCfg(flags, name="IDVKalMultiVxInJet", **kwargs):
    kwargs.setdefault("getNegativeTail", False)
    kwargs.setdefault("ConeForTag", 1.0)
    kwargs.setdefault("MultiVertex", True)
    return InDetVKalVxInJetToolCfg(flags, name, **kwargs)

def InDetVKalVxInHiPtJetToolCfg(flags, name="InDetVKalVxInHiPtJetTool", **kwargs):
    kwargs.setdefault("CutSharedHits", 0)
    kwargs.setdefault("Sel2VrtChi2Cut", 4.)
    kwargs.setdefault("CutBVrtScore", 0.002)
    return InDetVKalVxInJetToolCfg(flags, name, **kwargs)
