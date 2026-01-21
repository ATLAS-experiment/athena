# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


# ATTENTION: This is a ComponentAccumulator-based configuration file
# (not to be confused with ConfigAccumulator). If you are an analysis
# user you are likely looking for some other Block in
# AsgAnalysisConfig.py. That uses the block-configuration generally
# used for algorithms under PhysicsAnalysis/Algorithms. If you are
# using the block configuration do not use this configuration. We
# generally do not provide ComponentAccumulator-based configurations
# for the CP algorithms, and a special exception was made for this
# algorithm. Do not cite this as example why any other
# ComponentAccumulator-based configurations should be added. You can
# use this as an example for how ComponentAccumulator-based
# configurations can look like, but do not try to extend this to cover
# more CP algorithms. That is not supported, and you are likely to
# encounter problems if you scale this up in the wrong way. This
# configuration is covered by a unit test. Should it fail use your
# best judgement whether to fix this configuration or to change it to
# wrap the block configuration instead.


def McEventWeightCfg(flags, name="MyWeights", **kwargs):
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.McEventWeight(name, UseTruthEvents=True))
    return acc


def PileupReweightingToolCfg(flags, name="PileupReweightingTool", commonPRW=True, **kwargs):
    acc = ComponentAccumulator()
    from Campaigns.Utils import getMCCampaign
    campaign = getMCCampaign(flags.Input.Files)

    from PileupReweighting.AutoconfigurePRW import defaultConfigFiles,getConfigurationFiles,getLumicalcFiles
    kwargs.setdefault("LumiCalcFiles", getLumicalcFiles(campaign))
    if commonPRW:
        kwargs.setdefault("ConfigFiles", defaultConfigFiles(campaign))
    else:
        kwargs.setdefault("ConfigFiles", getConfigurationFiles(files=flags.Input.Files))

    acc.setPrivateTools(CompFactory.CP.PileupReweightingTool(**kwargs))
    return acc


def PileupReweightingAlgCfg(flags, name="PileupReweightingAlg", **kwargs):
    acc = ComponentAccumulator()
    acc.addService(CompFactory.CP.SystematicsSvc("SystematicsSvc"))
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
    return acc.setPrivateTools(acc.popToolsAndMerge(PileupReweightingToolCfg(flags, name, **kwargs)))
