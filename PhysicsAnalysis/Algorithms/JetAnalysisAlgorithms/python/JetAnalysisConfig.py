# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration



# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigSequence import groupBlocks
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType
from AthenaCommon.SystemOfUnits	import GeV
from AthenaConfiguration.Enums import LHCPeriod
from AthenaCommon.Logging import logging
import re


class PreJetAnalysisConfig (ConfigBlock) :
    """the ConfigBlock for the common preprocessing of jet sequences"""

    def __init__ (self) :
        super (PreJetAnalysisConfig, self).__init__ ()
        self.addOption ('containerName', '', type=str,
            noneAction='error',
            info="the name of the output container after calibration.")
        self.addOption ('jetCollection', '', type=str,
            noneAction='error',
            info="the jet container to run on. It is interpreted to determine "
            "the correct config blocks to call for small- or large-R jets.")
        self.addOption('outputTruthLabelIDs', False, type=bool,
            info='Enable or disable HadronConeExclTruthLabelID and PartonTruthLabelID decorations')
        # TODO: add info string
        self.addOption ('runOriginalObjectLink', False, type=bool,
            info="")
        self.addOption ('runGhostMuonAssociation', None, type=bool,
            info="whether to set up the jet-ghost-muon association algorithm "
            "CP::JetGhostMuonAssociationAlg. The default is True for non-PHYSLITE and False for PHYSLITE.")
        self.addOption ('runTruthJetTagging', None, type=bool,
            info="whether to set up the jet truth tagging algorithm "
            "CP::JetTruthTagAlg. The default is True.")

    def instanceName (self) :
        """Return the instance name for this block"""
        return self.containerName

    def makeAlgs (self, config) :


        if config.isPhyslite() and self.jetCollection == 'AntiKt4EMPFlowJets' :
            config.setSourceName (self.containerName, "AnalysisJets", originalName = self.jetCollection)
        elif config.isPhyslite() and self.jetCollection == 'AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets' :
            config.setSourceName (self.containerName, "AnalysisLargeRJets", originalName = self.jetCollection)
        else :
            config.setSourceName (self.containerName, self.jetCollection, originalName = self.jetCollection)

        # Relink original jets in case of b-tagging calibration
        if self.runOriginalObjectLink :
            alg = config.createAlgorithm( 'CP::AsgOriginalObjectLinkAlg',
                                          'JetOriginalObjectLinkAlg',
                                           reentrant=True )
            alg.baseContainerName = self.jetCollection
            alg.particles = config.readName (self.containerName)
            if config.wantCopy (self.containerName) :
                alg.particlesOut = config.copyName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')

        # Set up the jet ghost muon association algorithm:
        if (self.runGhostMuonAssociation is None and not config.isPhyslite()) or \
           (self.runGhostMuonAssociation is True):
            alg = config.createAlgorithm( 'CP::JetGhostMuonAssociationAlg',
                                          'JetGhostMuonAssociationAlg' )
            alg.jets = config.readName (self.containerName)
            if config.isPhyslite():
                alg.muons = "AnalysisMuons"
            if config.wantCopy (self.containerName) :
                alg.jetsOut = config.copyName (self.containerName)

        # NB: I'm assuming that the truth tagging is done in PHYSLITE, if not this will
        # need to change
        if (self.runTruthJetTagging or (self.runTruthJetTagging is None)
        ) and config.dataType() is not DataType.Data:
            # Decorate jets with isHS labels (required to retrieve Jvt SFs)
            alg = config.createAlgorithm( 'CP::JetDecoratorAlg', 'JetPileupLabelAlg' )
            config.addPrivateTool( 'decorator', 'JetPileupLabelingTool' )
            alg.jets = config.readName (self.containerName)
            alg.jetsOut = config.copyName (self.containerName)
            alg.decorator.RecoJetContainer = alg.jetsOut.replace ('%SYS%', 'NOSYS')
            alg.decorator.SuppressOutputDependence=True

        # Set up shallow copy if needed and not yet done
        if config.wantCopy (self.containerName) :
            alg = config.createAlgorithm( 'CP::AsgShallowCopyAlg', 'JetShallowCopyAlg' )
            alg.input = config.readName (self.containerName)
            alg.output = config.copyName (self.containerName)

        config.addOutputVar (self.containerName, 'pt', 'pt')
        config.addOutputVar (self.containerName, 'eta', 'eta', noSys=True)
        config.addOutputVar (self.containerName, 'phi', 'phi', noSys=True)
        config.addOutputVar (self.containerName, 'charge', 'charge', noSys=True, enabled=False)

        if self.outputTruthLabelIDs and config.dataType() is not DataType.Data:
            config.addOutputVar (self.containerName, 'HadronConeExclTruthLabelID', 'HadronConeExclTruthLabelID', noSys=True)
            config.addOutputVar (self.containerName, 'PartonTruthLabelID', 'PartonTruthLabelID', noSys=True)



class SmallRJetAnalysisConfig (ConfigBlock) :
    """the ConfigBlock for the small-r jet sequence"""

    def __init__ (self) :
        super (SmallRJetAnalysisConfig, self).__init__ ()
        self.addOption ('containerName', '', type=str,
            noneAction='error',
            info="the name of the output container after calibration.")
        self.addOption ('jetCollection', '', type=str,
            noneAction='error',
            info="the jet container to run on. It is interpreted to determine "
            "the correct config blocks to call for small- or large-R jets.")
        # TODO: add info string
        self.addOption ('jetInput', '', type=str,
            noneAction='error',
            info="")
        self.addOption ('runJvtUpdate', False, type=bool,
            info="whether to update the JVT. The default is False.")
        self.addOption ('runNNJvtUpdate', False, type=bool,
            info="whether to update the NN-JVT. The default is False.")
        self.addOption ('runJvtSelection', True, type=bool,
            info="whether to run JVT selection. The default is True.")
        self.addOption ('runFJvtSelection', False, type=bool,
            info="whether to run forward JVT selection. The default is False.")
        self.addOption ('jvtWP', "FixedEffPt", type=str,
            info="which Jvt WP to apply. The default is FixedEffPt.")
        self.addOption ('fJvtWP', "Loose", type=str,
            info="which fJvt WP to apply. The default is Loose.")
        self.addOption ('runJvtEfficiency', True, type=bool,
            info="whether to calculate the JVT efficiency. The default is True.")
        self.addOption ('runFJvtEfficiency', False, type=bool,
            info="whether to calculate the forward JVT efficiency. The default is False.")
        self.addOption ('recalibratePhyslite', True, type=bool,
            info="whether to run the CP::JetCalibrationAlg on PHYSLITE derivations. "
            "The default is True.")
        # Calibration tool options
        self.addOption ('calibToolConfigFile', None, type=str,
            info="name (str) of the config file to use for the jet calibration "
            "tool. Expert option to override JetETmiss recommendations. The "
            "default is None.",
            expertMode=True)
        self.addOption ('calibToolCalibArea', None, type=str,
            info="name (str) of the CVMFS area to use for the jet calibration "
            "tool. Expert option to override JetETmiss recommendations. The "
            "default is None.",
            expertMode=True)
        self.addOption ('calibToolCalibSeq', None, type=str,
            info="name (str) of the sequence to use for the jet calibration "
            "tool (e.g. 'JetArea_Residual_EtaJES_GSC'). Expert option to override "
            "JetETmiss recommendations. The default is None.",
            expertMode=True)

    def instanceName (self) :
        """Return the instance name for this block"""
        return self.containerName

    def makeAlgs (self, config) :

        jetCollectionName=self.jetCollection
        if(self.jetCollection=="AnalysisJets") :
            jetCollectionName="AntiKt4EMPFlowJets"
        if(self.jetCollection=="AnalysisLargeRJets") :
            jetCollectionName="AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets"

        if self.jetInput not in ["EMTopo", "EMPFlow", "HI"]:
            raise ValueError(
                "Unsupported input type '{0}' for R=0.4 jets!".format(self.jetInput) )

        if self.jvtWP not in ["FixedEffPt"]:
            raise ValueError(
                "Unsupported NNJvt WP '{0}'".format(self.jvtWP) )

        if self.fJvtWP not in ["Loose", "Tight", "Tighter"]:
            raise ValueError(
                "Unsupported fJvt WP '{0}'".format(self.fJvtWP) )

        if not config.isPhyslite() or self.recalibratePhyslite:
            # Prepare the jet calibration algorithm
            alg = config.createAlgorithm( 'CP::JetCalibrationAlg', 'JetCalibrationAlg' )
            config.addPrivateTool( 'calibrationTool', 'JetCalibrationTool' )
            alg.calibrationTool.JetCollection = jetCollectionName[:-4]
            # Get the correct string to use in the config file name
            if self.jetInput == "EMPFlow":
                if config.geometry() is LHCPeriod.Run2:
                    configFile = "PreRec_R22_PFlow_ResPU_EtaJES_GSC_February23_230215.config"
                    alg.calibrationTool.CalibArea = "00-04-82"
                elif config.geometry() >= LHCPeriod.Run3:
                    configFile = "AntiKt4EMPFlow_MC23a_PreRecR22_Phase2_CalibConfig_ResPU_EtaJES_GSC_241208_InSitu.config"
                    alg.calibrationTool.CalibArea = "00-04-83"
            elif self.jetInput == "HI":
                if config.geometry() is LHCPeriod.Run2:
                    configFile = "JES_MC16_HI_Jan2021_5TeV.config"
                if config.geometry() is LHCPeriod.Run3:
                    configFile = "AntiKt4HI_JES_constants_11-05-2024_13p6TeVFinalConfiguration.config"
                    alg.calibrationTool.CalibArea = "00-04-83"
            else:
                if config.dataType() is DataType.FastSim:
                    configFile = "JES_MC16Recommendation_AFII_{0}_Apr2019_Rel21.config"
                else:
                    configFile = "JES_MC16Recommendation_Consolidated_{0}_Apr2019_Rel21.config"
                configFile = configFile.format(self.jetInput)
            if self.calibToolCalibArea is not None:
                alg.calibrationTool.CalibArea = self.calibToolCalibArea
            if self.calibToolConfigFile is not None:
                configFile = self.calibToolConfigFile
            alg.calibrationTool.ConfigFile = configFile
            if config.dataType() is DataType.Data:
                if self.jetInput == "HI":
                    if config.geometry() is LHCPeriod.Run2:
                        alg.calibrationTool.CalibSequence = 'EtaJES_Insitu'
                    if config.geometry() is LHCPeriod.Run3:
                        alg.calibrationTool.CalibSequence = 'EtaJES'
                else:
                    alg.calibrationTool.CalibSequence = 'JetArea_Residual_EtaJES_GSC_Insitu'
            else:
                if self.jetInput == "EMPFlow":
                    alg.calibrationTool.CalibSequence = 'JetArea_Residual_EtaJES_GSC'
                elif self.jetInput == "HI":
                    alg.calibrationTool.CalibSequence = 'EtaJES'
                else:
                    alg.calibrationTool.CalibSequence = 'JetArea_Residual_EtaJES_GSC_Smear'
            if self.calibToolCalibSeq is not None:
                alg.calibrationTool.CalibSequence = self.calibToolCalibSeq
            alg.calibrationTool.IsData = (config.dataType() is DataType.Data)
            alg.jets = config.readName (self.containerName)
            alg.jetsOut = config.copyName (self.containerName)

        # Set up the JVT update algorithm:
        if self.runJvtUpdate :
            alg = config.createAlgorithm( 'CP::JvtUpdateAlg', 'JvtUpdateAlg' )
            config.addPrivateTool( 'jvtTool', 'JetVertexTaggerTool' )
            alg.jvtTool.JetContainer = self.jetCollection
            alg.jvtTool.SuppressInputDependence=True
            alg.jets = config.readName (self.containerName)
            alg.jetsOut = config.copyName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')

        if self.runNNJvtUpdate:
            assert self.jetInput=="EMPFlow", "NN JVT only defined for PFlow jets"
            alg = config.createAlgorithm( 'CP::JetDecoratorAlg', 'NNJvtUpdateAlg' )
            config.addPrivateTool( 'decorator', 'JetPileupTag::JetVertexNNTagger' )
            # Set this actually to the *output* collection
            alg.jets = config.readName (self.containerName)
            alg.jetsOut = config.copyName (self.containerName)
            alg.decorator.JetContainer = alg.jetsOut.replace ('%SYS%', 'NOSYS')
            alg.decorator.SuppressInputDependence=True
            alg.decorator.SuppressOutputDependence=True

        # Set up the jet efficiency scale factor calculation algorithm
        # Change the truthJetCollection property to AntiKt4TruthWZJets if preferred
        if self.runJvtSelection :
            assert self.jetInput=="EMPFlow", "NNJvt WPs and SFs only valid for PFlow jets"
            alg = config.createAlgorithm('CP::AsgSelectionAlg', 'JvtSelectionAlg')
            config.addPrivateTool('selectionTool', 'CP::NNJvtSelectionTool')
            alg.selectionTool.JetContainer = config.readName(self.containerName)
            alg.selectionTool.JvtMomentName = "NNJvt"
            alg.selectionTool.WorkingPoint = self.jvtWP
            alg.selectionTool.MaxPtForJvt = 60*GeV
            alg.selectionDecoration = "jvt_selection,as_char"
            alg.particles = config.readName(self.containerName)

            if self.runJvtEfficiency and config.dataType() is not DataType.Data:
                alg = config.createAlgorithm( 'CP::JvtEfficiencyAlg', 'JvtEfficiencyAlg' )
                config.addPrivateTool( 'efficiencyTool', 'CP::NNJvtEfficiencyTool' )
                alg.efficiencyTool.JetContainer = config.readName(self.containerName)
                alg.efficiencyTool.MaxPtForJvt = 60*GeV
                alg.efficiencyTool.WorkingPoint = self.jvtWP
                if config.geometry() is LHCPeriod.Run2:
                    alg.efficiencyTool.SFFile = "JetJvtEfficiency/May2024/NNJvtSFFile_Run2_EMPFlow.root"
                else:
                    alg.efficiencyTool.SFFile = "JetJvtEfficiency/May2024/NNJvtSFFile_Run3_EMPFlow.root"
                alg.selection = 'jvt_selection,as_char'
                alg.scaleFactorDecoration = 'jvt_effSF_%SYS%'
                alg.outOfValidity = 2
                alg.outOfValidityDeco = 'no_jvt'
                alg.skipBadEfficiency = False
                alg.jets = config.readName (self.containerName)
                alg.preselection = config.getPreselection (self.containerName, '')
                config.addOutputVar (self.containerName, alg.scaleFactorDecoration, 'jvtEfficiency')
            config.addSelection (self.containerName, 'baselineJvt', 'jvt_selection,as_char', preselection=False)

        if self.runFJvtSelection :
            assert self.jetInput=="EMPFlow", "fJvt WPs and SFs only valid for PFlow jets"
            alg = config.createAlgorithm('CP::AsgSelectionAlg', 'FJvtSelectionAlg')
            config.addPrivateTool('selectionTool', 'CP::FJvtSelectionTool')
            alg.selectionTool.JetContainer = config.readName(self.containerName)
            alg.selectionTool.JvtMomentName = "DFCommonJets_fJvt"
            alg.selectionTool.WorkingPoint = self.fJvtWP
            alg.selectionDecoration = "fjvt_selection,as_char"
            alg.particles = config.readName(self.containerName)

            if self.runFJvtEfficiency and config.dataType() is not DataType.Data:
                alg = config.createAlgorithm( 'CP::JvtEfficiencyAlg', 'FJvtEfficiencyAlg' )
                config.addPrivateTool( 'efficiencyTool', 'CP::FJvtEfficiencyTool' )
                alg.efficiencyTool.JetContainer = config.readName(self.containerName)
                alg.efficiencyTool.WorkingPoint = self.fJvtWP
                if config.geometry() is LHCPeriod.Run2:
                    alg.efficiencyTool.SFFile = "JetJvtEfficiency/May2024/fJvtSFFile_Run2_EMPFlow.root"
                else:
                    alg.efficiencyTool.SFFile = "JetJvtEfficiency/May2024/fJvtSFFile_Run3_EMPFlow.root"
                alg.selection = 'fjvt_selection,as_char'
                alg.scaleFactorDecoration = 'fjvt_effSF_%SYS%'
                alg.outOfValidity = 2
                alg.outOfValidityDeco = 'no_fjvt'
                alg.skipBadEfficiency = False
                alg.jets = config.readName (self.containerName)
                alg.preselection = config.getPreselection (self.containerName, '')
                config.addOutputVar (self.containerName, alg.scaleFactorDecoration, 'fjvtEfficiency')
            config.addSelection (self.containerName, 'baselineFJvt', 'fjvt_selection,as_char', preselection=False)

        # Additional decorations
        alg = config.createAlgorithm( 'CP::AsgEnergyDecoratorAlg', 'AsgEnergyDecoratorAlg' )
        alg.particles = config.readName (self.containerName)

        config.addOutputVar (self.containerName, 'e_%SYS%', 'e')


class RScanJetAnalysisConfig (ConfigBlock) :
    """the ConfigBlock for the r-scan jet sequence"""

    def __init__ (self) :
        super (RScanJetAnalysisConfig, self).__init__ ()
        self.addOption ('containerName', '', type=str,
            noneAction='error',
            info="the name of the output container after calibration.")
        self.addOption ('jetCollection', '', type=str,
            noneAction='error',
            info="the jet container to run on. It is interpreted to determine "
            "the correct config blocks to call for small- or large-R jets.")
        # TODO: add info string
        self.addOption ('jetInput', '', type=str,
            noneAction='error',
            info="")
        # TODO: add info string
        self.addOption ('radius', None, type=int,
            noneAction='error',
            info="")
        self.addOption ('recalibratePhyslite', True, type=bool,
            info="whether to run the CP::JetCalibrationAlg on PHYSLITE "
            "derivations. The default is True.")

    def instanceName (self) :
        """Return the instance name for this block"""
        return self.containerName

    def makeAlgs (self, config) :

        log = logging.getLogger('RScanJetAnalysisConfig')

        jetCollectionName=self.jetCollection
        if(self.jetCollection=="AnalysisJets") :
            jetCollectionName="AntiKt4EMPFlowJets"
        if(self.jetCollection=="AnalysisLargeRJets") :
            jetCollectionName="AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets"

        if not config.isPhyslite() or self.recalibratePhyslite:
            if self.jetInput != "LCTopo":
                raise ValueError(
                    "Unsupported input type '{0}' for R-scan jets!".format(self.jetInput) )
            # Prepare the jet calibration algorithm
            alg = config.createAlgorithm( 'CP::JetCalibrationAlg', 'JetCalibrationAlg' )
            config.addPrivateTool( 'calibrationTool', 'JetCalibrationTool' )
            alg.calibrationTool.JetCollection = jetCollectionName[:-4]
            alg.calibrationTool.ConfigFile = \
                "JES_MC16Recommendation_Rscan{0}LC_Feb2022_R21.config".format(self.radius)
            if config.dataType() is DataType.Data:
                alg.calibrationTool.CalibSequence = "JetArea_Residual_EtaJES_GSC_Insitu"
            else:
                alg.calibrationTool.CalibSequence = "JetArea_Residual_EtaJES_GSC_Smear"
            alg.calibrationTool.IsData = (config.dataType() is DataType.Data)
            alg.jets = config.readName (self.containerName)
            # Logging would be good
            log.warning("Uncertainties for R-Scan jets are not yet released!")


def _largeLCTopoConfigFile(config, self):
    is_sim = config.dataType() in {DataType.FullSim}
    if self.largeRMass == "Comb":
        if config.dataType() is DataType.Data:
            return "JES_MC16recommendation_FatJet_Trimmed_JMS_comb_March2021.config"
        if is_sim:
            return "JES_MC16recommendation_FatJet_Trimmed_JMS_comb_17Oct2018.config"
    elif self.largeRMass == "Calo":
        if config.dataType() is DataType.Data:
            return "JES_MC16recommendation_FatJet_Trimmed_JMS_comb_March2021.config"
        if is_sim:
            return "JES_MC16recommendation_FatJet_Trimmed_JMS_calo_12Oct2018.config "
    elif self.largeRMass == "TA":
        if config.dataType() is DataType.Data:
            return "JES_MC16recommendation_FatJet_Trimmed_JMS_comb_March2021.config"
        if is_sim:
            return "JES_MC16recommendation_FatJet_Trimmed_JMS_TA_12Oct2018.config"
    return None


class LargeRJetAnalysisConfig (ConfigBlock) :
    """the ConfigBlock for the large-r jet sequence"""

    def __init__ (self) :
        super (LargeRJetAnalysisConfig, self).__init__ ()
        self.addOption ('containerName', '', type=str,
            noneAction='error',
            info="the name of the output container after calibration.")
        self.addOption ('jetCollection', '', type=str,
            noneAction='error',
            info="the jet container to run on. It is interpreted to determine "
            "the correct config blocks to call for small- or large-R jets.")
        # TODO: add info string
        self.addOption ('jetInput', '', type=str,
            noneAction='error',
            info="")
        # TODO: add info string
        self.addOption ('largeRMass', "Comb", type=str,
            info="")
        self.addOption ('recalibratePhyslite', True, type=bool,
            info="whether to run the CP::JetCalibrationAlg on PHYSLITE "
            "derivations. The default is True.")
        self.addOption ('systematicsModelJMR', "Full", type=str,
            info="the NP reduction scheme to use for JMR: Full, Simple. The default is Full.")
        # Adding these options to override the jet uncertainty config file when we have new recommendations
        # Calibration tool options
        self.addOption ('calibToolConfigFile', None, type=str,
            info="name (str) of the config file to use for the jet calibration "
            "tool. Expert option to override JetETmiss recommendations. The "
            "default is None.",
            expertMode=True)
        self.addOption ('calibToolCalibArea', None, type=str,
            info="name (str) of the CVMFS area to use for the jet calibration "
            "tool. Expert option to override JetETmiss recommendations. The "
            "default is None.",
            expertMode=True)
        self.addOption ('calibToolCalibSeq', None, type=str,
            info="name (str) of the sequence to use for the jet calibration "
            "tool (e.g. 'JetArea_Residual_EtaJES_GSC'). Expert option to override "
            "JetETmiss recommendations. The default is None.",
            expertMode=True)
        # Uncertainties tool options
        self.addOption ('uncertToolConfigPath', None, type=str,
            info="name (str) of the config file to use for the JES, JER, and JMS uncertainty "
            "tool. Expert option to override JetETmiss recommendations. The "
            "default is None.",
            expertMode=True)
        self.addOption ('uncertToolConfigPathJMR', None, type=str,
            info="name (str) of the config file to use for the JMR uncertainty "
            "tool. Expert option to override JetETmiss recommendations. The "
            "default is None.",
            expertMode=True)
        self.addOption ('minPt', 200.*GeV, type=float,
            info="the minimum pt cut to apply to calibrated large-R jets. "
            "The default is 200 GeV.")
        self.addOption ('maxPt', 3000.*GeV, type=float,
            info="the maximum pt cut to apply to calibrated large-R jets. "
            "The default is 3000 GeV.")
        self.addOption ('maxEta', 0., type=float,
            info="the maximum |eta| cut to apply to calibrated large-R jets. "
            "The default is 0.")
        self.addOption ('maxRapidity', 2., type=float,
            info="the maximum rapidity cut to apply to calibrated large-R jets. "
            "The default is 2.")        
        self.addOption ('minMass', 40.*GeV, type=float,
            info="the minimum mass cut to apply to calibrated large-R jets. "
            "The default is 40 GeV.")
        self.addOption ('maxMass', 600.*GeV, type=float,
            info="the maximum mass cut to apply to calibrated large-R jets. "
            "The default is 600 GeV.")

    def instanceName (self) :
        """Return the instance name for this block"""
        return self.containerName


    def createFFSmearingTool(self, jetFFSmearingAlg, config):
        # Retrieve appropriate large-R jet mass resolution recommendations for the FFJetSmearingTool.

        log = logging.getLogger('LargeRJetAnalysisConfig')

        # Config file:
        if self.systematicsModelJMR in ["Simple", "Full"]:
            config_file = f"R10_{self.systematicsModelJMR}JMR.config"
        else:
            raise ValueError(
                f"Invalid request for systematicsModelJMR settings: {self.systematicsModelJMR}"
            )

        # Expert override for config path:
        if self.uncertToolConfigPathJMR is not None:
            config_file = self.uncertToolConfigPathJMR
        else:
            config_file = "rel22/Summer2025_PreRec/" + config_file
            if config.geometry() is LHCPeriod.Run4:
                log.warning("Uncertainties for UFO jets are not for Run 4!")

        # MC type:
        if config.geometry() is LHCPeriod.Run2:
            if config.dataType() is DataType.FastSim:
                mc_type = "MC20AF3"
            else:
                mc_type = "MC20"
        elif config.geometry() >= LHCPeriod.Run3:
            if config.dataType() is DataType.FastSim:
                mc_type = "MC23AF3"
            else:
                mc_type = "MC23"

        # Set up the FF smearing tool
        config.addPrivateTool( 'FFSmearingTool', 'CP::FFJetSmearingTool')
        jetFFSmearingAlg.FFSmearingTool.MassDef = "UFO"
        jetFFSmearingAlg.FFSmearingTool.MCType = mc_type
        jetFFSmearingAlg.FFSmearingTool.ConfigFile = config_file

    def makeAlgs (self, config) :

        configFile = None
        calibSeq = None
        calibArea = None

        jetCollectionName=self.jetCollection
        if(self.jetCollection=="AnalysisJets") :
            jetCollectionName="AntiKt4EMPFlowJets"
        if(self.jetCollection=="AnalysisLargeRJets") :
            jetCollectionName="AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets"

        if self.largeRMass not in ["Comb", "Calo", "TA"]:
            raise ValueError("Invalid large-R mass defintion {0}!".format(self.largeRMass) )

        if self.jetInput not in ["LCTopo", "TrackCaloCluster", "UFO"]:
            raise ValueError("Invalid input type '{0}' for large-R jets!".format(self.jetInput) )

        if self.jetInput == "TrackCaloCluster":
            # Only one mass defintion supported
            if self.largeRMass != "Calo":
                raise ValueError("Invalid large-R TCC jet mass '{0}'!".format(self.largeRMass) )
            configFile = "JES_MC16recommendation_FatJet_TCC_JMS_calo_30Oct2018.config"
        if self.jetInput == "LCTopo":
            configFile = _largeLCTopoConfigFile(config, self)
        if self.jetInput == "UFO":
            configFile = "JES_MC20PreRecommendation_R10_UFO_CSSK_SoftDrop_JMS_R21Insitu_26Nov2024.config"
            calibArea = "00-04-83"
        if self.calibToolConfigFile is not None:
            configFile = self.calibToolConfigFile

        if self.jetInput == "TrackCaloCluster" or self.jetInput == "UFO" or config.dataType() is DataType.FullSim:
            calibSeq = "EtaJES_JMS"
        elif config.dataType() is DataType.Data:
            calibSeq = "EtaJES_JMS_Insitu"
        if self.calibToolCalibSeq is not None:
            calibSeq = self.calibToolCalibSeq

        if self.calibToolCalibArea is not None:
            calibArea = self.calibToolCalibArea

        if not config.isPhyslite() or self.recalibratePhyslite:
            # Prepare the jet calibration algorithm
            alg = config.createAlgorithm( 'CP::JetCalibrationAlg', 'JetCalibrationAlg' )
            config.addPrivateTool( 'calibrationTool', 'JetCalibrationTool' )

            alg.calibrationTool.JetCollection = jetCollectionName[:-4]

            if configFile is None:
                raise ValueError(f'Unsupported: {self.jetInput=}, {config.dataType()=}')
            alg.calibrationTool.ConfigFile = configFile

            if calibSeq is None:
                raise ValueError(f'Unsupported: {self.jetInput=}, {config.dataType()=}')
            alg.calibrationTool.CalibSequence = calibSeq

            if calibArea is not None:
                alg.calibrationTool.CalibArea = calibArea

            alg.calibrationTool.IsData = (config.dataType() is DataType.Data)
            alg.jets = config.readName(self.containerName)

        if self.jetInput == "UFO" and config.dataType() is not DataType.Data:
            # set up the FF smearing algorithm
            alg = config.createAlgorithm( 'CP::JetFFSmearingAlg', 'JetFFSmearingAlg' )
            self.createFFSmearingTool(alg, config)
            alg.outOfValidity = 2 # SILENT
            alg.outOfValidityDeco = 'outOfValidityJMR'
            alg.jets = config.readName (self.containerName)
            alg.jetsOut = config.copyName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')

        if self.minPt > 0 or self.maxPt > 0 or self.maxEta > 0 or self.maxRapidity > 0:
            # Set up the the pt-eta selection
            alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'JetPtEtaCutAlg' )
            alg.selectionDecoration = 'selectPtEta,as_bits'
            config.addPrivateTool( 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
            alg.selectionTool.minPt = self.minPt
            alg.selectionTool.maxPt = self.maxPt
            alg.selectionTool.maxEta = self.maxEta
            alg.selectionTool.maxRapidity = self.maxRapidity            
            alg.particles = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')
            config.addSelection (self.containerName, '', alg.selectionDecoration,
                                 preselection=True)
            
        if self.minMass > 0 or self.maxMass > 0:
            # Set up the the mass selection
            alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'JetMassCutAlg' )
            alg.selectionDecoration = 'selectMass,as_bits'
            config.addPrivateTool( 'selectionTool', 'CP::AsgMassSelectionTool' )
            alg.selectionTool.minM = self.minMass
            alg.selectionTool.maxM = self.maxMass
            alg.particles = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')
            config.addSelection (self.containerName, '', alg.selectionDecoration,
                                 preselection=True)
            
        config.addOutputVar (self.containerName, 'm', 'm')

# These algorithms set up the jet recommendations as-of 04/02/2019.
# Jet calibration recommendations
# https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/ApplyJetCalibrationR21
# Jet uncertainties recommendations
# Small-R
# https://twiki.cern.ch/twiki/bin/view/AtlasProtected/JetUncertaintiesRel21Summer2018SmallR
# Large-R
# https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/JetUncertaintiesRel21Moriond2018LargeR
# JVT recommendations
# https://twiki.cern.ch/twiki/bin/view/AtlasProtected/JVTCalibrationRel21

@groupBlocks
def makeJetAnalysisConfig( seq, containerName, jetCollection,
                           runGhostMuonAssociation = None):
    """Create a jet analysis algorithm sequence
      The jet collection is interpreted and selects the correct function to call,
      makeSmallRJetAnalysisConfig, makeRScanJetAnalysisConfig or
      makeLargeRJetAnalysisConfig

      Keyword arguments
        jetCollection -- The jet container to run on.
    """

    # Remove b-tagging calibration from the container name
    btIndex = jetCollection.find('_BTagging')
    if btIndex != -1:
        jetCollection = jetCollection[:btIndex]

    jetCollectionName=jetCollection
    # needed for PHYSLITE
    if(jetCollection=="AnalysisJets") :
        jetCollectionName="AntiKt4EMPFlowJets"
    if(jetCollection=="AnalysisLargeRJets") :
        jetCollectionName="AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets"

    # interpret the jet collection
    collection_pattern = re.compile(
        r"AntiKt(\d+)(EMTopo|EMPFlow|LCTopo|TrackCaloCluster|UFO|Track|HI)(TrimmedPtFrac5SmallR20|CSSKSoftDropBeta100Zcut10)?Jets")
    match = collection_pattern.match(jetCollectionName)
    if not match:
        raise ValueError(
            "Jet collection {0} does not match expected pattern!".format(jetCollectionName) )
    radius = int(match.group(1) )
    if radius not in [2, 4, 6, 10]:
        raise ValueError("Jet collection has an unsupported radius '{0}'!".format(radius) )
    jetInput = match.group(2)

    config = PreJetAnalysisConfig()
    config.setOptionValue ('containerName', containerName)
    config.setOptionValue ('jetCollection', jetCollection)
    config.runOriginalObjectLink = (btIndex != -1)
    config.setOptionValue ('runGhostMuonAssociation', runGhostMuonAssociation)
    seq.append (config)

    if radius == 4:
        makeSmallRJetAnalysisConfig(seq, containerName,
            jetCollection, jetInput=jetInput)
    elif radius in [2, 6]:
        makeRScanJetAnalysisConfig(seq, containerName,
            jetCollection, jetInput=jetInput, radius=radius)
    else:
        trim = match.group(3)
        if trim == "":
            raise ValueError("Untrimmed large-R jets are not supported!")
        makeLargeRJetAnalysisConfig(seq, containerName,
            jetCollection, jetInput=jetInput)



def makeSmallRJetAnalysisConfig( seq, containerName, jetCollection, jetInput,
                                 runJvtUpdate = None, runNNJvtUpdate = None,
                                 runJvtSelection = None, runFJvtSelection = None,
                                 jvtWP = None, fJvtWP = None,
                                 runJvtEfficiency = None, runFJvtEfficiency = None):
    """Add algorithms for the R=0.4 jets.

      Keyword arguments
        seq -- The sequence to add the algorithms to
        jetCollection -- The jet container to run on.
        jetInput -- The type of input used, read from the collection name.
        runJvtUpdate -- Determines whether or not to update JVT on the jets
        runNNJvtUpdate -- Determines whether or not to update NN JVT on the jets
        runJvtSelection -- Determines whether or not to run JVT selection on the jets
        runFJvtSelection -- Determines whether or not to run forward JVT selection on the jets
        jvtWP -- Defines the NNJvt WP to apply on the jets
        fJvtWP -- Defines the fJvt WP to apply on the jets
        runJvtEfficiency -- Determines whether or not to calculate the JVT efficiency
        runFJvtEfficiency -- Determines whether or not to calculate the forward JVT efficiency
    """

    if jetInput not in ["EMTopo", "EMPFlow", "HI"]:
        raise ValueError(
            "Unsupported input type '{0}' for R=0.4 jets!".format(jetInput) )

    config = SmallRJetAnalysisConfig()
    config.setOptionValue ('containerName', containerName)
    config.setOptionValue ('jetCollection', jetCollection)
    config.setOptionValue ('jetInput', jetInput)
    config.setOptionValue ('runJvtUpdate', runJvtUpdate)
    config.setOptionValue ('runNNJvtUpdate', runNNJvtUpdate)
    config.setOptionValue ('runJvtSelection', runJvtSelection)
    config.setOptionValue ('runFJvtSelection', runFJvtSelection)
    config.setOptionValue ('jvtWP', jvtWP)
    config.setOptionValue ('fJvtWP', fJvtWP)
    config.setOptionValue ('runJvtEfficiency', runJvtEfficiency)
    config.setOptionValue ('runFJvtEfficiency', runFJvtEfficiency)
    seq.append (config)


def makeRScanJetAnalysisConfig( seq, containerName, jetCollection,
                                  jetInput, radius ):
    """Add algorithms for the R-scan jets.

      Keyword arguments
        seq -- The sequence to add the algorithms to
        jetCollection -- The jet container to run on.
        jetInput -- The type of input used, read from the collection name.
        radius -- The radius of the r-scan jets.
    """

    config = RScanJetAnalysisConfig()
    config.setOptionValue ('containerName', containerName)
    config.setOptionValue ('jetCollection', jetCollection)
    config.setOptionValue ('jetInput', jetInput)
    config.setOptionValue ('radius', radius)
    seq.append (config)




def makeLargeRJetAnalysisConfig( seq, containerName, jetCollection,
                                 jetInput, largeRMass = None):
    """Add algorithms for the R=1.0 jets.

      Keyword arguments
        seq -- The sequence to add the algorithms to
        jetCollection -- The jet container to run on.
        jetInput -- The type of input used, read from the collection name.
        largeRMass -- Which large-R mass definition to use. Ignored if not running on large-R jets ("Comb", "Calo", "TA")
    """
    config = LargeRJetAnalysisConfig()
    config.setOptionValue ('containerName', containerName)
    config.setOptionValue ('jetCollection', jetCollection)
    config.setOptionValue ('jetInput', jetInput)
    config.setOptionValue ('largeRMass', largeRMass)
    seq.append (config)

