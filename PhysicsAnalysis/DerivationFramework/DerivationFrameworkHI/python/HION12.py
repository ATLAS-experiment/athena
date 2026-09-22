# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
# HION12.py  

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory
from AthenaCommon.CFElements import seqAND

#########################################################################################
#Skiming
def HION12SkimmingToolCfg(flags):
    """Configure the example skimming tool"""
    acc = ComponentAccumulator()
    
    ExtraData  = []
    ExtraData += ['xAOD::VertexContainer/PrimaryVertices']
    if (flags.Input.ProjectName == "data15_hi" or flags.Input.ProjectName == "data18_hi"):
        ExtraData += ['xAOD::JetContainer/AntiKt4EMTopoJets']
    ExtraData += ['xAOD::JetContainer/AntiKt4LCTopoJets']
    ExtraData += ['xAOD::JetContainer/AntiKt4EMPFlowJets']
    ExtraData += ['xAOD::JetContainer/AntiKt4HIJets']
    ExtraData += ['xAOD::JetContainer/AntiKt4HITrackJets']
    ExtraData += ['xAOD::JetContainer/AntiKt10LCTopoJets']
    
    acc.addSequence( seqAND("HION12Sequence") )
    acc.getSequence("HION12Sequence").ExtraDataForDynamicConsumers = ExtraData
    acc.getSequence("HION12Sequence").ProcessDynamicDataDependencies = True
    
    #Building jet skimming triggers
    from DerivationFrameworkHI import ListTriggers
    
    objectSelection = '(count(PrimaryVertices.z < 1000) < 10)'
    nJetCuts    = ListTriggers.GetHION12nJetCuts(flags.Input.ProjectName, flags.Input.isMC)
    triggers    = ListTriggers.GetHION12Triggers(flags.Input.ProjectName)
    filterList = []

    expression = '('+objectSelection+ ' && ' + '(' + ' || '.join(nJetCuts) + ')' + ')'
    
    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import (
        xAODStringSkimmingToolCfg)
    HION12StringSkimmingTool = acc.addPublicTool(acc.getPrimaryAndMerge(
        xAODStringSkimmingToolCfg(flags, name = "HION12StringSkimmingTool",
                                  expression = expression)))
    filterList += [HION12StringSkimmingTool]

    if not flags.Input.isMC:
        HION12TriggerSkimmingTool = CompFactory.DerivationFramework.TriggerSkimmingTool(
            name = "HION12TriggerSkimmingTool", TriggerListOR = triggers)
        acc.addPublicTool(HION12TriggerSkimmingTool)
        filterList += [HION12TriggerSkimmingTool]

    HION12SkimmingTool  = CompFactory.DerivationFramework.FilterCombinationAND(
        name="HION12SkimmingTool",  FilterList=filterList)
    acc.addPublicTool(HION12SkimmingTool, primary = True)

    return(acc)


def HION12KernelCfg(flags, name='HION12Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel)"""
    acc = ComponentAccumulator()
    from DerivationFrameworkHI.HION7 import (
        PhysAugmentationsHION7Cfg)
    acc.merge(PhysAugmentationsHION7Cfg(flags))

    if flags.HeavyIon.doHIBTagging:
        #Rebuild jets
        from DerivationFrameworkJetEtMiss.JetCommonConfig import JetCommonCfg
        acc.merge(JetCommonCfg(flags))
        from BTagging.FlavorTaggingConfig import FlavorTaggingCfg
        acc.merge(FlavorTaggingCfg(flags, "AntiKt4EMPFlowJets"))
        from BTagging.TrackLeptonConfig import TrackLeptonDecorationCfg
        acc.merge(TrackLeptonDecorationCfg(flags))

    
#########################################################################################
#Thinning
    from DerivationFrameworkInDet.InDetToolsConfig import TrackParticleThinningCfg
    
    thinningTools = []
    # track thinning for data only, all tracks in MC used for training
    if not flags.Input.isMC:
        expression = "abs(InDetTrackParticles.d0)< 1000000000 && abs(InDetTrackParticles.z0*sin(InDetTrackParticles.theta)) < 1000000000 && InDetTrackParticles.pt > 200" #check limits
        HION12TrackThinningTool = acc.getPrimaryAndMerge(TrackParticleThinningCfg(
        flags,
        name                    = "HION12TrackThinningTool",
        StreamName              = kwargs['StreamName'],
        SelectionString         = expression,
        InDetTrackParticlesKey  = "InDetTrackParticles"))

        thinningTools = [HION12TrackThinningTool]

    skimmingTool = acc.getPrimaryAndMerge(HION12SkimmingToolCfg(flags))
    
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name,
                                      SkimmingTools = [skimmingTool],
                                      ThinningTools = thinningTools),
                                      sequenceName = "HION12Sequence")
      
    return acc

def HION12Cfg(flags):
    
    acc = ComponentAccumulator()
    acc.merge(HION12KernelCfg(flags, name="HION12Kernel",StreamName = "StreamDAOD_HION12"))

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
#########################################################################################
#Slimming
    from DerivationFrameworkHI import ListSlimming
    from DerivationFrameworkFlavourTag import FtagBaseContent
    
    HION12SlimmingHelper = SlimmingHelper("HION12SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    HION12SlimmingHelper.SmartCollections = ListSlimming.HION12SmartCollections()
    AllVars = []
    AllVars += ListSlimming.HION12AllVarContent(flags.Input.ProjectName,flags.Input.isMC)
    ExtraVars = []
    ExtraVars += ListSlimming.HION12Extra()
    if flags.Input.isMC and flags.HeavyIon.doHIBTagging:
        FtagBaseContent.add_truth_to_slimming_helper(HION12SlimmingHelper)
    if flags.HeavyIon.doHIBTagging:
        from DerivationFrameworkFlavourTag.FtagBaseContent import add_common_augmentation
        add_common_augmentation(flags, acc, HION12SlimmingHelper, "AntiKt4EMPFlowJets")
        AllVars += ListSlimming.HION7AllVarFromFTAG1()
        # update AppendToDictionary
        extra_AppendToDictionary = {}
        FtagBaseContent.update_append_to_dictionary_in_slimming_helper(flags, HION12SlimmingHelper, extra_AppendToDictionary)
        # Add ExtraVariables
        ExtraVars += ListSlimming.HION7ExtraVarForBtag("AntiKt4EMPFlowJets")
        FtagBaseContent.add_extra_variables_to_slimming_helper(flags, HION12SlimmingHelper)
    HION12SlimmingHelper.AllVariables     = AllVars
    HION12SlimmingHelper.ExtraVariables   = ExtraVars
    
    
    HION12ItemList = HION12SlimmingHelper.GetItemList()

    acc.merge(OutputStreamCfg(flags, "DAOD_HION12", ItemList=HION12ItemList, AcceptAlgs=["HION12Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_HION12", AcceptAlgs=["HION12Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc
