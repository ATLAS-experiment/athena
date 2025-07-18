# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from TriggerMenuMT.HLT.Config.Utility.ChainDictTools import splitChainDict
from ..Electron.ElectronChainConfiguration import ElectronChainConfiguration
from TriggerMenuMT.HLT.Config.Utility.ChainMerging import mergeChainDefs
from AthenaConfiguration.AthConfigFlags import AthConfigFlags

import pprint
from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)
log.debug("Importing %s",__name__)



def generateChainConfigs(flags, chainDict, perSig_lengthOfChainConfigs):

    if log.isEnabledFor(logging.DEBUG):  # pprint.pformat is expensive
        log.debug('dictionary is: %s\n', pprint.pformat(chainDict))

    listOfChainDicts = splitChainDict(chainDict)
    listOfChainDefs = []

    for subChainDict in listOfChainDicts:
        log.debug('Assembling subChainsDict %s for chain %s', len(listOfChainDefs), subChainDict['chainName'] )
        Electron = ElectronChainConfiguration(subChainDict).assembleChain(flags) 

        listOfChainDefs += [Electron]
        

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
    flags = cloneFlagsToActiveConfig(inflags, "electron", log)
    return flags




    

    
