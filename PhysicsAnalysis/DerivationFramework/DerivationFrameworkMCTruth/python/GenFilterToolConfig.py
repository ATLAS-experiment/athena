# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Common code for setting up the gen filter tools

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def GenFilterAlgCfg(flags):
    """Configure the generator filter tool"""
    acc = ComponentAccumulator()
    # Set up the MCTruthClassifier
    from MCTruthClassifier.MCTruthClassifierConfig import DFCommonMCTruthClassifierCfg
    #Save the post-shower HT and MET filter values that will make combining filtered samples easier (adds to the EventInfo)
    acc.addEventAlgo(CompFactory.DerivationFramework.GenFilterTool(name = "DFCommonTruthGenFilt",
                                                                   TruthClassifier=acc.addPublicTool(acc.popToolsAndMerge(DFCommonMCTruthClassifierCfg(flags)))
                                                                   ))
    return acc
