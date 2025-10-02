# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#====================================================================
# DAOD_FTAG4.py
# This defines DAOD_FTAG4, an unskimmed DAOD format for Run 3.
# It is designed to do free data derivations for calibrations.
# It requires the flag FTAG4 in Derivation_tf.py   
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
#from AthenaCommon.Logging import logging
#logFTAG4 = logging.getLogger('FTAG4')

# Main algorithm config
def FTAG4KernelCfg(flags, name='FTAG4Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for FTAG4"""
    acc = ComponentAccumulator()
    

    from DerivationFrameworkPhys.PHYS import PHYSKernelCfg
    acc.merge(PHYSKernelCfg(flags, name, StreamName = kwargs['StreamName'], TriggerListsHelper = kwargs['TriggerListsHelper']))

    # augmentation tools
    augmentationTools = []

    # skimming tools
    skimmingTools = []
    # filter leptons
    lepton_skimming_expression = 'count( (Muons.pt > 25*GeV) && (0 == Muons.muonType || 1 == Muons.muonType || 4 == Muons.muonType) ) + count(( Electrons.pt > 25*GeV) && ((Electrons.Loose) || (Electrons.DFCommonElectronsLHLoose))) >= 1'

    FTAG4LeptonSkimmingTool = CompFactory.DerivationFramework.xAODStringSkimmingTool(
            name = "FTAG4LeptonSkimmingTool",
            expression = lepton_skimming_expression )
    acc.addPublicTool(FTAG4LeptonSkimmingTool)
    
    # thinning tools
    thinningTools = []

    skimmingTools += [
            FTAG4LeptonSkimmingTool,
            ]

    thinningTools = [
            ]

    # Finally the kernel itself
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name, AugmentationTools = augmentationTools, ThinningTools = thinningTools, SkimmingTools = skimmingTools))
    return acc


def FTAG4Cfg(flags):
    acc = ComponentAccumulator()
    
    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    #from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    #FTAG4TriggerListsHelper = TriggerListsHelper(flags)

    # the name_tag has to consistent between KernelCfg and CoreCfg
    FTAG4_name_tag = 'FTAG4'

    
    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    PHYSTriggerListsHelper = TriggerListsHelper(flags)

    # Common augmentations
    acc.merge(FTAG4KernelCfg(flags,
        name= FTAG4_name_tag + "Kernel", 
        StreamName = 'StreamDAOD_'+FTAG4_name_tag,
        TriggerListsHelper = PHYSTriggerListsHelper,
        ))
    
    # PHYS content
    from DerivationFrameworkPhys.PHYS import PHYSCoreCfg
    acc.merge(PHYSCoreCfg(flags, 
        FTAG4_name_tag,
        StreamName = 'StreamDAOD_'+FTAG4_name_tag,
        TriggerListsHelper = PHYSTriggerListsHelper,
        ))

    return acc


