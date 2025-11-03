# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#====================================================================
# DAOD_FTAG2.py
# This defines DAOD_FTAG2, an unskimmed DAOD format for Run 3 with an
# event-level skim that requires at least two leptons (e or mu) with pT>18 GeV,
# at least one of which must have pT>25 GeV.
# It contains the variables and objects needed for the large majority 
# of physics analyses in ATLAS.
# It requires the flag FTAG2 in Derivation_tf.py   
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from DerivationFrameworkFlavourTag.FTAG1 import (
    FTAG1Cfg
)

# Main algorithm config
def FTAG2KernelCfg(flags, name='FTAG2Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for FTAG2"""
    acc = ComponentAccumulator()

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(flags, TriggerListsHelper = kwargs['TriggerListsHelper']))

    lepton_skimming_expression = (
        'count( (Muons.pt > 18*GeV) && (0 == Muons.muonType || 1 == Muons.muonType || 4 == Muons.muonType) ) '
        '+ count(( Electrons.pt > 18*GeV) && ((Electrons.Loose) || (Electrons.DFCommonElectronsLHLoose))) >= 2 '
        '&& '
        'count( (Muons.pt > 25*GeV) && (0 == Muons.muonType || 1 == Muons.muonType || 4 == Muons.muonType) ) '
        '+ count(( Electrons.pt > 25*GeV) && ((Electrons.Loose) || (Electrons.DFCommonElectronsLHLoose))) >= 1'
    )

    skim_expr = lepton_skimming_expression

    FTAG2SkimmingTool = CompFactory.DerivationFramework.xAODStringSkimmingTool(
        name = "FTAG2SkimmingTool",
        expression = skim_expr
    )
    acc.addPublicTool(FTAG2SkimmingTool)


    from DerivationFrameworkInDet.InDetToolsConfig import JetTrackParticleThinningCfg, MuonTrackParticleThinningCfg, EgammaTrackParticleThinningCfg, JetConstituentThinningCfg, JetGhostThinningCfg
    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import GenericObjectThinningCfg

    thinningTools = []
    # Include inner detector tracks associated with muons
    FTAG2MuonTPThinningTool = acc.getPrimaryAndMerge(MuonTrackParticleThinningCfg(
        flags,
        name                    = "FTAG2MuonTPThinningTool",
        StreamName              = kwargs['StreamName'],
        MuonKey                 = "Muons",
        InDetTrackParticlesKey  = "InDetTrackParticles"))
    thinningTools.append(FTAG2MuonTPThinningTool)
    
    # Include inner detector tracks associated with electrons
    FTAG2ElectronTPThinningTool = acc.getPrimaryAndMerge(EgammaTrackParticleThinningCfg(
        flags,
        name                    = "FTAG2ElectronTPThinningTool",
        StreamName              = kwargs['StreamName'],
        SGKey                   = "Electrons",
        InDetTrackParticlesKey  = "InDetTrackParticles"))
    thinningTools.append(FTAG2ElectronTPThinningTool)


    # Thin jets that are below their pT requirement GeV
    FTAG2JetThinningTool = acc.getPrimaryAndMerge(GenericObjectThinningCfg(
        flags,
        name = "FTAG2AntiKt4EMPFlowJetsThinningTool",
        StreamName = kwargs['StreamName'],
        ContainerName = "AntiKt4EMPFlowJets",
        SelectionString = 'AntiKt4EMPFlowJets.pt > 15*GeV',
    ))
    thinningTools.append(FTAG2JetThinningTool)

    # TrackParticles associated with small-R jets
    FTAG2JetTPThinningTool = acc.getPrimaryAndMerge(JetTrackParticleThinningCfg(flags,
        name            = "FTAG2AntiKt4EMPFlowJetsTPThinningTool",
        StreamName      = kwargs['StreamName'],
        JetKey          = "AntiKt4EMPFlowJets",
        SelectionString = 'AntiKt4EMPFlowJets.pt > 15*GeV',
        InDetTrackParticlesKey  = "InDetTrackParticles"))
    thinningTools.append(FTAG2JetTPThinningTool)
    
    # Only keep forward calo towers associated with jets above pT threshold
    FTAG2GhostTowerThinningTool = acc.getPrimaryAndMerge(JetGhostThinningCfg(
        flags,
        name                = "FTAG2AntiKt4EMPFlowJetsGhostTowerThinningTool",
        StreamName          = kwargs['StreamName'],
        JetKey              = "AntiKt4EMPFlowJets",
        SelectionString = 'AntiKt4EMPFlowJets.pt > 15*GeV',
        GhostName           = "GhostTower",
        GhostContainerName  = "CaloCalFwdTopoTowers"
    ))
    thinningTools.append(FTAG2GhostTowerThinningTool)

    # Only keep the charged and neutral constituents of jets above pT threshold
    FTAG2JetConstituentThinningTool = acc.getPrimaryAndMerge(JetConstituentThinningCfg(
        flags,
        name                     = "FTAG2AntiKt4EMPFlowJetsConstituentThinningTool",
        StreamName               = kwargs['StreamName'],
        JetKey                   = "AntiKt4EMPFlowJets",
        SelectionString = 'AntiKt4EMPFlowJets.pt > 15*GeV',
        JetConstituentName       = "CHSG",
        GlobalConstituentName    = "Global",
        OtherObjectsName = "CaloCalTopoClusters"
    ))
    thinningTools.append(FTAG2JetConstituentThinningTool)

    # Use ONLY the combined skimming tool
    skimmingTools = [FTAG2SkimmingTool]

    # Finally the kernel itself
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name, SkimmingTools = skimmingTools, ThinningTools = thinningTools))
    return acc


def FTAG2Cfg(flags):
    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    FTAG2TriggerListsHelper = TriggerListsHelper(flags)
    
    # FTAG2 is FTAG1 content plus our skimming and thinning defined above
    acc.merge(FTAG1Cfg(flags, name_tag='FTAG2'))

    # Common augmentations + skimming kernel
    acc.merge(FTAG2KernelCfg(flags,
                             name="FTAG2Kernel",
                             StreamName = 'StreamDAOD_FTAG2',
                             TriggerListsHelper = FTAG2TriggerListsHelper))

    return acc