# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# TLACommonConfig
# Contains the configuration for the common physics containers/decorations used in analysis DAODs

# Actual configuration is subcontracted to other config files since some of them are very long

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod

def TLACommonAugmentationsCfg(ConfigFlags,**kwargs):
    """Configure the common augmentation"""
    acc = ComponentAccumulator()

    # MC truth
    if ConfigFlags.Input.isMC:
        from DerivationFrameworkMCTruth.MCTruthCommonConfig import (
            # AddStandardTruthContentsCfg,
            AddHFAndDownstreamParticlesCfg,
            AddMiniTruthCollectionLinksCfg,
            AddPVCollectionCfg,
            AddTruthCollectionNavigationDecorationsCfg)
        from DerivationFrameworkTLA.TLACommonConfigFunctions import AddStandardTLATruthContentsCfg
        from DerivationFrameworkMCTruth.TruthDerivationToolsConfig import TruthCollectionMakerCfg
        TLACommonTruthCharmTool = acc.getPrimaryAndMerge(TruthCollectionMakerCfg(
            ConfigFlags,
            name                    = "TLACommonTruthCharmTool",
            NewCollectionName       = "TruthCharm",
            KeepNavigationInfo      = False,
            ParticleSelectionString = "(abs(TruthParticles.pdgId) == 4)",
            Do_Compress             = True)) 
        CommonAugmentation = CompFactory.DerivationFramework.CommonAugmentation
        acc.addEventAlgo(CommonAugmentation("TLACommonTruthCharmKernel",AugmentationTools=[TLACommonTruthCharmTool]))
        acc.merge(AddHFAndDownstreamParticlesCfg(ConfigFlags))
        acc.merge(AddStandardTLATruthContentsCfg(ConfigFlags, useTLAPostJetAugmentations=True))
        acc.merge(AddTruthCollectionNavigationDecorationsCfg(
            ConfigFlags,
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
        acc.merge(AddMiniTruthCollectionLinksCfg(ConfigFlags))
        acc.merge(AddPVCollectionCfg(ConfigFlags))
    
    # Muon common augmentations
    from DerivationFrameworkMuons.MuonsCommonConfig import MuonsCommonCfg
    acc.merge(MuonsCommonCfg(ConfigFlags))

    # InDet common augmentations
    from DerivationFrameworkInDet.InDetCommonConfig import InDetCommonCfg
   
    acc.merge(InDetCommonCfg(ConfigFlags,
                             DoVertexFinding = ConfigFlags.Tracking.doVertexFinding,
                             AddPseudoTracks = ConfigFlags.Tracking.doPseudoTracking and ConfigFlags.GeoModel.Run<=LHCPeriod.Run3,
                             DecoLRTTTVA = False,
                             DoR3LargeD0 = ConfigFlags.Tracking.doLargeD0,
                             StoreSeparateLargeD0Container = ConfigFlags.Tracking.storeSeparateLargeD0Container,
                             MergeLRT = False))

    # Egamma common augmentations
    from DerivationFrameworkEGamma.EGammaCommonConfig import EGammaCommonCfg
    acc.merge(EGammaCommonCfg(ConfigFlags))

    # Jets, flavour tagging
    from DerivationFrameworkTLA.TLACommonConfigFunctions import TLAJetCommonCfg
    from DerivationFrameworkFlavourTag.FtagDerivationConfig import FtagJetCollectionsCfg, HLTJetFTagDecorationCfg
    acc.merge(TLAJetCommonCfg(ConfigFlags))
    if ConfigFlags.Input.isMC:
        acc.merge(HLTJetFTagDecorationCfg(ConfigFlags))

    FTagJetColl = ['AntiKt4EMPFlowJets']
    if ConfigFlags.GeoModel.Run >= LHCPeriod.Run4: 
        FTagJetColl.append('AntiKt4EMTopoJets')
    acc.merge(FtagJetCollectionsCfg(ConfigFlags,FTagJetColl))
    
    # Trigger matching (from PhysCommonConfig.py)
    if ConfigFlags.Reco.EnableTrigger or ConfigFlags.Trigger.triggerConfig == 'INFILE':
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import TriggerMatchingCommonRun2Cfg
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import TriggerMatchingCommonRun3Cfg
        # requires some wrangling due to the difference between run 2 and 3
        triggerListsHelper = kwargs['TriggerListsHelper']
        if ConfigFlags.Trigger.EDMVersion == 2:
            acc.merge(TriggerMatchingCommonRun2Cfg(ConfigFlags, 
                                                   name = "TLACommonTrigMatchNoTau", 
                                                   OutputContainerPrefix = "TrigMatch_", 
                                                   ChainNames = triggerListsHelper.Run2TriggerNamesNoTau))
            acc.merge(TriggerMatchingCommonRun2Cfg(ConfigFlags, 
                                                   name = "TLACommonTrigMatchTau", 
                                                   OutputContainerPrefix = "TrigMatch_", 
                                                   ChainNames = triggerListsHelper.Run2TriggerNamesTau, 
                                                   DRThreshold = 0.2))
        if ConfigFlags.Trigger.EDMVersion == 3:
            acc.merge(TriggerMatchingCommonRun3Cfg(ConfigFlags, TriggerList = triggerListsHelper.Run3TriggerNames))

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
        # "TruthForwardProtons",
        # "BornLeptons",
        "TruthBosonsWithDecayParticles",
        "TruthBosonsWithDecayVertices",
        "TruthBSMWithDecayParticles",
        "TruthBSMWithDecayVertices",
        "HardScatterParticles",
        "HardScatterVertices",
    ]
    slimmer.ExtraVariables += [
        "AntiKt4TruthDressedWZJets.GhostCHadronsFinalCount.GhostBHadronsFinalCount.pt.HadronConeExclTruthLabelID.ConeTruthLabelID.PartonTruthLabelID.TrueFlavor",
        "TruthEvents.Q.XF1.XF2.PDGID1.PDGID2.PDFID1.PDFID2.X1.X2.crossSection"]
