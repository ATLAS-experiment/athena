#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
from DecisionHandling.DecisionHandlingConfig import ComboHypoCfg
from TriggerMenuMT.HLT.Config.MenuComponents import Chain, ChainStep
from TriggerMenuMT.HLT.Config.Utility.ChainDefInMenu import ChainProp
from TriggerMenuMT.HLT.Config.Utility.ChainDictTools import splitChainDictInLegs
from TriggerMenuMT.HLT.Config.Utility.DictFromChainName import dictFromChainName
from TriggerMenuMT.HLT.Config.Utility.HLTMenuConfig import HLTMenuConfig
from HLTSeeding.HLTSeedingConfig import mapThresholdToL1DecisionCollection

import functools

def writeEmulationFiles(data):
    """Writes emulation files. key in the dict is a file name (+.dat), list which is value of each dict el is enetered into the file, one el. per line"""
    for name, d in data.items():
        with open(name+".dat", "w") as f:
            for event in d:
                f.write(event)
                f.write("\n")


class makeChainStep(object):
    """Used to store the step info, regardless of the chainDict"""
    def __init__(self, name, seq=None, comboHypoCfg=None, comboToolConfs=None, chainDicts=None, isEmpty=False):
        self.name = name
        self.seq = seq if seq is not None else []
        self.comboHypoCfg = comboHypoCfg if comboHypoCfg is not None else functools.partial(ComboHypoCfg)
        self.comboToolConfs = comboToolConfs if comboToolConfs is not None else []
        self.chainDicts = chainDicts
        self.isEmpty = isEmpty
    

chainsCounter = 0

def makeChain( flags, name, L1Thresholds, ChainSteps, Streams="physics:Main", Groups=["RATE:TestRateGroup", "BW:TestBW"]):
    """
    In addition to making the chain object fills the flags that are used to generate MnuCOnfig JSON file
    """
    prop = ChainProp( name=name,  l1SeedThresholds=L1Thresholds, groups=Groups )
    chainDict = dictFromChainName( flags, prop )
    global chainsCounter
    chainDict["chainCounter"] = chainsCounter
    chainsCounter += 1

    #set default chain prescale
    chainDict['prescale'] = 1

    listOfChainDicts = splitChainDictInLegs(chainDict)
    L1decisions = [ mapThresholdToL1DecisionCollection(stri) for stri in L1Thresholds]
    # create the ChainSteps, with the chaindict
    StepConfig = []
    for step in ChainSteps:        
        StepConfig+=[ChainStep(step.name, 
                                step.seq,  
                                chainDicts=step.chainDicts if step.chainDicts else listOfChainDicts, 
                                comboHypoCfg=step.comboHypoCfg, 
                                comboToolConfs=step.comboToolConfs, isEmpty=step.isEmpty)]

    chainConfig = Chain( name=name, L1decisions=L1decisions, ChainSteps=StepConfig )
    HLTMenuConfig.registerChain( chainDict )

    return chainConfig
