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

def DFCommonMatchingToolCfg(flags, variation, **kwargs):

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
  acc.setPrivateTools(CompFactory.DerivationFramework.JetMatchingTool(name='DFCommonJetMatchingTool'+variation, **kwargs))
  return acc


def DFCommonMatchingToolJ20Cfg(flags, **kwargs):
  kwargs.setdefault("jetPtCut", 20000.)

  return DFCommonMatchingToolCfg(flags, "J20", **kwargs)


def DFCommonMatchingToolJ25Cfg(flags, **kwargs):
  kwargs.setdefault("jetPtCut", 25000.)

  return DFCommonMatchingToolCfg(flags, "J25", **kwargs)


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


def DFCommonClassifyAndCalculateHFToolCfg(flags, variation, **kwargs):
  acc = ComponentAccumulator()
  ##################################
  ##### HF Classification Tool #####
  ##################################

  # Configure the tool that computes the HF Classification.
  # Parameters:
  #  -m_jetPtCut:                Cut on the pt of the jets.
  #  -m_jetEtaCut:               Cut on the eta of the jets.
  #  -m_leadingBHadronPtCut:      Cut on the pt of the leading B-hadron.
  #  -m_leadingCHadronPtCut:      Cut on the pt of the leading C-hadron.
  #  -m_leadingHadronPtRatioCut: Cut on the ratio between the pt of the leading hadron and the pt of its associated jet.
  kwargs.setdefault("jetPtCut", 15000.)
  kwargs.setdefault("jetEtaCut", 2.5)
  kwargs.setdefault("leadingBHadronPtCut", 5000.)
  kwargs.setdefault("leadingCHadronPtCut", 5000.)
  kwargs.setdefault("leadingHadronPtRatioCut", -1)
  acc.setPrivateTools(CompFactory.DerivationFramework.ClassifyAndCalculateHFTool(name='DFCommonClassifyAndCalculateHFTool'+variation, **kwargs))
  return acc


def DFCommonClassifyAndCalculateHFToolC5J20Cfg(flags, **kwargs):
  kwargs.setdefault("jetPtCut", 20000.)
  kwargs.setdefault("leadingCHadronPtCut", 5000.)

  return DFCommonClassifyAndCalculateHFToolCfg(flags, "C5J20", **kwargs)


def DFCommonClassifyAndCalculateHFToolC5J25Cfg(flags, **kwargs):
  kwargs.setdefault("jetPtCut", 25000.)
  kwargs.setdefault("leadingCHadronPtCut", 5000.)

  return DFCommonClassifyAndCalculateHFToolCfg(flags, "C5J25", **kwargs)


def DFCommonClassifyAndCalculateHFToolC15J20Cfg(flags, **kwargs):
  kwargs.setdefault("jetPtCut", 20000.)
  kwargs.setdefault("leadingCHadronPtCut", 15000.)

  return DFCommonClassifyAndCalculateHFToolCfg(flags, "C15J20", **kwargs)


def DFCommonClassifyAndCalculateHFToolC15J25Cfg(flags, **kwargs):
  kwargs.setdefault("jetPtCut", 25000.)
  kwargs.setdefault("leadingCHadronPtCut", 15000.)

  return DFCommonClassifyAndCalculateHFToolCfg(flags, "C15J25", **kwargs)


def ClassifyAndCalculateHFAugmentationCfg(flags, mc_channel_number, variation, **kwargs):

  """Configure and add the tools to compute and add the HF classifier in the derivation"""

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

  acc = ComponentAccumulator()
  kwargs.setdefault("jetCollectionName", "AntiKt4TruthDressedWZJets")
  kwargs.setdefault("TruthParticleContainerName", "TruthParticles")
  kwargs.setdefault("hfDecorationName", "HF_Classification"+variation)
  kwargs.setdefault("SimplehfDecorationName", "HF_SimpleClassification"+variation)
  kwargs.setdefault("HadronOriginIDDecorationName", "HFHadronOriginID"+variation)
  kwargs.setdefault("HadronOriginClassifierTool", acc.addPublicTool(acc.popToolsAndMerge(DFCommonHadronOriginClassifierCfg(flags))))
  acc.addEventAlgo(CompFactory.DerivationFramework.ClassifyAndCalculateHFAugmentation(name = "DFCommonClassifyAndCalculateHFAugmentation"+variation))

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
          acc.merge(ClassifyAndCalculateHFAugmentationCfg(
            flags,
            mc_channel_number,
            variation = "",
            ClassifyAndComputeHFtool = acc.addPublicTool(acc.popToolsAndMerge(DFCommonClassifyAndCalculateHFToolCfg(flags, ""))),
            JetMatchingTool = acc.addPublicTool(acc.popToolsAndMerge(DFCommonMatchingToolCfg(flags, "")))
          ))

          acc.merge(ClassifyAndCalculateHFAugmentationCfg(
            flags,
            mc_channel_number,
            variation = "C5J20",
            ClassifyAndComputeHFtool = acc.addPublicTool(acc.popToolsAndMerge(DFCommonClassifyAndCalculateHFToolC5J20Cfg(flags))),
            JetMatchingTool = acc.addPublicTool(acc.popToolsAndMerge(DFCommonMatchingToolJ20Cfg(flags)))
          ))

          acc.merge(ClassifyAndCalculateHFAugmentationCfg(
            flags,
            mc_channel_number,
            variation = "C5J25",
            ClassifyAndComputeHFtool = acc.addPublicTool(acc.popToolsAndMerge(DFCommonClassifyAndCalculateHFToolC5J25Cfg(flags))),
            JetMatchingTool = acc.addPublicTool(acc.popToolsAndMerge(DFCommonMatchingToolJ25Cfg(flags)))
          ))

          acc.merge(ClassifyAndCalculateHFAugmentationCfg(
            flags,
            mc_channel_number,
            variation = "C15J20",
            ClassifyAndComputeHFtool = acc.addPublicTool(acc.popToolsAndMerge(DFCommonClassifyAndCalculateHFToolC15J20Cfg(flags))),
            JetMatchingTool = acc.addPublicTool(acc.popToolsAndMerge(DFCommonMatchingToolJ20Cfg(flags)))
          ))

          acc.merge(ClassifyAndCalculateHFAugmentationCfg(
            flags,
            mc_channel_number,
            variation = "C15J25",
            ClassifyAndComputeHFtool = acc.addPublicTool(acc.popToolsAndMerge(DFCommonClassifyAndCalculateHFToolC15J25Cfg(flags))),
            JetMatchingTool = acc.addPublicTool(acc.popToolsAndMerge(DFCommonMatchingToolJ25Cfg(flags)))
          ))

    return acc
