# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from .UnconventionalTrackingChainConfiguration import UnconventionalTrackingChainConfiguration
from TriggerMenuMT.HLT.Config.Utility.ChainDictTools import splitChainDict
from TriggerMenuMT.HLT.Config.Utility.ChainMerging import mergeChainDefs
from AthenaConfiguration.AthConfigFlags import AthConfigFlags

import pprint
from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)
log.debug("Importing %s",__name__)



def generateChainConfigs(flags,  chainDict ):

    if log.isEnabledFor(logging.DEBUG):  # pprint.pformat is expensive
        log.debug('dictionary is: %s\n', pprint.pformat(chainDict))

    listOfChainDicts = splitChainDict(chainDict)

    listOfChainDefs=[]
    for subChainDict in listOfChainDicts:
        subChain = UnconventionalTrackingChainConfiguration(subChainDict).assembleChain(flags)
        listOfChainDefs += [subChain]

    log.debug('length of chaindefs %s', len(listOfChainDefs))

    if len(listOfChainDefs) > 1:
        chainDef = mergeChainDefs(listOfChainDefs, chainDict)
    else:
        chainDef = listOfChainDefs[0]

    log.debug('ChainDef %s', chainDef)
    return chainDef

def prepareDefaultSignatureFlags(inflags : AthConfigFlags) -> AthConfigFlags:
    """
    invoked before generateChainConfigs method to prevent repeated cloning of flags within chain generation
    """
    from TrigInDetConfig.utils import cloneFlagsToActiveConfig
    flags = cloneFlagsToActiveConfig(inflags, "fullScan", log)
    return flags
