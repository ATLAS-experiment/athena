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
    __allChainConfigs = {}
    __allChainDicts   = {}

    @classmethod
    def registerChain(cls, chainDict, chainConfig):
        """ 
        Register chain for future use
        
        Name reuse and inconsistency is reported
        """
        assert chainDict['chainName'] not in cls.__allChainDicts, 'Chain dictionary {} already registered'.format(chainDict['chainName'])
        assert chainConfig.name not in cls.__allChainConfigs, 'Chain configuration {} already registered'.format(chainConfig.name)
        assert chainDict['chainName'] == chainConfig.name, 'Registering chain dictionary and config that have differnet names: in dictionary {}, in config {}'.format(chainDict['chainName'], chainConfig.name)

        cls.__allChainConfigs[chainConfig.name] = chainConfig
        cls.__allChainDicts[chainDict['chainName']] = chainDict
        log.debug("Registered chain %s", chainConfig.name ) 

    @classmethod
    def destroy(cls):
        """Release memory and make the class unusable"""
        del cls.__allChainConfigs
        del cls.__allChainDicts

    @classmethod
    def isChainRegistered(cls, chainName):
        return chainName in cls.__allChainDicts

    @classmethod
    def dicts(cls):
        return cls.__allChainDicts

    @classmethod
    def configs(cls):
        return cls.__allChainConfigs

    @classmethod
    def dictsList(cls):
        return cls.__allChainDicts.values()

    @classmethod
    def configsList(cls):
        return cls.__allChainConfigs.values()

    @classmethod
    def getChainDictFromChainName(cls, chainName):
        return cls.__allChainDicts[chainName]


if __name__ == "__main__": # selftest
    log.info('Self testing')
    class ChainClassGoodForThisTest:
        def __init__(self, n):
            self.name = n

    HLTMenuConfig.registerChain({'chainName':'HLT_bla1'}, ChainClassGoodForThisTest('HLT_bla1'))
    HLTMenuConfig.registerChain({'chainName':'HLT_bla2'}, ChainClassGoodForThisTest('HLT_bla2'))
    HLTMenuConfig.getChainDictFromChainName('HLT_bla1') # if missing will assert
    HLTMenuConfig.getChainDictFromChainName('HLT_bla2') # if missing will assert
    log.info("ok, registration works")
    try:
        HLTMenuConfig.getChainDictFromChainName('HLT_blabla')
    except Exception as e:
        if isinstance(e, AssertionError):
            log.info("ok, unregistered chain handling works")
        else:
            log.error("unhandled missing chain")
            
