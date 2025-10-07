# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# TLACommonConfig
# Contains the configuration for the common physics containers/decorations used in analysis DAODs

# Actual configuration is subcontracted to other config files since some of them are very long

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod

def TLACommonAugmentationsCfg(flags,**kwargs):
    """Configure the common augmentation"""
    acc = ComponentAccumulator()

    # MC truth
    if flags.Input.isMC:
        from DerivationFrameworkMCTruth.MCTruthCommonConfig import (
            # AddStandardTruthContentsCfg,
            AddHFAndDownstreamParticlesCfg,
            AddMiniTruthCollectionLinksCfg,
            AddPVCollectionCfg,
            AddTruthCollectionNavigationDecorationsCfg,
            TruthClassificationAugmentationsCfg)
        acc.merge(TruthClassificationAugmentationsCfg(flags))
        from DerivationFrameworkTLA.TLACommonConfigFunctions import AddStandardTLATruthContentsCfg
        from DerivationFrameworkMCTruth.TruthDerivationToolsConfig import DFCommonTruthCharmToolCfg
        TLACommonTruthCharmTool = acc.getPrimaryAndMerge(DFCommonTruthCharmToolCfg(
            flags,
            name = "TLACommonTruthCharmTool"))
        CommonAugmentation = CompFactory.DerivationFramework.CommonAugmentation
        acc.addEventAlgo(CommonAugmentation("TLACommonTruthCharmKernel",AugmentationTools=[TLACommonTruthCharmTool]))
        acc.merge(AddHFAndDownstreamParticlesCfg(flags))
        acc.merge(AddStandardTLATruthContentsCfg(flags, useTLAPostJetAugmentations=True))
        acc.merge(AddTruthCollectionNavigationDecorationsCfg(
            flags,
            TruthCollections=["TruthElectrons", 
                              "TruthMuons", 
                              "TruthPhotons", 
                              "TruthTaus", 
                              "TruthNeutrinos", 
                              "TruthBSM", 
                              "TruthBottom", 
                              "TruthTop", 
                              "TruthBoson",
                              "TruthCharm",
                              "TruthHFWithDecayParticles"],
            prefix = 'PHYS_'))
        # Re-point links on reco objects
        acc.merge(AddMiniTruthCollectionLinksCfg(flags))
        acc.merge(AddPVCollectionCfg(flags))
    
    # Muon common augmentations
    from DerivationFrameworkMuons.MuonsCommonConfig import MuonsCommonCfg
    acc.merge(MuonsCommonCfg(flags))

    # InDet common augmentations
    from DerivationFrameworkInDet.InDetCommonConfig import InDetCommonCfg
   
    acc.merge(InDetCommonCfg(flags,
                             DoVertexFinding = flags.Tracking.doVertexFinding,
                             AddPseudoTracks = flags.Tracking.doPseudoTracking and flags.GeoModel.Run<=LHCPeriod.Run3,
                             DecoLRTTTVA = False,
                             DoR3LargeD0 = flags.Tracking.doLargeD0,
                             StoreSeparateLargeD0Container = flags.Tracking.storeSeparateLargeD0Container,
                             MergeLRT = False))

    # Egamma common augmentations
    from DerivationFrameworkEGamma.EGammaCommonConfig import EGammaCommonCfg
    acc.merge(EGammaCommonCfg(flags))

    # Jets, flavour tagging
    from DerivationFrameworkTLA.TLACommonConfigFunctions import TLAJetCommonCfg
    from DerivationFrameworkFlavourTag.FtagDerivationConfig import FtagJetCollectionsCfg, HLTJetFTagDecorationCfg
    acc.merge(TLAJetCommonCfg(flags))
    if flags.Input.isMC and flags.Trigger.EDMVersion == 3:
        acc.merge(HLTJetFTagDecorationCfg(flags))

    FTagJetColl = ['AntiKt4EMPFlowJets']
    if flags.GeoModel.Run >= LHCPeriod.Run4:
        FTagJetColl.append('AntiKt4EMTopoJets')
    acc.merge(FtagJetCollectionsCfg(flags,FTagJetColl))
    
    # Trigger matching (from PhysCommonConfig.py)
    if flags.Reco.EnableTrigger or flags.Trigger.triggerConfig == 'INFILE':
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import TriggerMatchingCommonRun2Cfg
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import TriggerMatchingCommonRun3Cfg
        # requires some wrangling due to the difference between run 2 and 3
        triggerListsHelper = kwargs['TriggerListsHelper']
        if flags.Trigger.EDMVersion == 2:
            acc.merge(TriggerMatchingCommonRun2Cfg(flags,
                                                   name = "TLACommonTrigMatchNoTau", 
                                                   OutputContainerPrefix = "TrigMatch_", 
                                                   ChainNames = triggerListsHelper.Run2TriggerNamesNoTau))
            acc.merge(TriggerMatchingCommonRun2Cfg(flags,
                                                   name = "TLACommonTrigMatchTau", 
                                                   OutputContainerPrefix = "TrigMatch_", 
                                                   ChainNames = triggerListsHelper.Run2TriggerNamesTau, 
                                                   DRThreshold = 0.2))
        if flags.Trigger.EDMVersion == 3:
            acc.merge(TriggerMatchingCommonRun3Cfg(flags, TriggerList = triggerListsHelper.Run3TriggerNames))

    return acc


def addTLATruth3ContentToSlimmerTool(slimmer):
    slimmer.AllVariables += [
        # "MET_Truth",
        "TruthElectrons",
        "TruthMuons",
        "TruthPhotons",
        "TruthTaus",
        "TruthNeutrinos",
        "TruthBSM",
        "TruthBottom",
        "TruthTop",
        "TruthBoson",
        "TruthBosonsWithDecayParticles",
        "TruthBosonsWithDecayVertices",
        "TruthBSMWithDecayParticles",
        "TruthBSMWithDecayVertices",
    ]
    slimmer.ExtraVariables += [
        "AntiKt4TruthDressedWZJets.GhostCHadronsFinalCount.GhostBHadronsFinalCount.pt.HadronConeExclTruthLabelID.PartonTruthLabelID.TrueFlavor",
        "TruthEvents.Q.XF1.XF2.PDGID1.PDGID2.PDFID1.PDFID2.X1.X2.crossSection"]
