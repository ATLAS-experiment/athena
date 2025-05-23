# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

###########################################################################
# SliceDef file for Tau chains
###########################################################################

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)
logging.getLogger().info("Importing %s",__name__)

from TriggerMenuMT.HLT.Config.Utility.ChainDictTools import splitChainDict
from TriggerMenuMT.HLT.Config.Utility.ChainMerging import mergeChainDefs
from .TauChainConfiguration import TauChainConfiguration


def generateChainConfigs(flags, chainDict, perSig_lengthOfChainConfigs):

    
    listOfChainDicts = splitChainDict(chainDict)
    listOfChainDefs=[]

    for subChainDict in listOfChainDicts:
        log.debug('Assembling subChainsDict %s for chain %s', len(listOfChainDefs), subChainDict['chainName'] )        
        Tau = TauChainConfiguration(subChainDict).assembleChain(flags) 

        listOfChainDefs += [Tau]
        

    if len(listOfChainDefs)>1:
        theChainDef, perSig_lengthOfChainConfigs = mergeChainDefs(listOfChainDefs, chainDict, perSig_lengthOfChainConfigs)

    else:
        theChainDef = listOfChainDefs[0]

    log.debug("theChainDef: %s" , theChainDef)
    return theChainDef, perSig_lengthOfChainConfigs


