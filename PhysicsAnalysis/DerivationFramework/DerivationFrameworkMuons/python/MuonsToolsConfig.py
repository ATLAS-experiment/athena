# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MuonJetDrAlgCfg(flags, name):
    acc = ComponentAccumulator()
    acc.addEventAlgo(CompFactory.DerivationFramework.MuonJetDrAlg(name))
    return acc


### Configuration for the MuonTPExtrapolation tool
def MuonTPExtrapolationAlgCfg(flags, name = "MuonTPExtrapolationAlg", **kwargs):
    acc= ComponentAccumulator()
    from TrkConfig.AtlasExtrapolatorConfig import MuonExtrapolatorCfg
    kwargs.setdefault("Extrapolator", acc.popToolsAndMerge(MuonExtrapolatorCfg(flags)))
    the_alg = CompFactory.DerivationFramework.MuonTPExtrapolationAlg(name = name,**kwargs)
    acc.addEventAlgo(the_alg, primary = True)
    return acc


### Algorithm that decorates the calorimeter deposits in form of 3 vectors to the
### muon. The deposits are used to identify the track as CT muon
def MuonCaloDepositAlgCfg(flags, name= "MuonCaloDepositAlg", **kwargs):
    acc = ComponentAccumulator()
    from MuonCombinedConfig.MuonCombinedRecToolsConfig import TrackDepositInCaloToolCfg
    kwargs.setdefault("TrackDepositInCaloTool", acc.popToolsAndMerge(TrackDepositInCaloToolCfg(flags)))
    the_alg = CompFactory.DerivationFramework.IDTrackCaloDepositsDecoratorAlg(name, **kwargs)
    acc.addEventAlgo(the_alg, primary = True)
    return acc


### Algorithm used to thin bad muons from the analysis stream
def AnalysisMuonThinningAlgCfg(flags, name="AnalysisMuonThinningAlg", **kwargs):
    acc = ComponentAccumulator()
    from MuonSelectorTools.MuonSelectorToolsConfig import MuonLoosenedNonCalibratedSelectionToolCfg
    kwargs.setdefault("SelectionTool", acc.popToolsAndMerge(MuonLoosenedNonCalibratedSelectionToolCfg(flags,
                                                            name="MuonSelThinningTool")))
    the_alg = CompFactory.DerivationFramework.AnalysisMuonThinningAlg(name, **kwargs)
    acc.addEventAlgo(the_alg, primary = True)
    return acc


### Di-muon tagging tool, for T&P studies
def DiMuonTaggingAlgCfg(flags, name="DiMuonTaggingTool", **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("applyTrigger",True)
    if kwargs["applyTrigger"]:
        from TriggerMatchingTool.TriggerMatchingToolConfig import TriggerMatchingToolCfg
        kwargs.setdefault("TrigMatchingTool",  acc.popToolsAndMerge(
            TriggerMatchingToolCfg(flags)))

    from MuonSelectorTools.MuonSelectorToolsConfig import MuonLoosenedNonCalibratedSelectionToolCfg
    kwargs.setdefault("SelectionTool", acc.popToolsAndMerge(MuonLoosenedNonCalibratedSelectionToolCfg(flags)))
    kwargs.setdefault("isMC", flags.Input.isMC)
    the_alg = CompFactory.DerivationFramework.DiMuonTaggingAlg(name, **kwargs)
    acc.addEventAlgo(the_alg, primary = True)
    return acc

