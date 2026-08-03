# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock


class L1jFexJetThresholdsDecoratorBlock(ConfigBlock):

    def __init__(self):
        super().__init__()
        self.addOption(
            'l1jFexContainer', 'L1_jFexSRJetRoI', type=str,
            info="StoreGate key of the Phase-I L1 jFEX SR jet RoI "
                 "container to decorate."
        )
        self.addOption(
            'l1ThresholdType', 'jJ', type=str,
            info="L1 menu threshold type for the bit-to-name lookup "
                 "(jJ = small-R jFEX jet thresholds)."
        )

    def makeAlgs(self, config):
        alg = config.createAlgorithm(
            'CP::L1jFexJetThresholdsDecoratorAlg',
            'L1jFexJetThresholdsDecoratorAlg'
        )
        alg.l1Jets = self.l1jFexContainer
        alg.l1ThresholdType = self.l1ThresholdType
