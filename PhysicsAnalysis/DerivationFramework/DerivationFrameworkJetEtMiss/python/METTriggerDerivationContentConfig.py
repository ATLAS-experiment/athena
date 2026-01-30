# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.SystemOfUnits import GeV

def LooseMETTriggerDerivationKernelCfg(ConfigFlags, name, StreamName, skimmingTools):

   acc = ComponentAccumulator()

   # augmentation tools
   acc.merge(AugmentationToolsCfg(ConfigFlags, StreamName = StreamName, trackSel="Loose"))
   # PFlow augmentation
   from DerivationFrameworkJetEtMiss.PFlowCommonConfig import PFlowCommonCfg
   acc.merge(PFlowCommonCfg(ConfigFlags))
   # derivation kernel (including thinning tools)
   acc.merge(METTriggerDerivationKernelCfg(ConfigFlags, name, StreamName=StreamName, skimmingTools=skimmingTools, trackThreshold=1*GeV, trackSel="Loose"))

   return acc


def TightMETTriggerDerivationKernelCfg(ConfigFlags, name, StreamName, skimmingTools):

   acc = ComponentAccumulator()

   # augmentation tools
   acc.merge(AugmentationToolsCfg(ConfigFlags, StreamName = StreamName, trackSel=None))
   # derivation kernel (including thinning tools)
   acc.merge(METTriggerDerivationKernelCfg(ConfigFlags, name, StreamName=StreamName, skimmingTools=skimmingTools, trackThreshold=10*GeV, trackSel=None))

   return acc


def METTriggerDerivationKernelCfg(ConfigFlags, name, **kwargs):
   
   acc = ComponentAccumulator()

   from DerivationFrameworkInDet.InDetToolsConfig import TrackParticleThinningCfg, MuonTrackParticleThinningCfg, EgammaTrackParticleThinningCfg, TauTrackParticleThinningCfg

   thinning_expression = "InDetTrackParticles.pt > {0}".format(kwargs['trackThreshold'])
   if kwargs['trackSel'] is not None:
      thinning_expression = "("+thinning_expression+" || (InDetTrackParticles.DFMETTrig"+kwargs['trackSel']+"))"

   trackParticleThinningTool = acc.getPrimaryAndMerge(TrackParticleThinningCfg(
      ConfigFlags,
      name                    = kwargs['StreamName']+"TrackParticleThinningTool",
      StreamName              = kwargs['StreamName'],
      SelectionString         = thinning_expression,
      InDetTrackParticlesKey  = "InDetTrackParticles"))

   muonTPThinningTool = acc.getPrimaryAndMerge(MuonTrackParticleThinningCfg(
      ConfigFlags,
      name                    = kwargs['StreamName']+"MuonTPThinningTool",
      StreamName              = kwargs['StreamName'],
      MuonKey                 = "Muons",
      InDetTrackParticlesKey  = "InDetTrackParticles"))

   electronTPThinningTool = acc.getPrimaryAndMerge(EgammaTrackParticleThinningCfg(
      ConfigFlags,
      name                    = kwargs['StreamName']+"ElectronTPThinningTool",
      StreamName              = kwargs['StreamName'],
      SGKey                   = "Electrons",
      InDetTrackParticlesKey  = "InDetTrackParticles"))

   photonTPThinningTool = acc.getPrimaryAndMerge(EgammaTrackParticleThinningCfg(
      ConfigFlags,
      name                     = kwargs['StreamName']+"PhotonTPThinningTool",
      StreamName               = kwargs['StreamName'],
      SGKey                    = "Photons",
      InDetTrackParticlesKey   = "InDetTrackParticles",
      GSFConversionVerticesKey = "GSFConversionVertices"))

   tauTPThinningTool = acc.getPrimaryAndMerge(TauTrackParticleThinningCfg(
      ConfigFlags,
      name                   = kwargs['StreamName']+"TauTPThinningTool",
      StreamName             = kwargs['StreamName'],
      TauKey                 = "TauJets",
      InDetTrackParticlesKey = "InDetTrackParticles",
      DoTauTracksThinning    = True,
      TauTracksKey           = "TauTracks"))

   thinningTools = [trackParticleThinningTool,
                    muonTPThinningTool,
                    electronTPThinningTool,
                    photonTPThinningTool,
                    tauTPThinningTool]

   DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
   acc.addEventAlgo(DerivationKernel(name,
                                     ThinningTools = thinningTools,
                                     SkimmingTools = kwargs['skimmingTools']))

   return acc


def AugmentationToolsCfg(ConfigFlags, **kwargs):
   
   acc = ComponentAccumulator()

   from DerivationFrameworkJetEtMiss.JetToolConfig import TVAAugmentationToolCfg
   tva_tool = acc.getPrimaryAndMerge(TVAAugmentationToolCfg(ConfigFlags, 'DFMETTrig', 'Nominal'))

   augTools = [tva_tool]

   if kwargs["trackSel"] is not None:
      from DerivationFrameworkInDet.InDetToolsConfig import InDetTrackSelectionToolWrapperCfg
      DFCommonTrackSelection = acc.getPrimaryAndMerge(InDetTrackSelectionToolWrapperCfg(
         ConfigFlags,
         name           = "DFCommonTrackSelection"+kwargs['trackSel'],
         CutLevel       = kwargs['trackSel'],
         DecorationName = "DFMETTrig"+kwargs['trackSel']))

      augTools.append(DFCommonTrackSelection)

   acc.addEventAlgo(CompFactory.DerivationFramework.CommonAugmentation(kwargs['StreamName']+"CommonKernel", AugmentationTools = augTools))
   
   return acc


def addMETTriggerDerivationContent(slimmingHelper, isLoose=True):
   
   slimmingHelper.SmartCollections = ["Electrons", "Muons", "Photons", "TauJets", "PrimaryVertices", "InDetTrackParticles", "EventInfo",
                                      "AntiKt4EMTopoJets", "AntiKt4EMPFlowJets",
                                      "MET_Baseline_AntiKt4EMTopo","MET_Baseline_AntiKt4EMPFlow"]


   slimmingHelper.AllVariables = ["HLT_xAOD__TrigMissingETContainer_TrigEFMissingET",
                                  "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_mht",
                                  "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_topocl_PS",
                                  "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_topocl_PUC",
                                  "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_topocl",
                                  "LVL1EnergySumRoI",
                                  "LVL1JetRoIs",
                                  "LVL1JetEtRoI",
                                  "MET_Core_AntiKt4EMTopo", "MET_Core_AntiKt4EMPFlow",
                                  "METAssoc_AntiKt4EMTopo", "METAssoc_AntiKt4EMPFlow"]

   slimmingHelper.ExtraVariables = ["AntiKt4EMTopoJets.Timing", "AntiKt4EMPFlowJets.Timing",
                                    "InDetTrackParticles.DFMETTrigNominalTVA"]

   if isLoose:
      slimmingHelper.AllVariables += ["HLT_xAOD__MuonContainer_MuonEFInfo",
                                      "CaloCalTopoClusters",
                                      "GlobalChargedParticleFlowObjects",
                                      "GlobalNeutralParticleFlowObjects",
                                      "CHSGChargedParticleFlowObjects",
                                      "CHSGNeutralParticleFlowObjects",
                                      "Kt4EMPFlowEventShape"]

      slimmingHelper.ExtraVariables += ["InDetTrackParticles.DFMETTrigLoose"]

      slimmingHelper.AppendToDictionary.update({'GlobalChargedParticleFlowObjects':'xAOD::FlowElementContainer','GlobalChargedParticleFlowObjectsAux':'xAOD::FlowElementAuxContainer',
                                                'GlobalNeutralParticleFlowObjects':'xAOD::FlowElementContainer', 'GlobalNeutralParticleFlowObjectsAux':'xAOD::FlowElementAuxContainer',
                                                'CHSGChargedParticleFlowObjects':'xAOD::FlowElementContainer','CHSGChargedParticleFlowObjectsAux':'xAOD::ShallowAuxContainer',
                                                'CHSGNeutralParticleFlowObjects':'xAOD::FlowElementContainer','CHSGNeutralParticleFlowObjectsAux':'xAOD::ShallowAuxContainer'})
