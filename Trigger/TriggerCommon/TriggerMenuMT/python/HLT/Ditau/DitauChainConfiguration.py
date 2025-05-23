# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)

from ..Config.ChainConfigurationBase import ChainConfigurationBase
from .DitauMenuSequences import ditauSequenceGenCfg


#----------------------------------------------------------------
# Class to configure chain
#----------------------------------------------------------------
class DitauChainConfiguration(ChainConfigurationBase):

    def __init__(self, chainDict, jet_name):
        self.jetName = jet_name
        ChainConfigurationBase.__init__(self, chainDict)

    # ----------------------
    # Assemble the chain depending on information from chainName
    # ----------------------
    def assembleChainImpl(self, flags):
        log.debug("Assembling chain for %s", self.chainName)
        chainStep = self.getStep(
            flags, 
            'ditau_step', 
            [ditauSequenceGenCfg],
            seq_name = 'ditau',
            jet_name = self.jetName
        )

        myChain = self.buildChain([chainStep])
        return myChain

