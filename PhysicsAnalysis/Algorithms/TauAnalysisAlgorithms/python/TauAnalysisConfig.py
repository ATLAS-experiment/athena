# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType
from Campaigns.Utils import Campaign
from AthenaCommon.Logging import logging


class TauCalibrationConfig (ConfigBlock):
    """the ConfigBlock for the tau four-momentum correction"""

    def __init__ (self, containerName='') :
        super (TauCalibrationConfig, self).__init__ ()
        self.setBlockName('Taus')
        self.containerName = containerName
        self.addOption ('inputContainer', 'TauJets', type=str,
            info="select tau input container, by default set to TauJets")
        self.addOption ('containerName', containerName, type=str,
            noneAction='error',
            info="the name of the output container after calibration.")
        self.addOption ('postfix', '', type=str,
            info="a postfix to apply to decorations and algorithm names. "
            "Typically not needed here since the calibration is common to "
            "all taus.")
        self.addOption ('rerunTruthMatching', True, type=bool,
            info="whether to rerun truth matching (sets up an instance of "
            "CP::TauTruthMatchingAlg). The default is True.")
        self.addOption ('decorateTruth', False, type=bool,
            info="decorate truth particle information on the reconstructed one")


    def makeAlgs (self, config) :

        postfix = self.postfix
        if postfix != '' and postfix[0] != '_' :
            postfix = '_' + postfix

        if config.isPhyslite() :
            config.setSourceName (self.containerName, "AnalysisTauJets")
        else :
            config.setSourceName (self.containerName, self.inputContainer)

        # Set up the tau truth matching algorithm:
        if self.rerunTruthMatching and config.dataType() is not DataType.Data:
            alg = config.createAlgorithm( 'CP::TauTruthMatchingAlg',
                                          'TauTruthMatchingAlg' + postfix )
            config.addPrivateTool( 'matchingTool',
                                   'TauAnalysisTools::TauTruthMatchingTool' )
            alg.matchingTool.TruthJetContainerName = 'AntiKt4TruthDressedWZJets'
            alg.taus = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')

        # decorate truth tau information on the reconstructed object:
        if self.decorateTruth and config.dataType() is not DataType.Data:
            alg = config.createAlgorithm( 'CP::TauTruthDecorationsAlg',
                                        'TauTruthDecorationsAlg' + postfix )
            alg.taus = config.readName (self.containerName)
            alg.doubleDecorations = ['pt_vis', 'eta_vis', 'phi_vis', 'm_vis']
            alg.floatDecorations = []
            alg.intDecorations = ['pdgId']
            alg.unsignedIntDecorations = ['classifierParticleOrigin', 'classifierParticleType']
            alg.charDecorations = ['IsHadronicTau']
            alg.prefix = 'truth_'

            # these are "_ListHelper" objects, and not "list", need to copy to lists to allow concatenate
            for var in ['DecayMode', 'ParticleType', 'PartonTruthLabelID'] + alg.doubleDecorations[:] + alg.floatDecorations[:] + alg.intDecorations[:] + alg.unsignedIntDecorations[:] + alg.charDecorations[:]:
                branchName = alg.prefix + var
                if 'classifierParticle' in var:
                    branchOutput = alg.prefix + var.replace('classifierParticle', '').lower()
                else:
                    branchOutput = branchName
                config.addOutputVar (self.containerName, branchName, branchOutput, noSys=True)

        # Decorate extra variables
        alg = config.createAlgorithm( 'CP::TauExtraVariablesAlg', 'TauExtraVariables' + self.containerName + self.postfix )
        alg.taus = config.readName (self.containerName)

        # Set up the tau 4-momentum smearing algorithm:
        alg = config.createAlgorithm( 'CP::TauSmearingAlg', 'TauSmearingAlg' + postfix )
        config.addPrivateTool( 'smearingTool', 'TauAnalysisTools::TauSmearingTool' )
        alg.smearingTool.useFastSim = config.dataType() is DataType.FastSim
        alg.taus = config.readName (self.containerName)
        alg.tausOut = config.copyName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')

        # Additional decorations
        alg = config.createAlgorithm( 'CP::AsgEnergyDecoratorAlg', 'EnergyDecorator' + self.containerName + self.postfix )
        alg.particles = config.readName (self.containerName)

        config.addOutputVar (self.containerName, 'pt', 'pt')
        config.addOutputVar (self.containerName, 'eta', 'eta', noSys=True)
        config.addOutputVar (self.containerName, 'phi', 'phi', noSys=True)
        config.addOutputVar (self.containerName, 'e_%SYS%', 'e')
        config.addOutputVar (self.containerName, 'charge', 'charge', noSys=True)
        config.addOutputVar (self.containerName, 'NNDecayMode', 'NNDecayMode', noSys=True)
        config.addOutputVar (self.containerName, 'nTracks', 'nTracks', noSys=True)


class TauWorkingPointConfig (ConfigBlock) :
    """the ConfigBlock for the tau working point

    This may at some point be split into multiple blocks (16 Mar 22)."""

    def __init__ (self, containerName='', selectionName='') :
        super (TauWorkingPointConfig, self).__init__ ()
        self.addOption ('containerName', containerName, type=str,
            noneAction='error',
            info="the name of the input container.")
        self.addOption ('selectionName', selectionName, type=str,
            noneAction='error',
            info="the name of the tau-jet selection to define (e.g. tight or "
            "loose).")
        self.addOption ('postfix', None, type=str,
            info="a postfix to apply to decorations and algorithm names. "
            "Typically not needed here as selectionName is used internally.")
        self.addOption ('quality', None, type=str,
            info="the ID WP (string) to use. Supported ID WPs: Tight, Medium, "
            "Loose, VeryLoose, Baseline.")
        self.addOption ('use_eVeto', False, type=bool,
            info="use selection with or without eVeto combined with tauID "
            "recommendations: set it to True if electron mis-reconstructed as tau is a large background for your analysis")
        self.addOption ('useGNTau', False, type=bool,
            info="use GNTau based ID instead of RNNTau ID "
            "recommendations: that's new experimental feature and might come default soon")
        self.addOption ('noEffSF', False, type=bool,
            info="disables the calculation of efficiencies and scale factors. "
            "Experimental! only useful to test a new WP for which scale "
            "factors are not available. The default is False.")

    def makeAlgs (self, config) :

        selectionPostfix = self.selectionName
        if selectionPostfix != '' and selectionPostfix[0] != '_' :
            selectionPostfix = '_' + selectionPostfix

        postfix = self.postfix
        if postfix is None :
            postfix = self.selectionName
        if postfix != '' and postfix[0] != '_' :
            postfix = '_' + postfix

        if self.useGNTau:
            nameFormat = 'TauAnalysisAlgorithms/tau_selection_gntau_{}_eleid.conf'
            if not self.use_eVeto:
                nameFormat = 'TauAnalysisAlgorithms/tau_selection_gntau_{}_noeleid.conf'
        else:
            nameFormat = 'TauAnalysisAlgorithms/tau_selection_{}_eleid.conf'
            if not self.use_eVeto:
                nameFormat = 'TauAnalysisAlgorithms/tau_selection_{}_noeleid.conf'

        if self.quality not in ['Tight', 'Medium', 'Loose', 'VeryLoose', 'Baseline'] :
            raise ValueError ("invalid tau quality: \"" + self.quality +
                              "\", allowed values are Tight, Medium, Loose, " +
                              "VeryLoose, Baseline")
        inputfile = nameFormat.format(self.quality.lower())

        # Set up the algorithm selecting taus:
        alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'TauSelectionAlg' + postfix )
        config.addPrivateTool( 'selectionTool', 'TauAnalysisTools::TauSelectionTool' )
        alg.selectionTool.ConfigPath = inputfile
        alg.selectionDecoration = 'selected_tau' + selectionPostfix + ',as_bits'
        alg.particles = config.readName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, self.selectionName)
        config.addSelection (self.containerName, self.selectionName, alg.selectionDecoration)

        # Set up the algorithm calculating the efficiency scale factors for the
        # taus:
        if config.dataType() is not DataType.Data and not self.noEffSF and not self.useGNTau:
            # need multiple instances of the TauEfficiencyCorrectionTool
            # 1) Reco 2) TauID, 3) eVeto for fake tau 4) eVeto for true tau
            # 3) and 4) are optional if eVeto is used in TauSelectionTool

            # TauEfficiencyCorrectionTool for Reco, this should be always enabled
            alg = config.createAlgorithm( 'CP::TauEfficiencyCorrectionsAlg',
                                   'TauEfficiencyCorrectionsAlgReco' + postfix )
            config.addPrivateTool( 'efficiencyCorrectionsTool',
                            'TauAnalysisTools::TauEfficiencyCorrectionsTool' )
            alg.efficiencyCorrectionsTool.EfficiencyCorrectionTypes = [0]
            alg.efficiencyCorrectionsTool.useFastSim = config.dataType() is DataType.FastSim
            alg.scaleFactorDecoration = 'tau_Reco_effSF' + selectionPostfix + '_%SYS%'
            alg.outOfValidity = 2 #silent
            alg.outOfValidityDeco = 'bad_Reco_eff' + selectionPostfix
            alg.taus = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, self.selectionName)
            config.addOutputVar (self.containerName, alg.scaleFactorDecoration, 'Reco_effSF' + postfix)

            # TauEfficiencyCorrectionTool for Identification, use only in case TauID is requested in TauSelectionTool
            if self.quality not in ('VeryLoose','Baseline'):

                alg = config.createAlgorithm( 'CP::TauEfficiencyCorrectionsAlg',
                                   'TauEfficiencyCorrectionsAlgID' + postfix )
                config.addPrivateTool( 'efficiencyCorrectionsTool',
                                'TauAnalysisTools::TauEfficiencyCorrectionsTool' )
                alg.efficiencyCorrectionsTool.EfficiencyCorrectionTypes = [4]
                if self.quality=="Loose":
                    JetIDLevel = 7
                elif self.quality=="Medium":
                    JetIDLevel = 8
                elif self.quality=="Tight":
                    JetIDLevel = 9
                else:
                    raise ValueError ("invalid tauID: \"" + self.quality + "\". Allowed values are loose, medium, tight")

                alg.efficiencyCorrectionsTool.JetIDLevel = JetIDLevel
                alg.efficiencyCorrectionsTool.useFastSim = config.dataType() is DataType.FastSim
                alg.scaleFactorDecoration = 'tau_ID_effSF' + selectionPostfix + '_%SYS%'
                alg.outOfValidity = 2 #silent
                alg.outOfValidityDeco = 'bad_ID_eff' + selectionPostfix
                alg.taus = config.readName (self.containerName)
                alg.preselection = config.getPreselection (self.containerName, self.selectionName)
                config.addOutputVar (self.containerName, alg.scaleFactorDecoration, 'ID_effSF' + postfix)

            # TauEfficiencyCorrectionTool for eVeto both on true tau and fake tau, use only in case eVeto is requested in TauSelectionTool
            if self.use_eVeto:

                # correction for fake tau
                alg = config.createAlgorithm( 'CP::TauEfficiencyCorrectionsAlg',
                                   'TauEfficiencyCorrectionsAlgEvetoFakeTau' + postfix )
                config.addPrivateTool( 'efficiencyCorrectionsTool',
                                'TauAnalysisTools::TauEfficiencyCorrectionsTool' )

                alg.efficiencyCorrectionsTool.EfficiencyCorrectionTypes = [10]
                # since all TauSelectionTool config files have loose eRNN, code only this option for now
                alg.efficiencyCorrectionsTool.EleIDLevel = 2
                alg.efficiencyCorrectionsTool.useFastSim = config.dataType() is DataType.FastSim
                alg.scaleFactorDecoration = 'tau_EvetoFakeTau_effSF' + selectionPostfix + '_%SYS%'
                alg.outOfValidity = 2 #silent
                alg.outOfValidityDeco = 'bad_EvetoFakeTau_eff' + selectionPostfix
                alg.taus = config.readName (self.containerName)
                alg.preselection = config.getPreselection (self.containerName, self.selectionName)
                config.addOutputVar (self.containerName, alg.scaleFactorDecoration, 'EvetoFakeTau_effSF' + postfix)

                # correction for true tau
                alg = config.createAlgorithm( 'CP::TauEfficiencyCorrectionsAlg',
                                   'TauEfficiencyCorrectionsAlgEvetoTrueTau' + postfix )
                config.addPrivateTool( 'efficiencyCorrectionsTool',
                                'TauAnalysisTools::TauEfficiencyCorrectionsTool' )

                alg.efficiencyCorrectionsTool.EfficiencyCorrectionTypes = [8]
                alg.efficiencyCorrectionsTool.useFastSim = config.dataType() is DataType.FastSim
                alg.scaleFactorDecoration = 'tau_EvetoTrueTau_effSF' + selectionPostfix + '_%SYS%'
                alg.outOfValidity = 2 #silent
                alg.outOfValidityDeco = 'bad_EvetoTrueTau_eff' + selectionPostfix
                alg.taus = config.readName (self.containerName)
                alg.preselection = config.getPreselection (self.containerName, self.selectionName)
                config.addOutputVar (self.containerName, alg.scaleFactorDecoration, 'EvetoTrueTau_effSF' + postfix)


class EXPERIMENTAL_TauCombineMuonRemovalConfig (ConfigBlock) :
    def __init__ (self, inputTaus = 'TauJets', inputTausMuRM = 'TauJets_MuonRM', outputTaus = 'TauJets_MuonRmCombined', postfix = '') :
        super (EXPERIMENTAL_TauCombineMuonRemovalConfig, self).__init__ ()
        self.addOption (
            'inputTaus', inputTaus, type=str,
            noneAction='error',
            info="the name of the input tau container."
        )
        self.addOption (
            'inputTausMuRM', inputTausMuRM, type=str,
            noneAction='error',
            info="the name of the input tau container with muon removal applied."
        )
        self.addOption ('postfix', postfix, type=str,
            info="a postfix to apply to decorations and algorithm names. "
            "Typically not needed here as selectionName is used internally."
        )
        self.addOption (
            'outputTaus', outputTaus, type=str,
            noneAction='error',
            info="the name of the output tau container."
        )

    def makeAlgs (self, config) :

        postfix = self.postfix
        if postfix != '' and postfix[0] != '_' :
            postfix = '_' + postfix

        if config.isPhyslite() :
            raise(RuntimeError("Muon removal taus is not available in Physlite mode"))

        alg = config.createAlgorithm( 'CP::TauCombineMuonRMTausAlg', 'TauCombineMuonRMTausAlg' + postfix )
        alg.taus = self.inputTaus
        alg.muonrm_taus = self.inputTausMuRM
        alg.combined_taus = self.outputTaus


def EXPERIMENTAL_makeTauCombineMuonRemovalConfig( seq, inputTaus = 'TauJets',
                                                 inputTausMuRM = 'TauJets_MuonRM',
                                                 outputTaus = 'TauJets_MuonRmCombined',
                                                 postfix = ''):
    config = EXPERIMENTAL_TauCombineMuonRemovalConfig (
        inputTaus = inputTaus,
        inputTausMuRM = inputTausMuRM,
        outputTaus = outputTaus,
        postfix = postfix,
    )
    seq.append (config)


def makeTauCalibrationConfig( seq, containerName, inputContainer='TauJets',
                              postfix = None, rerunTruthMatching = None):
    """Create tau calibration analysis algorithms

    This makes all the algorithms that need to be run first befor
    all working point specific algorithms and that can be shared
    between the working points.

    Keyword arguments:
      postfix -- a postfix to apply to decorations and algorithm
                 names.  this is mostly used/needed when using this
                 sequence with multiple working points to ensure all
                 names are unique.
      rerunTruthMatching -- Whether or not to rerun truth matching
    """

    config = TauCalibrationConfig (containerName)
    config.setOptionValue ('inputContainer', inputContainer)
    if postfix is not None :
        config.setOptionValue ('postfix', postfix)
    if rerunTruthMatching is not None :
        config.setOptionValue ('rerunTruthMatching', rerunTruthMatching)
    seq.append (config)





def makeTauWorkingPointConfig( seq, containerName, workingPoint, selectionName,
                               noEffSF = None ):
    """Create tau analysis algorithms for a single working point

    Keyword arguments:
      selectionName -- a postfix to apply to decorations and algorithm
                 names.  this is mostly used/needed when using this
                 sequence with multiple working points to ensure all
                 names are unique.
      noEffSF -- Disables the calculation of efficiencies and scale factors
    """

    config = TauWorkingPointConfig (containerName, selectionName)
    if workingPoint is not None :
        splitWP = workingPoint.split ('.')
        if len (splitWP) != 1 :
            raise ValueError ('working point should be of format "quality", not ' + workingPoint)
        config.setOptionValue ('quality', splitWP[0])
    config.setOptionValue ('noEffSF', noEffSF)
    seq.append (config)


class TauTriggerAnalysisSFBlock (ConfigBlock):

    def __init__ (self, configName='') :
        super (TauTriggerAnalysisSFBlock, self).__init__ ()

        self.addOption ('triggerChainsPerYear', {}, type=None,
                        info="a dictionary with key (string) the year and value (list of "
                        "strings) the trigger chains. The default is {} (empty dictionary).")
        self.addOption ('tauID', '', type=str,
                        info="the tau quality WP (string) to use.")
        self.addOption ('containerName', '', type=str,
                        info="the input tau container, with a possible selection, in "
                        "the format container or container.selection.")

    def get_year_data(self, dictionary: dict, year: int | str) -> list:
        return dictionary.get(int(year), dictionary.get(str(year), []))

    def makeAlgs (self, config) :

        if config.dataType() is not DataType.Data:
            if config.campaign() is Campaign.MC20a:
                triggers = self.get_year_data(self.triggerChainsPerYear, 2015)
                triggers += self.get_year_data(self.triggerChainsPerYear, 2016)
                # Remove potential duplicates
                triggers = list(set(triggers))
            elif config.campaign() is Campaign.MC20d:
                triggers = self.get_year_data(self.triggerChainsPerYear, 2017)
            elif config.campaign() is Campaign.MC20e:
                triggers = self.get_year_data(self.triggerChainsPerYear, 2018)
            elif config.campaign() in [Campaign.MC21a, Campaign.MC23a]:
                triggers = self.get_year_data(self.triggerChainsPerYear, 2022)
            elif config.campaign() in [Campaign.MC23c, Campaign.MC23d]:
                triggers = self.get_year_data(self.triggerChainsPerYear, 2023)
            else:
                logging.warning("unknown campaign, skipping triggers: " + str(config.campaign()))
                triggers = []

            for trig in triggers:
                trig = trig.replace("HLT_","")
                alg = config.createAlgorithm( 'CP::TauEfficiencyCorrectionsAlg',
                                              'TauTrigEfficiencyCorrectionsAlg' + trig )
                config.addPrivateTool( 'efficiencyCorrectionsTool',
                                       'TauAnalysisTools::TauEfficiencyCorrectionsTool' )
                # SFTriggerHadTau correction type from
                # https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/TauID/TauAnalysisTools/TauAnalysisTools/Enums.h#L79
                alg.efficiencyCorrectionsTool.EfficiencyCorrectionTypes = [12]
                alg.efficiencyCorrectionsTool.TriggerName = 'HLT_' + trig

                # JetIDLevel from
                # https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/TauID/TauAnalysisTools/TauAnalysisTools/Enums.h#L79
                if self.tauID=="Loose":
                    JetIDLevel = 7
                elif self.tauID=="Medium":
                    JetIDLevel = 8
                elif self.tauID=="Tight":
                    JetIDLevel = 9
                else:
                    raise ValueError ("invalid tauID: \"" + self.tauID + "\". Allowed values are loose, medium, tight")
                alg.efficiencyCorrectionsTool.JetIDLevel = JetIDLevel
                alg.efficiencyCorrectionsTool.TriggerSFMeasurement = "combined"
                alg.efficiencyCorrectionsTool.useFastSim = config.dataType() is DataType.FastSim

                alg.scaleFactorDecoration = 'tau_trigEffSF_' + trig + '_%SYS%'
                alg.outOfValidity = 2 #silent
                alg.outOfValidityDeco = 'bad_eff_tautrig_' + trig
                alg.taus = config.readName (self.containerName)
                alg.preselection = config.getPreselection (self.containerName, self.tauID)
                config.addOutputVar (self.containerName, alg.scaleFactorDecoration, 'trigEffSF_' + trig)

        return
