# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
# file to simulate the HypoTool configuration of the signatures

from AthenaConfiguration.ComponentFactory import CompFactory

def TestHypoTool(name, prop, threshold_value):
    value  =  int(threshold_value)*1000
    UseThisLinkName="initialRoI"   
    HLTTest__TestHypoTool=CompFactory.getComp("HLTTest::TestHypoTool") 
    return HLTTest__TestHypoTool(name, Threshold=value, Property=prop, LinkName=UseThisLinkName)

def MuTestHypoTool(flags, chainDict):
    name = chainDict['chainName']
    threshold = getThreshold(chainDict) 
    return TestHypoTool(name,prop="pt", threshold_value=threshold)

def ElTestHypoTool(flags, chainDict):
    name = chainDict['chainName']
    threshold = getThreshold(chainDict) 
    return TestHypoTool(name,prop="et", threshold_value=threshold)

def GammTestHypoTool(flags, chainDict):
    name = chainDict['chainName']
    threshold = getThreshold(chainDict) 
    return TestHypoTool(name,prop="et", threshold_value=threshold)


def MuTest2HypoTool(flags, chainDict):
    name = chainDict['chainName']
    threshold = getThreshold(chainDict) 
    return TestHypoTool(name,prop="pt2", threshold_value=threshold)

def ElTest2HypoTool(flags, chainDict):
    name = chainDict['chainName']
    threshold = getThreshold(chainDict) 
    return TestHypoTool(name,prop="et", threshold_value=threshold)


def getThreshold(chainDict):
    name = chainDict['chainParts'][0]['chainPartName']
    from TriggerMenuMT.HLT.Config.Utility.DictFromChainName import getChainThresholdFromName
    return getChainThresholdFromName( name.split("_"), "TestChain")



def dimuDrComboHypoTool(flags, chainDict):
    name = chainDict['chainName']
    tool= CompFactory.DeltaRRoIComboHypoTool(name)
    tool.DRcut=0.3
    return tool

