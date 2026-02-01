# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
# STDM7.py - derivation for exclusive dilepton analyses
#            skims dilepton (e, mu or taujet) events, contains InDetTracks and AFP information

# Changes with respect to previous versions are:
# -- Added TauJets to lepton skim with tau thinning tools inherited from PHYS
# -- Increased muon string skim condition from 4 GeV to 10 GeV
# -- Increased electron string skim condition from 11 GeV to 15 GeV
#============================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory
from AthenaCommon.Logging import logging

from TriggerMenuMT.TriggerAPI import TriggerType, TriggerAPI, TriggerPeriod
from DerivationFrameworkPhys.TriggerListsHelper import read_trig_list_file, read_trig_list_flags, getTapisSession

logSTDM7 = logging.getLogger('STDM7')

def STDM7TriggerSkimmingToolCfg(flags):
    '''Configure the STDM7 trigger skimming tool'''
    acc = ComponentAccumulator()

    TriggerAPI.setConfigFlags(flags)

    if flags.Trigger.EDMVersion <= 2:

        # Trigger API for Runs 1, 2
        #====================================================================
        # TRIGGER CONTENT
        #====================================================================
        ## See https://twiki.cern.ch/twiki/bin/view/Atlas/TriggerAPI
        ## Get single and multi mu, e
        ## tau, multi-object triggers not available in the matching code
        allperiods = TriggerPeriod.y2015 | TriggerPeriod.y2016 | TriggerPeriod.y2017 | TriggerPeriod.y2018 | TriggerPeriod.future2e34
        trig_el  = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.el,  livefraction=0.8)
        trig_mu  = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.mu,  livefraction=0.8)
        trig_tau = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.tau, livefraction=0.8)
        ## Add cross-triggers for some sets
        trig_em = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.el, additionalTriggerType=TriggerType.mu,  livefraction=0.8)
        trig_et = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.el, additionalTriggerType=TriggerType.tau, livefraction=0.8)
        trig_mt = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.mu, additionalTriggerType=TriggerType.tau, livefraction=0.8)

        ## Add extra chains which were specified via repository include-lists
        extra_file_notau = read_trig_list_file("DerivationFrameworkPhys/run2ExtraMatchingTriggers.txt")
        extra_file_tau = read_trig_list_file("DerivationFrameworkPhys/run2ExtraMatchingTauTriggers.txt")
    
        ## Add extra chains from flags
        extra_flag_notau, extra_flag_tau = read_trig_list_flags(flags)

        ## Merge and remove duplicates
        trigger_names = list(set(trig_el+trig_mu+trig_tau+trig_em+trig_et+trig_mt+extra_file_notau+extra_file_tau+extra_flag_notau+extra_flag_tau))

    else: # Run 3 and Run 4
        # TriggerAPI Session based trigger lists
        session = getTapisSession(flags)
        api_trigger_names = set()
        api_trigger_names = session.getLowestUnprescaled(triggerType=TriggerType.el, livefraction=0.8).union(api_trigger_names)
        api_trigger_names = session.getLowestUnprescaled(triggerType=TriggerType.mu, livefraction=0.8).union(api_trigger_names)
        api_trigger_names = session.getLowestUnprescaled(triggerType=TriggerType.tau, livefraction=0.8).union(api_trigger_names)
        ## Add Run 2 cross-triggers for some sets
        api_trigger_names = session.getLowestUnprescaled(triggerType=[TriggerType.el,  TriggerType.mu], livefraction=0.8).union(api_trigger_names)
        api_trigger_names = session.getLowestUnprescaled(triggerType=[TriggerType.el,  TriggerType.tau], livefraction=0.8).union(api_trigger_names)
        api_trigger_names = session.getLowestUnprescaled(triggerType=[TriggerType.mu,  TriggerType.tau], livefraction=0.8).union(api_trigger_names)

        ## Add extra chains from flags
        extra_flag_notau, extra_flag_tau = read_trig_list_flags(flags)
        extra_flag = extra_flag_notau + extra_flag_tau
        
        ## Add extra chains from file
        extra_file = read_trig_list_file("DerivationFrameworkPhys/run3ExtraMatchingTriggers.txt")

        ## Merge and remove duplicates
        trigger_names = list(set(extra_file + extra_flag + list(api_trigger_names)))

    STDM7TriggerSkimmingTool = CompFactory.DerivationFramework.TriggerSkimmingTool(name = "STDM7TriggerSkimmingTool",
                                                                                   OutputLevel   = 0,
                                                                                   TriggerListOR = trigger_names,
                                                                                   TriggerListAND = [] )
    acc.addPublicTool(STDM7TriggerSkimmingTool,primary=True)
    return(acc)

def STDM7StringSkimmingToolCfg(flags):
    '''Configure the STDM7 string skimming tool'''
    acc = ComponentAccumulator()

    # skim on two good leptons    
    muonsRequirements = '(Muons.pt >= 10.*GeV) && (abs(Muons.eta) < 2.6) && (Muons.DFCommonMuonPassPreselection) && (Muons.DFCommonMuonPassIDCuts)'
    electronsRequirements = '(Electrons.pt >= 15.*GeV) && (abs(Electrons.eta) < 2.6) && ((Electrons.DFCommonElectronsLHLoose) || (Electrons.DFCommonElectronsDNNLoose))'
    tausRequirements = '(TauJets.pt >= 20.*GeV) && (abs(TauJets.eta) < 2.6) && ((TauJets.DFTauRNNLoose || TauJets.DFTauGNTauLoose))'
    
    chargedParticleRequirements = '(TruthParticles.pt >= 500) && (TruthParticles.isGenStable)' \
                                  '&& (TruthParticles.charge != 0)' \
                                  '&& (TruthParticles.theta > 0.163803) && (TruthParticles.theta < 2.97778)' \
                                  '&& (TruthParticles.HSBool)' 
    # theta selection correspond to |eta|<2.5 (minus epsilon); avoids floating point exception if theta=0 or pi
    
    muonOnlySelection = 'count('+muonsRequirements+') >=2'
    electronOnlySelection = 'count('+electronsRequirements+') >= 2'
    tauOnlySelection = 'count('+tausRequirements+') >= 2'
    electronMuonSelection = '(count('+electronsRequirements+') + count('+muonsRequirements+')) >= 2'
    electronTauSelection = '(count('+electronsRequirements+') + count('+tausRequirements+')) >= 2'
    muonTauSelection = '(count('+muonsRequirements+') + count('+tausRequirements+')) >= 2'
    
    # for MC we may skim on the charged-particle multiplicity
    filterVal = -1 # Todo: nChFilter flag to be implemented later
    if filterVal > -1 and flags.Input.isMC :
        chargedParticleSelection = 'count('+chargedParticleRequirements+') < '+str(filterVal)
        offlineExpression = '(('+muonOnlySelection+' || '+electronOnlySelection+' || '+tauOnlySelection+' || '+electronMuonSelection+' || '+electronTauSelection+' || '+muonTauSelection+')' \
            '&& ('+chargedParticleSelection+'))'
    else:
        offlineExpression = '('+muonOnlySelection+' || '+electronOnlySelection+' || '+tauOnlySelection+' || '+electronMuonSelection+' || '+electronTauSelection+' || '+muonTauSelection+')'

    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import xAODStringSkimmingToolCfg
    STDM7StringSkimmingTool = acc.getPrimaryAndMerge(xAODStringSkimmingToolCfg(
        flags, name = "STDM7StringSkimmingTool", expression = offlineExpression))
    acc.addPublicTool(STDM7StringSkimmingTool, primary=True)
    
    return acc

# Main algorithm config
def STDM7KernelCfg(flags, name='STDM7Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for STDM7"""
    acc = ComponentAccumulator()

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(
        flags, 
        TriggerListsHelper     = kwargs['TriggerListsHelper'], 
    ))
    # Needed to decorate taus with DFTauGNTau WPs
    from DerivationFrameworkTau.TauCommonConfig import AddTauAugmentationCfg
    acc.merge(AddTauAugmentationCfg(flags, prefix="STDM7", doGNTauLoose=True))

    ## IFF augmentation - Adding Lepton Taggers 
    from LeptonTaggers.LeptonTaggersConfig import DecoratePLITAlgsCfg
    acc.merge(DecoratePLITAlgsCfg(flags))

    filterList = []
    filterList += [acc.getPrimaryAndMerge(STDM7StringSkimmingToolCfg(flags))]
    # Only run trigger skimming tool after trigger EDM check to allow running on MC where trigger has not been simulated 
    if flags.Trigger.EDMVersion >= 0:
        filterList += [acc.getPrimaryAndMerge(STDM7TriggerSkimmingToolCfg(flags))]

    # AND combination of lepton and trigger skimming tools - event must fire lepton trigger and pass lepton reco skim to be kept
    skimmingTools = [ acc.addPublicTool(CompFactory.DerivationFramework.FilterCombinationAND(
        name="STDM7SkimmingTool",
        FilterList = filterList,
    ))]
    
    nametag = name.replace('Kernel', '') #get the name to label the tools below such that other formats can use this KernelCfg
    thinningToolsArgs = {
        'TauJetThinningToolName'              : nametag+"TauJetThinningTool",
    }

    # Inherit tau thinning tools from PHYS
    from DerivationFrameworkPhys.PhysCommonThinningConfig import PhysCommonThinningCfg
    acc.merge(PhysCommonThinningCfg(flags, StreamName = kwargs['StreamName'], **thinningToolsArgs))

    # Implicit track thinning happening in tau thinning tools
    # Add explict tool to tell derivation to keep all tracks
    from DerivationFrameworkInDet.InDetToolsConfig import TrackParticleThinningCfg
    acc.merge(TrackParticleThinningCfg(flags,
                                       name        = 'STDM7TrackParticleThinningTool',
                                       StreamName  = kwargs['StreamName'],
                                       SelectionString = "InDetTrackParticles.pt > -1",
                                       InDetTrackParticlesKey = "InDetTrackParticles"))
    
    thinningTools = []
    for key in thinningToolsArgs:
        thinningTools.append(acc.getPublicTool(thinningToolsArgs[key]))
    thinningTools.append(acc.getPublicTool('STDM7TrackParticleThinningTool'))

    # The kernel algorithm itself
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name,
                                      ThinningTools = thinningTools,
                                      SkimmingTools = skimmingTools)) 
    return acc

def STDM7CoreCfg(flags, name_tag='STDM7', StreamName='StreamDAOD_STDM7', TriggerListsHelper=None):
    
    if TriggerListsHelper is None:
        from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
        TriggerListsHelper = TriggerListsHelper(flags)

    acc = ComponentAccumulator()
    
    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
    STDM7SlimmingHelper = SlimmingHelper(name_tag+"SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    STDM7SlimmingHelper.SmartCollections = ["EventInfo",
                                            "Electrons",
                                            "Photons",
                                            "Muons",
                                            "PrimaryVertices",
                                            "InDetTrackParticles",
                                            "AntiKt4EMTopoJets",
                                            "AntiKt4EMPFlowJets",
                                            "MET_Baseline_AntiKt4EMTopo",
                                            "MET_Baseline_AntiKt4EMPFlow",
                                            "TauJets",
                                            ]
    
    excludedVertexAuxData = "-vxTrackAtVertex.-MvfFitInfo.-isInitialized.-VTAV"
    StaticContent = []
    StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Tight_Vertices"]
    StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Tight_VerticesAux." + excludedVertexAuxData]
    StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Medium_Vertices"]
    StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Medium_VerticesAux." + excludedVertexAuxData]
    StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Loose_Vertices"]
    StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Loose_VerticesAux." + excludedVertexAuxData]   

    STDM7SlimmingHelper.StaticContent = StaticContent
   
    # IFF extra content
    from LeptonTaggers.LeptonTaggersConfig import GetExtraPLITVariablesForDxAOD
    STDM7SlimmingHelper.ExtraVariables += GetExtraPLITVariablesForDxAOD()
    
    # Truth extra content
    if flags.Input.isMC:

        from DerivationFrameworkMCTruth.MCTruthCommonConfig import addTruth3ContentToSlimmerTool
        addTruth3ContentToSlimmerTool(STDM7SlimmingHelper)
        STDM7SlimmingHelper.AllVariables += ['TruthLHEParticles', 'TruthHFWithDecayParticles','TruthHFWithDecayVertices','TruthCharm','TruthPileupParticles','InTimeAntiKt4TruthJets','OutOfTimeAntiKt4TruthJets']
        STDM7SlimmingHelper.ExtraVariables += ["Electrons.TruthLink",
                                               "Muons.TruthLink",
                                               "Photons.TruthLink",
                                               "TruthPrimaryVertices.t.x.y.z",
                                               "EventInfo.hardScatterVertexLink.timeStampNSOffset",
                                               "InDetTrackParticles.TTVA_AMVFVertices.TTVA_AMVFWeights.eProbabilityHT.numberOfTRTHits.numberOfTRTOutliers",
                                               "TauJets.dRmax.etOverPtLeadTrk",
                                               ]
 
        from DerivationFrameworkMCTruth.MCTruthCommonConfig import AddTauAndDownstreamParticlesCfg
        acc.merge(AddTauAndDownstreamParticlesCfg(flags))
        STDM7SlimmingHelper.AllVariables += ['TruthTausWithDecayParticles','TruthTausWithDecayVertices']

    # STDM7 needs AFP information - hits only available in data
    else:
        STDM7SlimmingHelper.AllVariables = [ "AFPSiHitContainer",
                                             "AFPToFHitContainer",
                                             "AFPSiHitsClusterContainer",
                                             "AFPTrackContainer",
                                             "AFPToFTrackContainer",    
                                             "AFPProtonContainer",
                                             "AFPVertexContainer",
                                            ]
        
    # Trigger content
    STDM7SlimmingHelper.IncludeTriggerNavigation = False
    STDM7SlimmingHelper.IncludeJetTriggerContent = False
    STDM7SlimmingHelper.IncludeMuonTriggerContent = False
    STDM7SlimmingHelper.IncludeEGammaTriggerContent = False
    STDM7SlimmingHelper.IncludeTauTriggerContent = False
    STDM7SlimmingHelper.IncludeEtMissTriggerContent = False
    STDM7SlimmingHelper.IncludeBJetTriggerContent = False
    STDM7SlimmingHelper.IncludeBPhysTriggerContent = False
    STDM7SlimmingHelper.IncludeMinBiasTriggerContent = False
    # Compact b-jet trigger matching info
    STDM7SlimmingHelper.IncludeBJetTriggerByYearContent = False

    # Trigger matching
    # Run 2
    if flags.Trigger.EDMVersion == 2:
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = STDM7SlimmingHelper, 
                                               OutputContainerPrefix = "TrigMatch_", 
                                               TriggerList = TriggerListsHelper.Run2TriggerNamesTau)
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = STDM7SlimmingHelper, 
                                               OutputContainerPrefix = "TrigMatch_",
                                               TriggerList = TriggerListsHelper.Run2TriggerNamesNoTau)
    # Run 3, or Run 2 with navigation conversion
    if flags.Trigger.EDMVersion == 3 or (flags.Trigger.EDMVersion == 2 and flags.Trigger.doEDMVersionConversion):
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(STDM7SlimmingHelper)

    # Output stream    
    STDM7ItemList = STDM7SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_"+name_tag, ItemList=STDM7ItemList, AcceptAlgs=[name_tag+"Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_"+name_tag, AcceptAlgs=[name_tag+"Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData, MetadataCategory.TruthMetaData]))

    return acc

def STDM7Cfg(flags):

    logSTDM7.info('****************** STARTING STDM7 *****************')

    stream_name = 'StreamDAOD_STDM7'
    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    STDM7TriggerListsHelper = TriggerListsHelper(flags)

    # Common augmentations
    acc.merge(STDM7KernelCfg(
        flags,
        name="STDM7Kernel",
        StreamName = stream_name, 
        TriggerListsHelper = STDM7TriggerListsHelper, 
    ))
    # STDM7 content
    acc.merge(STDM7CoreCfg(
        flags,
        "STDM7",
        StreamName = stream_name, 
        TriggerListsHelper = STDM7TriggerListsHelper, 
    ))
    
    return acc
