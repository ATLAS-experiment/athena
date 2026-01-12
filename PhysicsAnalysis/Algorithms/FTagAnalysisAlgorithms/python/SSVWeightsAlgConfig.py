# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# @author Hagen Möbius, hagen.mobius@cern.ch

from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType

class SSVWeightsAlgConfig(ConfigBlock):

    def __init__ (self) :
        super (SSVWeightsAlgConfig, self).__init__ ()
        self.addOption('jets', None, type = str, info = "jet container that will be used to define the good SSVs (ΔR(SSV,jet)>0.6) and the truth b-hadrons in acceptance (ΔR(truthBh,jet)>0.6) and that will be used to count the number of b-jets for b-jet based SSV weight calculation")
        self.addOption('electrons', None, type = str, info = "electron container that will be used to define the good SSVs (ΔR(SSV,electron)>0.2)")
        self.addOption('muons', None, type = str,  info = "muon container that will be used to define the good SSVs (ΔR(SSV,muon)>0.2)")
        self.addOption('NVSI_WP', "NVSI_SecVrt_Tight", type = str, info = "Working point of the NewVrtSecInclusiveTool (algorithm that constructs the soft secondary vertices); possible working points are 'NVSI_SecVrt_Tight','NVSI_SecVrt_Medium','NVSI_SecVrt_Loose'")
        self.addOption("JsonConfigFile_SSVWeightsAlg", None, type = str, info ="Path to the JSON config file that contains the SSV calibration results which are needed to calculate the SSV weights")
        self.addOption("BTaggingWP", "ftag_select_GN2v01_FixedCutBEff_85" , type = str, info = "b-tagging working point that is used to count the number of b-jets in the event for b-jet based SSV weight calculation")
        self.addOption("EfficiencyMethod", "Bhadron_pT_eta_based", type = str, info = "efficiency definition that will be used to calculate the SSV weights, string can be 'Bhadron_pT_eta_based' or 'bjet_based'")
        self.addOption("nFMethod", "pileup_based_binned", type = str, info = "average number of fake SSV definition that will be used to calculate the SSV weights, string can be 'pileup_bjet_based','pileup_based_linearfit' or 'pileup_based_binned'")
        self.addOption("OutputVariableSize", "standard", type = str, info ="number of variables that will be saved to the output, string can be 'standard','extended','additional' or 'all'")

    def makeAlgs(self, config):
        #Algorithm is Monte Carlo only -> skip algorithm if it is run on Data
        if config.dataType() == DataType.Data: 
            return

        alg = config.createAlgorithm('CP::SSVWeightsAlg', 'SSVWeightsAlg')

        jetContainer, sep, jetSelectionName = self.jets.partition('.') 
        alg.jetSelection = config.getFullSelection(jetContainer, jetSelectionName)
        alg.jets = config.readName(jetContainer)

        electronContainer, sep, electronSelectionName = self.electrons.partition('.') 
        alg.electronSelection = config.getFullSelection(electronContainer, electronSelectionName)
        alg.electrons = config.readName(electronContainer)

        muonContainer, sep, muonSelectionName = self.muons.partition('.') 
        alg.muonSelection = config.getFullSelection(muonContainer, muonSelectionName)
        alg.muons = config.readName(muonContainer)

        print("Py:SSVWeightsAlg        ","You run with the following jet selections: ", alg.jetSelection)
        print("Py:SSVWeightsAlg        ","You run with the following electron selections: ", alg.electronSelection)
        print("Py:SSVWeightsAlg        ","You run with the following muon selections: ", alg.muonSelection)

        alg.NVSI_WP = self.NVSI_WP
        alg.JsonConfigFile_SSVWeightsAlg = self.JsonConfigFile_SSVWeightsAlg
        alg.BTaggingWP = self.BTaggingWP
        alg.EfficiencyMethod = self.EfficiencyMethod
        alg.nFMethod = self.nFMethod
        alg.OutputVariableSize = self.OutputVariableSize

        config.addOutputVar('EventInfo', 'SSV_weight_%SYS%', 'SSV_weight')

        if alg.OutputVariableSize in ["extended", "additional", "all"] : 
            config.addOutputVar('EventInfo', 'P_eff_%SYS%', 'P_eff')
            config.addOutputVar('EventInfo', 'P_ineff_%SYS%', 'P_ineff')
            config.addOutputVar('EventInfo', 'P_fake_%SYS%', 'P_fake')

        if alg.OutputVariableSize in ["additional", "all"] : 
            config.addOutputVar('EventInfo', 'N_matched_%SYS%', 'N_matched')
            config.addOutputVar('EventInfo', 'N_missed_%SYS%', 'N_missed')
            config.addOutputVar('EventInfo', 'N_fake_%SYS%', 'N_fake')
            config.addOutputVar('EventInfo', 'number_of_bjets_%SYS%', 'number_of_bjets')
            config.addOutputVar('EventInfo', 'number_of_accepted_Bhadrons_%SYS%', 'number_of_accepted_Bhadrons')
            config.addOutputVar('EventInfo', 'number_of_good_SSVs_%SYS%', 'number_of_good_SSVs')

        if alg.OutputVariableSize == "all": 
            config.addOutputVar('EventInfo', 'P_ineff_bjet_based_%SYS%', 'P_ineff_bjet_based')
            config.addOutputVar('EventInfo', 'P_ineff_pt_eta_based_%SYS%', 'P_ineff_pt_eta_based')
            config.addOutputVar('EventInfo', 'P_fake_pileup_bjet_based_%SYS%', 'P_fake_pileup_bjet_based')
            config.addOutputVar('EventInfo', 'P_fake_pileup_based_linearfit_%SYS%', 'P_fake_pileup_based_linearfit')
            config.addOutputVar('EventInfo', 'P_fake_pileup_based_binned_%SYS%', 'P_fake_pileup_based_binned') 