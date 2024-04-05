# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

from TriggerMenuMT.HLT.Config.Utility.ChainDictTools import splitChainDict
from TriggerMenuMT.HLT.Config.Utility.ChainMerging import mergeChainDefs
from .EnhancedBiasChainConfiguration import EnhancedBiasChainConfiguration

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)
log.debug("Importing %s",__name__)


def generateChainConfigs(flags,  chainDict ):
    log.debug("Generate EnhancedBias...")
    listOfChainDicts = splitChainDict(chainDict)
    listOfChainDefs = []

    for subChainDict in listOfChainDicts:
        EnhancedBiasChain = EnhancedBiasChainConfiguration(subChainDict).assembleChain(flags)
        listOfChainDefs += [EnhancedBiasChain]

    if len(listOfChainDefs)>1:
        theChainDef = mergeChainDefs(listOfChainDefs, chainDict)
    else:
        theChainDef = listOfChainDefs[0]

    log.debug("theChainDef %s" , theChainDef)

    return theChainDef
