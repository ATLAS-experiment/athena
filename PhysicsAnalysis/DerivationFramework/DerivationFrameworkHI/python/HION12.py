# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# HION12.py  

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory
from AthenaCommon.CFElements import seqAND

#########################################################################################
#Skiming
def HION12SkimmingToolCfg(flags):
    """Configure the example skimming tool"""
    from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
    acc = ComponentAccumulator()
    
    ExtraData  = []
    ExtraData += ['xAOD::VertexContainer/PrimaryVertices']
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
    nJetCuts    = ListTriggers.HION12nJetCuts2018()
    MB_triggers = ListTriggers.HION12MBtriggers2018()
    triggers    = ListTriggers.HION12triggers2018()
    
    expression = '( (' + ' || '.join(triggers+MB_triggers) + ') && '+objectSelection+ ' && ' + '(' + ' || '.join(nJetCuts) + ')' + ')'
    
    tdt = acc.getPrimaryAndMerge(TrigDecisionToolCfg(flags))
    acc.addPublicTool(CompFactory.DerivationFramework.xAODStringSkimmingTool(name       = "HION12StringSkimmingTool",
                                                                             expression = expression,
                                                                             TrigDecisionTool=tdt), 
                                                                             primary = True) 
    return(acc)                    


def HION12KernelCfg(flags, name='HION12Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel)"""
    acc = ComponentAccumulator()
    
#########################################################################################
#Thinning
    from DerivationFrameworkInDet.InDetToolsConfig import TrackParticleThinningCfg
    
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
    
    HION12SlimmingHelper = SlimmingHelper("HION12SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    
    HION12SlimmingHelper.SmartCollections = ListSlimming.HION12SmartCollections()
    HION12SlimmingHelper.AllVariables     = ListSlimming.HION12AllVarContent()
    HION12SlimmingHelper.ExtraVariables   = ListSlimming.HION12Extra()
    
    
    HION12ItemList = HION12SlimmingHelper.GetItemList()

    acc.merge(OutputStreamCfg(flags, "DAOD_HION12", ItemList=HION12ItemList, AcceptAlgs=["HION12Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_HION12", AcceptAlgs=["HION12Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc
