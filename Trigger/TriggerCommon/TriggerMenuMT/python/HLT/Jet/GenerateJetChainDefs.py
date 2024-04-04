# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from .JetChainConfiguration import JetChainConfiguration
from TriggerMenuMT.HLT.Config.Utility.ChainMerging import mergeChainDefs

import pprint
from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)
log.debug("Importing %s",__name__)



def generateChainConfigs(flags,  chainDict ):

    if log.isEnabledFor(logging.DEBUG):  # pprint.pformat is expensive
        log.debug('full jet dictionary is: %s\n', pprint.pformat(chainDict))

    # Jet chain is assembled always from the full dictionary (multiple legs are handled internally by the jet reco / hypo)
    theChainDef = JetChainConfiguration(chainDict)

    listOfChainDefs = []

    for leg in range(len(chainDict['chainParts'])):

        listOfChainDefs += [theChainDef.assembleChain(flags)]

    if len(listOfChainDefs) > 1:
        jetChain = mergeChainDefs(listOfChainDefs, chainDict) 
    else:
        jetChain = listOfChainDefs[0]
    return jetChain



    

    
