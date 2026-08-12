# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
import AthenaCommon.SystemOfUnits as Units


class BJetCalibAnalysisConfig (ConfigBlock) :
    """the ConfigBlock for the b-jet calibration sequence"""

    def __init__ (self) :
        super (BJetCalibAnalysisConfig, self).__init__ ()
        self.setBlockName('BJetCalib')
        self.addDependency('FTag', required=False)
        self.addDependency('Muons', required=True)
        self.addDependency('MuonsWorkingPoint', required=False)
        self.addDependency('FTagJetSF', required=False)
        self.addDependency('JvtWorkingPointEfficiencyConfig', required=False)
        self.addOption ('containerName', '', type=str,
            noneAction='error',
            info="the name of the input jet container.")
        self.addOption ('muonContainerName', '', type=str,
            noneAction='error',
            info="the name of the input muon container.")
        self.addOption ('jetPreselection', "", type=str,
            info="the jet preselection.")
        self.addOption ('muonPreselection', "", type=str,
            info="the muon preselection.")
        self.addOption ('doPtCorr', True, type=bool,
            info=r"whether to run the b-jet $p_\mathrm{T}$ correction on top of the muon-in-jet one.")
        self.addOption ('onlyDecorate', False, type=bool,
            info="whether to only decorate jets with the updated 4-vector.")
        self.addOption ('changeAngularComponents', False, type=bool,
            info="whether to change the angular components of the jet 4-vector when applying the muon-in-jet correction."
                 " This is not recommended, as it can cause unexpected downstream issues as eta/phi is usually not systematically varied.",
            expertMode=True)

    def instanceName (self) :
        """Return the instance name for this block"""
        return self.containerName

    def makeAlgs(self, config):

        if not self.onlyDecorate and self.changeAngularComponents and not config.noSystematics():
            raise ValueError("BJetCalibAnalysisConfig: changeAngularComponents=True is not compatible with systematics. Please set changeAngularComponents=False or run without systematics.")

        # Set up kinematic selection for which ftag selection should be used downstream
        jetPreselection = config.getFullSelection(self.containerName, self.jetPreselection)
        if jetPreselection:
            alg = config.createAlgorithm('CP::AsgSelectionAlg',
                                         'FtagPTEtaCutAlg')
            alg.selectionDecoration = 'selectPtEtaFtag'
            config.addPrivateTool('selectionTool', 'CP::AsgPtEtaSelectionTool')
            alg.selectionTool.maxEta = 2.5
            alg.selectionTool.minPt = 20. * Units.GeV
            alg.particles = config.readName(self.containerName)
            alg.preselection = config.getPreselection(self.containerName, '')
            jetPreselection = "selectPtEtaFtag&&"+jetPreselection

        alg = config.createAlgorithm('CP::BJetCalibrationAlg',
                                     'BJetCalibAlg')
        alg.muons = config.readName(self.muonContainerName)
        alg.muonPreselection = config.getPreselection(self.muonContainerName,
                                                      self.muonPreselection)
        alg.jets = config.readName(self.containerName)
        alg.jetPreselection = jetPreselection
        alg.jetsOut = config.copyName(self.containerName)

        alg.onlyDecorate = self.onlyDecorate

        config.addPrivateTool('muonInJetTool', 'MuonInJetCorrectionTool')
        alg.muonInJetTool.changeAngularComponents = self.changeAngularComponents
        # Adjust dR matching for large-R jets
        if "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets" in alg.jets:
            alg.muonInJetTool.doLargeR = True

        if self.doPtCorr:
            config.addPrivateTool('bJetTool', 'BJetCorrectionTool')

        # (re-)decorate jets with the updated energy
        if not self.onlyDecorate:
            alg = config.createAlgorithm( 'CP::AsgEnergyDecoratorAlg', 'EnergyDecoratorBJetCalib' )
            alg.particles = config.readName (self.containerName)
