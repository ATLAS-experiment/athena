# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# CA Configuration of BJetCalibrationAlg for CP CA-based frameworks

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def BJetCalibrationAlgCfg(flags, name="BJetCalibrationAlg", doPtCorr=True, **kwargs):
    cfg = ComponentAccumulator()

    kwargs.setdefault("jets", "AntiKt4EMPFlow")

    if "muonInJetTool" not in kwargs:
        kwargs.setdefault("muonInJetTool", CompFactory.MuonInJetCorrectionTool(
            doLargeR = "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets" in kwargs["jets"]
        ))

    if "muonSelectionTool" not in kwargs:
        # MuQuality=1, Medium muons, see Event/xAOD/xAODMuon/xAODMuon/versions/Muon_v1.h
        from MuonSelectorTools.MuonSelectorToolsConfig import MuonLoosenedNonCalibratedSelectionToolCfg
        kwargs.setdefault("muonSelectionTool", cfg.popToolsAndMerge(
            MuonLoosenedNonCalibratedSelectionToolCfg(flags, MaxEta=2.5, MuQuality=1)))

    if doPtCorr and "bJetTool" not in kwargs:
        kwargs.setdefault("bJetTool", CompFactory.BJetCorrectionTool())

    cfg.addEventAlgo(CompFactory.CP.BJetCalibrationAlg(name, **kwargs))
    return cfg
