# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

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

def TauLRTThinningCfg(flags, name, **kwargs):
    """configure tau thinning"""

    acc = ComponentAccumulator()
    TauLRTThinningTool = CompFactory.DerivationFramework.TauLRTThinningTool
    acc.addPublicTool(TauLRTThinningTool(name, **kwargs), primary=True)
    return acc

# RC jet substructure computation tool
def RCJetSubstructureAugCfg(flags, name, **kwargs):
    """Configure the RC jet substructure computation tool"""
    acc = ComponentAccumulator()
    suffix = kwargs.pop("Suffix", "")
    if not suffix:
        raise AttributeError("Suffix not set!")
    kwargs.setdefault("dec_Qw", "Qw_" + suffix)
    kwargs.setdefault("dec_Tau1", "Tau1_" + suffix)
    kwargs.setdefault("dec_Tau2", "Tau2_" + suffix)
    kwargs.setdefault("dec_Tau3", "Tau3_" + suffix)
    kwargs.setdefault("dec_Tau4", "Tau4_" + suffix)
    kwargs.setdefault("dec_Tau21", "Tau21_" + suffix)
    kwargs.setdefault("dec_Tau32", "Tau32_" + suffix)
    kwargs.setdefault("dec_Split12", "Split12_" + suffix)
    kwargs.setdefault("dec_Split23", "Split23_" + suffix)
    kwargs.setdefault("dec_Split34", "Split34_" + suffix)
    kwargs.setdefault("dec_ECF1", "ECF1_" + suffix)
    kwargs.setdefault("dec_ECF2", "ECF2_" + suffix)
    kwargs.setdefault("dec_ECF3", "ECF3_" + suffix)
    kwargs.setdefault("dec_ECF4", "ECF4_" + suffix)
    kwargs.setdefault("dec_C2", "C2_" + suffix)
    kwargs.setdefault("dec_D2", "D2_" + suffix)
    kwargs.setdefault("dec_pT", "pT_" + suffix)
    kwargs.setdefault("dec_m", "m_" + suffix)
    kwargs.setdefault("dec_NConstits", "NConstits_" + suffix)
    kwargs.setdefault("dec_eta", "eta_" + suffix)
    kwargs.setdefault("dec_phi", "phi_" + suffix)
    kwargs.setdefault("dec_timing", "timing_" + suffix)
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
    """Configure the vertex constraint tool"""
    acc = ComponentAccumulator()

    if "IPEstimator" not in kwargs:
        from TrkConfig.TrkVertexFitterUtilsConfig import AtlasTrackToVertexIPEstimatorCfg
        kwargs.setdefault("IPEstimator", acc.popToolsAndMerge(
            AtlasTrackToVertexIPEstimatorCfg(flags)))

    if "VertexTrackUpdator" not in kwargs:
        from TrkConfig.TrkVertexFitterUtilsConfig import KalmanVertexTrackUpdatorCfg
        kwargs.setdefault("VertexTrackUpdator", acc.popToolsAndMerge(
            KalmanVertexTrackUpdatorCfg(flags, SkipInvertibleCheck = True)))

    if "LinearizedTrackFactory" not in kwargs:
        from TrkConfig.TrkVertexFitterUtilsConfig import AtlasFullLinearizedTrackFactoryCfg
        kwargs.setdefault("LinearizedTrackFactory", acc.popToolsAndMerge(
            AtlasFullLinearizedTrackFactoryCfg(flags)))

    if "TrackExtrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
        kwargs.setdefault("TrackExtrapolator", acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags)))

    acc.setPrivateTools(CompFactory.DerivationFramework.TrackParametersKVU(name, **kwargs))
    return acc


# Calo cell cluster decorator
def TrackParticleCaloCellDecoratorCfg(flags, name, **kwargs):
    """Confiure the isolation decorator tool"""
    acc = ComponentAccumulator()
    prefix = kwargs.pop("DecorationPrefix", "LLP1")
    kwargs.setdefault("ClusterAssocContainerName", kwargs["ContainerName"] + "ClusterAssociations")
    kwargs.setdefault("decCellEtaKey", prefix + "_CaloCellEta")
    kwargs.setdefault("decCellPhiKey", prefix + "_CaloCellPhi")
    kwargs.setdefault("decCellRKey", prefix + "_CaloCellR")
    kwargs.setdefault("decCelldEtaKey", prefix + "_CaloCelldEta")
    kwargs.setdefault("decCelldPhiKey", prefix + "_CaloCelldPhi")
    kwargs.setdefault("decCelldRKey", prefix + "_CaloCelldR")
    kwargs.setdefault("decCellXKey", prefix + "_CaloCellX")
    kwargs.setdefault("decCellYKey", prefix + "_CaloCellY")
    kwargs.setdefault("decCellZKey", prefix + "_CaloCellZ")
    kwargs.setdefault("decCelldXKey", prefix + "_CaloCelldX")
    kwargs.setdefault("decCelldYKey", prefix + "_CaloCelldY")
    kwargs.setdefault("decCelldZKey", prefix + "_CaloCelldZ")
    kwargs.setdefault("decCellTKey", prefix + "_CaloCellTime")
    kwargs.setdefault("decCellEKey", prefix + "_CaloCellE")
    kwargs.setdefault("decCellIDKey", prefix + "_CaloCellID")
    kwargs.setdefault("decCellSamplingKey", prefix + "_CaloCellSampling")
    kwargs.setdefault("decCellQualityKey", prefix + "_CaloCellQuality")
    kwargs.setdefault("decCellProvenanceKey", prefix + "_CaloCellProvenance")
    kwargs.setdefault("decCellGainKey", prefix + "_CaloCellGain")
    kwargs.setdefault("decCellEneDiffKey", prefix + "_CaloCellEneDiff")
    kwargs.setdefault("decCellTimeDiffKey", prefix + "_CaloCellTimeDiff")
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

    # tapi chain lists
    trig_tapis = {k: [] for k in ('el','mu','g','xe','elmu','mug','tau','tauxe','exotics','gxe')}
    if flags.Trigger.EDMVersion <= 2:
        # Run 2: use the Run 2 TriggerAPI
        allperiods = TriggerPeriod.y2015 | TriggerPeriod.y2016 | TriggerPeriod.y2017 | TriggerPeriod.y2018 | TriggerPeriod.future2e34
        TriggerAPI.setConfigFlags(flags)
        trig_tapis['el']   = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.el, livefraction=0.8)
        trig_tapis['mu']   = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.mu, livefraction=0.8)
        trig_tapis['g']    = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.g,  livefraction=0.8)
        trig_tapis['xe']   = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.xe, livefraction=0.8)
        trig_tapis['elmu'] = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.el, additionalTriggerType=TriggerType.mu, livefraction=0.8)
        trig_tapis['mug']  = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.mu, additionalTriggerType=TriggerType.g,  livefraction=0.8)
        trig_tapis['tau']  = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.tau, livefraction=0.8)
        trig_tapis['tauxe']= TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.tau, additionalTriggerType=TriggerType.xe, livefraction=0.8)
    else:
        # Run 3: use the TriggerAPI session via tapis session
        from DerivationFrameworkPhys.TriggerListsHelper import getTapisSession
        session = getTapisSession(flags)
        trig_tapis['el']      = list(session.getLowestUnprescaled(triggerType=TriggerType.el, livefraction=0.75))
        trig_tapis['mu']      = list(session.getLowestUnprescaled(triggerType=TriggerType.mu, livefraction=0.75))
        trig_tapis['g']       = list(session.getLowestUnprescaled(triggerType=TriggerType.g,  livefraction=0.75))
        trig_tapis['xe']      = list(session.getLowestUnprescaled(triggerType=TriggerType.xe, livefraction=0.75))
        trig_tapis['elmu']    = list(session.getLowestUnprescaled(triggerType=[TriggerType.el, TriggerType.mu], livefraction=0.75))
        trig_tapis['mug']     = list(session.getLowestUnprescaled(triggerType=[TriggerType.mu, TriggerType.g],  livefraction=0.75))
        trig_tapis['tau']     = list(session.getLowestUnprescaled(triggerType=TriggerType.tau, livefraction=0.75))
        trig_tapis['tauxe']   = list(session.getLowestUnprescaled(triggerType=[TriggerType.tau, TriggerType.xe], livefraction=0.75))
        trig_tapis['exotics'] = list(session.getLowestUnprescaled(triggerType=TriggerType.exotics, livefraction=0.75))
        trig_tapis['gxe']     = list(session.getLowestUnprescaled(triggerType=[TriggerType.g, TriggerType.xe], livefraction=0.75))

    # All chains not picked up by tapis are hardcoded in LLP1Triggers.txt and loaded below; see comments there for details
    # (just take every non-comment, non-empty line)
    from PathResolver import PathResolver
    with open(PathResolver.FindCalibFile("DerivationFrameworkLLP/LLP1Triggers.txt")) as fp:
        trig_hardcoded = [
            line for ln in fp
            if (line := ln.strip()) and not line.startswith('#')
        ]

    triggers = sorted(set(sum(trig_tapis.values(), []) + trig_hardcoded))

    # debug printout of the final trigger list when needed
    from AthenaCommon.Logging import logging
    log = logging.getLogger("LLP1TriggerSkimmingToolCfg")
    log.debug("LLP1 skimming trigger list (%d chains):", len(triggers))
    for t in triggers:
        log.debug("  %s", t)

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
    prompt = kwargs.setdefault ('PromptElectronLocation', 'Electrons')
    lrt = kwargs.setdefault ('LRTElectronLocation', 'LRTElectrons')
    ExtraInputs = kwargs.setdefault ('ExtraInputs', [])
    from IsolationAlgs.DerivationTrackIsoConfig import iso_vars
    ExtraInputs += [('xAOD::IParticleContainer', f'{prompt}.{v}')
                    for v in iso_vars()]
    ExtraInputs += [('xAOD::IParticleContainer', f'{lrt}.{v}')
                    for v in iso_vars()]
    ExtraInputs += [
             ('xAOD::IParticleContainer', f'{lrt}.core57cellsEnergyCorrection'),
             ('xAOD::IParticleContainer', f'{lrt}.ptcone20'),
             ('xAOD::IParticleContainer', f'{lrt}.neflowisol20'),
             ('xAOD::IParticleContainer', f'{prompt}.neflowisol20'),
        ]
    alg = CompFactory.CP.ElectronLRTMergingAlg \
        (name,
         **kwargs)
    acc.addEventAlgo(alg,
                     primary=True)
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
    ElectronPassLHVeryLooseNoPix = acc.addPublicTool(acc.popToolsAndMerge(EGElectronLikelihoodToolWrapperCfg(
        flags,
        name="ElectronPassLHVeryLooseNoPix",
        EGammaElectronLikelihoodTool=ElectronLHSelectorVeryLooseNoPix,
        CutType="",
        StoreGateEntryName="DFCommonElectronsLHVeryLooseNoPix",
        ContainerName="Electrons",
        StoreTResult=True)))

    ElectronPassLHVeryLooseNoPixLRT = acc.addPublicTool(acc.popToolsAndMerge(EGElectronLikelihoodToolWrapperCfg(
        flags,
        name="ElectronPassLHVeryLooseNoPixLRT",
        EGammaElectronLikelihoodTool=ElectronLHSelectorVeryLooseNoPix,
        CutType="",
        StoreGateEntryName="DFCommonElectronsLHVeryLooseNoPix",
        ContainerName="LRTElectrons",
        StoreTResult=True)))

    # decorate electrons with the output of LH loose
    ElectronPassLHLooseNoPix = acc.addPublicTool(acc.popToolsAndMerge(EGElectronLikelihoodToolWrapperCfg(
        flags,   
        name="ElectronPassLHLooseNoPix",
        EGammaElectronLikelihoodTool=ElectronLHSelectorLooseNoPix,
        CutType="",
        StoreGateEntryName="DFCommonElectronsLHLooseNoPix",
        ContainerName="Electrons",
        StoreTResult=False)))

    ElectronPassLHLooseNoPixLRT = acc.addPublicTool(acc.popToolsAndMerge(EGElectronLikelihoodToolWrapperCfg(
        flags,   
        name="ElectronPassLHLooseNoPixLRT",
        EGammaElectronLikelihoodTool=ElectronLHSelectorLooseNoPix,
        CutType="",
        StoreGateEntryName="DFCommonElectronsLHLooseNoPix",
        ContainerName="LRTElectrons",
        StoreTResult=False)))

    # decorate electrons with the output of LH medium
    ElectronPassLHMediumNoPix = acc.addPublicTool(acc.popToolsAndMerge(EGElectronLikelihoodToolWrapperCfg(
        flags,
        name="ElectronPassLHMediumNoPix",
        EGammaElectronLikelihoodTool=ElectronLHSelectorMediumNoPix,
        CutType="",
        StoreGateEntryName="DFCommonElectronsLHMediumNoPix",
        ContainerName="Electrons",
        StoreTResult=False)))

    ElectronPassLHMediumNoPixLRT = acc.addPublicTool(acc.popToolsAndMerge(EGElectronLikelihoodToolWrapperCfg(
        flags,
        name="ElectronPassLHMediumNoPixLRT",
        EGammaElectronLikelihoodTool=ElectronLHSelectorMediumNoPix,
        CutType="",
        StoreGateEntryName="DFCommonElectronsLHMediumNoPix",
        ContainerName="LRTElectrons",
        StoreTResult=False)))

    # decorate electrons with the output of LH tight
    ElectronPassLHTightNoPix = acc.addPublicTool(acc.popToolsAndMerge(EGElectronLikelihoodToolWrapperCfg(
        flags,
        name="ElectronPassLHTightNoPix",
        EGammaElectronLikelihoodTool=ElectronLHSelectorTightNoPix,
        CutType="",
        StoreGateEntryName="DFCommonElectronsLHTightNoPix",
        ContainerName="Electrons",
        StoreTResult=False)))

    ElectronPassLHTightNoPixLRT = acc.addPublicTool(acc.popToolsAndMerge(EGElectronLikelihoodToolWrapperCfg(
        flags,
        name="ElectronPassLHTightNoPixLRT",
        EGammaElectronLikelihoodTool=ElectronLHSelectorTightNoPix,
        CutType="",
        StoreGateEntryName="DFCommonElectronsLHTightNoPix",
        ContainerName="LRTElectrons",
        StoreTResult=False)))

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
    from IsolationAlgs.DerivationTrackIsoConfig import iso_vars
    ExtraInputs = [('xAOD::IParticleContainer', 'Muons.' + v)
                   for v in iso_vars()]
    acc.addEventAlgo(CompFactory.RecoverZeroPixelHitMuons(name="RecoverZeroPixelHitMuons", ExtraInputs=ExtraInputs))
    
    return acc 
