# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# HION2.py  

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory
from AthenaCommon.CFElements import seqAND
from InDetConfig.InDetTrackSelectionToolConfig import InDetTrackSelectionToolCfg

#########################################################################################
#Skiming
def HION2SkimmingToolCfg(flags):
    """Configure the example skimming tool"""
    from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
    acc = ComponentAccumulator()
    
    ExtraData  = []
    ExtraData += ['xAOD::VertexContainer/PrimaryVertices']

    acc.addSequence( seqAND("HION2Sequence") )
    acc.getSequence("HION2Sequence").ExtraDataForDynamicConsumers = ExtraData
    acc.getSequence("HION2Sequence").ProcessDynamicDataDependencies = True
    
    #Building jet skimming triggers
    from DerivationFrameworkHI import ListTriggers
    
    triggers  = []
    triggers += ListTriggers.HION2MinBias2023()
    
    expression = ' ( ' +' || '.join(triggers) + ' ) && (count(abs(PrimaryVertices.z)<100)>1)'
    
    tdt = acc.getPrimaryAndMerge(TrigDecisionToolCfg(flags))
    acc.addPublicTool(CompFactory.DerivationFramework.xAODStringSkimmingTool(name       = "HION2StringSkimmingTool",
                                                                             expression = expression,
                                                                             TrigDecisionTool=tdt), 
                                                                             primary = True) 
    return(acc)                    
#########################################################################################
#Creating InDetTrackTools

def InDetTrackSelectionTool_HITight_Cfg(flags, name="InDetTrackSelectionTool_HITight", **kwargs):
    kwargs.setdefault("CutLevel", "HITight")
    return InDetTrackSelectionToolCfg(flags, name, **kwargs)    

#########################################################################################
def HION2AugmentationToolCfg(flags):
    """Configure the example augmentation tool"""
    acc = ComponentAccumulator()
    
    from InDetConfig.InDetTrackSelectionToolConfig import InDetTrackSelectionTool_HILoose_Cfg
    
    TrkSelTool_hi_loose = acc.popToolsAndMerge(InDetTrackSelectionTool_HILoose_Cfg(flags,
                                                            name = "TrackSelectionTool_hi_loose",
                                                            minPt = 100))
    
    TrkSelTool_hi_tight = acc.popToolsAndMerge(InDetTrackSelectionTool_HITight_Cfg(flags,
                                                            name = "TrackSelectionTool_hi_tight",
                                                            minPt = 100))
    acc.addPublicTool(TrkSelTool_hi_loose)
    acc.addPublicTool(TrkSelTool_hi_tight)
    
    acc.addPublicTool(CompFactory.DerivationFramework.HITrackQualityAugmentationTool(name= "HION2AugmentationTool",
                      TrackSelectionTool_pp = TrkSelTool_hi_loose,  #didnt find a tool for minbias
                      TrackSelectionTool_hi_loose =TrkSelTool_hi_loose,
                      TrackSelectionTool_hi_tight =TrkSelTool_hi_tight),
                      primary = True)
    return(acc)

def HION2KernelCfg(flags, name='HION2Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel)"""
    acc = ComponentAccumulator()

    skimmingTool = acc.getPrimaryAndMerge(HION2SkimmingToolCfg(flags))
    augmentationTool = acc.getPrimaryAndMerge(HION2AugmentationToolCfg(flags))
    
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name,
                                      SkimmingTools     = [skimmingTool],
                                      AugmentationTools = [augmentationTool]),
                                      sequenceName = "HION2Sequence")
      
    return acc

def HION2Cfg(flags):
    
    acc = ComponentAccumulator()
    acc.merge(HION2KernelCfg(flags, name="HION2Kernel",StreamName = "StreamDAOD_HION2"))

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
#########################################################################################
#Slimming
    from DerivationFrameworkHI import ListSlimming
    
    HION2SlimmingHelper = SlimmingHelper("HION2SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    
    AllVar  = []
    AllVar += ListSlimming.HION2AllVariablesGeneral()
    AllVar += ListSlimming.HION2AllVariablesPbPb_2015_5TeV()
    AllVar += ListSlimming.HION2AllVariablespPb_2016()
    
    ExtraVar  = []
    ExtraVar += ListSlimming.HION2ExtraVariablesGeneral()
    ExtraVar += ListSlimming.HION2ExtraVariablesPbPb_2015_5TeV()
    
    HION2SlimmingHelper.AllVariables     = AllVar
    HION2SlimmingHelper.ExtraVariables   = ExtraVar
    
    HION2ItemList = HION2SlimmingHelper.GetItemList()

    acc.merge(OutputStreamCfg(flags, "DAOD_HION2", ItemList=HION2ItemList, AcceptAlgs=["HION2Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_HION2", AcceptAlgs=["HION2Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc
