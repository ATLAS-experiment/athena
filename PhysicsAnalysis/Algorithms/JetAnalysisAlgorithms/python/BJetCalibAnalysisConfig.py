# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
import AthenaCommon.SystemOfUnits as Units


class BJetCalibAnalysisConfig (ConfigBlock) :
    """the ConfigBlock for the b-jet calibration sequence"""

    def __init__ (self, containerName='', muonContainerName='') :
        super (BJetCalibAnalysisConfig, self).__init__ ()
        self.setBlockName('BJetCalib')
        self.addDependency('FTag', required=False)
        self.addDependency('Muons', required=True)
        self.addDependency('MuonsWorkingPoint', required=False)
        self.addOption ('containerName', containerName, type=str,
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
        jetPreselection = config.getFullSelection(self.containerName, self.jetPreselection)
        if jetPreselection:
            alg = config.createAlgorithm('CP::AsgSelectionAlg',
                                         'FtagPTEtaCutAlg' + self.containerName)
            alg.selectionDecoration = 'selectPtEtaFtag'
            config.addPrivateTool('selectionTool', 'CP::AsgPtEtaSelectionTool')
            alg.selectionTool.maxEta = 2.5
            alg.selectionTool.minPt = 20. * Units.GeV
            alg.particles = config.readName(self.containerName)
            alg.preselection = config.getPreselection(self.containerName, '')
            jetPreselection = "selectPtEtaFtag&&"+jetPreselection

        alg = config.createAlgorithm('CP::BJetCalibrationAlg',
                                     'BJetCalibAlg_' + self.containerName)
        alg.muons = config.readName(self.muonContainerName)
        alg.muonPreselection = config.getPreselection(self.muonContainerName,
                                                      self.muonPreselection)
        alg.jets = config.readName(self.containerName)
        alg.jetPreselection = jetPreselection
        alg.jetsOut = config.copyName(self.containerName)

        config.addPrivateTool('muonInJetTool', 'MuonInJetCorrectionTool')
        # Adjust dR matching for large-R jets
        if "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets" in alg.jets:
            alg.muonInJetTool.doLargeR = True

        if self.doPtCorr:
            config.addPrivateTool('bJetTool', 'BJetCorrectionTool')

        # (re-)decorate jets with the updated energy
        alg = config.createAlgorithm( 'CP::AsgEnergyDecoratorAlg', 'EnergyDecoratorBJetCalib' + self.containerName )
        alg.particles = config.readName (self.containerName)