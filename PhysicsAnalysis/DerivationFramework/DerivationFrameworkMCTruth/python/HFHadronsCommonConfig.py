# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#############################################
# Heavy flavour from tt tools
#############################################

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from DerivationFrameworkMCTruth.HFDSIDList import DSIDList


def HFHadronsCommonCfg(flags):
    """Heavy flavour decorations config"""
    acc = ComponentAccumulator()

    if flags.Input.MCChannelNumber > 0 and flags.Input.MCChannelNumber in DSIDList:
        from DerivationFrameworkMCTruth.TruthDerivationToolsConfig import HadronOriginDecoratorCfg
        DFCommonhadronorigindecorator = acc.getPrimaryAndMerge(HadronOriginDecoratorCfg(flags,
                                                                                        name="DFCommonHadronOriginDecorator"))
        CommonAugmentation = CompFactory.DerivationFramework.CommonAugmentation
        acc.addEventAlgo(CommonAugmentation(name="HFHadronsCommonKernel",
                                            AugmentationTools=[DFCommonhadronorigindecorator]))
    return acc
