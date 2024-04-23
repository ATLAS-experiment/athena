# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#########################################################################################
#
# HLTMenuConfig class, providing basic functionality for assembling the menu
#
#########################################################################################

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

class HLTMenuConfig:
    """
    Repository of all configured chains. Contains chain dictionaries and Menu Chain objects.
    """
    __allChainDicts   = {}

    @classmethod
    def registerChain(cls, chainDict):
        """Register chain for future use"""
        chainName = chainDict['chainName']
        assert chainName not in cls.__allChainDicts, f'Chain dictionary {chainName} already registered'

        cls.__allChainDicts[chainName] = chainDict
        log.debug("Registered chain %s", chainName)

    @classmethod
    def destroy(cls):
        """Release memory and make the class unusable"""
        del cls.__allChainDicts

    @classmethod
    def isChainRegistered(cls, chainName):
        return chainName in cls.__allChainDicts

    @classmethod
    def dicts(cls):
        return cls.__allChainDicts

    @classmethod
    def dictsList(cls):
        return cls.__allChainDicts.values()

    @classmethod
    def getChainDictFromChainName(cls, chainName):
        return cls.__allChainDicts[chainName]


if __name__ == "__main__": # selftest
    log.info('Self testing')

    HLTMenuConfig.registerChain({'chainName':'HLT_bla1'})
    HLTMenuConfig.registerChain({'chainName':'HLT_bla2'})
    try:
        HLTMenuConfig.registerChain({'chainName':'HLT_bla2'})
    except AssertionError:
        log.info("ok, cannot register same chain twice")
    else:
        log.error("registered same chain twice")

    HLTMenuConfig.getChainDictFromChainName('HLT_bla1') # if missing will assert
    HLTMenuConfig.getChainDictFromChainName('HLT_bla2') # if missing will assert

    log.info("ok, registration works")
    try:
        HLTMenuConfig.getChainDictFromChainName('HLT_blabla')
    except Exception as e:
        if isinstance(e, KeyError):
            log.info("ok, unregistered chain handling works")
        else:
            log.error("unhandled missing chain")
            
