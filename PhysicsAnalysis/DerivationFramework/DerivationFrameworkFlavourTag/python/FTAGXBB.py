# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#====================================================================
# DAOD_FTAGXBB.py
# This defines DAOD_FTAGXBB, an skimmed DAOD format for Run 3.
# It is designed for the X->bb calibration.
# It requires the flag FTAGXBB in Derivation_tf.py   
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


# Main algorithm config
def FTAGXBBKernelCfg(flags, name='FTAGXBBKernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for FTAGXBB"""
    acc = ComponentAccumulator()
    
    from DerivationFrameworkPhys.PHYS import PHYSKernelCfg
    acc.merge(PHYSKernelCfg(flags, name, StreamName = kwargs['StreamName'], TriggerListsHelper = kwargs['TriggerListsHelper']))

    # augmentation tools
    augmentationTools = []

    # skimming tools
    skimmingTools = []

    # filter large-R jets
    UFOjets_skimming_expression = 'count( AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets.pt > 150*GeV ) >= 1' 
    FTAGXBBUFOjetsSkimmingTool = CompFactory.DerivationFramework.xAODStringSkimmingTool(
            name = "FTAGXBBUFOjetsSkimmingTool",
            expression = UFOjets_skimming_expression )
    acc.addPublicTool(FTAGXBBUFOjetsSkimmingTool)

    # Trigger skimming
    acc.merge(FTAGXBBTriggerSkimmingToolCfg(flags, skimmingTools))

    # thinning tools
    thinningTools = []

    skimmingTools += [
        FTAGXBBUFOjetsSkimmingTool,
        ]

    thinningTools = [
            ]

    # Finally the kernel itself
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name, AugmentationTools = augmentationTools, ThinningTools = thinningTools, SkimmingTools = skimmingTools))
    return acc


def FTAGXBBCfg(flags, skimmingTools=None):
    acc = ComponentAccumulator()
    
    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    FTAGXBBTriggerListsHelper = TriggerListsHelper(flags)

    # the name_tag has to consistent between KernelCfg and CoreCfg
    FTAGXBB_name_tag = 'FTAGXBB'

    # Common augmentations
    acc.merge(FTAGXBBKernelCfg(flags, 
                                name= FTAGXBB_name_tag + "Kernel", 
                                StreamName = 'StreamDAOD_'+FTAGXBB_name_tag, 
                                TriggerListsHelper = FTAGXBBTriggerListsHelper))

    gn3x_extra_variables = ["AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets.GN3XV00_phtautauhad.GN3XV00_phbb.GN3XV00_phcc.GN3XV00_ptop.GN3XV00_pqcdbb.GN3XV00_pqcdbx.GN3XV00_pqcdcx.GN3XV00_pqcdll.GN3XV00_pWqq"]
    
    # PHYS content
    from DerivationFrameworkPhys.PHYS import PHYSCoreCfg
    acc.merge(PHYSCoreCfg(flags, 
        FTAGXBB_name_tag,
        StreamName = 'StreamDAOD_'+FTAGXBB_name_tag,
        TriggerListsHelper = FTAGXBBTriggerListsHelper,
        addExtraVariables = gn3x_extra_variables
        ))

    return acc

def FTAGXBBTriggerSkimmingToolCfg(flags, skimmingTools=None):
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
    large_r_jet_run3 = ["HLT_j420_35smcINF_a10sd_cssk_pf_jes_ftf_preselj225_L1jJ160", # needed for > 2024 period K
                        "HLT_j460_a10sd_cssk_pf_jes_ftf_preselj225_L1J100","HLT_j460_a10sd_cssk_pf_jes_ftf_preselj225_L1SC111-CJ15",
                        "HLT_j420_35smcINF_a10sd_cssk_pf_jes_ftf_preselj225_L1J100", "HLT_j420_35smcINF_a10sd_cssk_pf_jes_ftf_preselj225_L1SC111-CJ15"]

    lepton_run2 = ["HLT_e24_lhmedium_L1EM20VH", "HLT_e60_lhmedium", "HLT_e120_lhloose", "HLT_mu20_iloose_L1MU15",
                   "HLT_mu50", "HLT_e26_lhtight_nod0_ivarloose", "HLT_e60_lhmedium_nod0", "HLT_e140_lhloose_nod0",
                   "HLT_mu26_ivarmedium"]
    lepton_run3 = ["HLT_e26_lhtight_ivarloose_L1EM22VHI","HLT_e60_lhmedium_L1EM22VHI","HLT_e140_lhloose_L1EM22VHI",
                   "HLT_mu24_ivarmedium_L1MU14FCH","HLT_mu50_L1MU14FCH", "HLT_e26_lhtight_ivarloose_L1eEM26M",
                   "HLT_e60_lhmedium_L1eEM26M", "HLT_e140_lhloose_L1eEM26M"]

    triggers = photon_run2 + photon_run3 + large_r_jet_run2 + large_r_jet_run3 + lepton_run2 + lepton_run3

    FTAGXBBTrigSkimmingTool = CompFactory.DerivationFramework.TriggerSkimmingTool( name                   = "FTAGXBBTrigSkimmingTool1",
            TriggerListOR          = triggers )
    acc.addPublicTool(FTAGXBBTrigSkimmingTool)
    skimmingTools += [
            FTAGXBBTrigSkimmingTool
            ]
    return(acc)