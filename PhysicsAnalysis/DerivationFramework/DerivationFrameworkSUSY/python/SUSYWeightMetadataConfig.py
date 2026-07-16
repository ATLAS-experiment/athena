# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# Sum of weights algorithm
def SumOfWeightsAlgCfg(flags, name, **kwargs):
    """ Get the sum of weights algorithm """
    from EventBookkeeperTools.EventBookkeeperToolsConfig import CutFlowSvcCfg
    acc = CutFlowSvcCfg( flags )
    SumOfWeightsAlg = CompFactory.SumOfWeightsAlg
    acc.addEventAlgo( SumOfWeightsAlg(name=name, **kwargs) )
    return acc

# McEventWeight Tool
def McEventWeightToolCfg(flags, name, **kwargs):
    """ Get the MC Event Weight Tool """
    acc = ComponentAccumulator()
    McEventWeight = CompFactory.McEventWeight
    acc.addPublicTool( McEventWeight(name=name, **kwargs) )
    return acc

# SUSY ID Weight Tool
def SUSYIDWeightToolCfg(flags, name, **kwargs):
    """ Get the SUSY ID Weight Tool """
    acc = ComponentAccumulator()
    SUSYIDWeight = CompFactory.SUSYIDWeight
    acc.addPublicTool( SUSYIDWeight(name=name, **kwargs) )
    return acc

# SUSY Event Weights
def AddSUSYWeightsCfg(flags, pref = ""):
    """Add SUSY weights"""

    acc = ComponentAccumulator()

    # Load all potential SUSY process IDs following Prospino conventions
    # https://twiki.cern.ch/twiki/bin/view/AtlasProtected/SUSYSignalUncertainties#Subprocess_IDs 
    listTools = []
    for i in range(0, 225):
        myName = pref+"SUSYWeight_ID"+"_"+str(i)
        if i==0: #flat sum of all processes (i.e. sum the weight no matter what)
            acc.merge( McEventWeightToolCfg(flags, name=myName, UseTruthEvents=True) )
        else: #add weight only to keeper associated to current process id
            acc.merge( SUSYIDWeightToolCfg(flags, name=myName, SUSYProcID=i, UseTruthEvents=True) )
        listTools.append( acc.getPublicTool(myName) )

    acc.merge( SumOfWeightsAlgCfg(flags, name=pref+"SUSYSumWeightsAlg", WeightTools=listTools) )
    return acc
