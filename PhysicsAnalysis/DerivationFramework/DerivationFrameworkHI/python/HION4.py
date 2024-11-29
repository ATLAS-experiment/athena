# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
# HION4.py  

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory
from AthenaCommon.CFElements import seqAND

#########################################################################################
#Skiming
def HION4SkimmingToolCfg(flags):
    """Configure the example skimming tool"""
    acc = ComponentAccumulator()
    
    ExtraData  = []
    ExtraData += ['xAOD::MuonContainer/Muons']
    ExtraData += ['xAOD::ElectronContainer/Electrons']
    ExtraData += ['xAOD::PhotonContainer/Photons']
    ExtraData += ['xAOD::TrackParticleContainer/InDetTrackParticles']
    
    acc.addSequence( seqAND("HION4Sequence") )
    acc.getSequence("HION4Sequence").ExtraDataForDynamicConsumers = ExtraData
    acc.getSequence("HION4Sequence").ProcessDynamicDataDependencies = True
                                     
    muonsRequirements = '(Muons.pt >= 2.0*GeV) && (abs(Muons.eta) < 2.6)'
    muonOnlySelection = 'count('+muonsRequirements+') >= 1'
   
    electronsRequirements = '(Electrons.pt > 1.5*GeV) && (abs(Electrons.eta) < 2.6)'
    electronOnlySelection = 'count('+electronsRequirements+') >= 1'
    
    photonsRequirements = '(Photons.pt >= 1.5*GeV)'
    photonOnlySelection = 'count('+photonsRequirements+') >=2'
    
    electronPhotonSelection = '(count('+electronsRequirements+') + count('+photonsRequirements+')) >= 2'

    trackRequirements = '(InDetTrackParticles.pt >= 0.2*GeV) && (abs(InDetTrackParticles.eta) < 2.5)'
    trackOnlySelection = '( count('+trackRequirements+') >= 2 && 5 >= count('+trackRequirements+') )' 

    tightTrackRequirements = '(InDetTrackParticles.pt >= 1*GeV) && (abs(InDetTrackParticles.eta) < 2.5)'
    tightTrackOnlySelection = '( count('+tightTrackRequirements+') == 2 )'

    objectSelection = '('+muonOnlySelection+' || '+electronOnlySelection+' || '+photonOnlySelection+' || '+electronPhotonSelection+' || '+trackOnlySelection+')'
    
    from DerivationFrameworkHI import ListTriggers
    VMtrigger=ListTriggers.HION4SkimmingTriggersVM()
    triggers=ListTriggers.HION4SkimmingTriggersALL()
        
    tdt = None
    if flags.Trigger.EDMVersion != -1: # Only for files with trigger payload
        from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
        tdt = acc.getPrimaryAndMerge(TrigDecisionToolCfg(flags))
        expression = '( (' + ' || '.join(triggers) + ') && '+objectSelection+') || ( '+ ' || '.join(VMtrigger)+ ' && '+tightTrackOnlySelection+')'
    else:
        expression = '( '+objectSelection+' ) || ( '+tightTrackOnlySelection+' )'


    acc.addPublicTool(CompFactory.DerivationFramework.xAODStringSkimmingTool(name       = "HION4StringSkimmingTool",
                                                                             expression = expression,
                                                                             TrigDecisionTool=tdt), 
                      primary = True)
    
    return(acc)

def HION4AugmentationToolCfg(flags):
    """Configure the example augmentation tool"""
    acc = ComponentAccumulator()
    acc.addPublicTool(CompFactory.DerivationFramework.AugmentationDeltaPoverP(name       = "HION4AugmentationTool"),
                      primary = True)
    return(acc)                          
  

def HION4KernelCfg(flags, name='HION4Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel)"""
    acc = ComponentAccumulator()

    skimmingTool = acc.getPrimaryAndMerge(HION4SkimmingToolCfg(flags))
    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel(name, SkimmingTools = [skimmingTool]),
                     sequenceName="HION4Sequence") 
    return acc


def HION4Cfg(flags):
    
    acc = ComponentAccumulator()
    acc.merge(HION4KernelCfg(flags, name="HION4Kernel",StreamName = "StreamDAOD_HION4"))

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
#########################################################################################
#Slimming
    from DerivationFrameworkHI import ListSlimming
    
    HION4SlimmingHelper = SlimmingHelper("HION4SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    
    AllVariables  = []    
    AllVariables += ListSlimming.HION4ExtraContainersElectrons()
    AllVariables += ListSlimming.HION4ExtraContainersTrigger()
    AllVariables += ListSlimming.HION4AllVariablesGeneral()
    
    HION4SlimmingHelper.SmartCollections = ListSlimming.HION4SmartCollections()
    HION4SlimmingHelper.ExtraVariables   = ListSlimming.HION4ExtraContentAll()
    HION4SlimmingHelper.AllVariables = AllVariables
    
    # Add egamma trigger objects
    HION4SlimmingHelper.IncludeEGammaTriggerContent = True
    # Add muon trigger objects
    HION4SlimmingHelper.IncludeMuonTriggerContent = True
    
    HION4ItemList = HION4SlimmingHelper.GetItemList()

    acc.merge(OutputStreamCfg(flags, "DAOD_HION4", ItemList=HION4ItemList, AcceptAlgs=["HION4Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_HION4", AcceptAlgs=["HION4Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc
