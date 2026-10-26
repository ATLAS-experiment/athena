# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def METMakerToolCfg(flags, name="METMaker", **kwargs):
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.met.METMaker(name, **kwargs))
    return acc


def METMuonSelectionToolCfg(flags, name="MuonSelectionTool_METMakerAlg", **kwargs):
    from MuonSelectorTools.MuonSelectorToolsConfig import MuonSelectionToolCfg
    kwargs.setdefault("MaxEta", 2.4)
    kwargs.setdefault("MuQuality", 1)  # Medium
    kwargs.setdefault("TurnOffMomCorr", True)
    kwargs.setdefault("AllowSettingGeometryOnTheFly", True)
    return MuonSelectionToolCfg(flags, name=name, **kwargs)


def METElectronSelectionToolCfg(flags, name="EleSelLikelihood_METMakerAlg", **kwargs):
    from ElectronPhotonSelectorTools.AsgElectronLikelihoodToolsConfig import AsgElectronLikelihoodToolCfg
    from ElectronPhotonSelectorTools.LikelihoodEnums import LikeEnum
    return AsgElectronLikelihoodToolCfg(flags, name=name, quality=LikeEnum.Medium, **kwargs)


def METPhotonSelectionToolCfg(flags, name="PhotonSelIsEM_METMakerAlg", **kwargs):
    from ElectronPhotonSelectorTools.AsgPhotonIsEMSelectorsConfig import AsgPhotonIsEMSelectorCfg
    from ElectronPhotonSelectorTools.EgammaPIDdefs import egammaPID
    return AsgPhotonIsEMSelectorCfg(flags, name=name, quality=egammaPID.PhotonIDTight, **kwargs)


def METTauSelectionToolCfg(flags, name="TauSelectionTool_METMakerAlg", **kwargs):
    from TauAnalysisTools.TauAnalysisToolsConfig import TauSelectionToolCfg
    from ROOT import TauAnalysisTools
    SelectionCuts = TauAnalysisTools.SelectionCuts

    kwargs.setdefault("ConfigPath", "")
    kwargs.setdefault("SelectionCuts", int(SelectionCuts.CutPt
                                           | SelectionCuts.CutAbsEta
                                           | SelectionCuts.CutAbsCharge
                                           | SelectionCuts.CutNTrack
                                           | SelectionCuts.CutJetIDWP))
    kwargs.setdefault("PtMin", 20.0)
    kwargs.setdefault("JetIDWP", TauAnalysisTools.JETIDRNNMEDIUM)
    kwargs.setdefault("NTracks", (1, 3))
    kwargs.setdefault("AbsCharge", 1)
    kwargs.setdefault("AbsEtaRegion", (0.0, 1.37, 1.52, 2.5))
    return TauSelectionToolCfg(flags, name=name, **kwargs)


def METMakerCfg(flags, suffix, jetSelection="Tier0", jetColl=None,
                muonSelectionTool=None,
                electronSelectionTool=None,
                photonSelectionTool=None,
                tauSelectionTool=None,
                **kwargs):
    
    # Configure a METMakerAlg (and its METMaker tool) for METAssoc_<suffix>.

    acc = ComponentAccumulator()
    doPFlow = "PFlow" in suffix
    doTruth = suffix.startswith("Truth")

    if not jetColl:
        jetColl = (suffix.split("_")[1] if doTruth else suffix) + "Jets"

    if muonSelectionTool is None:
        muonSelectionTool = acc.popToolsAndMerge(METMuonSelectionToolCfg(flags))
    if electronSelectionTool is None:
        electronSelectionTool = acc.popToolsAndMerge(METElectronSelectionToolCfg(flags))
    if photonSelectionTool is None:
        photonSelectionTool = acc.popToolsAndMerge(METPhotonSelectionToolCfg(flags))
    if tauSelectionTool is None:
        tauSelectionTool = acc.popToolsAndMerge(METTauSelectionToolCfg(flags))

    if "Maker" not in kwargs:
        kwargs["Maker"] = acc.popToolsAndMerge(
            METMakerToolCfg(flags, f"METMaker_{suffix}",
                            DoPFlow=doPFlow,
                            DoSoftTruth=doTruth,
                            JetSelection=jetSelection))

    kwargs.setdefault("METMapName", f"METAssoc_{suffix}")
    kwargs.setdefault("METCoreName", f"MET_Core_{suffix}")
    kwargs.setdefault("METName", f"MET_Reference_{suffix}")
    kwargs.setdefault("InputJets", jetColl)

    acc.addEventAlgo(CompFactory.met.METMakerAlg(
        f"METMakerAlg_{suffix}",
        MuonSelectionTool=muonSelectionTool,
        ElectronLHSelectionTool=electronSelectionTool,
        PhotonIsEMSelectionTool=photonSelectionTool,
        TauSelectionTool=tauSelectionTool,
        **kwargs))

    return acc
