# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def PFlowCommonCfg(ConfigFlags):
    """Main config for PFlow decorations """

    acc = ComponentAccumulator()
    acc.flagPerfmonDomain('PFlow')
    CommonAugmentation = CompFactory.DerivationFramework.CommonAugmentation
    from DerivationFrameworkJetEtMiss.JetToolConfig import PFlowAugmentationToolCfg
    PFlowAugTool = acc.getPrimaryAndMerge(PFlowAugmentationToolCfg(ConfigFlags))
    acc.addEventAlgo(CommonAugmentation("PFlowAugmentation", AugmentationTools = [PFlowAugTool]))

    return acc

##################################################################
