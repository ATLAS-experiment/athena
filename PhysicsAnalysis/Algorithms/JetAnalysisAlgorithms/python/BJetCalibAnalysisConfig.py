# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
import AthenaCommon.SystemOfUnits as Units


class BJetCalibAnalysisConfig (ConfigBlock) :
    """the ConfigBlock for the b-jet calibration sequence"""

    def __init__ (self, jetContainerName='', muonContainerName='') :
        super (BJetCalibAnalysisConfig, self).__init__ ()
        self.setBlockName('BJetCalib')
        self.addDependency('FTag', required=False)
        self.addDependency('Muons', required=True)
        self.addOption ('jetContainerName', jetContainerName, type=str,
            noneAction='error',
            info="the name of the input jet container.")
        self.addOption ('muonContainerName', muonContainerName, type=str,
            noneAction='error',
            info="the name of the input muon container.")
        self.addOption ('jetPreselection', "", type=str,
            info="the jet preselection")
        self.addOption ('muonPreselection', "", type=str,
            info="the muon preselection")
        self.addOption ('doPtCorr', True, type=bool,
            info="whether to run the b-jet pT correction on top of the muon-in-jet one")

    def makeAlgs(self, config):

        # Set up kinematic selection for which ftag selection should be used downstream
        jetPreselection = config.getFullSelection(self.jetContainerName, self.jetPreselection)
        if jetPreselection:
            alg = config.createAlgorithm('CP::AsgSelectionAlg',
                                         'FtagPTEtaCutAlg' + self.jetContainerName)
            alg.selectionDecoration = 'selectPtEtaFtag'
            config.addPrivateTool('selectionTool', 'CP::AsgPtEtaSelectionTool')
            alg.selectionTool.maxEta = 2.5
            alg.selectionTool.minPt = 20. * Units.GeV
            alg.particles = config.readName(self.jetContainerName)
            alg.preselection = config.getPreselection(self.jetContainerName, '')
            jetPreselection = "selectPtEtaFtag&&"+jetPreselection

        alg = config.createAlgorithm('CP::BJetCalibrationAlg',
                                     'BJetCalibAlg_' + self.jetContainerName)
        alg.muons = config.readName(self.muonContainerName)
        alg.muonPreselection = config.getPreselection(self.muonContainerName,
                                                      self.muonPreselection)
        alg.jets = config.readName(self.jetContainerName)
        alg.jetPreselection = jetPreselection
        alg.jetsOut = config.copyName(self.jetContainerName)

        config.addPrivateTool('muonInJetTool', 'MuonInJetCorrectionTool')
        # Adjust dR matching for large-R jets
        if "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets" in alg.jets:
            alg.muonInJetTool.doLargeR = True

        if self.doPtCorr:
            config.addPrivateTool('bJetTool', 'BJetCorrectionTool')
