# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def McEventWeightCfg(flags, name="MyWeights", **kwargs):
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.McEventWeight(name, UseTruthEvents=True))
    return acc


def PileupReweightingToolCfg(flags, name="PileupReweightingTool", **kwargs):
    acc = ComponentAccumulator()
    from Campaigns.Utils import getMCCampaign,Campaign
    campaign = getMCCampaign(flags.Input.Files)
    from PileupReweighting.AutoconfigurePRW import defaultConfigFiles,getConfigurationFiles,getLumicalcFiles
    kwargs.setdefault("LumiCalcFiles", getLumicalcFiles(campaign))
    if campaign in [Campaign.MC23a,Campaign.MC23c]:
        kwargs.setdefault("ConfigFiles", defaultConfigFiles(campaign))
    else:
        kwargs.setdefault("ConfigFiles", getConfigurationFiles(files=flags.Input.Files))
    acc.setPrivateTools(CompFactory.CP.PileupReweightingTool(**kwargs))
    return acc


def PileupReweightingAlgCfg(flags, name="PileupReweightingAlg", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("pileupReweightingTool", acc.popToolsAndMerge(PileupReweightingToolCfg(flags)))
    acc.addEventAlgo(CompFactory.CP.PileupReweightingAlg(name, **kwargs))
    return acc


def PileupReweightingProviderToolCfg(flags, name="auto", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("WeightTool", acc.addPublicTool(acc.popToolsAndMerge(McEventWeightCfg(flags))))
    kwargs.setdefault("ConfigFiles", [])
    kwargs.setdefault("LumiCalcFiles", [])
    kwargs.setdefault("DataScaleFactor", 1.0)
    kwargs.setdefault("DataScaleFactorUP", 0.)
    kwargs.setdefault("DataScaleFactorDOWN", 0.)
    kwargs.setdefault("PeriodAssignments", [])
    acc.setPrivateTools(acc.popToolsAndMerge(PileupReweightingToolCfg(flags, name, **kwargs)))
    return acc
