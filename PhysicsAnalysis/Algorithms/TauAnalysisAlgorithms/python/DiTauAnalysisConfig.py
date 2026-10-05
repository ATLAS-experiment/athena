# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigSequence import groupBlocks
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType


class DiTauCalibrationConfig (ConfigBlock):
    """the ConfigBlock for the tau four-momentum correction"""

    def __init__ (self) :
        super().__init__()
        self.setBlockName('DiTaus')
        self.addOption ('inputContainer', '', type=str,
            info="the name of the input ditau-jet container.")
        self.addOption ('containerName', '', type=str,
            noneAction='error',
            info="the name of the output container after calibration.",
            meta={'role':'container'})
        self.addOption ('postfix', '', type=str,
            info="a postfix to apply to decorations and algorithm names. "
            "Typically not needed here since the calibration is common to "
            "all ditau-jets.")
        self.addOption ('quality', None, type=str,
            info="the ID WP to use. Supported ID WPs: `Tight`, `Medium`,`Loose`,`NoID`.",
            meta={'choices':(['Tight','Medium','Loose','NoID'],1)})
        self.addOption ('rerunTruthMatching', True, type=bool,
            info="whether to rerun truth matching (sets up an instance of "
            "`CP::DiTauTruthMatchingAlg`).")
        self.addOption ('decorateTruth', False, type=bool,
            info="decorate the truth particle information on the reconstructed one.")
        self.addOption ('decorateExtraVariables', True, type=bool,
            info="decorate extra variables for the reconstructed ditau-jet.")

    def instanceName (self) :
        """Return the instance name for this block"""
        return self.containerName + self.postfix

    def makeAlgs (self, config) :

        postfix = self.postfix
        if postfix != '' and postfix[0] != '_' :
            postfix = '_' + postfix

        inputContainer = "DiTauJets"
        if self.inputContainer:
            inputContainer = self.inputContainer
        config.setSourceName (self.containerName, inputContainer)

        # Set up the tau truth matching algorithm:
        if self.rerunTruthMatching and config.dataType() is not DataType.Data:
            alg = config.createAlgorithm( 'CP::DiTauTruthMatchingAlg',
                                   'DiTauTruthMatchingAlg' )
            config.addPrivateTool( 'matchingTool',
                            'TauAnalysisTools::DiTauTruthMatchingTool' )
            alg.taus = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')

       
        # decorate truth tau information on the reconstructed object:
        if self.decorateTruth and self.rerunTruthMatching and config.dataType() is not DataType.Data:
            # in the case of the ditau, the DiTauTruthMatchingTool decorates directly the reco ditau with truth information.
            # So information can be written directly out without any additional algorithm 
            config.addOutputVar (self.containerName, 'TruthVisLeadPt', 'TruthVisLeadPt', noSys=True) 
            config.addOutputVar (self.containerName, 'TruthVisLeadEta', 'TruthVisLeadEta', noSys=True) 
            config.addOutputVar (self.containerName, 'TruthVisLeadPhi', 'TruthVisLeadPhi', noSys=True)
            config.addOutputVar (self.containerName, 'TruthVisLeadM', 'TruthVisLeadM', noSys=True)
            config.addOutputVar (self.containerName, 'TruthLeadPdgID', 'TruthLeadPdgID', noSys=True)
            config.addOutputVar (self.containerName, 'TruthVisSubleadPt', 'TruthVisSubleadPt', noSys=True)
            config.addOutputVar (self.containerName, 'TruthVisSubleadEta', 'TruthVisSubleadEta', noSys=True)
            config.addOutputVar (self.containerName, 'TruthVisSubleadPhi', 'TruthVisSubleadPhi', noSys=True)
            config.addOutputVar (self.containerName, 'TruthVisSubleadM', 'TruthVisSubleadM', noSys=True)
            config.addOutputVar (self.containerName, 'TruthSubleadPdgID', 'TruthSubleadPdgID', noSys=True)
            config.addOutputVar (self.containerName, 'TruthVisDeltaR', 'TruthVisDeltaR', noSys=True)
            config.addOutputVar (self.containerName, 'TruthVisMass', 'TruthVisMass', noSys=True)
            config.addOutputVar (self.containerName, 'IsTruthMatched', 'IsTruthMatched', noSys=True)
            config.addOutputVar (self.containerName, 'IsTruthHadronic', 'IsTruthHadronic', noSys=True)

        # Decorate extra variables
        if self.decorateExtraVariables:
           alg = config.createAlgorithm( 'CP::DiTauExtraVariablesAlg',
                                         'DiTauExtraVariablesAlg',
                                         reentrant=True )
           alg.ditaus = config.readName (self.containerName)
           config.addOutputVar (self.containerName, 'omniScore', 'omniScore', noSys=True)
           config.addOutputVar (self.containerName, 'nSubjets', 'nSubjets', noSys=True) 
           config.addOutputVar (self.containerName, 'leadSubjetPt', 'leadSubjetPt', noSys=True)
           config.addOutputVar (self.containerName, 'leadSubjetEta', 'leadSubjetEta', noSys=True)
           config.addOutputVar (self.containerName, 'leadSubjetPhi', 'leadSubjetPhi', noSys=True)
           config.addOutputVar (self.containerName, 'leadSubjetE', 'leadSubjetE', noSys=True)
           config.addOutputVar (self.containerName, 'leadSubjetNTracks', 'leadSubjetNTracks', noSys=True)
           config.addOutputVar (self.containerName, 'leadSubjetCharge', 'leadSubjetCharge', noSys=True)
           config.addOutputVar (self.containerName, 'subleadSubjetPt', 'subleadSubjetPt', noSys=True)
           config.addOutputVar (self.containerName, 'subleadSubjetEta', 'subleadSubjetEta', noSys=True)
           config.addOutputVar (self.containerName, 'subleadSubjetPhi', 'subleadSubjetPhi', noSys=True)
           config.addOutputVar (self.containerName, 'subleadSubjetE', 'subleadSubjetE', noSys=True)
           config.addOutputVar (self.containerName, 'subleadSubjetNTracks', 'subleadSubjetNTracks', noSys=True)
           config.addOutputVar (self.containerName, 'subleadSubjetCharge', 'subleadSubjetCharge', noSys=True)

        # Set up the tau 4-momentum smearing algorithm:
        alg = config.createAlgorithm( 'CP::DiTauSmearingAlg', 'DiTauSmearingAlg' )
        config.addPrivateTool( 'smearingTool', 'TauAnalysisTools::DiTauSmearingTool' )
        alg.taus = config.readName (self.containerName)
        alg.tausOut = config.copyName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')

        # Save base kinematic ditau variables in output
        config.addOutputVar (self.containerName, 'pt', 'pt')
        config.addOutputVar (self.containerName, 'eta', 'eta', noSys=True)
        config.addOutputVar (self.containerName, 'phi', 'phi', noSys=True)
        config.addOutputVar (self.containerName, 'm', 'm', noSys=True)



class DiTauWorkingPointSelectionConfig (ConfigBlock) :
    """the ConfigBlock for the tau working point selection"""

    def __init__ (self) :
        super().__init__()
        self.setBlockName('DiTauWorkingPointSelection')
        self.addOption ('containerName', '', type=str,
            noneAction='error',
            info="the name of the input container.",
            meta={'role':'containerRef'})
        self.addOption ('selectionName', '', type=str,
            noneAction='error',
            info="the name of the ditau-jet selection to define (e.g. `tight` or "
            "`loose`).",
            meta={'role':'selection'})
        self.addOption ('postfix', None, type=str,
            info="a postfix to apply to decorations and algorithm names. "
            "Typically not needed here as `selectionName` is used internally.")
        self.addOption ('quality', None, type=str,
            info="the ID WP to use for high-pt ditau-jets (ignored for `DiTauJetsLowPt` containers). "
            "Supported ID WPs: `Tight`, `Medium`, `Loose`, `NoID`.",
            meta={'choices':(['Tight','Medium','Loose','NoID'],1)})
        self.addOption ('addSelectionToPreselection', True, type=bool,
            info="whether to retain only ditau-jets satisfying the working point "
            "requirements.")

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

        if "DiTauJetsLowPt" in self.containerName:
            inputfile = 'TauAnalysisAlgorithms/ditau_selection_lowpt.conf' 
        else:
            if self.quality not in ['Tight', 'Medium', 'Loose', 'NoID'] :
                raise ValueError (f"invalid ditau quality: \"{self.quality}\", allowed values are Tight, Medium, Loose, NoID")
            if self.quality == 'NoID':
                inputfile = 'TauAnalysisAlgorithms/ditau_selection_highpt.conf'
            else: 
                inputfile = 'TauAnalysisAlgorithms/ditau_selection_highpt_'+self.quality.lower()+'.conf'        

        # Set up the algorithm selecting taus:
        alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'DiTauSelectionAlg' )
        config.addPrivateTool( 'selectionTool', 'TauAnalysisTools::DiTauSelectionTool' )
        alg.selectionTool.ConfigPath = inputfile
        alg.selectionDecoration = 'selected_ditau' + selectionPostfix + ',as_char'
        alg.particles = config.readName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, self.selectionName)
        config.addSelection (self.containerName, self.selectionName, alg.selectionDecoration,
                             preselection=self.addSelectionToPreselection) 


class DiTauWorkingPointEfficiencyConfig (ConfigBlock) :
    """the ConfigBlock for the tau working point efficiency computation"""

    def __init__ (self) :
        super().__init__()
        self.setBlockName('DiTauWorkingPointEfficiency')
        self.addDependency('DiTauWorkingPointSelection', required=True)
        self.addDependency('EventSelection', required=False)
        self.addDependency('EventSelectionMerger', required=False)
        self.addOption ('containerName', '', type=str,
            noneAction='error',
            info="the name of the input container.",
            meta={'role':'containerRef'})
        self.addOption ('selectionName', '', type=str,
            noneAction='error',
            info="the name of the ditau-jet selection to define (e.g. `tight` or "
            "`loose`).",
            meta={'role':'selection'})
        self.addOption ('postfix', None, type=str,
            info="a postfix to apply to decorations and algorithm names. "
            "Typically not needed here as `selectionName` is used internally.")

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


        # keep this commented out until TauCP won't provide official recommendations
        # Set up the algorithm calculating the efficiency scale factors for the
        # taus:
        #if config.dataType() is not DataType.Data:
        #    alg = config.createAlgorithm( 'CP::DiTauEfficiencyCorrectionsAlg',
        #                           'DiTauEfficiencyCorrectionsAlg' )
        #    config.addPrivateTool( 'efficiencyCorrectionsTool',
        #                    'TauAnalysisTools::DiTauEfficiencyCorrectionsTool' )
        #    alg.efficiencyCorrectionsTool.JetIDLevel = IDLevel
        #    alg.scaleFactorDecoration = 'tau_effSF' + postfix + '_%SYS%'
        #    # alg.outOfValidity = 2 #silent
        #    # alg.outOfValidityDeco = "bad_eff"
        #    alg.taus = config.readName (self.containerName)
        #    alg.preselection = config.getPreselection (self.containerName, self.selectionName)
        #    config.addOutputVar (self.containerName, alg.scaleFactorDecoration,
        #                         'effSF' + postfix)


@groupBlocks
def DiTauWorkingPoint(seq):
    seq.append(DiTauWorkingPointSelectionConfig())
    seq.append(DiTauWorkingPointEfficiencyConfig())
