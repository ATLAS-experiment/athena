# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from TriggerMenuMT.HLT.Config.Utility.ChainDictTools import splitChainDict
from TriggerMenuMT.HLT.Config.Utility.ChainMerging import mergeChainDefs
from ..Photon.PhotonChainConfiguration import PhotonChainConfiguration

import pprint
from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)
log.debug("Importing %s",__name__)



def generateChainConfigs(flags,  chainDict, perSig_lengthOfChainConfigs):

    if log.isEnabledFor(logging.DEBUG):  # pprint.pformat is expensive
        log.debug('dictionary is: %s\n', pprint.pformat(chainDict))

    listOfChainDicts = splitChainDict(chainDict)
    listOfChainDefs = []

    for subChainDict in listOfChainDicts:
        log.debug('Assembling subChainsDict %s for chain %s', len(listOfChainDefs), subChainDict['chainName'] )  
        Photon = PhotonChainConfiguration(subChainDict).assembleChain(flags) 

        listOfChainDefs += [Photon]
        

    if len(listOfChainDefs)>1:
        theChainDef, perSig_lengthOfChainConfigs = mergeChainDefs(listOfChainDefs, chainDict, perSig_lengthOfChainConfigs)
    else:
        theChainDef = listOfChainDefs[0]

    return theChainDef, perSig_lengthOfChainConfigs



    

    
