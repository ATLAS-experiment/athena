# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# PhysCommonConfig
# Contains the configuration for the common physics containers/decorations used in analysis DAODs
# including PHYS(LITE)
# Actual configuration is subcontracted to other config files since some of them are very long

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod
from AthenaCommon.Logging import logging
msg = logging.getLogger('PHYSCommonConfig')

def PhysCommonAugmentationsCfg(flags,**kwargs):
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
        from DerivationFrameworkMCTruth.TruthDerivationToolsConfig import TruthCollectionMakerCfg
        PhysCommonTruthCharmTool = acc.getPrimaryAndMerge(TruthCollectionMakerCfg(
            flags,
            name                    = "PhysCommonTruthCharmTool",
            NewCollectionName       = "TruthCharm",
            KeepNavigationInfo      = False,
            ParticleSelectionString = "(abs(TruthParticles.pdgId) == 4)",
            Do_Compress             = True)) 
        CommonAugmentation = CompFactory.DerivationFramework.CommonAugmentation
        acc.addEventAlgo(CommonAugmentation("PhysCommonTruthCharmKernel",AugmentationTools=[PhysCommonTruthCharmTool]))
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
            prefix = 'PHYS_'))
        # Re-point links on reco objects
        acc.merge(AddMiniTruthCollectionLinksCfg(flags))
        acc.merge(AddPVCollectionCfg(flags))

    # InDet, Muon, Egamma common augmentations
    from DerivationFrameworkInDet.InDetCommonConfig import InDetCommonCfg
    from DerivationFrameworkMuons.MuonsCommonConfig import MuonsCommonCfg
    from DerivationFrameworkEGamma.EGammaCommonConfig import EGammaCommonCfg
    # TODO: need to find the new flags equivalent for the missing settings below, then we can
    # drop these kwargs and do everything via the flags
    acc.merge(InDetCommonCfg(flags,
                             DoVertexFinding = flags.Tracking.doVertexFinding,
                             AddPseudoTracks = flags.Tracking.doPseudoTracking,
                             DecoLRTTTVA = False,
                             DoR3LargeD0 = flags.Tracking.doLargeD0,
                             StoreSeparateLargeD0Container = flags.Tracking.storeSeparateLargeD0Container,
                             MergeLRT = False)) 
    acc.merge(MuonsCommonCfg(flags))
    acc.merge(EGammaCommonCfg(flags))
    # Jets, di-taus, tau decorations, flavour tagging, MET association
    from DerivationFrameworkJetEtMiss.JetCommonConfig import JetCommonCfg
    from DerivationFrameworkFlavourTag.FtagDerivationConfig import FtagJetCollectionsCfg
    from DerivationFrameworkTau.TauCommonConfig import (AddDiTauLowPtCfg, AddMuonRemovalTauAODReRecoAlgCfg, AddTauIDDecorationCfg)
    from DerivationFrameworkJetEtMiss.METCommonConfig import METCommonCfg 
    acc.merge(JetCommonCfg(flags))
    #We also need to build links between the newly created jet constituents (GlobalFE)
    #and electrons,photons,muons and taus
    from eflowRec.PFCfg import PFGlobalFlowElementLinkingCfg    
    acc.merge(PFGlobalFlowElementLinkingCfg(flags))
    acc.merge(AddDiTauLowPtCfg(flags))
    acc.merge(AddMuonRemovalTauAODReRecoAlgCfg(flags))
    # eVeto WP and DeepSet ID for taus and muon-subtracted taus
    acc.merge(AddTauIDDecorationCfg(flags, TauContainerName="TauJets"))
    acc.merge(AddTauIDDecorationCfg(flags, TauContainerName="TauJets_MuonRM"))
    # for AOD produced before 24.0.17, the electron removal tau is not available
    if flags.Tau.TauEleRM_isAvailable:
        acc.merge(AddTauIDDecorationCfg(flags, TauContainerName="TauJets_EleRM"))
    FTagJetColl = ['AntiKt4EMPFlowJets', 'AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets']
    if flags.GeoModel.Run >= LHCPeriod.Run4:
        FTagJetColl.append('AntiKt4EMTopoJets')
    if flags.Reco.EnableBTagging:
        acc.merge(FtagJetCollectionsCfg(flags,FTagJetColl))
    acc.merge(METCommonCfg(flags))

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

