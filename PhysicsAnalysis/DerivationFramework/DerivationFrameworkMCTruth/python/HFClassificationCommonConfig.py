# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

#####################################################################
#                                                                   #
# Heavy flavour Classification of ttbar events                      #
# Author: Adrian Berrocal Guardia <adrian.berrocal.guardia@cern.ch> #
#                                                                   #
#####################################################################

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from DerivationFrameworkMCTruth.HFDSIDList import DSIDList


def ClassifyAndCalculateHFAugmentationCfg(flags, mc_channel_number):

  """Configure and add the tools to compute and add the HF classifier in the derivation"""

  acc = ComponentAccumulator()

  #################################
  ####### Jet Matching Tool #######
  #################################

  # Configure the tool that matches each truth particle to the closest jet.
  # Parameters:
  #  -m_jetPtCut:  Cut on the pt of the jets.
  #  -m_jetEtaCut: Cut on the eta of the jets.
  #  -m_drCut:     Upper limit for the delta R between the particle and the jet to perform the matching.

  JetMatchingTool = CompFactory.DerivationFramework.JetMatchingTool(name='DFCommonJetMatchingTool')

  JetMatchingTool.jetPtCut  = 15000.
  JetMatchingTool.jetEtaCut = 2.5
  JetMatchingTool.drCut     = 0.3

  # Add the tool as a public tool.

  acc.addPublicTool(JetMatchingTool, primary=True)
  
  ##################################
  ##### HF Hadrons Origin Tool #####
  ##################################

  # Configure the tool that determines the origin of the HF hadrons.
  # Parameters:
  #  -DSID: DSID of the sample that is being processed.
  
  HadronOriginClassifierTool = CompFactory.DerivationFramework.HadronOriginClassifier(name='DFCommonHadronOriginClassifier')
  HadronOriginClassifierTool.DSID = mc_channel_number

  # Add the tool as a public tool.

  acc.addPublicTool(HadronOriginClassifierTool, primary=True)
  
  ##################################
  ##### HF Classification Tool #####
  ##################################

  # Configure the tool that computes the HF Classification.
  # Parameters:
  #  -m_jetPtCut:                Cut on the pt of the jets.
  #  -m_jetEtaCut:               Cut on the eta of the jets.
  #  -m_leadingHadronPtCut:      Cut on the pt of the leading hadron.
  #  -m_leadingHadronPtRatioCut: Cut on the ratio between the pt of the leading hadron and the pt of its associated jet.

  ClassifyAndCalculateHFTool = CompFactory.DerivationFramework.ClassifyAndCalculateHFTool(name='DFCommonClassifyAndCalculateHFTool')

  ClassifyAndCalculateHFTool.jetPtCut                = 15000.
  ClassifyAndCalculateHFTool.jetEtaCut               = 2.5
  ClassifyAndCalculateHFTool.leadingHadronPtCut      = 5000.
  ClassifyAndCalculateHFTool.leadingHadronPtRatioCut = -1

  # Add the tool as a public tool.

  acc.addPublicTool(ClassifyAndCalculateHFTool, primary=True)

  #################################
  ####### Augmentation Tool #######
  #################################

  # Configure the tool that adds the HF Classification in the derivation file.
  # Parameters:
  #  -ClassifyAndComputeHFtool:   It computes the HF classifier.
  #  -HadronOriginClassifierTool: It determines the origin of the HF hadrons.
  #  -ClassifyAndComputeHFtool:   It matches the hadrons with the jets.
  # Tools:
  #  -jetCollectionName:          It contains the name of the jets container.
  #  -TruthParticleContainerName: It contains the name of the truth particles container.
  #  -hfDecorationName:           It contains the name used to save the HF classifier.
  #  -SimplehfDecorationName:     It contains the name used to save the simple HF classifier.
  
  ClassifyAndCalculateHFAugmentation = CompFactory.DerivationFramework.ClassifyAndCalculateHFAugmentation(name = "DFCommonClassifyAndCalculateHFAugmentation")

  ClassifyAndCalculateHFAugmentation.jetCollectionName            = "AntiKt4TruthDressedWZJets"
  ClassifyAndCalculateHFAugmentation.TruthParticleContainerName   = "TruthParticles"
  ClassifyAndCalculateHFAugmentation.hfDecorationName             = "HF_Classification"
  ClassifyAndCalculateHFAugmentation.SimplehfDecorationName       = "HF_SimpleClassification"
  ClassifyAndCalculateHFAugmentation.HadronOriginIDDecorationName = "HFHadronOriginID"

  ClassifyAndCalculateHFAugmentation.ClassifyAndComputeHFtool   = ClassifyAndCalculateHFTool
  ClassifyAndCalculateHFAugmentation.HadronOriginClassifierTool = HadronOriginClassifierTool
  ClassifyAndCalculateHFAugmentation.JetMatchingTool            = JetMatchingTool

  # Add the augmentation too as public tool.

  acc.addPublicTool(ClassifyAndCalculateHFAugmentation, primary = True)

  # Return the tools.

  return acc   

#Configuration of the tools to compute the HF Classification of the ttbar+jets events.

def HFClassificationCommonCfg(flags):
  
    """HF Classification configuration."""
  
    acc = ComponentAccumulator()

    #Check the the DSID of the considered sample is in the list of ttbar samples.
    
    mc_channel_number = int(flags.Input.MCChannelNumber)
    
    if mc_channel_number > 0:
    
        if mc_channel_number in DSIDList:
          
          #In this case, the DSID is in the list so configure the tools.
          #Configure the tool that adds the HF Classification in the derivation file.

          DFCommonClassifyAndCalculateHFAugmentation = acc.getPrimaryAndMerge(ClassifyAndCalculateHFAugmentationCfg(flags, mc_channel_number))

          CommonAugmentation = CompFactory.DerivationFramework.CommonAugmentation  
          acc.addEventAlgo(CommonAugmentation(name              = "HFClassificationCommonKernel",
                                              AugmentationTools = [DFCommonClassifyAndCalculateHFAugmentation]))

    return acc
