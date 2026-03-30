# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#==============================================================================
# Contains the configuration for TLA jet reconstruction + decorations
# used in analysis DAODs
#==============================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# Jet related
from DerivationFrameworkJetEtMiss.JetCommonConfig import AddBadBatmanCfg, AddDistanceInTrainCfg, AddNeutralEventShapeCfg, AddEventCleanFlagsCfg

## These jet related functions are a copy of the ones in the JetCommonConfig file
## but we remove the jet collections we don't need
def TLAJetCommonCfg(ConfigFlags):
    """TLA config for jet reconstruction and decorations"""

    acc = ComponentAccumulator()

    acc.merge(StandardTLAJetsCfg(ConfigFlags))


    if "McEventCollection#GEN_EVENT" not in ConfigFlags.Input.TypedCollections:
        acc.merge(AddBadBatmanCfg(ConfigFlags))
    acc.merge(AddDistanceInTrainCfg(ConfigFlags))
    acc.merge(AddNeutralEventShapeCfg(ConfigFlags))
    acc.merge(AddEventCleanFlagsCfg(ConfigFlags))

    return acc

def StandardTLAJetsCfg(ConfigFlags):
    """Standard (offline) jets needed for TLA derivations"""

    from JetRecConfig.StandardSmallRJets import AntiKt4EMTopo,AntiKt4EMPFlow
    from JetRecConfig.JetRecConfig import JetRecCfg

    acc = ComponentAccumulator()
    
    AntiKt4EMTopo_deriv = AntiKt4EMTopo.clone(
        modifiers = AntiKt4EMTopo.modifiers+("JetPtAssociation","QGTagging")
    )

    AntiKt4EMPFlow_deriv = AntiKt4EMPFlow.clone(
        modifiers = AntiKt4EMPFlow.modifiers+("JetPtAssociation","QGTagging","fJVT","NNJVT","CaloEnergiesClus")
    )

    jetList = [AntiKt4EMTopo_deriv, AntiKt4EMPFlow_deriv]


    for jd in jetList:
        acc.merge(JetRecCfg(ConfigFlags,jd))
    
    return acc

## Custom functions for handling TLA truth jets (and related truth content) for MC
## designed to remove large-R jets that are un-needed at the moment
from DerivationFrameworkMCTruth.MCTruthCommonConfig import (HepMCtoXAODTruthCfg, PreJetMCTruthAugmentationsCfg, PostJetMCTruthAugmentationsCfg, AddTruthCollectionNavigationDecorationsCfg, AddBosonsAndDownstreamParticlesCfg, AddBSMAndDownstreamParticlesCfg, AddTruthEnergyDensityCfg)

def AddTLATruthJetsCfg(flags):
    acc = ComponentAccumulator()

    from JetRecConfig.StandardSmallRJets import AntiKt4Truth,AntiKt4TruthWZ,AntiKt4TruthDressedWZ
    from JetRecConfig.JetRecConfig import JetRecCfg


    jetList = [AntiKt4Truth,AntiKt4TruthWZ,AntiKt4TruthDressedWZ]

    for jd in jetList:
        acc.merge(JetRecCfg(flags,jd))

    return acc


def PostTLAJetMCTruthAugmentationsCfg(flags, **kwargs):

    acc = ComponentAccumulator()

    # Tau collections are built separately
    # truth tau matching needs truth jets, truth electrons and truth muons
    from DerivationFrameworkTau.TauTruthCommonConfig import TauTruthToolsCfg
    acc.merge(TauTruthToolsCfg(flags))

    from DerivationFrameworkMCTruth.TruthDerivationToolsConfig import DFCommonTruthTauDressingCfg
    acc.merge(DFCommonTruthTauDressingCfg(flags))

    from DerivationFrameworkMCTruth.TruthDerivationToolsConfig import DFCommonTruthDressedWZQGLabelCfg
    acc.merge(DFCommonTruthDressedWZQGLabelCfg(flags))

    # SUSY signal decorations
    from DerivationFrameworkSUSY.DecorateSUSYProcessConfig import IsSUSYSignalRun3
    if IsSUSYSignalRun3(flags):
        from DerivationFrameworkSUSY.DecorateSUSYProcessConfig import SUSYSignalTaggerCfg
        acc.merge(SUSYSignalTaggerCfg(flags, 'MCTruthCommon'))
        # add SoW of individual SUSY final states, relies on augmentation from DecorateSUSYProcess()
        from DerivationFrameworkSUSY.SUSYWeightMetadataConfig import AddSUSYWeightsCfg
        acc.merge(AddSUSYWeightsCfg(flags))

    return acc


# This adds the entirety of TRUTH3
def AddStandardTLATruthContentsCfg(flags,
                                decorationDressing='dressedPhoton',
                                includeTausInDressingPhotonRemoval=False,
                                prefix='',
                                # default is true to prevent crashes when AntiKt10TruthJets is not built
                                useTLAPostJetAugmentations=True):

    acc = ComponentAccumulator()

    # Schedule HepMC->xAOD truth conversion
    acc.merge(HepMCtoXAODTruthCfg(flags))

    # Local flag
    isEVNT = False
    if "McEventCollection#GEN_EVENT" in flags.Input.TypedCollections: isEVNT = True
    # Tools that must come before jets
    acc.merge(PreJetMCTruthAugmentationsCfg(flags,decorationDressing = decorationDressing))
    # Jets and MET
    acc.merge(AddTLATruthJetsCfg(flags))
    # Tools that must come after jets
    # configured for compatibility in case we want to 
    # revert to common MCTruth version
    if not useTLAPostJetAugmentations:
        acc.merge(PostJetMCTruthAugmentationsCfg(flags, decorationDressing = decorationDressing))
    else:
        acc.merge(PostTLAJetMCTruthAugmentationsCfg(flags, decorationDressing = decorationDressing))
    # Should photons that are dressed onto taus also be removed from truth jets?
    if includeTausInDressingPhotonRemoval:
        acc.getEventAlgo("DFCommonTruthTauDressingAlg").decorationName=decorationDressing

    # Add back the navigation contect for the collections we want
    acc.merge(AddTruthCollectionNavigationDecorationsCfg(flags, ["TruthElectrons", "TruthMuons", "TruthPhotons", "TruthTaus", "TruthNeutrinos", "TruthBSM", "TruthBottom", "TruthTop", "TruthBoson"], prefix=prefix))
    # Some more additions for standard TRUTH3
    acc.merge(AddBosonsAndDownstreamParticlesCfg(flags))
    
    # Special collection for BSM particles
    acc.merge(AddBSMAndDownstreamParticlesCfg(flags))

    # Energy density for isolation corrections
    if isEVNT: acc.merge(AddTruthEnergyDensityCfg(flags))

    return acc

