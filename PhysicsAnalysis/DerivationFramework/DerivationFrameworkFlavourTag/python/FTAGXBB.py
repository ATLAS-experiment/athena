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
    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import (
        xAODStringSkimmingToolCfg)
    FTAGXBBUFOjetsSkimmingTool = acc.getPrimaryAndMerge(xAODStringSkimmingToolCfg(
        flags, name = "FTAGXBBUFOjetsSkimmingTool",
        expression = UFOjets_skimming_expression))

    # thinning tools
    thinningTools = []

    skimmingTools += [FTAGXBBUFOjetsSkimmingTool]

    thinningTools = []

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
