# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

#############################################
# Heavy flavour from tt tools
#############################################

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from DerivationFrameworkMCTruth.HFDSIDList import DSIDList


def HFHadronsCommonCfg(flags):
    """Heavy flavour decorations config"""
    acc = ComponentAccumulator()

    if flags.Input.MCChannelNumber > 0 and flags.Input.MCChannelNumber in DSIDList:
        from DerivationFrameworkMCTruth.TruthDerivationToolsConfig import HadronOriginDecoratorCfg
        acc.merge(HadronOriginDecoratorCfg(flags, name="HFHadronsCommonKernel"))
    return acc
