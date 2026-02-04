# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#====================================================================
# DAOD_JETM12.py
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

# Main algorithm config
def JETM12SkimmingToolCfg(flags):
    """Configure the skimming tool"""
    acc = ComponentAccumulator()

    from DerivationFrameworkJetEtMiss import TriggerLists
    metTriggers = TriggerLists.MET_Trig(flags)
    elTriggers = TriggerLists.single_el_Trig(flags)
    muTriggers = TriggerLists.single_mu_Trig(flags)

    addTtbarEvents = True
    if not flags.Input.isMC:
        # Check if the solenoid was turned on to define skimming
        from CoolConvUtilities.MagFieldUtils import getFieldForRun
        magfield=getFieldForRun(flags.Input.RunNumbers[0],lumiblock=flags.Input.LumiBlockNumbers[0])
        addTtbarEvents = magfield.solenoidCurrent() > 0


    #xAODStringSkimmingTool cannot handle electron trigger names, therefore need to use TriggerSkimmingTool
    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import (
        xAODStringSkimmingToolCfg)

    tracks = 'InDetTrackParticles.TrkIsoPt1000_ptcone20 < 0.12*InDetTrackParticles.pt && InDetTrackParticles.DFCommonTightPrimary && abs(DFCommonInDetTrackZ0AtPV*sin(InDetTrackParticles.theta)) < 5.0*mm'

    trackRequirements = '(InDetTrackParticles.pt > 6.*GeV && '+tracks+' )'
    trackRequirementsMu = '(InDetTrackParticles.pt > 40.*GeV && '+tracks+' )'
    jetRequirementsTtbar = '(AntiKt4EMPFlowJets.pt > 18*GeV && log(AntiKt4EMPFlowJets.GN2v01_pb/(0.2*AntiKt4EMPFlowJets.GN2v01_pc + (1.-0.2-0.01)*AntiKt4EMPFlowJets.GN2v01_pu + 0.01*AntiKt4EMPFlowJets.GN2v01_ptau))>=0.844)'

    trackRequirementsNoIso = '(InDetTrackParticles.pt > 10.*GeV && abs(DFCommonInDetTrackZ0AtPV*sin(InDetTrackParticles.theta)) < 5.0*mm )'

    muonsRequirements = '(Muons.pt >= 20.*GeV) && (abs(Muons.eta) < 2.6) && (Muons.DFCommonMuonPassPreselection)'
    electronsRequirements = '(Electrons.pt > 20.*GeV) && (abs(Electrons.eta) < 2.6) && ((Electrons.Loose) || (Electrons.DFCommonElectronsLHLoose))'

    #String skimming selections
    expression_W = '( count('+trackRequirements+') >=1 )'
    expression_Mu = '( count('+trackRequirementsMu+') >=1 )'
    expression_ttbarEl = '( count('+electronsRequirements+') >=1 ) && ( count('+jetRequirementsTtbar+') >=1 ) && ( count('+trackRequirementsNoIso+') >=2 ) && ( count('+trackRequirements+') >=1 )'
    expression_ttbarElNoTag = '( count('+electronsRequirements+') >=1 ) && ( count('+trackRequirements+') >=1 )'
    expression_ttbarMu = '( count('+muonsRequirements+') >=1 ) && ( count('+jetRequirementsTtbar+') >=1 ) && ( count('+trackRequirementsNoIso+') >=2 ) && ( count('+trackRequirements+') >=1 )'
    expression_ttbarMuNoTag = '( count('+muonsRequirements+') >=1 ) && ( count('+trackRequirements+') >=1 )'

    skimmingTool_W = acc.getPrimaryAndMerge(xAODStringSkimmingToolCfg(
        flags, name = "skimmingTool_W", expression = expression_W))
    filterList_W = [skimmingTool_W]
    skimmingTool_Mu = acc.getPrimaryAndMerge(xAODStringSkimmingToolCfg(
        flags, name = "skimmingTool_mu", expression = expression_Mu))
    filterList_Mu = [skimmingTool_Mu]

    # Trigger skimming tools
    if flags.Trigger.EDMVersion >= 0:
        JETM12TriggerSkimmingTool_W = (
            CompFactory.DerivationFramework.TriggerSkimmingTool(
                name = "JETM12TriggerSkimmingTool_W",
                TriggerListOR = metTriggers))
        acc.addPublicTool(JETM12TriggerSkimmingTool_W)
        filterList_W += [JETM12TriggerSkimmingTool_W]

        JETM12TriggerSkimmingTool_ele = (
            CompFactory.DerivationFramework.TriggerSkimmingTool(
                name = "JETM12TriggerSkimmingTool_ele",
                TriggerListOR = elTriggers))
        acc.addPublicTool(JETM12TriggerSkimmingTool_ele)

        JETM12TriggerSkimmingTool_mu = (
            CompFactory.DerivationFramework.TriggerSkimmingTool(
                name = "JETM12TriggerSkimmingTool_mu",
                TriggerListOR = muTriggers))
        acc.addPublicTool(JETM12TriggerSkimmingTool_mu)
        filterList_Mu += [JETM12TriggerSkimmingTool_mu]

    JETM12SkimmingTool_W  = (
        CompFactory.DerivationFramework.FilterCombinationAND(
            name="JETM12SkimmingTool_W",  FilterList=filterList_W))
    acc.addPublicTool(JETM12SkimmingTool_W)
    JETM12SkimmingTool_Mu = (
        CompFactory.DerivationFramework.FilterCombinationAND(
            name="JETM12SkimmingTool_Mu", FilterList=filterList_Mu))
    acc.addPublicTool(JETM12SkimmingTool_Mu)

    finalSkimmingTools = [JETM12SkimmingTool_W,JETM12SkimmingTool_Mu]

    if addTtbarEvents:
        skimmingTool_ttbarEl = acc.getPrimaryAndMerge(
            xAODStringSkimmingToolCfg(
                flags, name = "skimmingTool_ttbarEl",
                expression = expression_ttbarEl))
        acc.addPublicTool(skimmingTool_ttbarEl)
        filterList_ttbarEl = [skimmingTool_ttbarEl]

        skimmingTool_ttbarElNoTag = acc.getPrimaryAndMerge(
            xAODStringSkimmingToolCfg(
                flags, name = "skimmingTool_ttbarElNoTag",
                expression = expression_ttbarElNoTag))
        acc.addPublicTool(skimmingTool_ttbarElNoTag)
        filterList_ttbarElNoTag = [skimmingTool_ttbarElNoTag]

        skimmingTool_ttbarMu = acc.getPrimaryAndMerge(
            xAODStringSkimmingToolCfg(
                flags, name = "skimmingTool_ttbarMu",
                expression = expression_ttbarMu))
        acc.addPublicTool(skimmingTool_ttbarMu)
        filterList_ttbarMu = [skimmingTool_ttbarMu]

        skimmingTool_ttbarMuNoTag = acc.getPrimaryAndMerge(
            xAODStringSkimmingToolCfg(
                flags, name = "skimmingTool_ttbarMuNoTag",
                expression = expression_ttbarMuNoTag))
        acc.addPublicTool(skimmingTool_ttbarMuNoTag)
        filterList_ttbarMuNoTag = [skimmingTool_ttbarMuNoTag]

        if flags.Trigger.EDMVersion >= 0:
            filterList_ttbarEl += [JETM12TriggerSkimmingTool_ele]
            filterList_ttbarElNoTag += [JETM12TriggerSkimmingTool_ele]
            filterList_ttbarMu += [JETM12TriggerSkimmingTool_mu]
            filterList_ttbarMuNoTag += [JETM12TriggerSkimmingTool_mu]

        JETM12SkimmingTool_ttbarEl      = (
            CompFactory.DerivationFramework.FilterCombinationAND(
                name="JETM12SkimmingTool_ttbarEl",
                FilterList=filterList_ttbarEl))
        acc.addPublicTool(JETM12SkimmingTool_ttbarEl)
        JETM12SkimmingTool_ttbarElNoTag = (
            CompFactory.DerivationFramework.FilterCombinationAND(
                name="JETM12SkimmingTool_ttbarElNoTag",
                FilterList=filterList_ttbarElNoTag))
        acc.addPublicTool(JETM12SkimmingTool_ttbarElNoTag)
        JETM12SkimmingTool_ttbarMu      = (
            CompFactory.DerivationFramework.FilterCombinationAND(
                name="JETM12SkimmingTool_ttbarMu",
                FilterList=filterList_ttbarMu))
        acc.addPublicTool(JETM12SkimmingTool_ttbarMu)
        JETM12SkimmingTool_ttbarMuNoTag = (
            CompFactory.DerivationFramework.FilterCombinationAND(
                name="JETM12SkimmingTool_ttbarMuNoTag",
                FilterList=filterList_ttbarMuNoTag))
        acc.addPublicTool(JETM12SkimmingTool_ttbarMuNoTag)

        finalSkimmingTools = [
            JETM12SkimmingTool_W, JETM12SkimmingTool_Mu,
            JETM12SkimmingTool_ttbarEl, JETM12SkimmingTool_ttbarMu,
            JETM12SkimmingTool_ttbarElNoTag, JETM12SkimmingTool_ttbarMuNoTag]

    JETM12SkimmingTool = CompFactory.DerivationFramework.FilterCombinationOR(
        name="JETM12SkimmingTool", FilterList=finalSkimmingTools)
    acc.addPublicTool(JETM12SkimmingTool, primary = True)

    return(acc)

def JETM12AugmentationToolsForSkimmingCfg(flags):
    """Configure the augmentation tool for skimming"""
    acc = ComponentAccumulator()

    # Loose tracks with pT > 1000 MeV and Nonprompt_All_MaxWeight TTVA
    toolkwargs = {}
    from InDetTrackSelectionTool.InDetTrackSelectionToolConfig import (
        InDetTrackSelectionTool_Loose_Cfg)
    toolkwargs["TrackSelectionTool"] = acc.popToolsAndMerge(InDetTrackSelectionTool_Loose_Cfg(flags,
                                                                                              name = "TrackSelectionTool1000_JETM12",
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

def JETM12AugmentationToolsCfg(flags):
    """Configure the augmentation tool"""
    acc = ComponentAccumulator()

    toolkwargs = {}
    # Loose tracks with pT > 500 MeV
    from InDetTrackSelectionTool.InDetTrackSelectionToolConfig import (
        InDetTrackSelectionTool_Loose_Cfg)
    toolkwargs["TrackSelectionTool"] = acc.popToolsAndMerge(InDetTrackSelectionTool_Loose_Cfg(flags,
                                                                                              name = "TrackSelectionTool500_JETM12",
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
def JETM12KernelCfg(flags, name='JETM12Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for JETM12"""
    acc = ComponentAccumulator()

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(flags, TriggerListsHelper = kwargs['TriggerListsHelper']))
    
    #Pre-selection kernel
    from AthenaCommon.CFElements import seqAND
    acc.addSequence( seqAND("JETM12Sequence") )
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    skimmingTool = acc.getPrimaryAndMerge(JETM12SkimmingToolCfg(flags))
    augmentationToolSkim = acc.getPrimaryAndMerge(JETM12AugmentationToolsForSkimmingCfg(flags))
    skimmingKernel = DerivationKernel(kwargs["PreselectionName"], SkimmingTools = [skimmingTool], AugmentationTools = [augmentationToolSkim])
    acc.addEventAlgo( skimmingKernel, sequenceName="JETM12Sequence" ) 

    # Thinning tools...
    from DerivationFrameworkInDet.InDetToolsConfig import TrackParticleThinningCfg, MuonTrackParticleThinningCfg, EgammaTrackParticleThinningCfg, TauTrackParticleThinningCfg

    # Increased cut (w.r.t. R21) on abs(z0) for new TTVA working points
    JETM12_thinning_expression = "( InDetTrackParticles.pt > 6*GeV && InDetTrackParticles.DFCommonTightPrimary && abs(DFCommonInDetTrackZ0AtPV*sin(InDetTrackParticles.theta)) < 5.0*mm )"
    JETM12TrackParticleThinningTool = acc.getPrimaryAndMerge(TrackParticleThinningCfg(
        flags,
        name                    = "JETM12TrackParticleThinningTool",
        StreamName              = kwargs['StreamName'], 
        SelectionString         = JETM12_thinning_expression,
        InDetTrackParticlesKey  = "InDetTrackParticles"))
    
    # Include inner detector tracks associated with muons
    JETM12MuonTPThinningTool = acc.getPrimaryAndMerge(MuonTrackParticleThinningCfg(
        flags,
        name                    = "JETM12MuonTPThinningTool",
        StreamName              = kwargs['StreamName'],
        MuonKey                 = "Muons",
        InDetTrackParticlesKey  = "InDetTrackParticles"))
    
    # Include inner detector tracks associated with electonrs
    JETM12ElectronTPThinningTool = acc.getPrimaryAndMerge(EgammaTrackParticleThinningCfg(
        flags,
        name                    = "JETM12ElectronTPThinningTool",
        StreamName              = kwargs['StreamName'],
        SGKey                   = "Electrons",
        InDetTrackParticlesKey  = "InDetTrackParticles"))

    # Include inner detector tracks associated with photons
    JETM12PhotonTPThinningTool = acc.getPrimaryAndMerge(EgammaTrackParticleThinningCfg(
        flags,
        name                     = "JETM12PhotonTPThinningTool",
        StreamName               = kwargs['StreamName'],
        SGKey                    = "Photons",
        InDetTrackParticlesKey   = "InDetTrackParticles",
        GSFConversionVerticesKey = "GSFConversionVertices"))

    # Include inner detector tracks associated with taus
    JETM12TauTPThinningTool = acc.getPrimaryAndMerge(TauTrackParticleThinningCfg(
        flags,
        name                   = "JETM12TauTPThinningTool",
        StreamName             = kwargs['StreamName'],
        TauKey                 = "TauJets",
        InDetTrackParticlesKey = "InDetTrackParticles",
        DoTauTracksThinning    = True,
        TauTracksKey           = "TauTracks"))

    thinningTools = [JETM12TrackParticleThinningTool,
                     JETM12MuonTPThinningTool,
                     JETM12ElectronTPThinningTool,
                     JETM12PhotonTPThinningTool,
                     JETM12TauTPThinningTool]

    #CaloClusterThinning
    from DerivationFrameworkCalo.DerivationFrameworkCaloConfig import CaloClusterThinningCfg
    selectionString = "( InDetTrackParticles.pt > 6*GeV && InDetTrackParticles.DFCommonTightPrimary && abs(DFCommonInDetTrackZ0AtPV*sin(InDetTrackParticles.theta)) < 5.0*mm )"
    JETM12CaloThinningTool = acc.getPrimaryAndMerge(CaloClusterThinningCfg(flags,
                                                                           name                  = "JETM12CaloClusterThinning",
                                                                           StreamName            = kwargs['StreamName'],
                                                                           SGKey                 = "InDetTrackParticles",
                                                                           TopoClCollectionSGKey = "CaloCalTopoClusters",
                                                                           SelectionString = selectionString,
                                                                           ConeSize = 0.6))
    acc.addPublicTool(JETM12CaloThinningTool)
    thinningTools.append(JETM12CaloThinningTool)

    if flags.Input.isMC:
        truth_cond_status    = "( (TruthParticles.isGenStable) && (TruthParticles.pt > 8*GeV) )"       # high pt pions for E/p
        truth_cond_Lepton = "((abs(TruthParticles.pdgId) >= 11) && (abs(TruthParticles.pdgId) <= 16) && !(TruthParticles.isSimulationParticle))" # Leptons
        truth_expression = '('+truth_cond_status+' || '+truth_cond_Lepton +')'

        JETM12TruthThinningTool = CompFactory.DerivationFramework.GenericTruthThinning(name = "JETM12TruthThinningTool",
                                                                                       StreamName              = kwargs['StreamName'],
                                                                                       ParticleSelectionString = truth_expression,
                                                                                       PreserveDescendants     = False,
                                                                                       PreserveGeneratorDescendants = True,
                                                                                       PreserveAncestors = False)

        acc.addPublicTool(JETM12TruthThinningTool)
        thinningTools.append(JETM12TruthThinningTool)

    # augmentation tool
    augmentationTool = acc.getPrimaryAndMerge(JETM12AugmentationToolsCfg(flags))

    # Main kernel
    acc.addEventAlgo(DerivationKernel(name, 
                                      ThinningTools = thinningTools,
                                      AugmentationTools = [augmentationTool]),
                     sequenceName="JETM12Sequence")
    
    return acc

def JETM12Cfg(flags):

    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    JETM12TriggerListsHelper = TriggerListsHelper(flags)

    # Skimming, thinning, augmentation, extra content
    acc.merge(JETM12KernelCfg(flags, name="JETM12Kernel", PreselectionName="JETM12PreselectionKernel", StreamName = 'StreamDAOD_JETM12', TriggerListsHelper = JETM12TriggerListsHelper))

    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
    JETM12SlimmingHelper = SlimmingHelper("JETM12SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)

    JETM12SlimmingHelper.SmartCollections = ["EventInfo",
                                             "Electrons", "Photons", "Muons", "TauJets",
                                             "InDetTrackParticles", "PrimaryVertices",
                                             "MET_Baseline_AntiKt4EMPFlow",
                                             "AntiKt4EMPFlowJets"]

    JETM12SlimmingHelper.AllVariables = ["MuonSegments","InDetTrackParticles",
                                         "Kt4EMTopoOriginEventShape","Kt4EMPFlowEventShape","CaloCalTopoClusters"]

    JETM12SlimmingHelper.ExtraVariables = ["InDetTrackParticles.TrkIsoPt1000_ptcone40.TrkIsoPt1000_ptcone30.TrkIsoPt1000_ptcone20.TrkIsoPt500_ptcone40.TrkIsoPt500_ptcone30.TrkIsoPt500_ptcone20"]

    if flags.Input.isMC:
        from DerivationFrameworkMCTruth.MCTruthCommonConfig import addTruth3ContentToSlimmerTool
        addTruth3ContentToSlimmerTool(JETM12SlimmingHelper)

        JETM12SlimmingHelper.AppendToDictionary.update({'TruthParticles': 'xAOD::TruthParticleContainer',
                                                       'TruthParticlesAux': 'xAOD::TruthParticleAuxContainer'})

        JETM12SlimmingHelper.SmartCollections += ["AntiKt4TruthJets"]
        JETM12SlimmingHelper.AllVariables += ["MuonTruthParticles", "TruthParticles", "TruthVertices"]

    # Trigger content
    JETM12SlimmingHelper.IncludeTriggerNavigation = False
    JETM12SlimmingHelper.IncludeJetTriggerContent = False
    JETM12SlimmingHelper.IncludeMuonTriggerContent = False
    JETM12SlimmingHelper.IncludeEGammaTriggerContent = False
    JETM12SlimmingHelper.IncludeTauTriggerContent = False
    JETM12SlimmingHelper.IncludeEtMissTriggerContent = False
    JETM12SlimmingHelper.IncludeBJetTriggerContent = False
    JETM12SlimmingHelper.IncludeBPhysTriggerContent = False
    JETM12SlimmingHelper.IncludeMinBiasTriggerContent = False

    # Output stream    
    JETM12ItemList = JETM12SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_JETM12", ItemList=JETM12ItemList, AcceptAlgs=["JETM12Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_JETM12", AcceptAlgs=["JETM12Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc

