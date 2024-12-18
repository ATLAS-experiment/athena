# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
# HION5.py  

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory
from AthenaCommon.CFElements import seqAND

def HION5SkimmingToolCfg(flags):
    """Configure the example skimming tool"""
    acc = ComponentAccumulator()
    
    ExtraData  = []
    ExtraData += ['xAOD::MuonContainer/Muons']
    ExtraData += ['xAOD::ElectronContainer/Electrons']
    ExtraData += ['xAOD::PhotonContainer/Photons']
    ExtraData += ['xAOD::TrackParticleContainer/InDetTrackParticles']
    ExtraData += ['xAOD::TrackParticleContainer/InDetTrackPaasdasdrticles']
    
    acc.addSequence( seqAND("HION5Sequence") )
    acc.getSequence("HION5Sequence").ExtraDataForDynamicConsumers = ExtraData
    acc.getSequence("HION5Sequence").ProcessDynamicDataDependencies = True
    
    from DerivationFrameworkHI import ListTriggers
    
    triggers = ListTriggers.HION5SkimmingTriggers()
    
    req_electrons = 'count( Electrons.DFCommonElectronsLHLoose && ( Electrons.pt > 15*GeV ))>0'
    req_muons     = 'count( Muons.DFCommonMuonPassPreselection && (Muons.pt > 15*GeV) && ( abs(Muons.eta) < 2.7))>0'
    req_photons = 'count( Photons.DFCommonPhotonsIsEMLoose && (Photons.pt > 30*GeV) ) > 0'
    req_total = '(' + req_electrons + ' || ' + req_muons + ' || ' + req_photons + ')'

    expression = ' ( ' +' || '.join(triggers) + ' )  && ' + req_total
    
    
    from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg    
    tdt = acc.getPrimaryAndMerge(TrigDecisionToolCfg(flags))
    acc.addPublicTool(CompFactory.DerivationFramework.xAODStringSkimmingTool(name       = "HION5StringSkimmingTool",
                                                                             expression = expression,
                                                                             TrigDecisionTool=tdt),
                      primary = True)
    
    return acc

def HION5Thinning(flags):    
    from DerivationFrameworkInDet.InDetToolsConfig import TrackParticleThinningCfg,JetTrackParticleThinningCfg
    acc = ComponentAccumulator()

    track_thinning_expression  = "InDetTrackParticles.pt > 0.9*GeV"
    TrackParticleThinningTool  = acc.getPrimaryAndMerge(TrackParticleThinningCfg(
         flags,
         name                    = "PHYSTrackParticleThinningTool",
         StreamName              = "streamDAOD_HION5", 
         SelectionString         = track_thinning_expression,
         InDetTrackParticlesKey  = "InDetTrackParticles"))

    AntiKt2HIJetsThinningTool  = acc.getPrimaryAndMerge(JetTrackParticleThinningCfg(
         flags,
         name                    = "AntiKt2HIJetsThinningTool",
         StreamName              = "streamDAOD_HION5",
         JetKey                  = "AntiKt2HIJets",
         SelectionString         = "AntiKt2HIJets.pt > 15*GeV",
         InDetTrackParticlesKey  = "InDetTrackParticles"))
    
    AntiKt4HIJetsThinningTool  = acc.getPrimaryAndMerge(JetTrackParticleThinningCfg(
         flags,
         name                    = "AntiKt4HIJetsThinningTool",
         StreamName              = "streamDAOD_HION5",
         JetKey                  = "AntiKt4HIJets",
         SelectionString         = "AntiKt4HIJets.pt > 15*GeV",
         InDetTrackParticlesKey  = "InDetTrackParticles"))
    
    acc.addPublicTool(TrackParticleThinningTool,primary = True)
    acc.addPublicTool(AntiKt2HIJetsThinningTool)
    acc.addPublicTool(AntiKt4HIJetsThinningTool)
    
    return acc

def HION5KernelCfg(flags, name="HION5Kernel", **kwargs):
    """Configure the derivation framework driving algorithm (kernel)
    for HION5"""
    acc = ComponentAccumulator()

    # Common augmentations which include DFCommon
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg

    acc.merge(PhysCommonAugmentationsCfg(flags, TriggerListsHelper=kwargs["TriggerListsHelper"]))

    # skimming
    skimmingTool = acc.getPrimaryAndMerge(HION5SkimmingToolCfg(flags))
    
    # Thinning
    thinningTool= acc.getPrimaryAndMerge(HION5Thinning(flags))

    # setup the kernel
    acc.addEventAlgo(
        CompFactory.DerivationFramework.DerivationKernel(
            name,
            SkimmingTools = [skimmingTool],
            ThinningTools = [thinningTool],
            AugmentationTools = [],
            ))

    return acc

def HION5Cfg(flags):
    acc = ComponentAccumulator()

    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    HION5TriggerListsHelper = TriggerListsHelper(flags)
        
    acc.merge(HION5KernelCfg(flags, name="HION5Kernel", StreamName="StreamDAOD_HION5", TriggerListsHelper = HION5TriggerListsHelper,))
    
    # configure slimming
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    from DerivationFrameworkHI import ListSlimming
    
    HION5SlimmingHelper = SlimmingHelper("HION5SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    
    HION5SlimmingHelper.SmartCollections = ListSlimming.HION5SmartCollections()
    HION5SlimmingHelper.AllVariables     = ListSlimming.HION5AllVariables()
    HION5SlimmingHelper.ExtraVariables   = ListSlimming.HION5Extravariables()
    
    HION5ItemList = HION5SlimmingHelper.GetItemList()

    acc.merge(OutputStreamCfg(flags, "DAOD_HION5", ItemList=HION5ItemList, AcceptAlgs=["HION5Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_HION5", AcceptAlgs=["HION5Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc
