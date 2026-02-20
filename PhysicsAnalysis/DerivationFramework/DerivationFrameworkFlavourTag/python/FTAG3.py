# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#====================================================================
# DAOD_FTAG3.py
# This defines DAOD_FTAG3, an unskimmed DAOD format for Run 3.
# It is designed for the g->bb calibration.
# It requires the flag FTAG3 in Derivation_tf.py   
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


# Main algorithm config
def FTAG3KernelCfg(flags, name='FTAG3Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for FTAG3"""
    acc = ComponentAccumulator()
    
    from DerivationFrameworkFlavourTag.FTAG1 import FTAG1KernelCfg
    acc.merge(FTAG1KernelCfg(flags, name, TriggerListsHelper = kwargs['TriggerListsHelper']))

    # augmentation tools
    augmentationTools = []

    # skimming tools
    skimmingTools = []
    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import (
        xAODStringSkimmingToolCfg)

    # filter leptons
    lepton_skimming_expression = 'count( (Muons.pt > 5*GeV) && (0 == Muons.muonType || 1 == Muons.muonType || 4 == Muons.muonType) ) >=1'
    FTAG3LeptonSkimmingTool = acc.getPrimaryAndMerge(
        xAODStringSkimmingToolCfg(flags, name = "FTAG3LeptonSkimmingTool",
                                  expression = lepton_skimming_expression))

    # filter large-R jets
    UFOjets_skimming_expression = 'count( AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets.pt > 150*GeV ) >= 1' 
    FTAG3UFOjetsSkimmingTool = acc.getPrimaryAndMerge(
        xAODStringSkimmingToolCfg(flags, name = "FTAG3UFOjetsSkimmingTool",
                                  expression = UFOjets_skimming_expression))

    # filter single-jet triggers for data
    if not flags.Input.isMC:
        acc.merge(FTAG3TriggerSkimmingToolCfg(flags, skimmingTools))

    # thinning tools
    thinningTools = []
    from DerivationFrameworkInDet.InDetToolsConfig import MuonTrackParticleThinningCfg
    # Include inner detector tracks associated with muons
    FTAG3MuonTPThinningTool = acc.getPrimaryAndMerge(MuonTrackParticleThinningCfg(
        flags,
        name                    = "FTAG3MuonTPThinningTool",
        StreamName              = kwargs['StreamName'],
        MuonKey                 = "Muons",
        InDetTrackParticlesKey  = "InDetTrackParticles"))

    skimmingTools += [
            FTAG3UFOjetsSkimmingTool,
            FTAG3LeptonSkimmingTool,
            ]

    thinningTools = [
            FTAG3MuonTPThinningTool,
            ]

    # Finally the kernel itself
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name, AugmentationTools = augmentationTools, ThinningTools = thinningTools, SkimmingTools = skimmingTools))
    return acc


def FTAG3Cfg(flags, skimmingTools=None):
    acc = ComponentAccumulator()
    
    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    FTAG3TriggerListsHelper = TriggerListsHelper(flags)

    # the name_tag has to consistent between KernelCfg and CoreCfg
    FTAG3_name_tag = 'FTAG3'

    # Common augmentations
    acc.merge(FTAG3KernelCfg(flags, name= FTAG3_name_tag + "Kernel", StreamName = 'StreamDAOD_'+FTAG3_name_tag, TriggerListsHelper = FTAG3TriggerListsHelper))

    from DerivationFrameworkFlavourTag.FTAG1 import FTAG1CoreCfg
    from BTagging.FlavorTaggingConfig import FlavorTaggingCfg

    #add SV1 info for gbb
    extra_SmartCollections = [ "AntiKtVR30Rmax4Rmin02PV0TrackJets" ]
    extra_AllVariables = [ "AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets"]
    trigger_option = 'FTAG3'
    acc.merge(FTAG1CoreCfg(flags, FTAG3_name_tag, extra_SmartCollections, extra_AllVariables, trigger_option, TriggerListsHelper = FTAG3TriggerListsHelper))
    #add SV1 info for gbb
    acc.merge(FlavorTaggingCfg(flags, "AntiKtVR30Rmax4Rmin02PV0TrackJets"))

    return acc


def FTAG3TriggerSkimmingToolCfg(flags, skimmingTools=None):
    """configure the trigger skimming tool"""
    acc = ComponentAccumulator()

    if skimmingTools is None:
        skimmingTools = []
    
    triggers = [
            'HLT_j150', 'HLT_j200', 'HLT_j260', 'HLT_j300', 'HLT_j320', 'HLT_j360', 'HLT_j380', 'HLT_j400', 'HLT_j420'
            ]

    FTAG3TrigSkimmingTool = CompFactory.DerivationFramework.TriggerSkimmingTool( name                   = "FTAG3TrigSkimmingTool1",
            TriggerListOR          = triggers )
    acc.addPublicTool(FTAG3TrigSkimmingTool)
    skimmingTools += [
            FTAG3TrigSkimmingTool
            ]
    return(acc)


