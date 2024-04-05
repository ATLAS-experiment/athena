# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType


class TauCalibrationConfig (ConfigBlock):
    """the ConfigBlock for the tau four-momentum correction"""

    def __init__ (self, containerName) :
        super (TauCalibrationConfig, self).__init__ ()
        self.containerName = containerName
        self.addOption ('postfix', '', type=str)
        self.addOption ('rerunTruthMatching', True, type=bool)
        self.addOption ('decorateTruth', False, type=bool)


    def makeAlgs (self, config) :

        postfix = self.postfix
        if postfix != '' and postfix[0] != '_' :
            postfix = '_' + postfix

        if config.isPhyslite() :
            config.setSourceName (self.containerName, "AnalysisTauJets")
        else :
            config.setSourceName (self.containerName, "TauJets")

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
            alg.preselection = config.getPreselection (self.containerName, '')
            alg.doubleDecorations = ['pt_vis', 'eta_vis', 'phi_vis', 'm_vis']
            alg.floatDecorations = []
            alg.intDecorations = ['pdgId']
            alg.charDecorations = ['IsHadronicTau']
            alg.prefix = 'truth_'

            # these are "_ListHelper" objects, and not "list", need to copy to lists to allow concatenate
            for var in ['DecayMode'] + alg.doubleDecorations[:] + alg.floatDecorations[:] + alg.intDecorations[:] + alg.charDecorations[:]:
                branchName = alg.prefix+var
                config.addOutputVar (self.containerName, branchName, branchName, noSys=True)

        # Set up the tau 4-momentum smearing algorithm:
        alg = config.createAlgorithm( 'CP::TauSmearingAlg', 'TauSmearingAlg' + postfix )
        config.addPrivateTool( 'smearingTool', 'TauAnalysisTools::TauSmearingTool' )
        alg.smearingTool.isAFII = config.dataType() is DataType.FastSim
        alg.taus = config.readName (self.containerName)
        alg.tausOut = config.copyName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')

        config.addOutputVar (self.containerName, 'pt', 'pt')
        config.addOutputVar (self.containerName, 'eta', 'eta', noSys=True)
        config.addOutputVar (self.containerName, 'phi', 'phi', noSys=True)
        config.addOutputVar (self.containerName, 'charge', 'charge', noSys=True)
        config.addOutputVar (self.containerName, 'NNDecayMode', 'NNDecayMode', noSys=True)




class TauWorkingPointConfig (ConfigBlock) :
    """the ConfigBlock for the tau working point

    This may at some point be split into multiple blocks (16 Mar 22)."""

    def __init__ (self, containerName, selectionName) :
        super (TauWorkingPointConfig, self).__init__ ()
        self.containerName = containerName
        self.selectionName = selectionName
        self.addOption ('postfix', None, type=str)
        self.addOption ('quality', None, type=str)
        self.addOption ('legacyRecommendations', False, type=bool)
        self.addOption ('noEffSF', False, type=bool)

    def createCommonSelectionTool (self, config, tauSelectionAlg, configPath, postfix) :

        # This should eventually be fixed in the TauEfficiencyCorrectionsTool directly...
        # ---
        # Create two instances of TauSelectionTool, one public and one private.
        # Both should share the same configuration options.
        # We attach the private tool to the CP::AsgSelectionAlg for tau selection,
        # and the public one is returned, to be retrieved later by TauEfficiencyCorrectionsTool.

        config.addPrivateTool( 'selectionTool', 'TauAnalysisTools::TauSelectionTool' )
        tauSelectionAlg.selectionTool.ConfigPath = configPath

        publicTool = config.createPublicTool( 'TauAnalysisTools::TauSelectionTool',
                                              'TauSelectionTool' + postfix)
        publicTool.ConfigPath = configPath

        return publicTool

    def makeAlgs (self, config) :

        selectionPostfix = self.selectionName
        if selectionPostfix != '' and selectionPostfix[0] != '_' :
            selectionPostfix = '_' + selectionPostfix

        postfix = self.postfix
        if postfix is None :
            postfix = self.selectionName
        if postfix != '' and postfix[0] != '_' :
            postfix = '_' + postfix

        nameFormat = 'TauAnalysisAlgorithms/tau_selection_{}.conf'
        if self.legacyRecommendations:
            nameFormat = 'TauAnalysisAlgorithms/tau_selection_{}_legacy.conf'

        if self.quality not in ['Tight', 'Medium', 'Loose', 'VeryLoose', 'NoID', 'Baseline'] :
            raise ValueError ("invalid tau quality: \"" + self.quality +
                              "\", allowed values are Tight, Medium, Loose, " +
                              "VeryLoose, NoID, Baseline")
        inputfile = nameFormat.format(self.quality.lower())

        # Set up the algorithm selecting taus:
        alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'TauSelectionAlg' + postfix )
        selectionTool = self.createCommonSelectionTool(config, alg, inputfile, postfix)
        alg.selectionDecoration = 'selected_tau' + selectionPostfix + ',as_bits'
        alg.particles = config.readName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, self.selectionName)
        config.addSelection (self.containerName, self.selectionName, alg.selectionDecoration)

        # Set up the algorithm calculating the efficiency scale factors for the
        # taus:
        if config.dataType() is not DataType.Data and not self.noEffSF:
            alg = config.createAlgorithm( 'CP::TauEfficiencyCorrectionsAlg',
                                   'TauEfficiencyCorrectionsAlg' + postfix )
            config.addPrivateTool( 'efficiencyCorrectionsTool',
                            'TauAnalysisTools::TauEfficiencyCorrectionsTool' )
            alg.efficiencyCorrectionsTool.TauSelectionTool = '%s/%s' % \
                ( selectionTool.getType(), selectionTool.getName() )
            alg.efficiencyCorrectionsTool.isAFII = config.dataType() is DataType.FastSim
            alg.scaleFactorDecoration = 'tau_effSF' + selectionPostfix + '_%SYS%'
            alg.outOfValidity = 2 #silent
            alg.outOfValidityDeco = 'bad_eff' + selectionPostfix
            alg.taus = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, self.selectionName)
            config.addOutputVar (self.containerName, alg.scaleFactorDecoration, 'effSF' + postfix)


def makeTauCalibrationConfig( seq, containerName, postfix = None,
                              rerunTruthMatching = None):
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
    if postfix is not None :
        config.setOptionValue ('postfix', postfix)
    if rerunTruthMatching is not None :
        config.setOptionValue ('rerunTruthMatching', rerunTruthMatching)
    seq.append (config)





def makeTauWorkingPointConfig( seq, containerName, workingPoint, selectionName,
                               legacyRecommendations = None,
                               noEffSF = None ):
    """Create tau analysis algorithms for a single working point

    Keyword arguments:
      legacyRecommendations -- use legacy tau BDT and electron veto recommendations
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
    config.setOptionValue ('legacyRecommendations', legacyRecommendations)
    config.setOptionValue ('noEffSF', noEffSF)
    seq.append (config)
