# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType


class DiTauCalibrationConfig (ConfigBlock):
    """the ConfigBlock for the tau four-momentum correction"""

    def __init__ (self, containerName, postfix) :
        super (DiTauCalibrationConfig, self).__init__ ()
        self.containerName = containerName
        self.postfix = postfix
        self.rerunTruthMatching = True


    def makeAlgs (self, config) :

        postfix = self.postfix
        if postfix != '' and postfix[0] != '_' :
            postfix = '_' + postfix

        # Set up the tau 4-momentum smearing algorithm:
        alg = config.createAlgorithm( 'CP::DiTauSmearingAlg', 'DiTauSmearingAlg' + postfix )
        config.addPrivateTool( 'smearingTool', 'TauAnalysisTools::DiTauSmearingTool' )
        alg.taus = config.readName (self.containerName, "DiTauJets")
        alg.tausOut = config.copyName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')

        # Set up the tau truth matching algorithm:
        if self.rerunTruthMatching and config.dataType() is not DataType.Data:
            alg = config.createAlgorithm( 'CP::DiTauTruthMatchingAlg',
                                   'DiTauTruthMatchingAlg' + postfix )
            config.addPrivateTool( 'matchingTool',
                            'TauAnalysisTools::DiTauTruthMatchingTool' )
            alg.taus = self.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')





class DiTauWorkingPointConfig (ConfigBlock) :
    """the ConfigBlock for the tau working point

    This may at some point be split into multiple blocks (16 Mar 22)."""

    def __init__ (self, containerName, postfix, quality) :
        super (DiTauWorkingPointConfig, self).__init__ ()
        self.containerName = containerName
        self.selectionName = postfix
        self.postfix = postfix
        self.quality = quality
        self.legacyRecommendations = False


    def makeAlgs (self, config) :

        postfix = self.postfix
        if postfix != '' and postfix[0] != '_' :
            postfix = '_' + postfix

        # using enum value from: https://gitlab.cern.ch/atlas/athena/blob/21.2/PhysicsAnalysis/TauID/TauAnalysisTools/TauAnalysisTools/Enums.h
        # the dictionary is missing in Athena, so hard-coding values here
        if self.quality == 'Tight' :
            IDLevel = 4 # ROOT.TauAnalysisTools.JETIDBDTTIGHT
        elif self.quality == 'Medium' :
            IDLevel = 3 # ROOT.TauAnalysisTools.JETIDBDTMEDIUM
        elif self.quality == 'Loose' :
            IDLevel = 2 # ROOT.TauAnalysisTools.JETIDBDTLOOSE
        else :
            raise ValueError ("invalid tau quality: \"" + self.quality +
                              "\", allowed values are Tight, Medium, Loose")


        # Set up the algorithm calculating the efficiency scale factors for the
        # taus:
        if config.dataType() is not DataType.Data:
            alg = config.createAlgorithm( 'CP::DiTauEfficiencyCorrectionsAlg',
                                   'DiTauEfficiencyCorrectionsAlg' + postfix )
            config.addPrivateTool( 'efficiencyCorrectionsTool',
                            'TauAnalysisTools::DiTauEfficiencyCorrectionsTool' )
            alg.efficiencyCorrectionsTool.IDLevel = IDLevel
            alg.scaleFactorDecoration = 'tau_effSF' + postfix
            # alg.outOfValidity = 2 #silent
            # alg.outOfValidityDeco = "bad_eff"
            alg.taus = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, self.selectionName)
