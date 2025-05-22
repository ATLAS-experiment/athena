# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#====================================================================
# DAOD_FTAG_XBB.py
# This defines DAOD_FTAG_XBB, an skimmed DAOD format for Run 3.
# It is designed for the X->bb calibration.
# It requires the flag FTAG_XBB in Derivation_tf.py   
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


# Main algorithm config
def FTAG_XBBKernelCfg(flags, name='FTAG_XBBKernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for FTAG_XBB"""
    acc = ComponentAccumulator()
    
    from DerivationFrameworkPhys.PHYS import PHYSKernelCfg
    acc.merge(PHYSKernelCfg(flags, name, StreamName = kwargs['StreamName'], TriggerListsHelper = kwargs['TriggerListsHelper']))

    # augmentation tools
    augmentationTools = []

    # skimming tools
    skimmingTools = []

    # filter leptons
    lepton_skimming_expression = 'count( (Muons.pt > 5*GeV) && (0 == Muons.muonType || 1 == Muons.muonType || 4 == Muons.muonType) ) + count(( Electrons.pt > 5*GeV) && ((Electrons.Loose) || (Electrons.DFCommonElectronsLHLoose))) >= 1'
    FTAG_XBBLeptonSkimmingTool = CompFactory.DerivationFramework.xAODStringSkimmingTool(
            name = "FTAG_XBBLeptonSkimmingTool",
            expression = lepton_skimming_expression )
    acc.addPublicTool(FTAG_XBBLeptonSkimmingTool)

    # filter large-R jets
    UFOjets_skimming_expression = 'count( AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets.pt > 150*GeV ) >= 1' 
    FTAG_XBBUFOjetsSkimmingTool = CompFactory.DerivationFramework.xAODStringSkimmingTool(
            name = "FTAG_XBBUFOjetsSkimmingTool",
            expression = UFOjets_skimming_expression )
    acc.addPublicTool(FTAG_XBBUFOjetsSkimmingTool)

    # Trigger skimming
    acc.merge(FTAG_XBBTriggerSkimmingToolCfg(flags, skimmingTools))

    # thinning tools
    thinningTools = []

    skimmingTools += [
        FTAG_XBBUFOjetsSkimmingTool,
        FTAG_XBBLeptonSkimmingTool,
        ]

    thinningTools = [
            ]

    # Finally the kernel itself
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name, AugmentationTools = augmentationTools, ThinningTools = thinningTools, SkimmingTools = skimmingTools))
    return acc


def FTAG_XBBCfg(flags, skimmingTools=None):
    acc = ComponentAccumulator()
    
    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    FTAG_XBBTriggerListsHelper = TriggerListsHelper(flags)

    # the name_tag has to consistent between KernelCfg and CoreCfg
    FTAG_XBB_name_tag = 'FTAG_XBB'

    # Common augmentations
    acc.merge(FTAG_XBBKernelCfg(flags, 
                                name= FTAG_XBB_name_tag + "Kernel", 
                                StreamName = 'StreamDAOD_'+FTAG_XBB_name_tag, 
                                TriggerListsHelper = FTAG_XBBTriggerListsHelper))

    # PHYS content
    from DerivationFrameworkPhys.PHYS import PHYSCoreCfg
    acc.merge(PHYSCoreCfg(flags, 
        FTAG_XBB_name_tag,
        StreamName = 'StreamDAOD_'+FTAG_XBB_name_tag,
        TriggerListsHelper = FTAG_XBBTriggerListsHelper,
        ))

    return acc

def FTAG_XBBTriggerSkimmingToolCfg(flags, skimmingTools=None):
    """configure the trigger skimming tool"""
    acc = ComponentAccumulator()

    if skimmingTools is None:
        skimmingTools = []

    # List of triggers to be used for skimming
    photon_run2 = ['HLT_g120_loose','HLT_g140_loose']
    photon_run3 = ['HLT_g140_loose_L1EM22VHI','HLT_g140_loose_L1eEM26M']
    large_r_jet_run2 = ['HLT_j360_a10_lcw_sub_L1J100', 'HLT_j420_a10_lcw_L1J100', 'HLT_j420_a10r_L1J100',
                        'HLT_j420_a10t_lcw_jes_40smcINF_L1J100','HLT_j390_a10t_lcw_jes_30smcINF_L1J100',
                        'HLT_j460_a10t_lcw_jes_L1J100','HLT_j420_a10t_lcw_jes_35smcINF_L1J100',
                        'HLT_j420_a10t_lcw_jes_35smcINF_L1SC111','HLT_j460_a10r_L1SC111','HLT_j460_a10r_L1J100',
                        'HLT_j460_a10_lcw_subjes_L1SC111','HLT_j460_a10_lcw_subjes_L1J100',
                        'HLT_j460_a10t_lcw_jes_L1SC111']
    large_r_jet_run3 = ["HLT_j460_a10sd_cssk_pf_jes_ftf_preselj225_L1J100","HLT_j460_a10sd_cssk_pf_jes_ftf_preselj225_L1SC111-CJ15",
                        "HLT_j420_35smcINF_a10sd_cssk_pf_jes_ftf_preselj225_L1J100", "HLT_j420_35smcINF_a10sd_cssk_pf_jes_ftf_preselj225_L1SC111-CJ15"]

    lepton_run2 = ["HLT_e24_lhmedium_L1EM20VH", "HLT_e60_lhmedium", "HLT_e120_lhloose", "HLT_mu20_iloose_L1MU15",
                   "HLT_mu50", "HLT_e26_lhtight_nod0_ivarloose", "HLT_e60_lhmedium_nod0", "HLT_e140_lhloose_nod0",
                   "HLT_mu26_ivarmedium"]
    lepton_run3 = ["HLT_e26_lhtight_ivarloose_L1EM22VHI","HLT_e60_lhmedium_L1EM22VHI","HLT_e140_lhloose_L1EM22VHI",
                   "HLT_mu24_ivarmedium_L1MU14FCH","HLT_mu50_L1MU14FCH", "HLT_e26_lhtight_ivarloose_L1eEM26M",
                   "HLT_e60_lhmedium_L1eEM26M", "HLT_e140_lhloose_L1eEM26M"]

    triggers = photon_run2 + photon_run3 + large_r_jet_run2 + large_r_jet_run3 + lepton_run2 + lepton_run3

    FTAG_XBBTrigSkimmingTool = CompFactory.DerivationFramework.TriggerSkimmingTool( name                   = "FTAG_XBBTrigSkimmingTool1",
            TriggerListOR          = triggers )
    acc.addPublicTool(FTAG_XBBTrigSkimmingTool)
    skimmingTools += [
            FTAG_XBBTrigSkimmingTool
            ]
    return(acc)