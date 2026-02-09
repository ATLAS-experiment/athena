# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
#====================================================================
# DAOD_STDM17.py
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

# Main algorithm config
def STDM17SkimmingToolCfg(flags):
    """Configure the skimming tool"""
    acc = ComponentAccumulator()

    filterList = []

    from DerivationFrameworkJetEtMiss import TriggerLists
    elTriggers = TriggerLists.single_el_Trig(flags)
    muTriggers = TriggerLists.single_mu_Trig(flags)

    addRun3ElectronTriggers = ["HLT_e17_lhvloose_L1EM15VHI","HLT_e20_lhvloose_L1EM15VH", "HLT_e250_etcut_L1EM22VHI",
                               "HLT_e26_lhtight_ivarloose_L1EM22VHI","HLT_e26_lhtight_ivarloose_L1eEM26M",
                               "HLT_e60_lhmedium_L1EM22VHI","HLT_e60_lhmedium_L1eEM26M",
                               "HLT_e140_lhloose_L1EM22VHI","HLT_e140_lhloose_L1eEM26M",
                               "HLT_e300_etcut_L1EM22VHI","HLT_e300_etcut_L1eEM26M",
                               "HLT_e140_lhloose_noringer_L1EM22VHI","HLT_e140_lhloose_noringer_L1eEM26M"]

    addRund3MuonTriggers = ["HLT_mu24_ivarmedium_L1MU14FCH","HLT_mu50_L1MU14FCH","HLT_mu60_0eta105_msonly_L1MU14FCH","HLT_mu60_L1MU14FCH","HLT_mu80_msonly_3layersEC_L1MU14FCH"]

    elTriggers = elTriggers+addRun3ElectronTriggers
    muTriggers = muTriggers+addRund3MuonTriggers
    lepTriggers = elTriggers+muTriggers

    #xAODStringSkimmingTool cannot handle electron trigger names, therefore need to use TriggerSkimmingTool
    tracks = 'InDetTrackParticles.TrkIsoPt1000_ptcone20 < 0.12*InDetTrackParticles.pt && InDetTrackParticles.DFCommonTightPrimary && abs(DFCommonInDetTrackZ0AtPV*sin(InDetTrackParticles.theta)) < 5.0*mm'

    trackRequirements = '(InDetTrackParticles.pt > 9.*GeV && '+tracks+' )'
    #b-jet requirement FixedCutBEff_85 of GN2v01
    jetRequirementsTtbar = '(AntiKt4EMPFlowJets.pt > 18*GeV && log(AntiKt4EMPFlowJets.GN2v01_pb/(0.2*AntiKt4EMPFlowJets.GN2v01_pc+0.01*AntiKt4EMPFlowJets.GN2v01_ptau+(1.0-0.2-0.01)*AntiKt4EMPFlowJets.GN2v01_pu)) > -0.378)'

    muonsRequirements = '(Muons.pt >= 24.*GeV) && (abs(Muons.eta) < 2.6) && (Muons.DFCommonMuonPassPreselection)'
    electronsRequirements = '(Electrons.pt > 24.*GeV) && (abs(Electrons.eta) < 2.6) && ((Electrons.Loose) || (Electrons.DFCommonElectronsLHLoose))'

    #String skimming selections
    expression = '( count('+trackRequirements+') >=2 && count('+jetRequirementsTtbar+') >=1 && ( count('+muonsRequirements+') >=1 || count('+electronsRequirements+') >=1 ) )'

    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import (
        xAODStringSkimmingToolCfg)
    skimmingTool = acc.getPrimaryAndMerge(xAODStringSkimmingToolCfg(
        flags, name = "skimmingTool", expression = expression))
    acc.addPublicTool(skimmingTool)
    filterList += [skimmingTool]

    # Trigger skimming tools
    if flags.Trigger.EDMVersion >= 0:
        STDM17TriggerSkimmingTool_lep = CompFactory.DerivationFramework.TriggerSkimmingTool(name = "STDM17TriggerSkimmingTool_lep", TriggerListOR = lepTriggers)
        acc.addPublicTool(STDM17TriggerSkimmingTool_lep)
        filterList += [STDM17TriggerSkimmingTool_lep]

    STDM17SkimmingTool_lep  = CompFactory.DerivationFramework.FilterCombinationAND(name="STDM17SkimmingTool_lep",  FilterList=filterList)
    acc.addPublicTool(STDM17SkimmingTool_lep)

    STDM17SkimmingTool = CompFactory.DerivationFramework.FilterCombinationOR(name="STDM17SkimmingTool", FilterList=[STDM17SkimmingTool_lep])
    acc.addPublicTool(STDM17SkimmingTool, primary = True)

    return(acc)

def STDM17AugmentationToolsForSkimmingCfg(flags):
    """Configure the augmentation tool for skimming"""
    acc = ComponentAccumulator()

    # Loose tracks with pT > 1000 MeV and Nonprompt_All_MaxWeight TTVA
    toolkwargs = {}
    from InDetTrackSelectionTool.InDetTrackSelectionToolConfig import (
        InDetTrackSelectionTool_Loose_Cfg)
    toolkwargs["TrackSelectionTool"] = acc.popToolsAndMerge(InDetTrackSelectionTool_Loose_Cfg(flags,
                                                                                              name = "TrackSelectionTool1000_STDM17",
                                                                                              minPt = 1000.))

    #Nonprompt_All_MaxWeight TTVA
    from TrackVertexAssociationTool.TrackVertexAssociationToolConfig import isoTTVAToolCfg
    toolkwargs['TTVATool'] = acc.popToolsAndMerge(isoTTVAToolCfg(flags))

    toolkwargs["name"] = "TrackIsolationToolPt1000"
    TrackIsoTool = CompFactory.xAOD.TrackIsolationTool(**toolkwargs)
    acc.addPublicTool(TrackIsoTool)


    from xAODPrimitives.xAODIso import xAODIso as isoPar
    from DerivationFrameworkInDet.InDetToolsConfig import IsolationTrackDecoratorCfg
    Pt1000IsoTrackDecorator = acc.getPrimaryAndMerge(IsolationTrackDecoratorCfg(flags,
                                                                                name               = "Pt1000IsoTrackDecorator",
                                                                                TrackIsolationTool = TrackIsoTool,
                                                                                TargetContainer    = "InDetTrackParticles",
                                                                                iso                = [isoPar.ptcone40, isoPar.ptcone30, isoPar.ptcone20],
                                                                                isoSuffix          = ["ptcone40", "ptcone30", "ptcone20"],
                                                                                Prefix             = "TrkIsoPt1000_"))
    acc.addPublicTool(Pt1000IsoTrackDecorator, primary=True)

    return(acc)

def STDM17AugmentationToolsCfg(flags):
    """Configure the augmentation tool"""
    acc = ComponentAccumulator()

    toolkwargs = {}
    # Loose tracks with pT > 500 MeV
    from InDetTrackSelectionTool.InDetTrackSelectionToolConfig import (
        InDetTrackSelectionTool_Loose_Cfg)
    toolkwargs["TrackSelectionTool"] = acc.popToolsAndMerge(InDetTrackSelectionTool_Loose_Cfg(flags,
                                                                                              name = "TrackSelectionTool500_STDM17",
                                                                                              minPt = 500.))
    #Nonprompt_All_MaxWeight TTVA
    from TrackVertexAssociationTool.TrackVertexAssociationToolConfig import isoTTVAToolCfg
    toolkwargs['TTVATool'] = acc.popToolsAndMerge(isoTTVAToolCfg(flags))

    toolkwargs["name"] = "TrackIsolationToolPt500"
    TrackIsoTool = CompFactory.xAOD.TrackIsolationTool(**toolkwargs)
    acc.addPublicTool(TrackIsoTool)


    from xAODPrimitives.xAODIso import xAODIso as isoPar
    from DerivationFrameworkInDet.InDetToolsConfig import IsolationTrackDecoratorCfg
    Pt500IsoTrackDecorator = acc.getPrimaryAndMerge(IsolationTrackDecoratorCfg(flags,
                                                                               name               = "Pt500IsoTrackDecorator",
                                                                               TrackIsolationTool = TrackIsoTool,
                                                                               TargetContainer    = "InDetTrackParticles",
                                                                               iso                = [isoPar.ptcone40, isoPar.ptcone30, isoPar.ptcone20],
                                                                               isoSuffix          = ["ptcone40", "ptcone30", "ptcone20"],
                                                                               Prefix             = "TrkIsoPt500_"))
    acc.addPublicTool(Pt500IsoTrackDecorator, primary=True)

    return(acc)

# Main algorithm config
def STDM17KernelCfg(flags, name='STDM17Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for STDM17"""
    acc = ComponentAccumulator()

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(flags, TriggerListsHelper = kwargs['TriggerListsHelper']))
    
    #Pre-selection kernel
    from AthenaCommon.CFElements import seqAND
    acc.addSequence( seqAND("STDM17Sequence") )
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    skimmingTool = acc.getPrimaryAndMerge(STDM17SkimmingToolCfg(flags))
    augmentationToolSkim = acc.getPrimaryAndMerge(STDM17AugmentationToolsForSkimmingCfg(flags))
    skimmingKernel = DerivationKernel(kwargs["PreselectionName"], SkimmingTools = [skimmingTool], AugmentationTools = [augmentationToolSkim])
    acc.addEventAlgo( skimmingKernel, sequenceName="STDM17Sequence" ) 

    # Thinning tools...
    from DerivationFrameworkInDet.InDetToolsConfig import TrackParticleThinningCfg, MuonTrackParticleThinningCfg, EgammaTrackParticleThinningCfg, TauTrackParticleThinningCfg

    # Increased cut (w.r.t. R21) on abs(z0) for new TTVA working points
    STDM17_thinning_expression = "( InDetTrackParticles.pt > 6*GeV && InDetTrackParticles.DFCommonTightPrimary && abs(DFCommonInDetTrackZ0AtPV*sin(InDetTrackParticles.theta)) < 5.0*mm )"
    STDM17TrackParticleThinningTool = acc.getPrimaryAndMerge(TrackParticleThinningCfg(
        flags,
        name                    = "STDM17TrackParticleThinningTool",
        StreamName              = kwargs['StreamName'], 
        SelectionString         = STDM17_thinning_expression,
        InDetTrackParticlesKey  = "InDetTrackParticles"))
    
    # Include inner detector tracks associated with muons
    STDM17MuonTPThinningTool = acc.getPrimaryAndMerge(MuonTrackParticleThinningCfg(
        flags,
        name                    = "STDM17MuonTPThinningTool",
        StreamName              = kwargs['StreamName'],
        MuonKey                 = "Muons",
        InDetTrackParticlesKey  = "InDetTrackParticles"))
    
    # Include inner detector tracks associated with electonrs
    STDM17ElectronTPThinningTool = acc.getPrimaryAndMerge(EgammaTrackParticleThinningCfg(
        flags,
        name                    = "STDM17ElectronTPThinningTool",
        StreamName              = kwargs['StreamName'],
        SGKey                   = "Electrons",
        InDetTrackParticlesKey  = "InDetTrackParticles"))

    # Include inner detector tracks associated with photons
    STDM17PhotonTPThinningTool = acc.getPrimaryAndMerge(EgammaTrackParticleThinningCfg(
        flags,
        name                     = "STDM17PhotonTPThinningTool",
        StreamName               = kwargs['StreamName'],
        SGKey                    = "Photons",
        InDetTrackParticlesKey   = "InDetTrackParticles",
        GSFConversionVerticesKey = "GSFConversionVertices"))

    # Include inner detector tracks associated with taus
    STDM17TauTPThinningTool = acc.getPrimaryAndMerge(TauTrackParticleThinningCfg(
        flags,
        name                   = "STDM17TauTPThinningTool",
        StreamName             = kwargs['StreamName'],
        TauKey                 = "TauJets",
        InDetTrackParticlesKey = "InDetTrackParticles",
        DoTauTracksThinning    = True,
        TauTracksKey           = "TauTracks"))

    thinningTools = [STDM17TrackParticleThinningTool,
                     STDM17MuonTPThinningTool,
                     STDM17ElectronTPThinningTool,
                     STDM17PhotonTPThinningTool,
                     STDM17TauTPThinningTool]

    #CaloClusterThinning
    from DerivationFrameworkCalo.DerivationFrameworkCaloConfig import CaloClusterThinningCfg
    selectionString = "( InDetTrackParticles.pt > 6*GeV && InDetTrackParticles.DFCommonTightPrimary && abs(DFCommonInDetTrackZ0AtPV*sin(InDetTrackParticles.theta)) < 5.0*mm )"
    STDM17CaloThinningTool = acc.getPrimaryAndMerge(CaloClusterThinningCfg(flags,
                                                                           name                  = "STDM17CaloClusterThinning",
                                                                           StreamName            = kwargs['StreamName'],
                                                                           SGKey                 = "InDetTrackParticles",
                                                                           TopoClCollectionSGKey = "CaloCalTopoClusters",
                                                                           SelectionString = selectionString,
                                                                           ConeSize = 0.6))
    acc.addPublicTool(STDM17CaloThinningTool)
    thinningTools.append(STDM17CaloThinningTool)

    if flags.Input.isMC:
        truth_cond_status    = "( (TruthParticles.pdgId == 24) || (TruthParticles.pdgId == -24) )"       # decay products of W so we know which are signal
        truth_cond_Lepton = "((abs(TruthParticles.pdgId) >= 11) && (abs(TruthParticles.pdgId) <= 16) && (TruthParticles.barcode < 200000))" # Leptons
        truth_expression = '('+truth_cond_status+' || '+truth_cond_Lepton +')'

        STDM17TruthThinningTool = CompFactory.DerivationFramework.GenericTruthThinning(name = "STDM17TruthThinningTool",
                                                                                       StreamName              = kwargs['StreamName'],
                                                                                       ParticleSelectionString = truth_expression,
                                                                                       PreserveDescendants     = False,
                                                                                       PreserveGeneratorDescendants = True,
                                                                                       PreserveAncestors = False)

        acc.addPublicTool(STDM17TruthThinningTool)
        thinningTools.append(STDM17TruthThinningTool)

    # augmentation tool
    augmentationTool = acc.getPrimaryAndMerge(STDM17AugmentationToolsCfg(flags))

    # Main kernel
    acc.addEventAlgo(DerivationKernel(name, 
                                      ThinningTools = thinningTools,
                                      AugmentationTools = [augmentationTool]),
                     sequenceName="STDM17Sequence")
    
    return acc

def STDM17Cfg(flags):

    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # Note this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    STDM17TriggerListsHelper = TriggerListsHelper(flags)

    # Skimming, thinning, augmentation, extra content
    acc.merge(STDM17KernelCfg(flags, name="STDM17Kernel", PreselectionName="STDM17PreselectionKernel", StreamName = 'StreamDAOD_STDM17', TriggerListsHelper = STDM17TriggerListsHelper))

    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
    STDM17SlimmingHelper = SlimmingHelper("STDM17SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)

    STDM17SlimmingHelper.SmartCollections = ["EventInfo",
                                             "Electrons", "Photons", "Muons", "TauJets", "TauJets_MuonRM",
                                             "InDetTrackParticles", "PrimaryVertices",
                                             "MET_Baseline_AntiKt4EMPFlow",
                                             "AntiKt4EMPFlowJets"]


    STDM17SlimmingHelper.AllVariables = ["MuonSegments","InDetTrackParticles",
                                         "Kt4EMTopoOriginEventShape","Kt4EMPFlowEventShape","CaloCalTopoClusters"]

    STDM17SlimmingHelper.ExtraVariables = ["InDetTrackParticles.TrkIsoPt1000_ptcone40.TrkIsoPt1000_ptcone30.TrkIsoPt1000_ptcone20.TrkIsoPt500_ptcone40.TrkIsoPt500_ptcone30.TrkIsoPt500_ptcone20"]

    if flags.Input.isMC:
        from DerivationFrameworkMCTruth.MCTruthCommonConfig import addTruth3ContentToSlimmerTool
        addTruth3ContentToSlimmerTool(STDM17SlimmingHelper)

        STDM17SlimmingHelper.AppendToDictionary.update({'TruthParticles': 'xAOD::TruthParticleContainer',
                                                       'TruthParticlesAux': 'xAOD::TruthParticleAuxContainer'})

        STDM17SlimmingHelper.SmartCollections += ["AntiKt4TruthJets"]
        STDM17SlimmingHelper.AllVariables += ["MuonTruthParticles", "TruthParticles", "TruthVertices"]

    # Trigger content
    STDM17SlimmingHelper.IncludeTriggerNavigation = False
    STDM17SlimmingHelper.IncludeJetTriggerContent = False
    STDM17SlimmingHelper.IncludeMuonTriggerContent = False
    STDM17SlimmingHelper.IncludeEGammaTriggerContent = False
    STDM17SlimmingHelper.IncludeTauTriggerContent = False
    STDM17SlimmingHelper.IncludeEtMissTriggerContent = False
    STDM17SlimmingHelper.IncludeBJetTriggerContent = False
    STDM17SlimmingHelper.IncludeBPhysTriggerContent = False
    STDM17SlimmingHelper.IncludeMinBiasTriggerContent = False
    

    # Trigger matching
    # Run 2
    if flags.Trigger.EDMVersion == 2:
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = STDM17SlimmingHelper, 
                                               OutputContainerPrefix = "TrigMatch_", 
                                               TriggerList = STDM17TriggerListsHelper.Run2TriggerNamesTau)
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = STDM17SlimmingHelper, 
                                               OutputContainerPrefix = "TrigMatch_",
                                               TriggerList = STDM17TriggerListsHelper.Run2TriggerNamesNoTau)
    # Run 3, or Run 2 with navigation conversion
    if flags.Trigger.EDMVersion == 3 or (flags.Trigger.EDMVersion == 2 and flags.Trigger.doEDMVersionConversion):
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(STDM17SlimmingHelper)

    

    # Output stream    
    STDM17ItemList = STDM17SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_STDM17", ItemList=STDM17ItemList, AcceptAlgs=["STDM17Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_STDM17", AcceptAlgs=["STDM17Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc

