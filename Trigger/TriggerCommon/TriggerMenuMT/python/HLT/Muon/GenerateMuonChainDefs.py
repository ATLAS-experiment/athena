# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

###########################################################################
# SliceDef file for Muon chains
###########################################################################

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)
logging.getLogger().info("Importing %s",__name__)
from TriggerMenuMT.HLT.Config.Utility.ChainDictTools import splitChainDict
from TriggerMenuMT.HLT.Config.Utility.ChainMerging import mergeChainDefs
from .MuonChainConfiguration import MuonChainConfiguration
from AthenaConfiguration.AthConfigFlags import AthConfigFlags


def generateChainConfigs(flags, chainDict, perSig_lengthOfChainConfigs):
    
    listOfChainDicts = splitChainDict(chainDict)
    listOfChainDefs=[]

    for subChainDict in listOfChainDicts:
        log.debug('Assembling subChainsDict %s for chain %s', len(listOfChainDefs), subChainDict['chainName'] )        
        Muon = MuonChainConfiguration(subChainDict).assembleChain(flags)

        listOfChainDefs += [Muon]
        

    if len(listOfChainDefs)>1:
        theChainDef, perSig_lengthOfChainConfigs = mergeChainDefs(listOfChainDefs, chainDict, perSig_lengthOfChainConfigs)
    else:
        theChainDef = listOfChainDefs[0]

    return theChainDef, perSig_lengthOfChainConfigs

def prepareDefaultSignatureFlags(inflags : AthConfigFlags) -> AthConfigFlags:
    """
    invoked before generateChainConfigs method to prevent repeated cloning of flags within chain generation
    """
    from TrigInDetConfig.utils import cloneFlagsToActiveConfig
    flags = cloneFlagsToActiveConfig(inflags, "muon", log)
    return flags


