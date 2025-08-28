# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# PhysCommonThinningConfig
# Contains the configuration for the thinning for PHYS(LITE)

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def PhysCommonThinningCfg(flags, StreamName = "StreamDAOD_PHYS", **kwargs):
    """Configure the common augmentation"""
    acc = ComponentAccumulator()

    # Thinning tools...
    from DerivationFrameworkInDet.InDetToolsConfig import TrackParticleThinningCfg, MuonTrackParticleThinningCfg, DiTauTrackParticleThinningCfg
    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import GenericObjectThinningCfg
    from DerivationFrameworkCalo.DerivationFrameworkCaloConfig import CaloClusterThinningCfg
    from DerivationFrameworkTau.TauCommonConfig import TauThinningCfg

    # Inner detector group recommendations for indet tracks in analysis
    # https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/DaodRecommendations
    if "TrackParticleThinningToolName" in kwargs:
        tp_thinning_expression = "InDetTrackParticles.DFCommonTightPrimary && abs(DFCommonInDetTrackZ0AtPV)*sin(InDetTrackParticles.theta) < 3.0*mm && InDetTrackParticles.pt > 10*GeV"
        acc.merge(TrackParticleThinningCfg(
            flags,
            name                    = kwargs['TrackParticleThinningToolName'],
            StreamName              = StreamName, 
            SelectionString         = tp_thinning_expression,
            InDetTrackParticlesKey  = "InDetTrackParticles"))
        
    # Include inner detector tracks associated with muons
    if "MuonTPThinningToolName" in kwargs:
        acc.merge(MuonTrackParticleThinningCfg(
            flags,
            name                    = kwargs['MuonTPThinningToolName'],
            StreamName              = StreamName,
            MuonKey                 = "Muons",
            InDetTrackParticlesKey  = "InDetTrackParticles"))

    # Tau-related containers: taus, tau tracks and associated ID tracks, neutral PFOs, secondary vertices
    if "TauJetThinningToolName" in kwargs:
        tau_thinning_expression = f"TauJets.pt >= {flags.Tau.MinPtDAOD} && TauJets.nTracks <= {flags.Tau.MaxTracksDAOD}"
        acc.merge(TauThinningCfg(flags,
            name                 = kwargs['TauJetThinningToolName'],
            StreamName           = StreamName,
            Taus                 = "TauJets",
            TauTracks            = "TauTracks",
            TrackParticles       = "InDetTrackParticles",
            TauNeutralPFOs       = "TauNeutralParticleFlowObjects",
            TauSecondaryVertices = "TauSecondaryVertices",
            SelectionString      = tau_thinning_expression))

    if "TauJets_MuonRMThinningToolName" in kwargs:
        tau_murm_thinning_expression = tau_thinning_expression.replace('TauJets', 'TauJets_MuonRM')
        acc.merge(TauThinningCfg(flags,
            name                 = kwargs['TauJets_MuonRMThinningToolName'],
            StreamName           = StreamName,
            Taus                 = "TauJets_MuonRM",
            TauTracks            = "TauTracks_MuonRM",
            TrackParticles       = "InDetTrackParticles",
            TauNeutralPFOs       = "TauNeutralParticleFlowObjects_MuonRM",
            TauSecondaryVertices = "TauSecondaryVertices_MuonRM",
            SelectionString      = tau_murm_thinning_expression))

    if "TauJets_EleRMThinningToolName" in kwargs:
        tau_erm_thinning_expression = tau_thinning_expression.replace('TauJets', 'TauJets_EleRM')
        acc.merge(TauThinningCfg(flags,
            name                 = kwargs['TauJets_EleRMThinningToolName'],
            StreamName           = StreamName,
            Taus                 = "TauJets_EleRM",
            TauTracks            = "TauTracks_EleRM",
            TrackParticles       = "InDetTrackParticles",
            TauNeutralPFOs       = "TauNeutralParticleFlowObjects_EleRM",
            TauSecondaryVertices = "TauSecondaryVertices_EleRM",
            SelectionString      = tau_erm_thinning_expression))

    # di-tau thinning
    if "DiTauThinningToolName" in kwargs:
        acc.merge(GenericObjectThinningCfg(
            flags,
            name            = kwargs['DiTauThinningToolName'],
            StreamName      = StreamName,
            ContainerName   = "DiTauJets",
            SelectionString = "DiTauJets.nSubjets > 1"))

    # ID tracks associated with high-pt di-tau
    if "DiTauTPThinningToolName" in kwargs:
        acc.merge(DiTauTrackParticleThinningCfg(
            flags,
            name                    = kwargs['DiTauTPThinningToolName'],
            StreamName              = StreamName,
            DiTauKey                = "DiTauJets",
            InDetTrackParticlesKey  = "InDetTrackParticles",
            SelectionString         = "DiTauJets.nSubjets > 1"))
 
    ## Low-pt di-tau thinning
    if "DiTauLowPtThinningToolName" in kwargs:
        acc.merge(GenericObjectThinningCfg(
            flags,
            name            = kwargs['DiTauLowPtThinningToolName'],
            StreamName      = StreamName,
            ContainerName   = "DiTauJetsLowPt",
            SelectionString = "DiTauJetsLowPt.nSubjets > 1"))
    
    # ID tracks associated with low-pt ditau
    if "DiTauLowPtTPThinningToolName" in kwargs:
        acc.merge(DiTauTrackParticleThinningCfg(
            flags,
            name                    = kwargs['DiTauLowPtTPThinningToolName'],
            StreamName              = StreamName,
            DiTauKey                = "DiTauJetsLowPt",
            InDetTrackParticlesKey  = "InDetTrackParticles",
            SelectionString         = "DiTauJetsLowPt.nSubjets > 1"))
 
    # keep calo clusters around electrons
    if "ElectronCaloClusterThinningToolName" in kwargs:
        acc.merge(CaloClusterThinningCfg(
            flags,
            name                  = kwargs['ElectronCaloClusterThinningToolName'],
            StreamName            = StreamName,
            SGKey                 = "AnalysisElectrons",
            CaloClCollectionSGKey = "egammaClusters",
            ConeSize              = -1.0))
 
    # keep calo clusters around photons
    if "PhotonCaloClusterThinningToolName" in kwargs:
        acc.merge(CaloClusterThinningCfg(
            flags,
            name                  = kwargs['PhotonCaloClusterThinningToolName'],
            StreamName            = StreamName,
            SGKey                 = "AnalysisPhotons",
            CaloClCollectionSGKey = "egammaClusters",
            ConeSize=-1.0))
 
    # GSF tracks associated to electrons
    if "ElectronGSFTPThinningToolName" in kwargs:
        ElectronGSFTPThinningTool = CompFactory.DerivationFramework.EgammaTrackParticleThinning(
            name                   = kwargs['ElectronGSFTPThinningToolName'],
            StreamName             = StreamName,
            SGKey                  = "AnalysisElectrons",
            GSFTrackParticlesKey   = "GSFTrackParticles",
            InDetTrackParticlesKey = "",
            BestMatchOnly          = True)
        acc.addPublicTool(ElectronGSFTPThinningTool)
 
    # GSF tracks associated to photons
    if "PhotonGSFTPThinningToolName" in kwargs:
        PhotonGSFTPThinningTool = CompFactory.DerivationFramework.EgammaTrackParticleThinning(
            name                     = kwargs['PhotonGSFTPThinningToolName'],
            StreamName               = StreamName,
            SGKey                    = "AnalysisPhotons",
            GSFTrackParticlesKey     = "GSFTrackParticles",
            GSFConversionVerticesKey = "GSFConversionVertices",
            InDetTrackParticlesKey   = "",
            BestMatchOnly            = True,
            BestVtxMatchOnly         = True)
        acc.addPublicTool(PhotonGSFTPThinningTool)

    return acc

