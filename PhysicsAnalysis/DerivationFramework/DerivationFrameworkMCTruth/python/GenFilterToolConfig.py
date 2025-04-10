# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Common code for setting up the gen filter tools

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def GenFilterToolCfg(flags):
    """Configure the generator filter tool"""
    acc = ComponentAccumulator()

    # Set up the MCTruthClassifier
    from MCTruthClassifier.MCTruthClassifierConfig import DFCommonMCTruthClassifierCfg
    acc.merge(DFCommonMCTruthClassifierCfg(flags))

    #Save the post-shower HT and MET filter values that will make combining filtered samples easier (adds to the EventInfo)
    GenFilterTool = CompFactory.DerivationFramework.GenFilterTool
    acc.addPublicTool(GenFilterTool(name = "DFCommonTruthGenFilt"), primary = True)

    return acc
