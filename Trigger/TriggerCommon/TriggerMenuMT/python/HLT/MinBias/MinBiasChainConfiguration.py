# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger( __name__ )

from TriggerMenuMT.HLT.Config.ChainConfigurationBase import ChainConfigurationBase

from TriggerMenuMT.HLT.MinBias.MinBiasMenuSequences import (MinBiasSPSequenceCfg, 
                                                            MinBiasTrkSequenceCfg,
                                                            MinBiasMbtsSequenceCfg,
                                                            MinBiasZVertexFinderSequenceCfg)
from TriggerMenuMT.HLT.MinBias.AFPMenuSequence import AFPTrkSequenceCfg, AFPGlobalSequenceCfg, AFPToFDeltaZSequenceCfg

class MinBiasChainConfig(ChainConfigurationBase):

    def __init__(self, chainDict):
        ChainConfigurationBase.__init__(self,chainDict)

    # ----------------------
    # Assemble the chain depending on information from chainName
    # ----------------------
    def assembleChainImpl(self, flags):
        log.debug("Assembling chain for %s", self.chainName)
        steps = []

        if "mbts" == self.chainPart['recoAlg'][0] or "mbts" in self.chainName:
            steps.append(self.getMinBiasMbtsStep(flags))
        elif "afprec" == self.chainPart['recoAlg'][0]:
            steps.append(self.getAFPTrkStep(flags))
        else:
            steps.append(self.getMinBiasEmptyMbtsStep(flags))

        if "afpdz5" in self.chainPart['recoAlg'] or "afpdz10" in self.chainPart['recoAlg']:
            # afpdz covers both the trigger hypo and afptof reconstruction
            steps.append(self.getAFPToFDeltaZStep(flags))
        elif "afptof" in self.chainPart['recoAlg']:
            steps.append(self.getAFPGlobalStep(flags))

        if self.chainPart['recoAlg'][0] in ['sp', 'sptrk', 'hmt', 'excl']:
            steps.append(self.getMinBiasSpStep(flags))

        if self.chainPart['recoAlg'][0] in ['sptrk', 'hmt', 'excl']:
            steps.append(self.getMinBiasZFindStep(flags))
            steps.append(self.getMinBiasTrkStep(flags))

        return self.buildChain(steps)

    def getMinBiasMbtsStep(self, flags):
        return self.getStep(flags,1,'Mbts', [MinBiasMbtsSequenceCfg])

    def getMinBiasEmptyMbtsStep(self, flags):
        return self.getEmptyStep(1,'EmptyMbts')

    def getMinBiasSpStep(self, flags):
        return self.getStep(flags,2,'SPCount', [MinBiasSPSequenceCfg])

    def getMinBiasZFindStep(self, flags):
        return self.getStep(flags,3,'ZFind', [MinBiasZVertexFinderSequenceCfg])

    def getMinBiasTrkStep(self, flags):
        return self.getStep(flags,4,'TrkCount', [MinBiasTrkSequenceCfg])

    def getAFPTrkStep(self, flags):
        return self.getStep(flags,1,'AFPTrk', [AFPTrkSequenceCfg])

    def getAFPGlobalStep(self, flags):
        return self.getStep(flags,2,'AFPGlobal', [AFPGlobalSequenceCfg])
    
    def getAFPToFDeltaZStep(self, flags):
        return self.getStep(flags,2,'AFPToFDeltaZ', [AFPToFDeltaZSequenceCfg])
