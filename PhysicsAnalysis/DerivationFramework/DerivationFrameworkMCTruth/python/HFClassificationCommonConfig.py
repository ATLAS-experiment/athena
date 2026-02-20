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

def DFCommonMatchingToolCfg(flags, **kwargs):

  #################################
  ####### Jet Matching Tool #######
  #################################

  acc = ComponentAccumulator()
  # Configure the tool that matches each truth particle to the closest jet.
  # Parameters:
  #  -m_jetPtCut:  Cut on the pt of the jets.
  #  -m_jetEtaCut: Cut on the eta of the jets.
  #  -m_drCut:     Upper limit for the delta R between the particle and the jet to perform the matching.
  kwargs.setdefault("jetPtCut", 15000.)
  kwargs.setdefault("jetEtaCut", 2.5)
  kwargs.setdefault("drCut", 0.3)
  acc.setPrivateTools(CompFactory.DerivationFramework.JetMatchingTool(name='DFCommonJetMatchingTool', **kwargs))
  return acc


def DFCommonHadronOriginClassifierCfg(flags, **kwargs):
  acc = ComponentAccumulator()
  ##################################
  ##### HF Hadrons Origin Tool #####
  ##################################

  # Configure the tool that determines the origin of the HF hadrons.
  # Parameters:
  #  -DSID: DSID of the sample that is being processed.
  kwargs.setdefault("DSID", int(flags.Input.MCChannelNumber))
  acc.setPrivateTools(CompFactory.DerivationFramework.HadronOriginClassifier(name='DFCommonHadronOriginClassifier', **kwargs))
  return acc


def DFCommonClassifyAndCalculateHFToolCfg(flags, **kwargs):
  acc = ComponentAccumulator()
  ##################################
  ##### HF Classification Tool #####
  ##################################

  # Configure the tool that computes the HF Classification.
  # Parameters:
  #  -m_jetPtCut:                Cut on the pt of the jets.
  #  -m_jetEtaCut:               Cut on the eta of the jets.
  #  -m_leadingHadronPtCut:      Cut on the pt of the leading hadron.
  #  -m_leadingHadronPtRatioCut: Cut on the ratio between the pt of the leading hadron and the pt of its associated jet.
  kwargs.setdefault("jetPtCut", 15000.)
  kwargs.setdefault("jetEtaCut", 2.5)
  kwargs.setdefault("leadingHadronPtCut", 5000.)
  kwargs.setdefault("leadingHadronPtRatioCut", -1)
  acc.setPrivateTools(CompFactory.DerivationFramework.ClassifyAndCalculateHFTool(name='DFCommonClassifyAndCalculateHFTool', **kwargs))
  return acc


def ClassifyAndCalculateHFAugmentationCfg(flags, mc_channel_number, **kwargs):

  """Configure and add the tools to compute and add the HF classifier in the derivation"""

  acc = ComponentAccumulator()

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
  kwargs.setdefault("jetCollectionName", "AntiKt4TruthDressedWZJets")
  kwargs.setdefault("TruthParticleContainerName", "TruthParticles")
  kwargs.setdefault("hfDecorationName",  "HF_Classification")
  kwargs.setdefault("SimplehfDecorationName", "HF_SimpleClassification")
  kwargs.setdefault("HadronOriginIDDecorationName", "HFHadronOriginID")
  kwargs.setdefault("ClassifyAndComputeHFtool", acc.addPublicTool(acc.popToolsAndMerge(DFCommonClassifyAndCalculateHFToolCfg(flags))))
  kwargs.setdefault("HadronOriginClassifierTool", acc.addPublicTool(acc.popToolsAndMerge(DFCommonHadronOriginClassifierCfg(flags))))
  kwargs.setdefault("JetMatchingTool", acc.addPublicTool(acc.popToolsAndMerge(DFCommonMatchingToolCfg(flags))))
  acc.setPrivateTools(CompFactory.DerivationFramework.ClassifyAndCalculateHFAugmentation(name = "DFCommonClassifyAndCalculateHFAugmentation", **kwargs))
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
          CommonAugmentation = CompFactory.DerivationFramework.CommonAugmentation
          acc.addEventAlgo(CommonAugmentation(name              = "HFClassificationCommonKernel",
                                              AugmentationTools = [acc.addPublicTool(acc.popToolsAndMerge(ClassifyAndCalculateHFAugmentationCfg(flags, mc_channel_number)))]))

    return acc
