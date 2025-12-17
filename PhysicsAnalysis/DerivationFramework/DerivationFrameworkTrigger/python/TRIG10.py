# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
#====================================================================
# DAOD_TRIG10.py
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

# Main algorithm config
def TRIG10MetTrigSkimmingToolCfg(flags):
    """Configure the skimming tool"""
    acc = ComponentAccumulator()

    TRIG10MetTrigSkimmingTool = CompFactory.DerivationFramework.TriggerSkimmingTool( name           = "TRIG10MetTrigSkimmingTool1",
                                                                                  TriggerListOR     = ["HLT_noalg_L1XE.*","HLT_noalg_L1jXE.*","HLT.*ZeroBias","HLT.*zb.*","HLT.*_eb_.*","HLT.*noPS.*","HLT_noalg_L1RD3.*","HLT_j70_j50a_j0_DJMASS1000j50dphi200x400deta.*"] )
    acc.addPublicTool(TRIG10MetTrigSkimmingTool, primary = True)

    return(acc)

def TRIG10LepTrigSkimmingToolCfg(flags):
    """Configure the trigger skimming tool"""
    acc = ComponentAccumulator()

    from TriggerMenuMT.TriggerAPI.TriggerAPI import TriggerAPI
    from TriggerMenuMT.TriggerAPI.TriggerEnums import TriggerPeriod, TriggerType
    TriggerAPI.setConfigFlags(flags)
    singleMuTriggers = TriggerAPI.getLowestUnprescaled(TriggerPeriod.y2015, TriggerType.mu_single) + TriggerAPI.getLowestUnprescaled(TriggerPeriod.y2016, TriggerType.mu_single) + TriggerAPI.getLowestUnprescaled(TriggerPeriod.y2017, TriggerType.mu_single) + TriggerAPI.getLowestUnprescaled(TriggerPeriod.y2018, TriggerType.mu_single) + TriggerAPI.getLowestUnprescaled(TriggerPeriod.future, TriggerType.mu_single)
    singleElTriggers = TriggerAPI.getLowestUnprescaled(TriggerPeriod.y2015, TriggerType.el_single) + TriggerAPI.getLowestUnprescaled(TriggerPeriod.y2016, TriggerType.el_single) + TriggerAPI.getLowestUnprescaled(TriggerPeriod.y2017, TriggerType.el_single) + TriggerAPI.getLowestUnprescaled(TriggerPeriod.y2018, TriggerType.el_single) + TriggerAPI.getLowestUnprescaled(TriggerPeriod.future, TriggerType.el_single)



    TRIG10LepTrigSkimmingTool = CompFactory.DerivationFramework.TriggerSkimmingTool( name         = "TRIG10LepTrigSkimmingTool1",
                                                                                  TriggerListOR  = singleMuTriggers+singleElTriggers)

    acc.addPublicTool(TRIG10LepTrigSkimmingTool, primary = True)

    return acc

def TRIG10StringSkimmingToolCfg(flags):
    """Configure the string skimming tool"""

    cutExpression = "(count(Electrons.DFCommonElectronsLHLoose && Electrons.pt > (24 * GeV) && abs(Electrons.eta) < 2.47) + count(Muons.DFCommonMuonPassPreselection && Muons.pt > (24*GeV) && abs(Muons.eta) < 2.47) ) >= 1"

    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import (
        xAODStringSkimmingToolCfg)
    return xAODStringSkimmingToolCfg(flags, name = "TRIG10StringSkimmingTool",
                                     expression = cutExpression)


# Main algorithm config
def TRIG10KernelCfg(flags, name='TRIG10Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for TRIG10"""
    acc = ComponentAccumulator()

    # Skimming
    TRIG10StringSkimmingTool  = acc.getPrimaryAndMerge(TRIG10StringSkimmingToolCfg(flags))
    filterListLep = [TRIG10StringSkimmingTool]

    if flags.Trigger.EDMVersion >= 0:
        TRIG10LepTrigSkimmingTool  = acc.getPrimaryAndMerge(TRIG10LepTrigSkimmingToolCfg(flags))
        filterListLep += [TRIG10LepTrigSkimmingTool]

    TRIG10LepTrigStringSkimmingTool = (
        CompFactory.DerivationFramework.FilterCombinationAND(
            name="TRIG10LepTrigStringSkimmingTool", FilterList=filterListLep))
    acc.addPublicTool(TRIG10LepTrigStringSkimmingTool)
    filterList = [TRIG10LepTrigStringSkimmingTool]

    if flags.Trigger.EDMVersion >= 0:
        TRIG10MetTrigSkimmingTool = acc.getPrimaryAndMerge(TRIG10MetTrigSkimmingToolCfg(flags))
        filterList += [TRIG10MetTrigSkimmingTool]

    TRIG10SkimmingTool = CompFactory.DerivationFramework.FilterCombinationOR(
        name="TRIG10SkimmingTool", FilterList=filterList)
    acc.addPublicTool(TRIG10SkimmingTool)

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(flags, TriggerListsHelper = kwargs['TriggerListsHelper']))

    from DerivationFrameworkInDet.InDetToolsConfig import MuonTrackParticleThinningCfg, EgammaTrackParticleThinningCfg, TauTrackParticleThinningCfg

    muonTPThinningTool = acc.getPrimaryAndMerge(MuonTrackParticleThinningCfg(
      flags,
      name                    = kwargs['StreamName']+"MuonTPThinningTool",
      StreamName              = kwargs['StreamName'],
      MuonKey                 = "Muons",
      InDetTrackParticlesKey  = "InDetTrackParticles"))

    electronTPThinningTool = acc.getPrimaryAndMerge(EgammaTrackParticleThinningCfg(
      flags,
      name                    = kwargs['StreamName']+"ElectronTPThinningTool",
      StreamName              = kwargs['StreamName'],
      SGKey                   = "Electrons",
      InDetTrackParticlesKey  = "InDetTrackParticles"))

    photonTPThinningTool = acc.getPrimaryAndMerge(EgammaTrackParticleThinningCfg(
      flags,
      name                     = kwargs['StreamName']+"PhotonTPThinningTool",
      StreamName               = kwargs['StreamName'],
      SGKey                    = "Photons",
      InDetTrackParticlesKey   = "InDetTrackParticles",
      GSFConversionVerticesKey = "GSFConversionVertices"))

    tauTPThinningTool = acc.getPrimaryAndMerge(TauTrackParticleThinningCfg(
      flags,
      name                   = kwargs['StreamName']+"TauTPThinningTool",
      StreamName             = kwargs['StreamName'],
      TauKey                 = "TauJets",
      InDetTrackParticlesKey = "InDetTrackParticles",
      DoTauTracksThinning    = True,
      TauTracksKey           = "TauTracks"))

    thinningTools = [muonTPThinningTool,
                    electronTPThinningTool,
                    photonTPThinningTool,
                    tauTPThinningTool]


    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name="TRIG10Kernel",
                                     ThinningTools = thinningTools,
                                     SkimmingTools = [TRIG10SkimmingTool]))

    return acc


def TRIG10Cfg(flags):

    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    TRIG10TriggerListsHelper = TriggerListsHelper(flags)

    # Skimming, thinning, augmentation
    acc.merge(TRIG10KernelCfg(flags, name="TRIG10Kernel", StreamName = 'StreamDAOD_TRIG10', TriggerListsHelper = TRIG10TriggerListsHelper))

    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
    TRIG10SlimmingHelper = SlimmingHelper("TRIG10SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)


    TRIG10SlimmingHelper.SmartCollections = ["Electrons", "Muons", "Photons", "TauJets", "PrimaryVertices", "EventInfo",
                                       "AntiKt4EMTopoJets", "AntiKt4EMPFlowJets",
                                       "MET_Baseline_AntiKt4EMTopo","MET_Baseline_AntiKt4EMPFlow","InDetTrackParticles"]


    TRIG10SlimmingHelper.AllVariables = ["HLT_xAOD__TrigMissingETContainer_TrigEFMissingET",
                                   "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_mht",
                                   "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_topocl_PS",
                                   "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_topocl_PUC",
                                   "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_topocl",
                                   "LVL1EnergySumRoI",
                                   "LVL1JetRoIs",
                                   "LVL1JetEtRoI",
                                   "MET_Core_AntiKt4EMTopo", "MET_Core_AntiKt4EMPFlow",
                                   "METAssoc_AntiKt4EMTopo", "METAssoc_AntiKt4EMPFlow"]

    TRIG10SlimmingHelper.ExtraVariables = ["AntiKt4EMTopoJets.Timing", "AntiKt4EMPFlowJets.Timing",
                                     "InDetTrackParticles.DFMETTrigNominalTVA"]

    # Trigger content
    TRIG10SlimmingHelper.IncludeTriggerNavigation = False
    TRIG10SlimmingHelper.IncludeJetTriggerContent = False
    TRIG10SlimmingHelper.IncludeMuonTriggerContent = False
    TRIG10SlimmingHelper.IncludeEGammaTriggerContent = False
    TRIG10SlimmingHelper.IncludeTauTriggerContent = False
    TRIG10SlimmingHelper.IncludeEtMissTriggerContent = False
    TRIG10SlimmingHelper.IncludeBJetTriggerContent = False
    TRIG10SlimmingHelper.IncludeBPhysTriggerContent = False
    TRIG10SlimmingHelper.IncludeMinBiasTriggerContent = False

    TRIG10SlimmingHelper.AllVariables += ['HLT_MET_tcpufit','HLT_MET_cell','HLT_MET_trkmht','HLT_MET_cvfpufit','HLT_MET_pfopufit','HLT_MET_mhtpufit_em','HLT_MET_mhtpufit_pf','HLT_MET_pfsum','HLT_MET_pfsum_vssk','HLT_MET_pfsum_cssk','HLT_MET_nn','L1_jFexMETRoI','L1_jFexMETxRoI','L1_gMETComponentsJwoj','L1_gMETComponentsRms','L1_gMETComponentsNoiseCut']

    # Output stream    
    TRIG10ItemList = TRIG10SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_TRIG10", ItemList=TRIG10ItemList, AcceptAlgs=["TRIG10Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_TRIG10", AcceptAlgs=["TRIG10Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc

