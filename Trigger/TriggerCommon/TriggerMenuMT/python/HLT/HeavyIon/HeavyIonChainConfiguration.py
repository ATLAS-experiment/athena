# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
logging.getLogger().info('Importing %s', __name__)
log = logging.getLogger(__name__)
from ..Config.ChainConfigurationBase import ChainConfigurationBase
from ..HeavyIon.HeavyIonMenuSequences import HIFwdGapMenuSequenceCfg


class HeavyIonChainConfig(ChainConfigurationBase):

  def __init__(self, chainDict):
    ChainConfigurationBase.__init__(self, chainDict)

  # ----------------------
  # Assemble the chain depending on information from chainName
  # ----------------------
  def assembleChainImpl(self, flags):
    log.debug('Assembling chain for %s', self.chainName)
    steps = []
    if 'Fgap' in self.chainPart['hypoFgapInfo'][0]:
        steps.append(self.getStep(flags,1, 'Fgap', [HIFwdGapMenuSequenceCfg]))
    return self.buildChain(steps)
