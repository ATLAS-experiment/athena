# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#==============================================================================
# Provides configs for the tools used for LLP Derivations
#==============================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod

# Thinning for VSI tracks
def VSITrackParticleThinningCfg(flags, name, **kwargs):
    """Configure the track particle merger tool"""
    acc = ComponentAccumulator()
    VSITrackParticleThinning = CompFactory.DerivationFramework.VSITrackParticleThinning
    acc.addPublicTool(VSITrackParticleThinning(name, **kwargs),
                      primary = True)
    return acc

# Thinning for Std tracks in jets
def JetTrackParticleThinningCfg(flags, name, **kwargs):
    """Configure the track particle merger tool"""
    acc = ComponentAccumulator()
    JetTrackParticleThinning = CompFactory.DerivationFramework.JetTrackParticleThinning
    acc.addPublicTool(JetTrackParticleThinning(name, **kwargs),
                      primary = True)
    return acc

# Thinning for LRT tracks in jets
def JetLargeD0TrackParticleThinningCfg(flags, name, **kwargs):
    """Configure the track particle merger tool"""
    acc = ComponentAccumulator()
    JetLargeD0TrackParticleThinning = CompFactory.DerivationFramework.JetLargeD0TrackParticleThinning
    acc.addPublicTool(JetLargeD0TrackParticleThinning(name, **kwargs),
                      primary = True)
    return acc

# RC jet substructure computation tool
def RCJetSubstructureAugCfg(flags, name, **kwargs):
    """Configure the RC jet substructure computation tool"""
    acc = ComponentAccumulator()
    RCJetSubstructureAug = CompFactory.DerivationFramework.RCJetSubstructureAug
    acc.addPublicTool(RCJetSubstructureAug(name, **kwargs),
                      primary = True)
    return acc

# tool to label leading/subleading jets
def AugmentationToolLeadingJetsCfg(flags):
    """Configure the RC jet substructure computation tool"""
    acc = ComponentAccumulator()
    acc.addPublicTool(CompFactory.DerivationFramework.AugmentationToolLeadingJets(name       = "LLP1AugmentationToolLeadingJets"),
                      primary = True)
    return acc

# Vertex constraint tool
def TrackParametersKVUCfg(flags, name, **kwargs):
    """Confiure the vertex constraint tool"""
    acc = ComponentAccumulator()

    if 'IPEstimator' not in kwargs:
        from TrkConfig.TrkVertexFitterUtilsConfig import AtlasTrackToVertexIPEstimatorCfg
        kwargs['IPEstimator'] = acc.popToolsAndMerge(AtlasTrackToVertexIPEstimatorCfg(flags))

    if 'VertexTrackUpdator' not in kwargs:
        from TrkConfig.TrkVertexFitterUtilsConfig import KalmanVertexTrackUpdatorCfg
        kwargs['VertexTrackUpdator']= acc.popToolsAndMerge(KalmanVertexTrackUpdatorCfg(flags,
                                                                                       SkipInvertibleCheck = True))
    TrackParametersKVU = CompFactory.DerivationFramework.TrackParametersKVU
    acc.addPublicTool(TrackParametersKVU(name, **kwargs),
                      primary = True)
    return acc

# Calo cell cluster decorator
def TrackParticleCaloCellDecoratorCfg(flags, name, **kwargs):
    """Confiure the isolation decorator tool"""
    acc = ComponentAccumulator()
    TrackParticleCaloCellDecorator = CompFactory.DerivationFramework.TrackParticleCaloCellDecorator
    acc.addPublicTool(TrackParticleCaloCellDecorator(name, **kwargs),
                      primary = True)
    return acc

# high dE/dx and low pT track thinning
def PixeldEdxTrackParticleThinningCfg(flags, name, **kwargs):
    """Confiure the lowpT + high dE/dx track thinniing tool"""    
    acc = ComponentAccumulator()
    PixeldEdxTrackParticleThinning = CompFactory.DerivationFramework.PixeldEdxTrackParticleThinning
    acc.addPublicTool(PixeldEdxTrackParticleThinning(name, **kwargs),
                      primary = True)
    return acc

def LLP1TriggerSkimmingToolCfg(flags, name, TriggerListsHelper, **kwargs):

    from TriggerMenuMT.TriggerAPI.TriggerAPI import TriggerAPI
    from TriggerMenuMT.TriggerAPI.TriggerEnums import TriggerPeriod, TriggerType
    import re

    # This is not all periods! Run 2 and current triggers only
    allperiods = TriggerPeriod.y2015 | TriggerPeriod.y2016 | TriggerPeriod.y2017 | TriggerPeriod.y2018 | TriggerPeriod.future2e34
    TriggerAPI.setConfigFlags(flags)
    trig_el  = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.el,  livefraction=0.8)
    trig_mu  = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.mu,  livefraction=0.8)
    trig_g   = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.g,   livefraction=0.8)
    trig_xe   = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.xe,  livefraction=0.8)
    trig_elmu = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.el, additionalTriggerType=TriggerType.mu,  livefraction=0.8)
    trig_mug = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.mu, additionalTriggerType=TriggerType.g,  livefraction=0.8)

    # Run 3 triggers.  Doesn't take into account prescales, so this is overly inclusive.  Needs updates to trigger api: ATR-25805
    all_run3 = TriggerListsHelper.Run3TriggerNames
    r_run3_nojets = re.compile("(HLT_[1-9]*(j).*)")
    r_tau = re.compile("HLT_.*tau.*")
    trig_run3_elmug = []
    for chain_name in all_run3:
        result_jets = r_run3_nojets.match(chain_name)
        result_taus = r_tau.match(chain_name) # no taus in llp1 atm
        # no perf chains
        if 'perf' in chain_name.lower(): continue
        if result_jets is None and result_taus is None: trig_run3_elmug.append(chain_name)

    trig_EJ_Run3 = ["HLT_j200_0eta180_emergingPTF0p08dR1p2_a10sd_cssk_pf_jes_ftf_preselj200_L1J100", "HLT_j460_a10r_L1J100","HLT_j200_0eta180_emergingPTF0p08dR1p2_a10sd_cssk_pf_jes_ftf_preselj200_L1gLJ140p0ETA25","HLT_j200_0eta180_emergingPTF0p08dR1p2_a10sd_cssk_pf_jes_ftf_preselj200_L1SC111-CjJ40","HLT_j200_0eta180_emergingPTF0p08dR1p2_a10sd_cssk_pf_jes_ftf_preselj200_L1jJ160"]
    trig_VBF_2018 =["HLT_j55_gsc80_bmv2c1070_split_j45_gsc60_bmv2c1085_split_j45_320eta490", "HLT_j45_gsc55_bmv2c1070_split_2j45_320eta490_L1J25.0ETA23_2J15.31ETA49", "HLT_j80_0eta240_j60_j45_320eta490_AND_2j35_gsc45_bmv2c1070_split", "HLT_ht300_2j40_0eta490_invm700_L1HT150-J20s5.ETA31_MJJ-400-CF_AND_2j35_gsc45_bmv2c1070_split", "HLT_j70_j50_0eta490_invm1100j70_dphi20_deta40_L1MJJ-500-NFF"]
    trig_VBF_Run3 = ["HLT_j70_j50a_j0_DJMASS1000j50dphi200x400deta_L1MJJ500NFF","HLT_j70_j50a_j0_DJMASS1000j50dphi200x400deta_L1jMJJ-500-NFF"] 
    trig_dispjet_Run3 = ["HLT_j180_hitdvjet260_tight_L1J100", "HLT_j180_dispjet50_3d2p_dispjet50_1p_L1J100", "HLT_j180_2dispjet50_3d2p_L1J100","HLT_j180_2dispjet50_3d2p_L1jJ160", "HLT_j180_dispjet50_3d2p_dispjet50_1p_L1jJ160", "HLT_j180_dispjet90_x3d1p_L1jJ160", "HLT_j180_dispjet100_x3d1p_L1jJ160", "HLT_j180_2dispjet50_3d2p_L1J100", "HLT_j180_dispjet50_3d2p_dispjet50_1p_L1J100", "HLT_j180_dispjet90_x3d1p_L1J100", "HLT_j180_dispjet90_x3d1p_L1J100", "HLT_j180_dispjet100_x3d1p_L1J100", "HLT_j180_2dispjet50_2p_L1jJ160", "HLT_j180_2dispjet50_2p_L1J100"]
    trig_dedx_Run3 = ["HLT_xe80_tcpufit_dedxtrk25_medium_L1XE50", "HLT_xe80_tcpufit_dedxtrk50_medium_L1XE50", "HLT_xe80_tcpufit_dedxtrk25_medium_L1XE55", "HLT_xe80_tcpufit_dedxtrk50_medium_L1XE55"]
    trig_dt_Run3 = ["HLT_xe80_tcpufit_distrk20_tight_L1XE50", "HLT_xe80_tcpufit_distrk20_medium_L1XE50", "HLT_xe80_tcpufit_distrk20_tight_L1XE55", "HLT_xe80_tcpufit_distrk20_medium_L1XE55"]
    trig_isohighpt_Run3  = ["HLT_xe80_tcpufit_isotrk100_medium_iaggrmedium_L1XE50", "HLT_xe80_tcpufit_isotrk120_medium_iaggrmedium_L1XE50", "HLT_xe80_tcpufit_isotrk120_medium_iaggrloose_L1XE50", "HLT_xe80_tcpufit_isotrk140_medium_iaggrmedium_L1XE50"]
    trig_latemu = ["HLT_mu10_mgonly_L1LATEMU10_J50", "HLT_mu10_mgonly_L1LATEMU10_XE50", "HLT_mu10_mgonly_L1LATE-MU10_XE40", "HLT_mu10_lateMu_L1LATE-MU8F_jJ90", "HLT_mu10_lateMu_L1LATE-MU8F_jXE70"]

    trig_multijet_ht_data22_23 = ["HLT_2j250c_j120c_pf_ftf_presel2j180XXj80_L1J100", "HLT_3j200_pf_ftf_L1J100", "HLT_4j115_pf_ftf_presel4j85_L13J50", "HLT_4j110_pf_ftf_presel4j85_L13J50", "HLT_4j120_L13J50", "HLT_5j70c_pf_ftf_presel5c50_L14J15", "HLT_5j65c_pf_ftf_presel5c55_L14J15", "HLT_5j85_pf_ftf_presel5j50_L14J15", "HLT_5j80_pf_ftf_presel5j50_L14J15", "HLT_5j80_pf_ftf_presel5j55_L14J15", "HLT_6j35c_020jvt_pf_ftf_presel6c25_L14J15", "HLT_6j55c_pf_ftf_presel6j40_L14J15", "HLT_6j70_pf_ftf_presel6j40_L14J15", "HLT_7j45_pf_ftf_presel7j30_L14J15", "HLT_10j40_pf_ftf_presel7j30_L14J15", "HLT_j0_HT1000_L1HT190-J15s5pETA21", "HLT_j0_HT1000_L1J100", "HLT_j0_HT1000_pf_ftf_preselcHT450_L1HT190-J15s5pETA21", "HLT_j0_HT940_pf_ftf_preselcHT450_L1HT190-J15s5pETA21", "HLT_j0_HT1000_pf_ftf_preselj180_L1HT190-J15s5pETA21", "HLT_j0_HT940_pf_ftf_preselj180_L1HT190-J15s5pETA21", "HLT_j0_HT1000_pf_ftf_preselj180_L1J100"] 

    triggers = trig_el + trig_mu + trig_g + trig_elmu + trig_mug + trig_VBF_2018 + trig_EJ_Run3 + trig_run3_elmug + trig_dispjet_Run3 + trig_xe + trig_multijet_ht_data22_23 + trig_dedx_Run3 + trig_dt_Run3 + trig_isohighpt_Run3 + trig_latemu + trig_VBF_Run3 

    #remove duplicates
    triggers = sorted(list(set(triggers)))

    acc = ComponentAccumulator()
    TriggerSkimmingTool = CompFactory.DerivationFramework.TriggerSkimmingTool
    acc.addPublicTool(TriggerSkimmingTool(name, 
                                          TriggerListAND = [],
                                          TriggerListOR  = triggers,
                                          **kwargs),
                                          primary = True)
    return acc


def LLP1TriggerMatchingToolRun2Cfg(flags, name, **kwargs):
    """Configure the common trigger matching for run 2 DAODs"""

    triggerList = kwargs['TriggerList']
    outputContainerPrefix = kwargs['OutputContainerPrefix']
    
    kwargs.setdefault('InputElectrons', 'LRTElectrons')
    kwargs.setdefault('InputMuons', 'MuonsLRT')
    kwargs.setdefault('DRThreshold', None) 

    acc = ComponentAccumulator()

    # Create trigger matching decorations
    from DerivationFrameworkTrigger.TriggerMatchingToolConfig import TriggerMatchingToolCfg
    if kwargs['DRThreshold'] is None:
        PhysCommonTriggerMatchingTool = acc.getPrimaryAndMerge(TriggerMatchingToolCfg(
            flags,
            name=name,
            ChainNames = triggerList,
            OutputContainerPrefix = outputContainerPrefix,
            InputElectrons = kwargs['InputElectrons'],
            InputMuons =  kwargs['InputMuons'])) 
    else:
        PhysCommonTriggerMatchingTool = acc.getPrimaryAndMerge(TriggerMatchingToolCfg(
            flags,
            name=name,
            ChainNames = triggerList,
            OutputContainerPrefix = outputContainerPrefix,  
            DRThreshold = kwargs['DRThreshold'],
            InputElectrons = kwargs['InputElectrons'],
            InputMuons =  kwargs['InputMuons'])) 
    CommonAugmentation = CompFactory.DerivationFramework.CommonAugmentation
    acc.addEventAlgo(CommonAugmentation(f"{outputContainerPrefix}TriggerMatchingKernel",
                                        AugmentationTools=[PhysCommonTriggerMatchingTool]))
    return(acc)

def LRTMuonMergerAlg(flags, name="LLP1_MuonLRTMergingAlg", **kwargs):
    acc = ComponentAccumulator()
    alg = CompFactory.CP.MuonLRTMergingAlg(name, **kwargs)
    acc.addEventAlgo(alg, primary=True)
    return acc

def ZeroPixelHitMuonMergerAlgCfg(flags, name='LLP1_MuonZPHMergingAlg', **kwargs):
    """Configure for merging ZeroPixelHitMuons with the LRTMerged muons for vertexing"""
    
    kwargs.setdefault('InputMuonContainers', ['Muons', 'MuonsLRT', 'ZeroPixelHitMuons'])
    kwargs.setdefault('OutputMuonLocation', 'StdWithLRTMuons_wZPH')
    kwargs.setdefault("CreateViewCollection", True)

    acc = ComponentAccumulator()
    alg = CompFactory.CP.MuonContainerMergingAlg(name, **kwargs)
    acc.addEventAlgo(alg, primary=True)
    return acc


def LRTElectronMergerAlg(flags, name="LLP1_ElectronLRTMergingAlg", **kwargs):
    acc = ComponentAccumulator()
    alg = CompFactory.CP.ElectronLRTMergingAlg(name, **kwargs)
    acc.addEventAlgo(alg, primary=True)
    return acc

# Photon IsEM setup for LLP1
def PhotonIsEMSelectorsCfg(flags):
    acc = ComponentAccumulator()
    from ROOT import egammaPID
    from ElectronPhotonSelectorTools.AsgPhotonIsEMSelectorsConfig import (
        AsgPhotonIsEMSelectorCfg,
    )

    isFullSim = flags.Sim.ISF.Simulator.isFullSim() if flags.Input.isMC else False

    if isFullSim:
        from EGammaVariableCorrection.EGammaVariableCorrectionConfig import (
            PhotonVariableCorrectionToolCfg,
        )

        PhotonVariableCorrectionTool = acc.popToolsAndMerge(
            PhotonVariableCorrectionToolCfg(flags))
        acc.addPublicTool(PhotonVariableCorrectionTool)

    PhotonIsEMSelectorMedium = acc.popToolsAndMerge(
        AsgPhotonIsEMSelectorCfg(
            flags, name="PhotonIsEMSelectorMedium", quality=egammaPID.PhotonIDMedium
        )
    )
    acc.addPublicTool(PhotonIsEMSelectorMedium)

    return acc
    
# Electron LLH setup for LLP1
def LRTElectronLHSelectorsCfg(flags):

    acc = ComponentAccumulator()

    from ElectronPhotonSelectorTools.AsgElectronLikelihoodToolsConfig import AsgElectronLikelihoodToolCfg
    from ElectronPhotonSelectorTools.ElectronLikelihoodToolMapping import electronLHmenu
    from ROOT import LikeEnum

    lhMenu = electronLHmenu.offlineMC21
    if flags.GeoModel.Run is LHCPeriod.Run2:
        lhMenu = electronLHmenu.offlineMC20

    ElectronLHSelectorVeryLooseNoPix = acc.popToolsAndMerge(AsgElectronLikelihoodToolCfg(
        flags,
        name="ElectronLHSelectorVeryLooseNoPix",
        quality=LikeEnum.VeryLooseLLP,
        menu=lhMenu)
    )
    ElectronLHSelectorVeryLooseNoPix.primaryVertexContainer = "PrimaryVertices"
    
    ElectronLHSelectorLooseNoPix = acc.popToolsAndMerge(AsgElectronLikelihoodToolCfg(
        flags,
        name="ElectronLHSelectorLooseNoPix",
        quality=LikeEnum.LooseLLP,
        menu=lhMenu)
    )
    ElectronLHSelectorLooseNoPix.primaryVertexContainer = "PrimaryVertices"

    ElectronLHSelectorMediumNoPix = acc.popToolsAndMerge(AsgElectronLikelihoodToolCfg(
        flags,
        name="ElectronLHSelectorMediumNoPix",
        quality=LikeEnum.MediumLLP,
        menu=lhMenu)
    )
    ElectronLHSelectorMediumNoPix.primaryVertexContainer = "PrimaryVertices"

    ElectronLHSelectorTightNoPix = acc.popToolsAndMerge(AsgElectronLikelihoodToolCfg(
        flags,
        name="ElectronLHSelectorTightNoPix",
        quality=LikeEnum.TightLLP,
        menu=lhMenu)
    )
    ElectronLHSelectorTightNoPix.primaryVertexContainer = "PrimaryVertices"

    from DerivationFrameworkEGamma.EGammaToolsConfig import EGElectronLikelihoodToolWrapperCfg

    # decorate electrons with the output of LH very loose
    ElectronPassLHVeryLooseNoPix = acc.getPrimaryAndMerge(EGElectronLikelihoodToolWrapperCfg(
        flags,
        name="ElectronPassLHVeryLooseNoPix",
        EGammaElectronLikelihoodTool=ElectronLHSelectorVeryLooseNoPix,
        EGammaFudgeMCTool="",
        CutType="",
        StoreGateEntryName="DFCommonElectronsLHVeryLooseNoPix",
        ContainerName="Electrons",
        StoreTResult=True))

    ElectronPassLHVeryLooseNoPixLRT = acc.getPrimaryAndMerge(EGElectronLikelihoodToolWrapperCfg(
        flags,
        name="ElectronPassLHVeryLooseNoPixLRT",
        EGammaElectronLikelihoodTool=ElectronLHSelectorVeryLooseNoPix,
        EGammaFudgeMCTool="",
        CutType="",
        StoreGateEntryName="DFCommonElectronsLHVeryLooseNoPix",
        ContainerName="LRTElectrons",
        StoreTResult=True))

    # decorate electrons with the output of LH loose
    ElectronPassLHLooseNoPix = acc.getPrimaryAndMerge(EGElectronLikelihoodToolWrapperCfg(
        flags,   
        name="ElectronPassLHLooseNoPix",
        EGammaElectronLikelihoodTool=ElectronLHSelectorLooseNoPix,
        EGammaFudgeMCTool="",
        CutType="",
        StoreGateEntryName="DFCommonElectronsLHLooseNoPix",
        ContainerName="Electrons",
        StoreTResult=False))

    ElectronPassLHLooseNoPixLRT = acc.getPrimaryAndMerge(EGElectronLikelihoodToolWrapperCfg(
        flags,   
        name="ElectronPassLHLooseNoPixLRT",
        EGammaElectronLikelihoodTool=ElectronLHSelectorLooseNoPix,
        EGammaFudgeMCTool="",
        CutType="",
        StoreGateEntryName="DFCommonElectronsLHLooseNoPix",
        ContainerName="LRTElectrons",
        StoreTResult=False))

    # decorate electrons with the output of LH medium
    ElectronPassLHMediumNoPix = acc.getPrimaryAndMerge(EGElectronLikelihoodToolWrapperCfg(
        flags,
        name="ElectronPassLHMediumNoPix",
        EGammaElectronLikelihoodTool=ElectronLHSelectorMediumNoPix,
        EGammaFudgeMCTool="",
        CutType="",
        StoreGateEntryName="DFCommonElectronsLHMediumNoPix",
        ContainerName="Electrons",
        StoreTResult=False))

    ElectronPassLHMediumNoPixLRT = acc.getPrimaryAndMerge(EGElectronLikelihoodToolWrapperCfg(
        flags,
        name="ElectronPassLHMediumNoPixLRT",
        EGammaElectronLikelihoodTool=ElectronLHSelectorMediumNoPix,
        EGammaFudgeMCTool="",
        CutType="",
        StoreGateEntryName="DFCommonElectronsLHMediumNoPix",
        ContainerName="LRTElectrons",
        StoreTResult=False))

    # decorate electrons with the output of LH tight
    ElectronPassLHTightNoPix = acc.getPrimaryAndMerge(EGElectronLikelihoodToolWrapperCfg(
        flags,
        name="ElectronPassLHTightNoPix",
        EGammaElectronLikelihoodTool=ElectronLHSelectorTightNoPix,
        EGammaFudgeMCTool="",
        CutType="",
        StoreGateEntryName="DFCommonElectronsLHTightNoPix",
        ContainerName="Electrons",
        StoreTResult=False))

    ElectronPassLHTightNoPixLRT = acc.getPrimaryAndMerge(EGElectronLikelihoodToolWrapperCfg(
        flags,
        name="ElectronPassLHTightNoPixLRT",
        EGammaElectronLikelihoodTool=ElectronLHSelectorTightNoPix,
        EGammaFudgeMCTool="",
        CutType="",
        StoreGateEntryName="DFCommonElectronsLHTightNoPix",
        ContainerName="LRTElectrons",
        StoreTResult=False))

    LRTEGAugmentationTools = [ElectronPassLHVeryLooseNoPix,
                              ElectronPassLHVeryLooseNoPixLRT,
                              ElectronPassLHLooseNoPix,
                              ElectronPassLHLooseNoPixLRT,
                              ElectronPassLHMediumNoPix,
                              ElectronPassLHMediumNoPixLRT,
                              ElectronPassLHTightNoPix,
                              ElectronPassLHTightNoPixLRT]

    acc.addEventAlgo(CompFactory.DerivationFramework.CommonAugmentation(
        "LLP1EGammaLRTKernel",
        AugmentationTools=LRTEGAugmentationTools
    ))

    return acc

# RecoverZeroPixelHitMuons setup
def RecoverZeroPixelHitMuonsCfg(flags):
    acc = ComponentAccumulator()
    acc.addEventAlgo(CompFactory.RecoverZeroPixelHitMuons(name="RecoverZeroPixelHitMuons"))
    
    return acc 
