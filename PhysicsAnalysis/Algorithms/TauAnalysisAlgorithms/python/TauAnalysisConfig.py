# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AthenaCommon.Logging import logging
from AthenaConfiguration.Enums import LHCPeriod
from Campaigns.Utils import Campaign


class TauCalibrationConfig (ConfigBlock):
    """the ConfigBlock for the tau four-momentum correction"""

    def __init__ (self) :
        super (TauCalibrationConfig, self).__init__ ()
        self.setBlockName('Taus')
        self.addOption ('inputContainer', '', type=str,
            info="select tau input container, by default set to TauJets")
        self.addOption ('containerName', '', type=str,
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
        self.addOption ('decorateExtraVariables', True, type=bool,
            info="decorate extra variables for the reconstructed tau")    

    def instanceName (self) :
        """Return the instance name for this block"""
        return self.containerName + self.postfix

    def makeAlgs (self, config) :

        # protection for EleRM taus, which are available only from 2024 onward
        if 'EleRM' in self.inputContainer:
            if config.dataType() is DataType.Data and config.dataYear() <= 2023:
                raise RuntimeError("EleRM taus are only available from 2024 dataset onward")
            elif config.dataType() is not DataType.Data and config.campaign() <= Campaign.MC23d:
                raise RuntimeError("EleRM taus are only available from 2024 dataset onward")

        postfix = self.postfix
        if postfix != '' and postfix[0] != '_' :
            postfix = '_' + postfix

        inputContainer = "AnalysisTauJets" if config.isPhyslite() else "TauJets"
        if self.inputContainer:
            inputContainer = self.inputContainer
        config.setSourceName (self.containerName, inputContainer)

        # Set up the tau truth matching algorithm:
        if self.rerunTruthMatching and config.dataType() is not DataType.Data:
            alg = config.createAlgorithm( 'CP::TauTruthMatchingAlg',
                                          'TauTruthMatchingAlg' )
            config.addPrivateTool( 'matchingTool',
                                   'TauAnalysisTools::TauTruthMatchingTool' )
            alg.matchingTool.TruthJetContainerName = 'AntiKt4TruthDressedWZJets'
            alg.taus = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')

        # decorate truth tau information on the reconstructed object:
        if self.decorateTruth and config.dataType() is not DataType.Data:
            alg = config.createAlgorithm( 'CP::TauTruthDecorationsAlg',
                                          'TauTruthDecorationsAlg',
                                           reentrant=True )
            alg.taus = config.readName (self.containerName)
            alg.doubleDecorations = ['pt_vis', 'pt_invis', 'eta_vis', 'eta_invis', 'phi_vis', 'phi_invis', 'm_vis', 'm_invis']
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
        if self.decorateExtraVariables:
           alg = config.createAlgorithm( 'CP::TauExtraVariablesAlg',
                                         'TauExtraVariablesAlg',
                                         reentrant=True )
           alg.taus = config.readName (self.containerName)

        # Set up the tau 4-momentum smearing algorithm:
        alg = config.createAlgorithm( 'CP::TauSmearingAlg', 'TauSmearingAlg' )
        config.addPrivateTool( 'smearingTool', 'TauAnalysisTools::TauSmearingTool' )
        alg.smearingTool.useFastSim = config.dataType() is DataType.FastSim
        alg.smearingTool.Campaign = "mc23" if config.geometry() is LHCPeriod.Run3 else "mc20"
        alg.taus = config.readName (self.containerName)
        alg.tausOut = config.copyName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')

        # Additional decorations
        alg = config.createAlgorithm( 'CP::AsgEnergyDecoratorAlg', 'EnergyDecorator' )
        alg.particles = config.readName (self.containerName)

        config.addOutputVar (self.containerName, 'pt', 'pt')
        config.addOutputVar (self.containerName, 'eta', 'eta', noSys=True)
        config.addOutputVar (self.containerName, 'phi', 'phi', noSys=True)
        config.addOutputVar (self.containerName, 'e_%SYS%', 'e')
        config.addOutputVar (self.containerName, 'charge', 'charge', noSys=True)
        config.addOutputVar (self.containerName, 'NNDecayMode', 'NNDecayMode', noSys=True)
        config.addOutputVar (self.containerName, 'passTATTauMuonOLR', 'passTATTauMuonOLR', noSys=True)
        config.addOutputVar (self.containerName, 'TESCompatibility', 'TESCompatibility')  
        if self.decorateExtraVariables:
            config.addOutputVar (self.containerName, 'nTracksCharged', 'nTracksCharged', noSys=True)


class TauWorkingPointConfig (ConfigBlock) :
    """the ConfigBlock for the tau working point

    This may at some point be split into multiple blocks (16 Mar 22)."""

    def __init__ (self) :
        super (TauWorkingPointConfig, self).__init__ ()
        self.addOption ('containerName', '', type=str,
            noneAction='error',
            info="the name of the input container.")
        self.addOption ('selectionName', '', type=str,
            noneAction='error',
            info="the name of the tau-jet selection to define (e.g. tight or "
            "loose).")
        self.addOption ('postfix', None, type=str,
            info="a postfix to apply to decorations and algorithm names. "
            "Typically not needed here as selectionName is used internally.")
        self.addOption ('quality', None, type=str,
            info="the ID WP (string) to use. Supported ID WPs: Tight, Medium, "
            "Loose, VeryLoose, Baseline, BaselineForFakes.")
        self.addOption ('use_eVeto', False, type=bool,
            info="use selection with or without eVeto combined with tauID "
            "recommendations: set it to True if electron mis-reconstructed as tau is a large background for your analysis")
        self.addOption ('use_muonOLR', False, type=bool,
            info="use selection with or without muonOLR with TauID "
            "recommendations: set it to True if muon mis-reconstructed as tau is a large background for your analysis")
        self.addOption ('useGNTau', False, type=bool,
            info="use GNTau based ID instead of RNNTau ID "
            "recommendations: that's new experimental feature and might come default soon",
            expertMode=True)
        self.addOption ('dropPtCut', False, type=bool,
            info="select taus without explicit min Pt cut. For PHYS/PHYSLITE, this would mean selecting taus starting from 13 GeV "
            "recommendations: that's experimental feature and not supported for all combinations of ID/eVeto WPs",
            expertMode=True)
        self.addOption ('useLowPt', False, type=bool, 
            info="select taus starting from 15 GeV instead of the default 20 GeV cut "
            "recommendations: that's experimental feature and not supported for all combinations of ID/eVeto WPs",
            expertMode=True)
        self.addOption ('useSelectionConfigFile', True, type=bool,
            info="use pre-defined configuration files for selecting taus "
            "recommendations: set this to False only if you want to test/optimise the tau selection for selections not already provided through config files")
        self.addOption ('manual_sel_minpt', 20.0, type=float,
            info="minimum pt cut used for tau selection when useSelectionConfigFile is set to false")
        self.addOption ('manual_sel_absetaregion', [0, 1.37, 1.52, 2.5], type=list,
            info="eta regions cut used for tau selection when useSelectionConfigFile is set to false") 
        self.addOption ('manual_sel_abscharges', [1,], type=list,
            info="charge of the tau cut used for tau selection when useSelectionConfigFile is set to false")
        self.addOption ('manual_sel_ntracks', [1,3], type=list,
            info="number of tau tracks used for tau selection when useSelectionConfigFile is set to false")
        self.addOption ('manual_sel_minrnnscore', -1, type=float,
            info="minimum rnn score cut used for tau selection when useSelectionConfigFile is set to false")
        self.addOption ('manual_sel_mingntauscore', -1, type=float,
            info="minimum gntau score selection when useSelectionConfigFile is set to false")
        self.addOption ('manual_sel_rnnwp', None, type=str,
            info="rnn working point used for tau selection when useSelectionConfigFile is set to false")
        self.addOption ('manual_sel_gntauwp', None, type=str,
            info="gntau working point used for tau selection when useSelectionConfigFile is set to false")
        self.addOption ('manual_sel_evetowp', None, type=str, 
            info="eveto working point used for tau selection when useSelectionConfigFile is set to false")
        self.addOption ('manual_sel_muonolr', False, type=bool,
            info="use muonolr used for tau selection when useSelectionConfigFile is set to false")    
        self.addOption ('noEffSF', False, type=bool,
            info="disables the calculation of efficiencies and scale factors. "
            "Experimental! only useful to test a new WP for which scale "
            "factors are not available. The default is False.",
            expertMode=True)
        self.addOption ('saveDetailedSF', True, type=bool,
            info="save all the independent detailed object scale factors. "
            "The default is True.")
        self.addOption ('saveCombinedSF', False, type=bool,
            info="save the combined object scale factor. "
            "The default is False.")
        self.addOption ('addSelectionToPreselection', True, type=bool,
            info="whether to retain only tau-jets satisfying the working point "
            "requirements. The default is True.")

    def instanceName (self) :
        """Return the instance name for this block"""
        if self.postfix is not None:
            return self.containerName + '_' + self.selectionName + self.postfix
        else:
            return self.containerName + '_' + self.selectionName

    def makeAlgs (self, config) :

        selectionPostfix = self.selectionName
        if selectionPostfix != '' and selectionPostfix[0] != '_' :
            selectionPostfix = '_' + selectionPostfix

        postfix = self.postfix
        if postfix is None :
            postfix = self.selectionName
        if postfix != '' and postfix[0] != '_' :
            postfix = '_' + postfix

        # do tau seletion through external txt config file
        if self.useSelectionConfigFile:
            nameFormat = 'TauAnalysisAlgorithms/tau_selection_'
            if self.dropPtCut:
                nameFormat = nameFormat + 'nopt_'
            if self.useLowPt:
                nameFormat = nameFormat + 'lowpt_'
            if self.useGNTau:
                nameFormat = nameFormat + 'gntau_'
            nameFormat = nameFormat + '{}_'
            if self.use_eVeto:
                nameFormat = nameFormat + 'eleid'
            else:
                nameFormat = nameFormat + 'noeleid'
            if self.use_muonOLR:
                nameFormat = nameFormat + '_muonolr' 
            nameFormat = nameFormat + '.conf'    

            if self.quality not in ['Tight', 'Medium', 'Loose', 'VeryLoose', 'Baseline', 'BaselineForFakes'] :
                raise ValueError ("invalid tau quality: \"" + self.quality +
                                  "\", allowed values are Tight, Medium, Loose, " +
                                  "VeryLoose, Baseline, BaselineForFakes")

        # Set up the algorithm selecting taus:
        alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'TauSelectionAlg' )
        config.addPrivateTool( 'selectionTool', 'TauAnalysisTools::TauSelectionTool' )
        if self.useSelectionConfigFile:
            inputfile = nameFormat.format(self.quality.lower())
            alg.selectionTool.ConfigPath = inputfile
        else:
            #build selection from user handmade selection
            from ROOT import TauAnalysisTools
            selectioncuts = TauAnalysisTools.SelectionCuts
            alg.selectionTool.ConfigPath = ""
            alg.selectionTool.SelectionCuts = int(selectioncuts.CutPt | 
                                                  selectioncuts.CutAbsEta | 
                                                  selectioncuts.CutAbsCharge | 
                                                  selectioncuts.CutNTrack | 
                                                  selectioncuts.CutJetRNNScoreSigTrans |
                                                  selectioncuts.CutGNTauScoreSigTrans |
                                                  selectioncuts.CutJetIDWP |
                                                  selectioncuts.CutEleIDWP |
                                                  selectioncuts.CutMuonOLR)

            alg.selectionTool.PtMin = self.manual_sel_minpt
            alg.selectionTool.AbsEtaRegion = self.manual_sel_absetaregion
            alg.selectionTool.AbsCharges = self.manual_sel_abscharges
            alg.selectionTool.NTracks = self.manual_sel_ntracks 
            alg.selectionTool.JetRNNSigTransMin = self.manual_sel_minrnnscore 
            alg.selectionTool.GNTauSigTransMin = self.manual_sel_mingntauscore
            #cross-check that min rnn score and min gntau score are not both set at the same time
            if self.manual_sel_minrnnscore != -1 and self.manual_sel_mingntauscore != -1:
               raise RuntimeError("manual_sel_minrnnscore and manual_sel_mingntauscore have been both set; please choose only one type of ID: RNN or GNTau, not both") 
            # working point following the Enums from https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/TauID/TauAnalysisTools/TauAnalysisTools/Enums.h
            if self.manual_sel_rnnwp is None:
               alg.selectionTool.JetIDWP = 1 
            elif self.manual_sel_rnnwp == "veryloose":
               alg.selectionTool.JetIDWP = 6
            elif self.manual_sel_rnnwp == "loose":
               alg.selectionTool.JetIDWP = 7
            elif self.manual_sel_rnnwp == "medium":
               alg.selectionTool.JetIDWP = 8
            elif self.manual_sel_rnnwp == "tight":
               alg.selectionTool.JetIDWP = 9
            else:   
               raise ValueError ("invalid RNN TauID WP: \"" + self.manual_sel_rnnwp + "\". Allowed values are None, veryloose, loose, medium, tight")

            # cross-check that min rnn score and RNN WPs are not set at the same time
            if self.manual_sel_minrnnscore != -1 and self.manual_sel_rnnwp is not None:
                raise RuntimeError("manual_sel_minrnnscore and manual_sel_rnnwp have been both set; please set only one of them") 

            # working point following the Enums from https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/TauID/TauAnalysisTools/TauAnalysisTools/Enums.h
            if self.manual_sel_gntauwp is None:
               alg.selectionTool.JetIDWP = 1
            elif self.manual_sel_gntauwp == "veryloose":
               alg.selectionTool.JetIDWP = 10
            elif self.manual_sel_gntauwp == "loose":
               alg.selectionTool.JetIDWP = 11
            elif self.manual_sel_gntauwp == "medium":
               alg.selectionTool.JetIDWP = 12
            elif self.manual_sel_gntauwp == "tight":
               alg.selectionTool.JetIDWP = 13  
            else:
               raise ValueError ("invalid GNN Tau ID WP: \"" + self.manual_sel_gntauwp + "\". Allowed values are None, veryloose, loose, medium, tight")

            # cross-check that min gntau score and GNTau WPs are not set at the same time
            if self.manual_sel_mingntauscore != -1 and self.manual_sel_gntauwp is not None:
                raise RuntimeError("manual_sel_mingntauscore and manual_sel_gntauwp have been both set; please set only one of them")

            # working point following the Enums from https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/TauID/TauAnalysisTools/TauAnalysisTools/Enums.h 
            if self.manual_sel_evetowp is None:
               alg.selectionTool.EleIDWP = 1
            elif self.manual_sel_evetowp == "loose":
               alg.selectionTool.EleIDWP = 2
            elif self.manual_sel_evetowp == "medium":
               alg.selectionTool.EleIDWP = 3
            elif self.manual_sel_evetowp == "tight":
               alg.selectionTool.EleIDWP = 4   
            else:
               raise ValueError ("invalid eVeto WP: \"" + self.manual_sel_evetowp + "\". Allowed values are None, loose, medium, tight")  

            # set MuonOLR option:
            alg.selectionTool.MuonOLR = self.manual_sel_muonolr

        alg.selectionDecoration = 'selected_tau' + selectionPostfix + ',as_char'
        alg.particles = config.readName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, self.selectionName)
        config.addSelection (self.containerName, self.selectionName, alg.selectionDecoration,
                             preselection=self.addSelectionToPreselection)

        sfList = []
        # Set up the algorithm calculating the efficiency scale factors for the
        # taus:
        if config.dataType() is not DataType.Data and not self.noEffSF:
            log = logging.getLogger('TauJetSFConfig')
            # need multiple instances of the TauEfficiencyCorrectionTool
            # 1) Reco 2) TauID, 3) eVeto for fake tau 4) eVeto for true tau
            # 3) and 4) are optional if eVeto is used in TauSelectionTool

            # TauEfficiencyCorrectionTool for Reco, this should be always enabled
            alg = config.createAlgorithm( 'CP::TauEfficiencyCorrectionsAlg',
                                   'TauEfficiencyCorrectionsAlgReco' )
            config.addPrivateTool( 'efficiencyCorrectionsTool',
                            'TauAnalysisTools::TauEfficiencyCorrectionsTool' )
            alg.efficiencyCorrectionsTool.EfficiencyCorrectionTypes = [0]
            alg.efficiencyCorrectionsTool.Campaign = "mc23" if config.geometry() is LHCPeriod.Run3 else "mc20"
            alg.efficiencyCorrectionsTool.useFastSim = config.dataType() is DataType.FastSim
            alg.scaleFactorDecoration = 'tau_Reco_effSF' + selectionPostfix + '_%SYS%'
            alg.outOfValidity = 2 #silent
            alg.outOfValidityDeco = 'bad_Reco_eff' + selectionPostfix
            alg.taus = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, self.selectionName)
            if self.saveDetailedSF:
                config.addOutputVar (self.containerName, alg.scaleFactorDecoration,
                                     'Reco_effSF' + postfix)
            sfList += [alg.scaleFactorDecoration]

            # TauEfficiencyCorrectionTool for Identification, use only in case TauID is requested in TauSelectionTool
            if self.quality not in ('VeryLoose','Baseline','BaselineForFakes'):
                if not self.useGNTau: # current recommendations are for RNN ID, so don't use in case of GNTau

                    alg = config.createAlgorithm( 'CP::TauEfficiencyCorrectionsAlg',
                                   'TauEfficiencyCorrectionsAlgID' )
                    config.addPrivateTool( 'efficiencyCorrectionsTool',
                                'TauAnalysisTools::TauEfficiencyCorrectionsTool' )
                    alg.efficiencyCorrectionsTool.EfficiencyCorrectionTypes = [4]
                    if self.quality=="Loose" or self.manual_sel_rnnwp == "loose":
                        JetIDLevel = 7
                    elif self.quality=="Medium" or self.manual_sel_rnnwp == "medium":
                        JetIDLevel = 8
                    elif self.quality=="Tight" or self.manual_sel_rnnwp == "tight":
                        JetIDLevel = 9
                    else:
                        raise ValueError ("invalid tauID: \"" + self.quality + "\". Allowed values are loose, medium, tight")

                    alg.efficiencyCorrectionsTool.JetIDLevel = JetIDLevel
                    alg.efficiencyCorrectionsTool.useFastSim = config.dataType() is DataType.FastSim
                    alg.efficiencyCorrectionsTool.Campaign = "mc23" if config.geometry() is LHCPeriod.Run3 else "mc20"
                    alg.scaleFactorDecoration = 'tau_ID_effSF' + selectionPostfix + '_%SYS%'
                    alg.outOfValidity = 2 #silent
                    alg.outOfValidityDeco = 'bad_ID_eff' + selectionPostfix
                    alg.taus = config.readName (self.containerName)
                    alg.preselection = config.getPreselection (self.containerName, self.selectionName)
                    if self.saveDetailedSF:
                        config.addOutputVar (self.containerName, alg.scaleFactorDecoration,
                                             'ID_effSF' + postfix)
                    sfList += [alg.scaleFactorDecoration]

            # TauEfficiencyCorrectionTool for eVeto both on true tau and fake tau, use only in case eVeto is requested in TauSelectionTool
            if self.use_eVeto:
                if not self.useGNTau: # eVeto correction for fake tau are for RNN ID, so don't use them for GNTau
                    # correction for fake tau
                    alg = config.createAlgorithm( 'CP::TauEfficiencyCorrectionsAlg',
                                       'TauEfficiencyCorrectionsAlgEvetoFakeTau' )
                    config.addPrivateTool( 'efficiencyCorrectionsTool',
                                    'TauAnalysisTools::TauEfficiencyCorrectionsTool' )
                    alg.efficiencyCorrectionsTool.EfficiencyCorrectionTypes = [10]
                    # since all TauSelectionTool config files have loose eRNN, code only this option for now
                    alg.efficiencyCorrectionsTool.EleIDLevel = 2
                    #overwrite decision in case user selects a WP manually
                    if self.manual_sel_evetowp == "loose":
                        alg.efficiencyCorrectionsTool.EleIDLevel = 2
                    elif self.manual_sel_evetowp == "medium":
                        alg.efficiencyCorrectionsTool.EleIDLevel = 3
                        
                    alg.efficiencyCorrectionsTool.useFastSim = config.dataType() is DataType.FastSim
                    alg.efficiencyCorrectionsTool.Campaign = "mc23" if config.geometry() is LHCPeriod.Run3 else "mc20"
                    alg.scaleFactorDecoration = 'tau_EvetoFakeTau_effSF' + selectionPostfix + '_%SYS%'
                    # for 2025-prerec, eVeto recommendations are given separately for Loose and Medium RNN 
                    if self.quality=="Loose" or self.manual_sel_rnnwp == "loose":
                        JetIDLevel = 7
                    elif self.quality=="Medium" or self.manual_sel_rnnwp == "medium":
                        JetIDLevel = 8
                    elif self.quality=="Tight" or self.manual_sel_rnnwp == "tight": 
                        log.warning("eVeto SFs are not available for Tight WP -> fallback to Medium WP")
                        JetIDLevel = 8
                    alg.efficiencyCorrectionsTool.JetIDLevel = JetIDLevel 
                    alg.outOfValidity = 2 #silent
                    alg.outOfValidityDeco = 'bad_EvetoFakeTau_eff' + selectionPostfix
                    alg.taus = config.readName (self.containerName)
                    alg.preselection = config.getPreselection (self.containerName, self.selectionName)
                    if self.saveDetailedSF:
                        config.addOutputVar (self.containerName, alg.scaleFactorDecoration,
                                             'EvetoFakeTau_effSF' + postfix)
                    sfList += [alg.scaleFactorDecoration]

                # correction for true tau
                alg = config.createAlgorithm( 'CP::TauEfficiencyCorrectionsAlg',
                                   'TauEfficiencyCorrectionsAlgEvetoTrueTau' )
                config.addPrivateTool( 'efficiencyCorrectionsTool',
                                'TauAnalysisTools::TauEfficiencyCorrectionsTool' )
                alg.efficiencyCorrectionsTool.EfficiencyCorrectionTypes = [8]
                alg.efficiencyCorrectionsTool.useFastSim = config.dataType() is DataType.FastSim
                alg.efficiencyCorrectionsTool.Campaign = "mc23" if config.geometry() is LHCPeriod.Run3 else "mc20"
                alg.scaleFactorDecoration = 'tau_EvetoTrueTau_effSF' + selectionPostfix + '_%SYS%'
                alg.outOfValidity = 2 #silent
                alg.outOfValidityDeco = 'bad_EvetoTrueTau_eff' + selectionPostfix
                alg.taus = config.readName (self.containerName)
                alg.preselection = config.getPreselection (self.containerName, self.selectionName)
                if self.saveDetailedSF:
                    config.addOutputVar (self.containerName, alg.scaleFactorDecoration,
                                         'EvetoTrueTau_effSF' + postfix)
                sfList += [alg.scaleFactorDecoration]

            if self.saveCombinedSF:
                alg = config.createAlgorithm( 'CP::AsgObjectScaleFactorAlg',
                                              'TauCombinedEfficiencyScaleFactorAlg' )
                alg.particles = config.readName (self.containerName)
                alg.inScaleFactors = sfList
                alg.outScaleFactor = 'effSF' + postfix + '_%SYS%'
                config.addOutputVar (self.containerName, alg.outScaleFactor,
                                     'effSF' + postfix)


class EXPERIMENTAL_TauCombineMuonRemovalConfig (ConfigBlock) :
    def __init__ (self) :
        super (EXPERIMENTAL_TauCombineMuonRemovalConfig, self).__init__ ()
        self.addOption (
            'inputTaus', 'TauJets', type=str,
            noneAction='error',
            info="the name of the input tau container."
        )
        self.addOption (
            'inputTausMuRM', 'TauJets_MuonRM', type=str,
            noneAction='error',
            info="the name of the input tau container with muon removal applied."
        )
        self.addOption (
            'outputTaus', 'TauJets_MuonRmCombined', type=str,
            noneAction='error',
            info="the name of the output tau container."
        )

    def instanceName (self) :
        """Return the instance name for this block"""
        return self.outputTaus

    def makeAlgs (self, config) :

        if config.isPhyslite() :
            raise(RuntimeError("Muon removal taus is not available in Physlite mode"))

        alg = config.createAlgorithm( 'CP::TauCombineMuonRMTausAlg', 'TauCombineMuonRMTausAlg' )
        alg.taus = self.inputTaus
        alg.muonrm_taus = self.inputTausMuRM
        alg.combined_taus = self.outputTaus

class TauTriggerAnalysisSFBlock (ConfigBlock):

    def __init__ (self) :
        super (TauTriggerAnalysisSFBlock, self).__init__ ()

        self.addOption ('triggerChainsPerYear', {}, type=None,
                        info="a dictionary with key (string) the year and value (list of "
                        "strings) the trigger chains. The default is {} (empty dictionary).")
        self.addOption ('tauID', '', type=str,
                        info="the tau quality WP (string) to use.")
        self.addOption ('prefixSF', 'trigEffSF', type=str,
                        info="the decoration prefix for trigger scale factors, "
                        "the default is 'trigEffSF'")
        self.addOption ('includeAllYearsPerRun', False, type=bool,
                        info="if True, all configured years in the LHC run will "
                        "be included in all jobs. The default is False.")
        self.addOption ('removeHLTPrefix', True, type=bool,
                        info="remove the HLT prefix from trigger chain names, "
                        "The default is True.")
        self.addOption ('containerName', '', type=str,
                        info="the input tau container, with a possible selection, in "
                        "the format container or container.selection.")

    def instanceName (self) :
        """Return the instance name for this block"""
        return self.containerName + '_' + self.prefixSF + '_' + self.tauID

    def get_year_data(self, dictionary: dict, year: int | str) -> list:
        return dictionary.get(int(year), dictionary.get(str(year), []))

    def makeAlgs (self, config) :

        if config.dataType() is not DataType.Data:
            log = logging.getLogger('TauJetTriggerSFConfig')

            from TriggerAnalysisAlgorithms.TriggerAnalysisConfig import is_year_in_current_period

            triggers = set()
            if self.includeAllYearsPerRun:
                for year in self.triggerChainsPerYear:
                    if not is_year_in_current_period(config, year):
                        continue
                    triggers.update(self.get_year_data(self.triggerChainsPerYear, year))
            elif config.campaign() is Campaign.MC20a:
                triggers.update(self.get_year_data(self.triggerChainsPerYear, 2015))
                triggers.update(self.get_year_data(self.triggerChainsPerYear, 2016))
            elif config.campaign() is Campaign.MC20d:
                triggers.update(self.get_year_data(self.triggerChainsPerYear, 2017))
            elif config.campaign() is Campaign.MC20e:
                triggers.update(self.get_year_data(self.triggerChainsPerYear, 2018))
            elif config.campaign() is Campaign.MC23a:
                triggers.update(self.get_year_data(self.triggerChainsPerYear, 2022))
            elif config.campaign() is Campaign.MC23d:
                triggers.update(self.get_year_data(self.triggerChainsPerYear, 2023))
            else:
                log.warning("unknown campaign, skipping triggers: %s", str(config.campaign()))

            for chain in triggers:
                chain_noHLT = chain.replace("HLT_", "")
                chain_out = chain_noHLT if self.removeHLTPrefix else chain
                alg = config.createAlgorithm( 'CP::TauEfficiencyCorrectionsAlg',
                                              'TauTrigEfficiencyCorrectionsAlg_' + chain )
                config.addPrivateTool( 'efficiencyCorrectionsTool',
                                       'TauAnalysisTools::TauEfficiencyCorrectionsTool' )
                # SFTriggerHadTau correction type from
                # https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/TauID/TauAnalysisTools/TauAnalysisTools/Enums.h#L79
                alg.efficiencyCorrectionsTool.EfficiencyCorrectionTypes = [12]
                if config.geometry() is LHCPeriod.Run2:
                    alg.efficiencyCorrectionsTool.Campaign = "mc20"
                else:
                    alg.efficiencyCorrectionsTool.Campaign = config.campaign().value
                alg.efficiencyCorrectionsTool.TriggerName = chain

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

                alg.scaleFactorDecoration = f"tau_{self.prefixSF}_{chain_out}_%SYS%"
                alg.outOfValidity = 2 #silent
                alg.outOfValidityDeco = f"bad_eff_tautrig_{chain_out}"
                alg.taus = config.readName (self.containerName)
                alg.preselection = config.getPreselection (self.containerName, self.tauID)
                config.addOutputVar (self.containerName, alg.scaleFactorDecoration, f"{self.prefixSF}_{chain_out}")
