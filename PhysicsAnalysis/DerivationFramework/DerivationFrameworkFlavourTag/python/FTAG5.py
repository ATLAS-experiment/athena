# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#====================================================================
# DAOD_FTAG5.py
# This defines DAOD_FTAG5, an unskimmed DAOD format for Run 3.
# It is designed to do free data derivations for calibrations.
# It requires the flag FTAG5 in Derivation_tf.py   
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

#from AthenaCommon.Logging import logging
#logFTAG5 = logging.getLogger('FTAG5')

# Main algorithm config
def FTAG5KernelCfg(flags, name='FTAG5Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for FTAG5"""
    acc = ComponentAccumulator()
    

    from DerivationFrameworkPhys.PHYS import PHYSKernelCfg
    acc.merge(PHYSKernelCfg(flags, name, StreamName = kwargs['StreamName'], TriggerListsHelper = kwargs['TriggerListsHelper']))
    
    # augmentation tools
    augmentationTools = []

    # skimming tools
    skimmingTools = []
    # filter leptons
    lepton_skimming_expression = 'count( (Muons.pt > 18*GeV) && (0 == Muons.muonType || 1 == Muons.muonType || 4 == Muons.muonType) ) + count(( Electrons.pt > 18*GeV) && ((Electrons.Loose) || (Electrons.DFCommonElectronsLHLoose))) >= 2 && count( (Muons.pt > 25*GeV) && (0 == Muons.muonType || 1 == Muons.muonType || 4 == Muons.muonType) ) + count(( Electrons.pt > 25*GeV) && ((Electrons.Loose) || (Electrons.DFCommonElectronsLHLoose))) >= 1'

    FTAG5LeptonSkimmingTool = CompFactory.DerivationFramework.xAODStringSkimmingTool(
            name = "FTAG5LeptonSkimmingTool",
            expression = lepton_skimming_expression )
    acc.addPublicTool(FTAG5LeptonSkimmingTool)

    # Finally the kernel itself
    thinningTools = []
    skimmingTools = [FTAG5LeptonSkimmingTool]

    # Finally the kernel itself
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name, AugmentationTools = augmentationTools, ThinningTools = thinningTools, SkimmingTools = skimmingTools))
    return acc


def FTAG5Cfg(flags):
    acc = ComponentAccumulator()
    
    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    #from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    #FTAG5TriggerListsHelper = TriggerListsHelper(flags)

    # the name_tag has to consistent between KernelCfg and CoreCfg
    FTAG5_name_tag = 'FTAG5'

    
    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    FTAG5TriggerListsHelper = TriggerListsHelper(flags)
    acc.merge(FTAG5KernelCfg(flags, name="FTAG5Kernel", StreamName = 'StreamDAOD_FTAG5', TriggerListsHelper = FTAG5TriggerListsHelper))


    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    from DerivationFrameworkFlavourTag import FtagBaseContent


    FTAG5SlimmingHelper = SlimmingHelper("FTAG5SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
     

    # Add truth containers
    if flags.Input.isMC:
        FtagBaseContent.add_truth_to_SlimmingHelper(FTAG5SlimmingHelper)
        if flags.Trigger.EDMVersion == 3:
            # Add truth labels to Run 3 trigger jets
            from DerivationFrameworkFlavourTag.FtagDerivationConfig import HLTJetFTagDecorationCfg
            acc.merge(HLTJetFTagDecorationCfg(flags))

    # Common augmentations

    acc.merge(FTAG5KernelCfg(flags,
        name= FTAG5_name_tag + "Kernel", 
        StreamName = 'StreamDAOD_'+FTAG5_name_tag,
        TriggerListsHelper = FTAG5TriggerListsHelper
        ))
    
    # PHYS content
    from DerivationFrameworkPhys.PHYS import PHYSCoreCfg
    acc.merge(PHYSCoreCfg(flags, 
        FTAG5_name_tag,
        StreamName = 'StreamDAOD_'+FTAG5_name_tag,
        TriggerListsHelper = FTAG5TriggerListsHelper
        ))

   
    # Trigger content
    FtagBaseContent.trigger_setup(FTAG5SlimmingHelper, 'FTAG5')
    FtagBaseContent.trigger_matching(FTAG5SlimmingHelper, FTAG5TriggerListsHelper, flags)

    # Output stream
    FTAG5ItemList = FTAG5SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_FTAG5", ItemList=FTAG5ItemList, AcceptAlgs=["FTAG5Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_FTAG5", AcceptAlgs=["FTAG5Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData, MetadataCategory.TruthMetaData]))

    return acc


