# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# NCBCommonConfig
# Contains the configuration for the common physics containers/decorations used in analysis DAODs
# Actual configuration is subcontracted to other config files since some of them are very long

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def StandardJetsInDerivCfg(flags):
    """Jet reconstruction needed for NCB1"""

    from JetRecConfig.StandardSmallRJets import AntiKt4EMTopo
    from JetRecConfig.JetRecConfig import JetRecCfg

    acc = ComponentAccumulator()

    AntiKt4EMTopo_deriv = AntiKt4EMTopo.clone(
        modifiers = AntiKt4EMTopo.modifiers+("JetPtAssociation","QGTagging")
    )

    jetList = [AntiKt4EMTopo_deriv]


    for jd in jetList:
        acc.merge(JetRecCfg(flags,jd))

    return acc


def NCBCommonAugmentationsCfg(flags,**kwargs):
    """Configure the common augmentation"""
    acc = ComponentAccumulator()

    # MC truth
    if flags.Input.isMC:
        from DerivationFrameworkMCTruth.MCTruthCommonConfig import (
            AddStandardTruthContentsCfg,
            AddHFAndDownstreamParticlesCfg,
            AddMiniTruthCollectionLinksCfg,
            AddPVCollectionCfg,
            AddTruthCollectionNavigationDecorationsCfg)
        from DerivationFrameworkMCTruth.TruthDerivationToolsConfig import DFCommonTruthCharmToolCfg
        NCBCommonTruthCharmTool = acc.getPrimaryAndMerge(DFCommonTruthCharmToolCfg(
            flags,
            name = "NCBCommonTruthCharmTool"))
        CommonAugmentation = CompFactory.DerivationFramework.CommonAugmentation
        acc.addEventAlgo(CommonAugmentation("NCBCommonTruthCharmKernel",AugmentationTools=[NCBCommonTruthCharmTool]))
        acc.merge(AddHFAndDownstreamParticlesCfg(flags))
        acc.merge(AddStandardTruthContentsCfg(flags))
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
            prefix = 'NCB_'))
        # Re-point links on reco objects
        acc.merge(AddMiniTruthCollectionLinksCfg(flags))
        acc.merge(AddPVCollectionCfg(flags))

    # InDet, Muon, Egamma common augmentations
    from DerivationFrameworkMuons.MuonsCommonConfig import MuonsCommonCfg
    acc.merge(MuonsCommonCfg(flags))
    # TODO: need to find the new flags equivalent for the missing settings below, then we can
    # drop these kwargs and do everything via the flags
    from DerivationFrameworkInDet.InDetCommonConfig import InDetCommonCfg
    acc.merge(InDetCommonCfg(flags,
                             DoVertexFinding = flags.Tracking.doVertexFinding,
                             AddPseudoTracks = flags.Tracking.doPseudoTracking,
                             DecoLRTTTVA = False,
                             DoR3LargeD0 = flags.Tracking.doLargeD0,
                             StoreSeparateLargeD0Container = flags.Tracking.storeSeparateLargeD0Container,
                             MergeLRT = False)) 
    # Jets, di-taus, tau decorations, flavour tagging, MET association
    from DerivationFrameworkJetEtMiss.METCommonConfig import METCommonCfg
    acc.merge(METCommonCfg(flags))
    acc.merge(StandardJetsInDerivCfg(flags))

    # Trigger matching
    if flags.Reco.EnableTrigger or flags.Trigger.triggerConfig == 'INFILE':
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import TriggerMatchingCommonRun2Cfg
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import TriggerMatchingCommonRun2ToRun3Cfg
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import TriggerMatchingCommonRun3Cfg
        # requires some wrangling due to the difference between run 2 and 3
        triggerListsHelper = kwargs['TriggerListsHelper']
        if flags.Trigger.EDMVersion == 2:
            # NOTE: Once Run-2 -> Run-3 trigger navigation is validated and doEDMVersionConversion is on by default, we will only want to do ONE of
            # TriggerMatchingCommonRun2Cfg(s) OR TriggerMatchingCommonRun2ToRun3Cfg
            # Otherwise we are doubling up on the analysis trigger data in both the Run-2 and Run-3 formats.

            # This sets up the Run-2 style matching during the derivation process
            acc.merge(TriggerMatchingCommonRun2Cfg(
                flags,
                name = "PhysCommonTrigMatchNoTau",
                OutputContainerPrefix = "TrigMatch_",
                ChainNames = triggerListsHelper.Run2TriggerNamesNoTau))
            acc.merge(TriggerMatchingCommonRun2Cfg(
                flags,
                name = "PhysCommonTrigMatchTau",
                OutputContainerPrefix = "TrigMatch_",
                ChainNames = triggerListsHelper.Run2TriggerNamesTau,
                DRThreshold = 0.2))
            # This sets up a conversion of the Run-2 trigger navigation to the Run-3 style, 
            # followed by Run-3 style navigation slimming for trigger-matching from DAOD.
            # This function is a noop if doEDMVersionConversion=False
            acc.merge(TriggerMatchingCommonRun2ToRun3Cfg(
                flags,
                TriggerList = triggerListsHelper.Run2TriggerNamesNoTau +
                triggerListsHelper.Run2TriggerNamesTau))
        if flags.Trigger.EDMVersion == 3:
            # This sets up the Run-3 style navigation slimming for trigger-matching from DAOD
            acc.merge(TriggerMatchingCommonRun3Cfg(
                flags, TriggerList = triggerListsHelper.Run3TriggerNames))

    return acc

