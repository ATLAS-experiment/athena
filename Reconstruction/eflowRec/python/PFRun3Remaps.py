# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

def ListRemaps():
    #function to get all of the remapped names needed in PFRun3Config.py
    from SGComps.AddressRemappingConfig import InputRenameCfg
    list_remaps=[        
        #Remap input containers, that we rebuild from the ESD
        #Remap the calibrated and origin corrected topoclusters
        InputRenameCfg ('xAOD::CaloClusterContainer','CaloCalTopoClusters','CaloCalTopoClusters_renamed'),
        InputRenameCfg ('xAOD::CaloClusterAuxContainer','CaloCalTopoClustersAux.','CaloCalTopoClusters_renamedAux.'),
        InputRenameCfg ('CaloClusterCellLinkContainer', 'CaloCalTopoClusters_links', 'CaloCalTopoClusters_links_renamed'),
        #rename topotowers
        InputRenameCfg ('xAOD::CaloTowerContainer','CaloCalFwdTopoTowers','CaloCalFwdTopoTowers_renamed'),
        InputRenameCfg ('xAOD::CaloTowerAuxContainer','CaloCalFwdTopoTowersAux.','CaloCalFwdTopoTowers_renamedAux.'),
        InputRenameCfg ('xAOD::CaloClusterContainer','LCOriginTopoClusters','LCOriginTopoClusters_renamed'),
        InputRenameCfg ('xAOD::ShallowAuxContainer', 'LCOriginTopoClustersAux.', 'LCOriginTopoClusters_renamedAux.'),
        InputRenameCfg ('xAOD::CaloClusterContainer','EMOriginTopoClusters','EMOriginTopoClusters_renamed'),
        InputRenameCfg( 'xAOD::ShallowAuxContainer', 'EMOriginTopoClustersAux.', 'EMOriginTopoClusters_renamedAux.'),

        #Remap containers that pflow will rebuild
        InputRenameCfg('xAOD::FlowElementContainer','JetETMissChargedParticleFlowObjects','JetETMissChargedParticleFlowObjects_renamed'),
        InputRenameCfg('xAOD::FlowElementAuxContainer','JetETMissChargedParticleFlowObjectsAux.','JetETMissChargedParticleFlowObjects_renamedAux.'),
        InputRenameCfg('xAOD::FlowElementContainer','JetETMissNeutralParticleFlowObjects','JetETMissNeutralParticleFlowObjects_renamed'),
        InputRenameCfg('xAOD::FlowElementAuxContainer','JetETMissNeutralParticleFlowObjectsAux.','JetETMissNeutralParticleFlowObjects_renamedAux.'),

        #Remap the decorations on other containers that pflow will recreate
        #EGamma
        InputRenameCfg ('xAOD::ElectronContainer','Electrons.chargedFELinks','Electrons.chargedFELinks_renamed'),
        InputRenameCfg ('xAOD::ElectronContainer','Electrons.neutralFELinks','Electrons.neutralFELinks_renamed'),
        InputRenameCfg ('xAOD::PhotonContainer','Photons.chargedFELinks','Photons.chargedFELinks_renamed'),
        InputRenameCfg ('xAOD::PhotonContainer','Photons.neutralFELinks','Photons.neutralFELinks_renamed'),
        InputRenameCfg ('xAOD::ElectronContainer','Electrons.neutralpfoLinks','Electrons.neutralpfoLinks_renamed'),
        InputRenameCfg ('xAOD::ElectronContainer','Electrons.chargedpfoLinks','Electrons.chargedpfoLinks_renamed'),
        InputRenameCfg ('xAOD::PhotonContainer','Photons.neutralpfoLinks','Photons.neutralpfoLinks_renamed'),
        InputRenameCfg ('xAOD::PhotonContainer','Photons.chargedpfoLinks','Photons.chargedpfoLinks_renamed'),
        #Muons
        InputRenameCfg ('xAOD::MuonContainer','Muons.chargedFELinks','Muons.chargedFELinks_renamed'),
        InputRenameCfg ('xAOD::MuonContainer','Muons.neutralFELinks','Muons.neutralFELinks_renamed'),
        InputRenameCfg ('xAOD::MuonContainer','Muons.muon_efrac_matched_FE','Muons.muon_efrac_matched_FE_renamed'),
        InputRenameCfg ('xAOD::MuonContainer','Muons.ClusterInfo_deltaR','Muons.ClusterInfo_deltaR_renamed'),  
        InputRenameCfg ('xAOD::CaloClusterContainer','MuonClusterCollection.constituentClusterLinks','MuonClusterCollection.constituentClusterLinks_renamed'),      
        #Taus
        InputRenameCfg ('xAOD::TauJetContainer','TauJets.neutralFELinks','TauJets.neutralFELinks_renamed'),
        InputRenameCfg ('xAOD::TauJetContainer','TauJets.chargedFELinks','TauJets.chargedFELinks_renamed'),        
    ]
    
    return list_remaps
